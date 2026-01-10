#include "VideoEncoder.h"
#include "Logger.h"
#include <iostream>
#include <chrono>

VideoEncoder::VideoEncoder() : running_(false), frame_count_(0), gpu_memory_manager_(GpuMemoryManager::getInstance()) {
}

VideoEncoder::~VideoEncoder() {
    stop();
    cleanup();
}

bool VideoEncoder::initialize(int width, int height, int fps, const std::string& output_url) {
    output_url_ = output_url;

    // 分配输出格式上下文
    avformat_alloc_output_context2(&format_ctx_, nullptr, "rtsp", output_url_.c_str());
    if (!format_ctx_) {
        LOG_ERROR("Failed to allocate output format context");
        return false;
    }

    // 查找CUDA硬件编码器
    LOG_INFO("Initializing video encoder for resolution: " + std::to_string(width) + "x" + std::to_string(height));
    const AVCodec* codec = avcodec_find_encoder_by_name("h264_nvenc");
    if (!codec) {
        LOG_WARNING("CUDA H.264 encoder not found, trying software encoder");
        codec = avcodec_find_encoder_by_name("libx264");
        if (!codec) {
            codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        }
    }

    if (!codec) {
        LOG_ERROR("No suitable H.264 encoder found");
        return false;
    }

    bool is_cuda_encoder = (std::string(codec->name).find("_nvenc") != std::string::npos);
    LOG_INFO("Found encoder: " + std::string(codec->name) +
             (is_cuda_encoder ? " (CUDA hardware)" : " (software)"));

    // 创建编码器上下文
    codec_ctx_ = avcodec_alloc_context3(codec);
    if (!codec_ctx_) {
        LOG_ERROR("Failed to allocate codec context");
        return false;
    }

    // 设置编码参数
    codec_ctx_->width = width;
    codec_ctx_->height = height;

    if (is_cuda_encoder) {
        LOG_INFO("Configuring CUDA hardware encoder parameters");
        codec_ctx_->pix_fmt = AV_PIX_FMT_CUDA;  // CUDA编码器使用CUDA像素格式

        // 初始化CUDA硬件设备上下文
        int ret = av_hwdevice_ctx_create(&hw_device_ctx_, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0);
        if (ret < 0) {
            LOG_ERROR("Failed to create CUDA hardware device context for encoder, error code: " + std::to_string(ret));
            return false;
        }

        // 设置编码器的硬件设备上下文
        codec_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
        if (!codec_ctx_->hw_device_ctx) {
            LOG_ERROR("Failed to reference CUDA hardware device context for encoder");
            return false;
        }

        LOG_INFO("CUDA hardware device context created for encoder");
    } else {
        LOG_INFO("Configuring software encoder parameters");
        codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;  // 软件编码器使用YUV420P
    }

    codec_ctx_->time_base = {1, fps};
    codec_ctx_->framerate = {fps, 1};
    codec_ctx_->bit_rate = 2000000; // 2Mbps
    codec_ctx_->gop_size = fps * 2; // 2秒一个关键帧
    codec_ctx_->max_b_frames = 1;

    // 设置编码选项
    AVDictionary* codec_options = nullptr;

    if (is_cuda_encoder) {
        LOG_INFO("Setting CUDA hardware encoder options");
        // CUDA编码器的选项
        av_dict_set(&codec_options, "preset", "fast", 0);
        av_dict_set(&codec_options, "tune", "zerolatency", 0);
        av_dict_set(&codec_options, "profile", "main", 0);
        // CUDA特定的选项
        av_dict_set(&codec_options, "gpu", "0", 0);  // 使用GPU 0
    } else {
        LOG_INFO("Setting software encoder options");
        av_dict_set(&codec_options, "preset", "fast", 0);
        av_dict_set(&codec_options, "tune", "zerolatency", 0);
        av_dict_set(&codec_options, "profile", "main", 0);
    }

    // 打开编码器
    LOG_DEBUG("Opening encoder");
    int ret = avcodec_open2(codec_ctx_, codec, &codec_options);
    av_dict_free(&codec_options);

    if (ret < 0) {
        LOG_ERROR("Failed to open encoder, error code: " + std::to_string(ret));
        // 如果CUDA硬件编码器失败，尝试回退到软件编码器
        if (is_cuda_encoder) {
            LOG_WARNING("CUDA hardware encoder failed, trying software encoder");
            // 清理当前的编解码器上下文
            avcodec_free_context(&codec_ctx_);
            codec_ctx_ = nullptr;

            // 重新查找软件编码器
            const AVCodec* sw_codec = avcodec_find_encoder_by_name("libx264");
            if (!sw_codec) {
                sw_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
            }

            if (sw_codec) {
                codec_ctx_ = avcodec_alloc_context3(sw_codec);
                if (codec_ctx_) {
                    codec_ctx_->width = width;
                    codec_ctx_->height = height;
                    codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
                    codec_ctx_->time_base = {1, fps};
                    codec_ctx_->framerate = {fps, 1};
                    codec_ctx_->bit_rate = 2000000;
                    codec_ctx_->gop_size = fps * 2;
                    codec_ctx_->max_b_frames = 1;

                    AVDictionary* sw_options = nullptr;
                    av_dict_set(&sw_options, "preset", "fast", 0);
                    av_dict_set(&sw_options, "tune", "zerolatency", 0);
                    av_dict_set(&sw_options, "profile", "main", 0);

                    ret = avcodec_open2(codec_ctx_, sw_codec, &sw_options);
                    av_dict_free(&sw_options);

                    if (ret >= 0) {
                        LOG_INFO("Successfully fell back to software encoder");
                        is_cuda_encoder = false;
                    }
                }
            }
        }

        if (ret < 0) {
            LOG_ERROR("Failed to open both CUDA hardware and software encoders");
            return false;
        }
    }

    LOG_INFO("Encoder opened successfully");

    // 创建流
    stream_ = avformat_new_stream(format_ctx_, nullptr);
    if (!stream_) {
        LOG_ERROR("Failed to create output stream");
        return false;
    }

    stream_->time_base = codec_ctx_->time_base;
    stream_->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    avcodec_parameters_from_context(stream_->codecpar, codec_ctx_);

    // 打开输出
    if (!(format_ctx_->oformat->flags & AVFMT_NOFILE)) {
        LOG_INFO("Opening output stream: " + output_url_);
        ret = avio_open(&format_ctx_->pb, output_url_.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOG_ERROR("Failed to open output stream: " + output_url_ + ", error code: " + std::to_string(ret));
            return false;
        }
        LOG_INFO("Output stream opened successfully");
    }

    // 写入文件头
    LOG_DEBUG("Writing output stream header");
    ret = avformat_write_header(format_ctx_, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to write output stream header, error code: " + std::to_string(ret));
        return false;
    }
    LOG_INFO("Output stream header written successfully");

    width_ = width;
    height_ = height;
    fps_ = fps;

    return true;
}

