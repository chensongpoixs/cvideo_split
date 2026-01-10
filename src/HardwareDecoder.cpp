#include "HardwareDecoder.h"
#include "Logger.h"
#include <iostream>
#include <cuda_runtime.h>

HardwareDecoder::HardwareDecoder() : initialized_(false) {
}

HardwareDecoder::~HardwareDecoder() {
    cleanup();
}

bool HardwareDecoder::initialize(const std::string& codec_name) {
    if (initialized_) return true;

    LOG_INFO("Initializing CUDA hardware decoder for codec: " + codec_name);

    // 查找CUDA硬件解码器
    if (codec_name == "h264") {
        codec_ = avcodec_find_decoder_by_name("h264_cuvid");
        if (!codec_) {
            LOG_WARNING("CUDA H.264 decoder not found, trying software decoder");
            codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
        }
    } else if (codec_name == "h265" || codec_name == "hevc") {
        codec_ = avcodec_find_decoder_by_name("hevc_cuvid");
        if (!codec_) {
            LOG_WARNING("CUDA HEVC decoder not found, trying software decoder");
            codec_ = avcodec_find_decoder(AV_CODEC_ID_HEVC);
        }
    } else {
        LOG_ERROR("Unsupported codec: " + codec_name);
        return false;
    }

    if (!codec_) {
        LOG_ERROR("No suitable decoder found for: " + codec_name);
        return false;
    }

    LOG_DEBUG("Found codec: " + std::string(codec_->name));

    // 创建编解码器上下文
    codec_ctx_ = avcodec_alloc_context3(codec_);
    if (!codec_ctx_) {
        LOG_ERROR("Failed to allocate codec context");
        return false;
    }

    // 设置硬件像素格式 - CUDA解码直接输出NV12
    codec_ctx_->pix_fmt = AV_PIX_FMT_CUDA;
    codec_ctx_->sw_pix_fmt = AV_PIX_FMT_NV12;

    // 初始化硬件上下文
    if (!initHWContext()) {
        LOG_ERROR("Failed to initialize CUDA hardware context");
        return false;
    }

    // 打开编解码器
    int ret = avcodec_open2(codec_ctx_, codec_, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to open codec, error code: " + std::to_string(ret));
        return false;
    }

    // 分配硬件帧（用于CUDA解码）
    hw_frame_ = av_frame_alloc();
    if (!hw_frame_) {
        LOG_ERROR("Failed to allocate hardware frame");
        return false;
    }

    LOG_INFO("CUDA hardware decoder initialized successfully for " + codec_name);
    initialized_ = true;
    return true;
}

bool HardwareDecoder::initHWContext() {
    LOG_DEBUG("Creating CUDA hardware device context");

    int ret = av_hwdevice_ctx_create(&hw_device_ctx_, hw_type_, nullptr, nullptr, 0);
    if (ret < 0) {
        LOG_ERROR("Failed to create CUDA hardware device context, error code: " + std::to_string(ret));
        return false;
    }

    codec_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
    if (!codec_ctx_->hw_device_ctx) {
        LOG_ERROR("Failed to reference CUDA hardware device context");
        return false;
    }

    LOG_DEBUG("CUDA hardware device context created successfully");
    return true;
}

