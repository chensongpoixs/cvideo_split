/**
 * @file VideoDecoder.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief 视频解码器实现（RTSP 拉流 + NVDEC 硬件解码）
 * @see VideoDecoder.h
 */

#include "VideoDecoder.h"
#include "CudaStreamManager.h"
#include "Logger.h"
#include <chrono>

VideoDecoder::VideoDecoder(int stream_id)
    : stream_id_(stream_id)
    , gpu_memory_manager_(GpuMemoryManager::getInstance())
    , platform_display_(nullptr)
    , decoder_region_id_(-1)
    , running_(false)
    , frames_decoded_(0)
    , packets_processed_(0) {
}

VideoDecoder::~VideoDecoder() {
    stop();
}

bool VideoDecoder::initialize(const std::string& rtsp_url, PlatformDisplay* display) {
    LOG_INFO("Initializing VideoDecoder for stream " + std::to_string(stream_id_) + " with URL: " + rtsp_url);

    rtsp_url_ = rtsp_url;
    platform_display_ = display;

    // 创建组件
    stream_puller_ = std::make_unique<VideoStreamPuller>(rtsp_url, stream_id_);
    if (!stream_puller_->initialize()) {
        // 允许启动时连接失败：拉流线程会按 3s 周期自动重连，解码线程会等待 codec 参数后再初始化硬解码
        LOG_WARNING("Stream puller init failed for stream " + std::to_string(stream_id_) +
                    ", will retry in background (decoder will start once stream reconnects)");
    } else {
        // 拉流器已连上：尝试立即初始化硬件解码器
        const AVCodecParameters* codec_params = stream_puller_->getVideoCodecParameters();
        if (!codec_params) {
            LOG_WARNING("Codec parameters not ready for stream " + std::to_string(stream_id_) +
                        ", hardware decoder will be initialized later");
        } else {
            auto hd = std::make_unique<HardwareDecoder>();
            if (!hd->initializeWithCodecParams(codec_params)) {
                LOG_WARNING("Failed to initialize hardware decoder for stream " + std::to_string(stream_id_) +
                            ", will retry later");
            } else {
                hardware_decoder_ = std::move(hd);
            }
        }
    }

    LOG_INFO("VideoDecoder initialized (async reconnect enabled) for stream " + std::to_string(stream_id_));
    return true;
}

void VideoDecoder::start() {
    if (running_) return;

    LOG_INFO("Starting VideoDecoder thread for stream " + std::to_string(stream_id_));
    running_ = true;

    // 启动流拉取器
    stream_puller_->start();

    // 启动解码线程
    decode_thread_ = std::thread(&VideoDecoder::decodeThread, this);
}

void VideoDecoder::stop() {
    if (!running_) return;

    LOG_INFO("Stopping VideoDecoder for stream " + std::to_string(stream_id_));
    running_ = false;

    // 停止流拉取器
    stream_puller_->stop();

    // 等待解码线程结束
    if (decode_thread_.joinable()) {
        decode_thread_.join();
    }

    LOG_INFO("VideoDecoder stopped for stream " + std::to_string(stream_id_) +
             ", decoded " + std::to_string(frames_decoded_.load()) + " frames, " +
             "processed " + std::to_string(packets_processed_.load()) + " packets");
}