bool VideoEncoder::encodeGpuFrame(std::shared_ptr<GpuFrame> gpu_frame, EncodedPacket& packet) {
    if (!gpu_frame || !gpu_frame->gpu_ptr || !codec_ctx_) {
        LOG_ERROR("Invalid parameters for GPU frame encoding");
        return false;
    }

    // 检查是否为CUDA硬件编码器
    bool is_cuda_encoder = (std::string(codec_ctx_->codec->name).find("_nvenc") != std::string::npos);

    AVFrame* frame = nullptr;

    if (is_cuda_encoder && hw_device_ctx_) {
        // CUDA硬件编码器：直接使用GPU内存
        frame = createFrameFromGpuDirect(gpu_frame);
        LOG_DEBUG("Using CUDA hardware encoder with GPU memory");
    } else {
        // 软件编码器或CUDA编码器失败：从GPU复制到CPU
        frame = createFrameFromGpu(gpu_frame);
        LOG_DEBUG("Using software encoder with CPU memory (copied from GPU)");
    }

    if (!frame) {
        LOG_ERROR("Failed to create frame from GPU memory");
        return false;
    }

    frame->pts = gpu_frame->pts;

    // 发送帧到编码器
    int ret = avcodec_send_frame(codec_ctx_, frame);
    if (ret < 0) {
        LOG_ERROR("Failed to send frame to encoder, error code: " + std::to_string(ret));
        av_frame_free(&frame);
        return false;
    }

    // 接收编码包
    AVPacket* av_packet = av_packet_alloc();
    if (!av_packet) {
        LOG_ERROR("Failed to allocate packet");
        av_frame_free(&frame);
        return false;
    }

    ret = avcodec_receive_packet(codec_ctx_, av_packet);
    if (ret < 0) {
        if (ret == AVERROR(EAGAIN)) {
            LOG_DEBUG("Encoder needs more frames");
        } else {
            LOG_ERROR("Failed to receive packet from encoder, error code: " + std::to_string(ret));
        }
        av_packet_free(&av_packet);
        av_frame_free(&frame);
        return false;
    }

    // 填充输出包
    packet.data = new uint8_t[av_packet->size];
    memcpy(packet.data, av_packet->data, av_packet->size);
    packet.size = av_packet->size;
    packet.pts = av_packet->pts;
    packet.dts = av_packet->dts;
    packet.flags = av_packet->flags;
    packet.is_keyframe = (av_packet->flags & AV_PKT_FLAG_KEY);

    av_packet_free(&av_packet);
    av_frame_free(&frame);

    LOG_DEBUG("GPU frame encoded successfully, size: " + std::to_string(packet.size) + " bytes, keyframe: " + (packet.is_keyframe ? "yes" : "no"));
    return true;
}