bool HardwareDecoder::decodePacket(const AVPacket* packet, DecodedFrame& decoded_frame) {
    if (!initialized_) return false;

    LOG_DEBUG("Sending packet for CUDA hardware decoding, packet size: " + std::to_string(packet->size));

    int ret = avcodec_send_packet(codec_ctx_, packet);
    if (ret < 0) {
        LOG_ERROR("Error sending packet for CUDA decoding, error code: " + std::to_string(ret));
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(codec_ctx_, hw_frame_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            LOG_DEBUG("No frame available yet or end of stream");
            return false;
        } else if (ret < 0) {
            LOG_ERROR("Error during CUDA decoding, error code: " + std::to_string(ret));
            return false;
        }

        LOG_INFO("=== CUDA Hardware Decoded Frame ===");
        LOG_INFO("Frame format: " + std::to_string(hw_frame_->format) +
                 " (" + (hw_frame_->format == AV_PIX_FMT_CUDA ? "CUDA" : "OTHER") + ")");
        LOG_INFO("Frame resolution: " + std::to_string(hw_frame_->width) + "x" + std::to_string(hw_frame_->height));
        LOG_INFO("Frame PTS: " + std::to_string(hw_frame_->pts));
        LOG_INFO("Frame linesize[0]: " + std::to_string(hw_frame_->linesize[0]));
        LOG_INFO("Frame linesize[1]: " + std::to_string(hw_frame_->linesize[1]));
        LOG_INFO("Frame linesize[2]: " + std::to_string(hw_frame_->linesize[2]));
        LOG_INFO("==================================");

        // CUDA硬件解码直接输出NV12格式，数据在GPU内存中
        if (hw_frame_->format == AV_PIX_FMT_CUDA) {
            // 对 hw_frame_ 做一次引用拷贝，保证底层 GPU surface 在下游使用期间不会被 FFmpeg 复用/回收
            AVFrame* frame_ref = av_frame_alloc();
            if (!frame_ref) {
                LOG_ERROR("Failed to allocate AVFrame for reference");
                return false;
            }
            if (av_frame_ref(frame_ref, hw_frame_) < 0) {
                LOG_ERROR("Failed to av_frame_ref() decoded CUDA frame");
                av_frame_free(&frame_ref);
                return false;
            }

            decoded_frame.width = hw_frame_->width;
            decoded_frame.height = hw_frame_->height;
            decoded_frame.pts = hw_frame_->pts;
            decoded_frame.is_cuda_frame = true;
            decoded_frame.is_nv12 = true;
            decoded_frame.av_frame_ref = frame_ref;
            decoded_frame.linesize_y = frame_ref->linesize[0];
            decoded_frame.linesize_uv = frame_ref->linesize[1] ? frame_ref->linesize[1] : frame_ref->linesize[0];

            // 计算NV12数据大小
            // 注意：CUDA/NVDEC 输出通常是 pitched memory（linesize >= width）
            // UV plane 起始通常是 base + linesize_y * height
            size_t y_size = static_cast<size_t>(decoded_frame.linesize_y) * decoded_frame.height;
            size_t uv_size = static_cast<size_t>(decoded_frame.linesize_uv) * (decoded_frame.height / 2);
            decoded_frame.data_size = y_size + uv_size;

            LOG_INFO("Decoded frame data size: " + std::to_string(decoded_frame.data_size) +
                     " bytes (Y: " + std::to_string(y_size) + ", UV: " + std::to_string(uv_size) + ")");

            // 直接使用引用帧的GPU内存，避免重新分配和复制
            decoded_frame.nv12_data = reinterpret_cast<uint8_t*>(frame_ref->data[0]);

            // 创建IPC内存句柄用于进程间共享
            cudaError_t cuda_ret = cudaIpcGetMemHandle(&decoded_frame.mem_handle, frame_ref->data[0]);
            if (cuda_ret != cudaSuccess) {
                LOG_WARNING("Failed to create IPC memory handle: " + std::string(cudaGetErrorString(cuda_ret)));
                // 继续执行，不返回false
            }

            // 标记这个结构体不拥有内存，由FFmpeg管理
            decoded_frame.owns_memory = false;

            LOG_INFO("CUDA hardware decoded frame ready - using GPU memory directly");
            LOG_INFO("Frame ready for further processing - Size: " +
                     std::to_string(decoded_frame.width) + "x" + std::to_string(decoded_frame.height) +
                     ", PTS: " + std::to_string(decoded_frame.pts) +
                     ", GPU ptr: " + std::to_string(reinterpret_cast<uintptr_t>(decoded_frame.nv12_data)));

            // 释放 hw_frame_ 对底层资源的引用（真正资源由 decoded_frame.av_frame_ref 持有）
            av_frame_unref(hw_frame_);
            return true;
        } else {
            LOG_WARNING("Unexpected frame format from CUDA decoder: " + std::to_string(hw_frame_->format) +
                       " (expected AV_PIX_FMT_CUDA)");
        }
    }

    return false;
}

void HardwareDecoder::flush() {
    if (codec_ctx_) {
        LOG_DEBUG("Flushing CUDA hardware decoder buffers");
        avcodec_flush_buffers(codec_ctx_);
    }
}

void HardwareDecoder::cleanup() {
    LOG_INFO("Cleaning up CUDA hardware decoder");

    if (sw_frame_) {
        av_frame_free(&sw_frame_);
        sw_frame_ = nullptr;
        LOG_DEBUG("Freed software frame");
    }

    if (hw_frame_) {
        av_frame_free(&hw_frame_);
        hw_frame_ = nullptr;
        LOG_DEBUG("Freed hardware frame");
    }

    if (codec_ctx_) {
        avcodec_free_context(&codec_ctx_);
        codec_ctx_ = nullptr;
        LOG_DEBUG("Freed codec context");
    }

    if (hw_device_ctx_) {
        av_buffer_unref(&hw_device_ctx_);
        hw_device_ctx_ = nullptr;
        LOG_DEBUG("Freed CUDA hardware device context");
    }

    initialized_ = false;
    LOG_INFO("CUDA hardware decoder cleanup completed");
}