void VideoDecoder::decodeThread() {
    LOG_INFO("VideoDecoder thread started for stream " + std::to_string(stream_id_));

    while (running_) {
        // 若硬解码器尚未初始化，等待拉流器重连成功并拿到 codec 参数后再初始化
        if (!hardware_decoder_) {
            const AVCodecParameters* codec_params = stream_puller_ ? stream_puller_->getVideoCodecParameters() : nullptr;
            if (codec_params) {
                auto hd = std::make_unique<HardwareDecoder>();
                if (hd->initializeWithCodecParams(codec_params)) {
                    hardware_decoder_ = std::move(hd);
                    LOG_INFO("Hardware decoder initialized in background for stream " + std::to_string(stream_id_));
                } else {
                    LOG_WARNING("Hardware decoder init failed in background for stream " +
                                std::to_string(stream_id_) + ", will retry");
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // 从流拉取器获取包
        AVPacket* packet = nullptr;
        if (stream_puller_->getLatestPacket(&packet)) {
            packets_processed_++;

            LOG_DEBUG("Stream " + std::to_string(stream_id_) + " processing packet " +
                     std::to_string(packets_processed_.load()) + ", size: " + std::to_string(packet->size) + " bytes");

            // 处理包
            if (processPacket(packet)) {
                frames_decoded_++;
                LOG_DEBUG("Stream " + std::to_string(stream_id_) + " decoded frame " +
                         std::to_string(frames_decoded_.load()));
            }

            // 释放包
            av_packet_free(&packet);
        } else {
            // 没有新包时短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    LOG_INFO("VideoDecoder thread stopped for stream " + std::to_string(stream_id_));
}

bool VideoDecoder::processPacket(AVPacket* packet) {
    if (!packet || !hardware_decoder_) {
        return false;
    }

    // 解码包
    DecodedFrame decoded_frame;
    if (!hardware_decoder_->decodePacket(packet, decoded_frame)) {
        LOG_DEBUG("No frame decoded from packet in stream " + std::to_string(stream_id_));
        return false;
    }

    // CUDA硬件解码器直接返回GPU内存，我们可以直接使用指针共享
    if (decoded_frame.is_cuda_frame && decoded_frame.nv12_data && !decoded_frame.owns_memory) {
        // CUDA/NVDEC 输出常为 pitched memory（linesize_y != width），
        // 当前拼接/显示/编码路径假设是紧密 NV12（pitch == width）。
        // 为保证 4K/任意分辨率正确，我们对 pitched 帧做一次 GPU->GPU 2D 打包拷贝，得到紧密 NV12。
        const int pitch_y = decoded_frame.linesize_y > 0 ? decoded_frame.linesize_y : decoded_frame.width;
        const int pitch_uv = decoded_frame.linesize_uv > 0 ? decoded_frame.linesize_uv : pitch_y;
        const bool needs_pack = (pitch_y != decoded_frame.width) || (pitch_uv != decoded_frame.width);

        std::shared_ptr<GpuFrame> gpu_frame;

        if (needs_pack) {
            LOG_INFO("Stream " + std::to_string(stream_id_) +
                     " packing pitched NV12 -> tight NV12, src " +
                     std::to_string(decoded_frame.width) + "x" + std::to_string(decoded_frame.height) +
                     ", pitchY=" + std::to_string(pitch_y) +
                     ", pitchUV=" + std::to_string(pitch_uv));

            gpu_frame = gpu_memory_manager_.allocateFrame(decoded_frame.width, decoded_frame.height, true, stream_id_);
            if (!gpu_frame) {
                LOG_ERROR("Failed to allocate packed GPU frame for stream " + std::to_string(stream_id_));
                if (decoded_frame.av_frame_ref) {
                    av_frame_free(&decoded_frame.av_frame_ref);
                }
                return false;
            }

            uint8_t* src_base = decoded_frame.nv12_data;
            uint8_t* dst_base = reinterpret_cast<uint8_t*>(gpu_frame->gpu_ptr);

            if (pitch_y < decoded_frame.width || pitch_uv < decoded_frame.width) {
                LOG_WARNING("Unexpected NV12 pitch smaller than width, forcing tight copy assumption. pitchY=" +
                            std::to_string(pitch_y) + ", pitchUV=" + std::to_string(pitch_uv) +
                            ", width=" + std::to_string(decoded_frame.width));
            }

            // 使用统一的 decode stream 进行异步拷贝（异步管道）
            auto& stream_mgr = CudaStreamManager::getInstance();
            cudaStream_t decode_stream = stream_mgr.getDecodeStream();

            // Y plane: height rows, width bytes per row (异步)
            cudaError_t e1 = cudaMemcpy2DAsync(dst_base, decoded_frame.width,
                                               src_base, pitch_y,
                                               decoded_frame.width, decoded_frame.height,
                                               cudaMemcpyDeviceToDevice, decode_stream);
            if (e1 != cudaSuccess) {
                LOG_ERROR("cudaMemcpy2DAsync pack Y failed: " + std::string(cudaGetErrorString(e1)));
                gpu_memory_manager_.freeFrame(gpu_frame);
                if (decoded_frame.av_frame_ref) {
                    av_frame_free(&decoded_frame.av_frame_ref);
                }
                return false;
            }

            // UV plane: height/2 rows, width bytes per row, src starts at base + pitch_y*height (异步)
            uint8_t* src_uv = src_base + static_cast<size_t>(pitch_y) * decoded_frame.height;
            uint8_t* dst_uv = dst_base + static_cast<size_t>(decoded_frame.width) * decoded_frame.height;
            cudaError_t e2 = cudaMemcpy2DAsync(dst_uv, decoded_frame.width,
                                               src_uv, pitch_uv,
                                               decoded_frame.width, decoded_frame.height / 2,
                                               cudaMemcpyDeviceToDevice, decode_stream);
            if (e2 != cudaSuccess) {
                LOG_ERROR("cudaMemcpy2DAsync pack UV failed: " + std::string(cudaGetErrorString(e2)));
                gpu_memory_manager_.freeFrame(gpu_frame);
                if (decoded_frame.av_frame_ref) {
                    av_frame_free(&decoded_frame.av_frame_ref);
                }
                return false;
            }

            // 记录解码完成事件（供下游 stitch/display 等待）
            stream_mgr.recordDecodeComplete();

            // 同步以确保 FFmpeg surface 可以释放
            // 注意：这里必须同步，因为 FFmpeg 的 surface 在 av_frame_free 后会被复用
            cudaStreamSynchronize(decode_stream);

            gpu_frame->pts = decoded_frame.pts;

            // 打包后就不再依赖FFmpeg的surface引用了，释放掉引用帧
            if (decoded_frame.av_frame_ref) {
                av_frame_free(&decoded_frame.av_frame_ref);
            }
        } else {
            // 不需要打包：直接使用硬件解码器的GPU内存指针创建GPU帧（零拷贝）
            gpu_frame = gpu_memory_manager_.createFrameFromPointer(
                decoded_frame.nv12_data,
                decoded_frame.mem_handle,
                decoded_frame.data_size,
                decoded_frame.width,
                decoded_frame.height,
                decoded_frame.is_nv12,
                stream_id_,
                decoded_frame.pts,
                false  // 不拥有内存，由FFmpeg管理
            );

            // 关键：把 FFmpeg 的 AVFrame 引用挂到 GpuFrame 上，保证 GPU surface 在下游使用期间不会被复用/回收
            if (decoded_frame.av_frame_ref) {
                AVFrame* ref = decoded_frame.av_frame_ref;
                decoded_frame.av_frame_ref = nullptr; // 转移所有权到 gpu_frame
                gpu_frame->external_ref = std::shared_ptr<void>(
                    ref,
                    [](void* p) {
                        AVFrame* f = reinterpret_cast<AVFrame*>(p);
                        av_frame_free(&f);
                    }
                );
            }
        }

        LOG_DEBUG("Stream " + std::to_string(stream_id_) + " created GPU frame from hardware decoder GPU memory: " +
                 std::to_string(decoded_frame.width) + "x" + std::to_string(decoded_frame.height) +
                 ", PTS: " + std::to_string(decoded_frame.pts) +
                 ", GPU ptr: " + std::to_string(reinterpret_cast<uintptr_t>(decoded_frame.nv12_data)));

        // 将帧推送到队列，供拼接线程使用
        if (!gpu_memory_manager_.pushFrameToQueue(stream_id_, gpu_frame)) {
            LOG_ERROR("Failed to push frame to queue for stream " + std::to_string(stream_id_));
            // 注意：这里不释放gpu_frame，因为它不拥有内存
            return false;
        }

        // 更新OpenGL显示（如果启用）
        if (platform_display_ && decoder_region_id_ >= 0) {
            platform_display_->updateRegionFrame(decoder_region_id_, gpu_frame);
        }

        return true;
    } else {
        // 回退到传统方式：分配新的GPU内存并复制数据
        LOG_WARNING("Hardware decoder returned CPU memory or owned GPU memory, falling back to copy mode");

        auto gpu_frame = gpu_memory_manager_.allocateFrame(decoded_frame.width, decoded_frame.height,
                                                           decoded_frame.is_nv12, stream_id_);
        if (!gpu_frame) {
            LOG_ERROR("Failed to allocate GPU frame for stream " + std::to_string(stream_id_));
            return false;
        }

        // 如果解码器返回的是CPU内存或拥有的GPU内存，需要复制
        if (decoded_frame.nv12_data && decoded_frame.data_size > 0) {
            cudaMemcpyKind copy_kind = decoded_frame.is_cuda_frame ?
                cudaMemcpyDeviceToDevice : cudaMemcpyHostToDevice;

            cudaError_t cuda_err = cudaMemcpy(gpu_frame->gpu_ptr, decoded_frame.nv12_data,
                                            decoded_frame.data_size, copy_kind);
            if (cuda_err != cudaSuccess) {
                LOG_ERROR("Failed to copy decoded frame to GPU for stream " + std::to_string(stream_id_) +
                         ": " + std::string(cudaGetErrorString(cuda_err)));
                gpu_memory_manager_.freeFrame(gpu_frame);
                return false;
            }

            // 释放源内存（如果是我们分配的）
            if (decoded_frame.owns_memory) {
                if (decoded_frame.is_cuda_frame) {
                    cudaFree(decoded_frame.nv12_data);
                } else {
                    delete[] decoded_frame.nv12_data;
                }
            }
        }

        // 设置帧元数据
        gpu_frame->pts = decoded_frame.pts;

        // 将帧推送到队列，供拼接线程使用
        if (!gpu_memory_manager_.pushFrameToQueue(stream_id_, gpu_frame)) {
            LOG_ERROR("Failed to push frame to queue for stream " + std::to_string(stream_id_));
            gpu_memory_manager_.freeFrame(gpu_frame);
            return false;
        }

        LOG_DEBUG("Stream " + std::to_string(stream_id_) + " decoded and queued frame (copy mode): " +
                 std::to_string(decoded_frame.width) + "x" + std::to_string(decoded_frame.height) +
                 ", PTS: " + std::to_string(decoded_frame.pts));

        // 如果启用了显示，更新显示器
        if (platform_display_ && decoder_region_id_ >= 0) {
            platform_display_->updateRegionFrame(decoder_region_id_, gpu_frame);
        }

        return true;
    }
}