bool VideoEncoder::flush(EncodedPacket& packet) {
    if (!codec_ctx_) {
        return false;
    }

    // 发送NULL帧以刷新编码器
    int ret = avcodec_send_frame(codec_ctx_, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to flush encoder, error code: " + std::to_string(ret));
        return false;
    }

    // 接收剩余的编码包
    AVPacket* av_packet = av_packet_alloc();
    if (!av_packet) {
        LOG_ERROR("Failed to allocate packet for flush");
        return false;
    }

    ret = avcodec_receive_packet(codec_ctx_, av_packet);
    if (ret < 0) {
        if (ret == AVERROR_EOF) {
            LOG_DEBUG("Encoder flush completed");
        } else {
            LOG_ERROR("Failed to receive flushed packet, error code: " + std::to_string(ret));
        }
        av_packet_free(&av_packet);
        return false;
    }

    // 填充输出包
    packet.data = new uint8_t[av_packet->size];
    memcpy(packet.data, av_packet->data, av_packet->size);
    packet.size = av_packet->size;
    packet.pts = av_packet->pts;
    packet.dts = av_packet->dts;
    packet.flags = av_packet->flags;
    packet.is_keyframe = (av_packet->flags & AV_PKT_FLAG_KEY);

    av_packet_free(&av_packet);

    LOG_DEBUG("Flushed packet encoded, size: " + std::to_string(packet.size) + " bytes");
    return true;
}

bool VideoEncoder::writePacket(const EncodedPacket& packet) {
    if (!format_ctx_ || !stream_) {
        LOG_ERROR("Encoder not properly initialized");
        return false;
    }

    // 创建AVPacket
    AVPacket* av_packet = av_packet_alloc();
    if (!av_packet) {
        LOG_ERROR("Failed to allocate AVPacket");
        return false;
    }

    av_packet->data = packet.data;
    av_packet->size = packet.size;
    av_packet->pts = packet.pts;
    av_packet->dts = packet.dts;
    av_packet->flags = packet.flags;
    av_packet->stream_index = stream_->index;

    // 写入包
    int ret = av_interleaved_write_frame(format_ctx_, av_packet);
    if (ret < 0) {
        LOG_ERROR("Failed to write packet, error code: " + std::to_string(ret));
        av_packet_free(&av_packet);
        return false;
    }

    av_packet_free(&av_packet);

    frame_count_++;
    LOG_DEBUG("Packet written successfully, frame count: " + std::to_string(frame_count_.load()));
    return true;
}

void VideoEncoder::start() {
    if (running_) return;

    LOG_INFO("Starting VideoEncoder thread");
    running_ = true;
    encode_thread_ = std::thread(&VideoEncoder::encodeThread, this);
}

void VideoEncoder::stop() {
    if (!running_) return;

    LOG_INFO("Stopping VideoEncoder");
    running_ = false;

    if (encode_thread_.joinable()) {
        encode_thread_.join();
    }

    LOG_INFO("VideoEncoder stopped, encoded " + std::to_string(frame_count_.load()) + " frames");
}