bool HardwareDecoder::initializeWithCodecParams(const AVCodecParameters* codec_params) {
    if (!codec_params) {
        LOG_ERROR("Invalid codec parameters provided");
        return false;
    }

    if (initialized_) return true;

    LOG_INFO("Initializing CUDA hardware decoder with codec parameters");
    LOG_INFO("Codec ID: " + std::to_string(codec_params->codec_id) +
             ", resolution: " + std::to_string(codec_params->width) + "x" + std::to_string(codec_params->height) +
             ", bitrate: " + std::to_string(codec_params->bit_rate));

    // 根据编解码器ID选择CUDA硬件解码器
    const AVCodec* codec = nullptr;
    if (codec_params->codec_id == AV_CODEC_ID_H264) {
        codec = avcodec_find_decoder_by_name("h264_cuvid");
        if (!codec) {
            LOG_WARNING("CUDA H.264 decoder not found, trying software decoder");
            codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        }
    } else if (codec_params->codec_id == AV_CODEC_ID_HEVC) {
        codec = avcodec_find_decoder_by_name("hevc_cuvid");
        if (!codec) {
            LOG_WARNING("CUDA HEVC decoder not found, trying software decoder");
            codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
        }
    } else {
        LOG_ERROR("Unsupported codec ID: " + std::to_string(codec_params->codec_id));
        return false;
    }

    if (!codec) {
        LOG_ERROR("No suitable decoder found for codec ID: " + std::to_string(codec_params->codec_id));
        return false;
    }
    LOG_INFO("Found decoder: " + std::string(codec->name) +
             (std::string(codec->name).find("_cuvid") != std::string::npos ? " (CUDA hardware)" : " (software)"));

    // 创建编解码器上下文
    codec_ctx_ = avcodec_alloc_context3(codec);
    if (!codec_ctx_) {
        LOG_ERROR("Failed to allocate codec context");
        return false;
    }

    // 复制编解码器参数
    int ret = avcodec_parameters_to_context(codec_ctx_, codec_params);
    if (ret < 0) {
        LOG_ERROR("Failed to copy codec parameters, error code: " + std::to_string(ret));
        return false;
    }

    // 检查是否为CUDA硬件解码器
    bool is_cuda_decoder = (std::string(codec->name).find("_cuvid") != std::string::npos);

    if (is_cuda_decoder) {
        LOG_INFO("Using CUDA hardware decoder: " + std::string(codec->name));
        // 对于CUDA硬件解码器，不需要手动设置像素格式和初始化硬件上下文
        // cuvid解码器会自动处理硬件加速
        codec_ctx_->pix_fmt = AV_PIX_FMT_CUDA;
    } else {
        LOG_INFO("Using software decoder: " + std::string(codec->name));
        // 对于软件解码器，设置软件像素格式
        codec_ctx_->pix_fmt = AV_PIX_FMT_NV12;
        codec_ctx_->sw_pix_fmt = AV_PIX_FMT_NV12;

        // 初始化硬件上下文（虽然是软件解码，但我们仍然使用硬件上下文用于数据传输）
        if (!initHWContext()) {
            LOG_ERROR("Failed to initialize hardware context for software decoder");
            return false;
        }
    }

    // 打开编解码器
    ret = avcodec_open2(codec_ctx_, codec, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to open codec, error code: " + std::to_string(ret));
        // 如果CUDA硬件解码器失败，尝试回退到软件解码器
        if (is_cuda_decoder) {
            LOG_WARNING("CUDA hardware decoder failed, trying software decoder");
            // 清理当前的编解码器上下文
            avcodec_free_context(&codec_ctx_);
            codec_ctx_ = nullptr;

            // 重新创建软件解码器
            const AVCodec* sw_codec = nullptr;
            if (codec_params->codec_id == AV_CODEC_ID_H264) {
                sw_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
            } else if (codec_params->codec_id == AV_CODEC_ID_HEVC) {
                sw_codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
            }

            if (sw_codec) {
                codec_ctx_ = avcodec_alloc_context3(sw_codec);
                if (codec_ctx_) {
                    ret = avcodec_parameters_to_context(codec_ctx_, codec_params);
                    if (ret >= 0) {
                        codec_ctx_->pix_fmt = AV_PIX_FMT_NV12;
                        if (initHWContext()) {
                            ret = avcodec_open2(codec_ctx_, sw_codec, nullptr);
                            if (ret >= 0) {
                                LOG_INFO("Successfully fell back to software decoder");
                                is_cuda_decoder = false;
                            }
                        }
                    }
                }
            }
        }

        if (ret < 0) {
            LOG_ERROR("Failed to open both CUDA hardware and software decoders");
            return false;
        }
    }

    // 分配硬件帧（用于CUDA解码）
    hw_frame_ = av_frame_alloc();
    if (!hw_frame_) {
        LOG_ERROR("Failed to allocate hardware frame");
        return false;
    }

    LOG_INFO("CUDA hardware decoder initialized successfully with codec parameters");
    initialized_ = true;
    return true;
}
