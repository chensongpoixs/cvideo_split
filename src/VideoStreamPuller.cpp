/**
 * @file VideoStreamPuller.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief RTSP 视频流拉取器实现（自动重连机制）
 * @see VideoStreamPuller.h
 */

#include "VideoStreamPuller.h"
#include "Logger.h"
#include <iostream>
#include <chrono>
#include <thread>

VideoStreamPuller::VideoStreamPuller(const std::string& rtsp_url, int stream_id)
    : rtsp_url_(rtsp_url), stream_id_(stream_id), running_(false) {
}

VideoStreamPuller::~VideoStreamPuller() {
    stop();
    cleanup();
}

bool VideoStreamPuller::initialize() {
    LOG_INFO("Initializing RTSP stream puller " + std::to_string(stream_id_) + " with URL: " + rtsp_url_);

    // 初始化FFmpeg
    avformat_network_init();

    // 重连/重复初始化时先清理旧连接与缓存
    cleanup();
    resetConnectionState();

    // 打开RTSP流
    AVDictionary* options = nullptr;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "max_delay", "500000", 0); // 500ms最大延迟
    av_dict_set(&options, "timeout", "10000000", 0); // 10秒连接超时
    // 读写超时（微秒）：避免 av_read_frame 长时间阻塞导致无法触发重连逻辑
    av_dict_set(&options, "rw_timeout", "5000000", 0); // 5秒

    LOG_DEBUG("Attempting to open RTSP stream: " + rtsp_url_);
    int ret = avformat_open_input(&format_ctx_, rtsp_url_.c_str(), nullptr, &options);
    if (ret < 0) {
        LOG_ERROR("Failed to open RTSP stream " + std::to_string(stream_id_) + ": " + rtsp_url_ + ", error code: " + std::to_string(ret));

        // 提供更详细的错误信息
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
        LOG_ERROR("FFmpeg error details: " + std::string(errbuf));

        av_dict_free(&options);
        return false;
    }
    av_dict_free(&options);
    LOG_INFO("Successfully opened RTSP stream " + std::to_string(stream_id_));

    // 获取流信息
    LOG_DEBUG("Finding stream info for RTSP stream " + std::to_string(stream_id_));
    ret = avformat_find_stream_info(format_ctx_, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to find stream info for RTSP stream " + std::to_string(stream_id_) + ", error code: " + std::to_string(ret));
        return false;
    }
    LOG_INFO("Found stream info for RTSP stream " + std::to_string(stream_id_) + ", " + std::to_string(format_ctx_->nb_streams) + " streams detected");

    // 查找视频流
    video_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_index_ < 0) {
        LOG_ERROR("No video stream found in RTSP stream " + std::to_string(stream_id_));
        return false;
    }
    LOG_INFO("Found video stream at index " + std::to_string(video_stream_index_) + " for RTSP stream " + std::to_string(stream_id_));

    // 获取编解码器参数
    AVCodecParameters* codec_par = format_ctx_->streams[video_stream_index_]->codecpar;
    LOG_INFO("Video codec ID: " + std::to_string(codec_par->codec_id) + ", resolution: " +
             std::to_string(codec_par->width) + "x" + std::to_string(codec_par->height) +
             ", bitrate: " + std::to_string(codec_par->bit_rate));

    // VideoStreamPuller现在只负责拉取包，不进行解码
    // 编解码器初始化将由HardwareDecoder处理

    LOG_INFO("VideoStreamPuller initialized successfully for stream " + std::to_string(stream_id_) +
             " (" + std::to_string(codec_par->width) + "x" + std::to_string(codec_par->height) + ")");

    return true;
}

void VideoStreamPuller::start() {
    if (running_) return;

    running_ = true;
    pull_thread_ = std::thread(&VideoStreamPuller::pullStream, this);
}

void VideoStreamPuller::stop() {
    running_ = false;
    if (pull_thread_.joinable()) {
        pull_thread_.join();
    }
}

bool VideoStreamPuller::getLatestPacket(AVPacket** packet) {
    std::unique_lock<std::mutex> lock(frame_mutex_);
    if (packet_queue_.empty()) {
        return false;
    }

    // 返回队列中最新的包
    *packet = packet_queue_.front();
    packet_queue_.pop();
    return true;
}

const AVCodecParameters* VideoStreamPuller::getVideoCodecParameters() const {
    if (!format_ctx_) {
        return nullptr;
    }

    int idx = video_stream_index_;
    if (idx < 0) {
        idx = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    }
    if (idx < 0) {
        return nullptr;
    }

    return format_ctx_->streams[idx]->codecpar;
}

