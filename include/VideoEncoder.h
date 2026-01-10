#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include "GpuMemoryManager.h"

struct EncodedPacket {
    uint8_t* data;
    size_t size;
    int64_t pts;
    int64_t dts;
    int flags;
    bool is_keyframe;
};

class VideoEncoder {
public:
    VideoEncoder();
    ~VideoEncoder();

    // 初始化编码器
    bool initialize(int width, int height, int fps, const std::string& output_url);

    // 从GPU内存编码帧
    bool encodeGpuFrame(std::shared_ptr<GpuFrame> gpu_frame, EncodedPacket& packet);

    // 刷新编码器（输出剩余帧）
    bool flush(EncodedPacket& packet);

    // 写入编码包到输出
    bool writePacket(const EncodedPacket& packet);

    // 启动编码线程（用于自动处理GPU队列）
    void start();

    // 停止编码线程
    void stop();

    // 检查是否正在运行
    bool isRunning() const { return running_.load(); }

    // 获取编码统计信息
    uint64_t getFramesEncoded() const { return frame_count_.load(); }

private:
    // 初始化内部函数
    void cleanup();

    // 编码线程函数
    void encodeThread();

    // 从GPU队列获取帧并编码
    bool processGpuQueue();

    // 创建AVFrame用于编码
    AVFrame* createFrameFromGpu(std::shared_ptr<GpuFrame> gpu_frame);

    // 直接使用GPU内存创建AVFrame（用于CUDA硬件编码器）
    AVFrame* createFrameFromGpuDirect(std::shared_ptr<GpuFrame> gpu_frame);

    AVFormatContext* format_ctx_;
    AVCodecContext* codec_ctx_;
    AVStream* stream_;
    const AVCodec* codec_;
    AVBufferRef* hw_device_ctx_;  // CUDA硬件设备上下文

    int width_;
    int height_;
    int fps_;
    std::string output_url_;

    // GPU内存管理器
    GpuMemoryManager& gpu_memory_manager_;

    // 线程管理
    std::thread encode_thread_;
    std::atomic<bool> running_;
    std::atomic<uint64_t> frame_count_;
};
