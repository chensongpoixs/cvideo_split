#pragma once

#include <cuda_runtime.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_cuda.h>
}

#include <string>
#include <memory>

struct DecodedFrame {
    uint8_t* nv12_data = nullptr;  // GPU memory pointer for CUDA decoding
    cudaIpcMemHandle_t mem_handle; // IPC memory handle for sharing GPU memory
    AVFrame* av_frame_ref = nullptr; // 引用的FFmpeg帧（用于延长GPU surface生命周期）
    int linesize_y = 0;            // CUDA NV12: Y plane pitch (bytes per row)
    int linesize_uv = 0;           // CUDA NV12: UV plane pitch (bytes per row)
    int width = 0;
    int height = 0;
    int64_t pts = 0;
    size_t data_size = 0;
    bool is_cuda_frame = false;   // true if data is in GPU memory
    bool is_nv12 = true;          // true if format is NV12, false if YUV420P
    bool owns_memory = true;      // true if this struct owns the GPU memory
};

class HardwareDecoder {
public:
    HardwareDecoder();
    ~HardwareDecoder();

    bool initialize(const std::string& codec_name = "h264");
    bool initializeWithCodecParams(const AVCodecParameters* codec_params);
    bool decodePacket(const AVPacket* packet, DecodedFrame& decoded_frame);
    void flush();

private:
    bool initHWContext();
    void cleanup();

    const AVCodec* codec_ = nullptr;
    AVCodecContext* codec_ctx_ = nullptr;
    AVBufferRef* hw_device_ctx_ = nullptr;
    AVFrame* hw_frame_ = nullptr;
    AVFrame* sw_frame_ = nullptr;

    enum AVHWDeviceType hw_type_ = AV_HWDEVICE_TYPE_CUDA;
    bool initialized_ = false;
};
