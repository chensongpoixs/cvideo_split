#pragma once

#include "VideoStreamPuller.h"
#include "HardwareDecoder.h"
#include "GpuMemoryManager.h"
#include "PlatformDisplay.h"
#include <thread>
#include <atomic>
#include <memory>

class VideoDecoder {
public:
    VideoDecoder(int stream_id);
    ~VideoDecoder();

    // 初始化解码器
    bool initialize(const std::string& rtsp_url, PlatformDisplay* display = nullptr);

    // 启动解码线程
    void start();

    // 停止解码线程
    void stop();

    // 获取流ID
    int getStreamId() const { return stream_id_; }

    // 检查是否正在运行
    bool isRunning() const { return running_.load(); }

    // 设置平台显示器
    void setPlatformDisplay(PlatformDisplay* display) { platform_display_ = display; }

    // 设置解码器显示区域ID
    void setDecoderRegionId(int region_id) { decoder_region_id_ = region_id; }

    // 统计信息
    uint64_t getFramesDecoded() const { return frames_decoded_.load(); }
    uint64_t getPacketsProcessed() const { return packets_processed_.load(); }

private:
    // 解码线程函数
    void decodeThread();

    // 处理单个视频包
    bool processPacket(AVPacket* packet);

    int stream_id_;
    std::string rtsp_url_;

    // 组件
    std::unique_ptr<VideoStreamPuller> stream_puller_;
    std::unique_ptr<HardwareDecoder> hardware_decoder_;

    // GPU内存管理器
    GpuMemoryManager& gpu_memory_manager_;

    // 线程管理
    std::thread decode_thread_;
    std::atomic<bool> running_;

    // 平台显示器
    PlatformDisplay* platform_display_;
    int decoder_region_id_;  // 解码器显示区域ID

    // 统计信息
    std::atomic<uint64_t> frames_decoded_;
    std::atomic<uint64_t> packets_processed_;
};