void VideoEncoder::encodeThread() {
    LOG_INFO("VideoEncoder thread started");

    while (running_) {
        if (!processGpuQueue()) {
            // 没有帧可处理，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    LOG_INFO("VideoEncoder thread stopped");
}

bool VideoEncoder::processGpuQueue() {
    // 从GPU队列（流ID -1）获取拼接后的帧
    auto gpu_frame = gpu_memory_manager_.popFrameFromQueue(-1);
    if (!gpu_frame) {
        return false;
    }

    LOG_DEBUG("Processing GPU frame for encoding: " + std::to_string(gpu_frame->width) + "x" +
             std::to_string(gpu_frame->height) + ", PTS: " + std::to_string(gpu_frame->pts));

    // 编码GPU帧
    LOG_DEBUG("Starting to encode GPU frame: " + std::to_string(gpu_frame->width) + "x" + std::to_string(gpu_frame->height));

    EncodedPacket packet;
    if (encodeGpuFrame(gpu_frame, packet)) {
        LOG_DEBUG("GPU frame encoded successfully, packet size: " + std::to_string(packet.size) + " bytes");

        // 写入编码包
        if (writePacket(packet)) {
            LOG_DEBUG("Encoded packet written successfully");
        } else {
            LOG_ERROR("Failed to write encoded packet");
        }

        // 释放编码包数据
        delete[] packet.data;
    } else {
        LOG_WARNING("Failed to encode GPU frame - will continue processing other frames");
    }

    // 释放GPU帧
    gpu_memory_manager_.freeFrame(gpu_frame);
    return true;
}

AVFrame* VideoEncoder::createFrameFromGpu(std::shared_ptr<GpuFrame> gpu_frame) {
    if (!gpu_frame) return nullptr;

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        LOG_ERROR("Failed to allocate AVFrame");
        return nullptr;
    }

    frame->format = codec_ctx_->pix_fmt;
    frame->width = gpu_frame->width;
    frame->height = gpu_frame->height;

    // 分配帧缓冲区
    int ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        LOG_ERROR("Failed to allocate frame buffer, error code: " + std::to_string(ret));
        av_frame_free(&frame);
        return nullptr;
    }

    // 从GPU内存复制数据到CPU内存（NV12紧密布局：Y plane + UV plane）
    // 这里不要写死 width*height*3/2，优先使用 gpu_frame->size（上游可能有对齐/打包差异）。
    size_t data_size = gpu_frame->size;
    const size_t expected_nv12 = static_cast<size_t>(gpu_frame->width) * gpu_frame->height * 3 / 2;
    if (data_size > expected_nv12) {
        data_size = expected_nv12;
    }

    LOG_DEBUG("Copying " + std::to_string(data_size) + " bytes from GPU to CPU for encoding");

    std::vector<uint8_t> cpu_data(data_size);
    if (!gpu_memory_manager_.copyFromGpu(gpu_frame, cpu_data.data(), data_size)) {
        LOG_ERROR("Failed to copy data from GPU to CPU for frame " +
                 std::to_string(gpu_frame->width) + "x" + std::to_string(gpu_frame->height));
        av_frame_free(&frame);
        return nullptr;
    }

    LOG_DEBUG("Successfully copied data from GPU to CPU");

    // 把 CPU NV12 填充到目标像素格式：
    // - 若编码器 pix_fmt 是 NV12：直接按行拷贝到 data[0]/data[1]
    // - 若编码器 pix_fmt 是 YUV420P：用 swscale 做 NV12 -> YUV420P
    if (codec_ctx_->pix_fmt == AV_PIX_FMT_NV12) {
        const uint8_t* src_y = cpu_data.data();
        const uint8_t* src_uv = cpu_data.data() + static_cast<size_t>(gpu_frame->width) * gpu_frame->height;
        // Y
        for (int y = 0; y < gpu_frame->height; ++y) {
            memcpy(frame->data[0] + y * frame->linesize[0],
                   src_y + static_cast<size_t>(y) * gpu_frame->width,
                   gpu_frame->width);
        }
        // UV
        const int uv_h = gpu_frame->height / 2;
        for (int y = 0; y < uv_h; ++y) {
            memcpy(frame->data[1] + y * frame->linesize[1],
                   src_uv + static_cast<size_t>(y) * gpu_frame->width,
                   gpu_frame->width);
        }
    } else if (codec_ctx_->pix_fmt == AV_PIX_FMT_YUV420P) {
        static SwsContext* sws_ctx = nullptr;
        static int sws_w = 0;
        static int sws_h = 0;

        if (!sws_ctx || sws_w != gpu_frame->width || sws_h != gpu_frame->height) {
            if (sws_ctx) {
                sws_freeContext(sws_ctx);
                sws_ctx = nullptr;
            }
            sws_w = gpu_frame->width;
            sws_h = gpu_frame->height;
            sws_ctx = sws_getContext(
                sws_w, sws_h, AV_PIX_FMT_NV12,
                sws_w, sws_h, AV_PIX_FMT_YUV420P,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            );
        }

        if (!sws_ctx) {
            LOG_ERROR("Failed to create SwsContext for NV12->YUV420P conversion");
            av_frame_free(&frame);
            return nullptr;
        }

        const uint8_t* src_slices[4] = { nullptr, nullptr, nullptr, nullptr };
        int src_linesize[4] = { 0, 0, 0, 0 };
        src_slices[0] = cpu_data.data();
        src_slices[1] = cpu_data.data() + static_cast<size_t>(gpu_frame->width) * gpu_frame->height;
        src_linesize[0] = gpu_frame->width;
        src_linesize[1] = gpu_frame->width;

        int scaled = sws_scale(sws_ctx, src_slices, src_linesize, 0, gpu_frame->height, frame->data, frame->linesize);
        if (scaled != gpu_frame->height) {
            LOG_WARNING("sws_scale returned " + std::to_string(scaled) + ", expected " + std::to_string(gpu_frame->height));
        }
    } else {
        LOG_ERROR("Unsupported encoder pix_fmt for CPU path: " + std::to_string(codec_ctx_->pix_fmt));
        av_frame_free(&frame);
        return nullptr;
    }

    return frame;
}