bool VideoStreamPuller::getLatestFrame(VideoFrame& frame) {
    std::unique_lock<std::mutex> lock(frame_mutex_);
    if (!has_new_frame_) {
        return false;
    }

    // 复制帧数据
    frame.width = latest_frame_.width;
    frame.height = latest_frame_.height;
    frame.pts = latest_frame_.pts;
    frame.format = latest_frame_.format;

    for (int i = 0; i < 4; i++) {
        frame.linesize[i] = latest_frame_.linesize[i];
        if (latest_frame_.data[i] && latest_frame_.linesize[i] > 0) {
            size_t data_size = latest_frame_.linesize[i] * latest_frame_.height;
            if (i > 0) data_size /= 2; // UV平面是Y平面的一半

            frame.data[i] = new uint8_t[data_size];
            memcpy(frame.data[i], latest_frame_.data[i], data_size);
        }
    }

    has_new_frame_ = false;
    return true;
}

void VideoStreamPuller::pullStream() {
    LOG_INFO("Starting RTSP stream puller thread for stream " + std::to_string(stream_id_));

    while (running_) {
        // 未连接/断开后：等待3秒再尝试重连
        if (!format_ctx_) {
            uint64_t attempt = ++reconnect_attempts_;
            LOG_WARNING("RTSP stream " + std::to_string(stream_id_) +
                        " not connected, will retry in 3s (attempt " + std::to_string(attempt) + ")");
            for (int i = 0; i < 30 && running_; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (!running_) break;
            if (!initialize()) {
                LOG_WARNING("RTSP stream " + std::to_string(stream_id_) + " reconnect attempt failed");
                continue;
            }
            LOG_INFO("RTSP stream " + std::to_string(stream_id_) + " reconnected successfully");
        }

        AVPacket* packet = av_packet_alloc();
        if (!packet) {
            LOG_ERROR("Failed to allocate packet for RTSP stream " + std::to_string(stream_id_));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int ret = av_read_frame(format_ctx_, packet);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
            if (ret == AVERROR_EOF) {
                LOG_WARNING("RTSP stream " + std::to_string(stream_id_) + " ended (EOF), will reconnect in 3s");
            } else {
                LOG_WARNING("RTSP stream " + std::to_string(stream_id_) +
                            " read failed (will reconnect), code=" + std::to_string(ret) +
                            ", detail=" + std::string(errbuf));
            }

            av_packet_free(&packet);
            cleanup();
            resetConnectionState();
            continue;
        }

        // 只处理视频包
        if (video_stream_index_ < 0) {
            video_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        }
        if (video_stream_index_ < 0 || packet->stream_index != video_stream_index_) {
            av_packet_free(&packet);
            continue;
        }

        // 将包添加到队列
        {
            std::unique_lock<std::mutex> lock(frame_mutex_);

            // 限制队列大小
            if (packet_queue_.size() >= MAX_QUEUE_SIZE) {
                AVPacket* old_packet = packet_queue_.front();
                packet_queue_.pop();
                av_packet_free(&old_packet);
                LOG_WARNING("Packet queue full for RTSP stream " + std::to_string(stream_id_) + ", dropping old packet");
            }

            packet_queue_.push(packet);
            LOG_DEBUG("Added packet to queue for RTSP stream " + std::to_string(stream_id_) + ", queue size: " + std::to_string(packet_queue_.size()));
        }

        frame_cv_.notify_one();
    }

    LOG_INFO("RTSP stream puller thread stopped for stream " + std::to_string(stream_id_));
}

void VideoStreamPuller::resetConnectionState() {
    video_stream_index_ = -1;
}

void VideoStreamPuller::cleanup() {
    LOG_INFO("Cleaning up RTSP stream puller for stream " + std::to_string(stream_id_));

    if (format_ctx_) {
        avformat_close_input(&format_ctx_);
        format_ctx_ = nullptr;
        LOG_DEBUG("Closed RTSP format context");
    }

    if (sws_ctx_) {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = nullptr;
        LOG_DEBUG("Freed software scaling context");
    }

    // 清理包队列
    std::unique_lock<std::mutex> lock(frame_mutex_);
    size_t packet_queue_size = packet_queue_.size();
    while (!packet_queue_.empty()) {
        AVPacket* packet = packet_queue_.front();
        packet_queue_.pop();
        av_packet_free(&packet);
    }
    if (packet_queue_size > 0) {
        LOG_DEBUG("Cleaned up " + std::to_string(packet_queue_size) + " packets from queue for RTSP stream " + std::to_string(stream_id_));
    }

    // 清理帧队列
    size_t frame_queue_size = frame_queue_.size();
    while (!frame_queue_.empty()) {
        VideoFrame& frame = frame_queue_.front();
        for (int i = 0; i < 4; i++) {
            if (frame.data[i]) {
                delete[] frame.data[i];
                frame.data[i] = nullptr;
            }
        }
        frame_queue_.pop();
    }
    if (frame_queue_size > 0) {
        LOG_DEBUG("Cleaned up " + std::to_string(frame_queue_size) + " frames from queue for RTSP stream " + std::to_string(stream_id_));
    }

    // 清理最新帧
    for (int i = 0; i < 4; i++) {
        if (latest_frame_.data[i]) {
            delete[] latest_frame_.data[i];
            latest_frame_.data[i] = nullptr;
        }
    }
}