AVFrame* VideoEncoder::createFrameFromGpuDirect(std::shared_ptr<GpuFrame> gpu_frame) {
    if (!gpu_frame || !gpu_frame->gpu_ptr) {
        LOG_ERROR("Invalid GPU frame for direct encoding");
        return nullptr;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        LOG_ERROR("Failed to allocate AVFrame for direct GPU encoding");
        return nullptr;
    }

    // 设置为CUDA像素格式
    frame->format = AV_PIX_FMT_CUDA;
    frame->width = gpu_frame->width;
    frame->height = gpu_frame->height;

    // 为CUDA硬件帧分配缓冲区
    frame->hw_frames_ctx = av_hwframe_ctx_alloc(hw_device_ctx_);
    if (!frame->hw_frames_ctx) {
        LOG_ERROR("Failed to allocate hardware frame context");
        av_frame_free(&frame);
        return nullptr;
    }

    AVHWFramesContext* hw_frames_ctx = (AVHWFramesContext*)frame->hw_frames_ctx->data;
    hw_frames_ctx->format = AV_PIX_FMT_CUDA;
    hw_frames_ctx->sw_format = AV_PIX_FMT_NV12;
    hw_frames_ctx->width = gpu_frame->width;
    hw_frames_ctx->height = gpu_frame->height;

    int ret = av_hwframe_ctx_init(frame->hw_frames_ctx);
    if (ret < 0) {
        LOG_ERROR("Failed to initialize hardware frame context, error code: " + std::to_string(ret));
        av_frame_free(&frame);
        return nullptr;
    }

    // 直接设置GPU内存指针到帧数据
    frame->data[0] = reinterpret_cast<uint8_t*>(gpu_frame->gpu_ptr);
    frame->linesize[0] = gpu_frame->width;  // 对于NV12，Y分量的行大小

    LOG_DEBUG("Created AVFrame for direct GPU encoding: " +
             std::to_string(gpu_frame->width) + "x" + std::to_string(gpu_frame->height) +
             ", GPU ptr: " + std::to_string(reinterpret_cast<uintptr_t>(gpu_frame->gpu_ptr)));

    return frame;
}

void VideoEncoder::cleanup() {
    if (codec_ctx_) {
        avcodec_free_context(&codec_ctx_);
        codec_ctx_ = nullptr;
    }

    if (hw_device_ctx_) {
        av_buffer_unref(&hw_device_ctx_);
        hw_device_ctx_ = nullptr;
    }

    if (format_ctx_) {
        if (!(format_ctx_->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&format_ctx_->pb);
        }
        avformat_free_context(format_ctx_);
        format_ctx_ = nullptr;
    }

    stream_ = nullptr;
    codec_ = nullptr;
}