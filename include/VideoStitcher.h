#pragma once

#include "GpuMemoryManager.h"
#include "CudaVideoStitcher.h"
#include "OSDOverlay.h"
#include "PlatformDisplay.h"
#include <thread>
#include <atomic>
#include <vector>
#include <memory>

class VideoStitcher {
public:
    VideoStitcher();
    ~VideoStitcher();

    // 初始化拼接器
    bool initialize(int num_streams, int output_width, int output_height, PlatformDisplay* display = nullptr);

    // 启动拼接线程
    void start();

    // 停止拼接线程
    void stop();

    // 检查是否正在运行
    bool isRunning() const { return running_.load(); }

    // 获取拼接统计信息
    uint64_t getFramesProcessed() const { return frames_processed_.load(); }

    // 设置拼接器显示区域ID
    void setStitcherRegionId(int region_id) { stitcher_region_id_ = region_id; }

    // 设置平台显示器
    void setPlatformDisplay(PlatformDisplay* display) { platform_display_ = display; }

private:
    // 拼接线程函数
    void stitchThread();

    // 执行拼接处理
    bool processStitching();

    // 配置拼接布局
    void setupStitchLayout();

    int num_streams_;
    int output_width_;
    int output_height_;

    // 组件
    std::unique_ptr<CudaVideoStitcher> cuda_stitcher_;
    std::unique_ptr<OSDOverlay> osd_overlay_;
    GpuMemoryManager& gpu_memory_manager_;

    // 平台显示器
    PlatformDisplay* platform_display_;
    int stitcher_region_id_;  // 拼接器显示区域ID

    // 拼接配置
    StitchConfig stitch_config_;

    // 线程管理
    std::thread stitch_thread_;
    std::atomic<bool> running_;

    // 统计信息
    std::atomic<uint64_t> frames_processed_;

    // OSD配置
    std::string osd_text_;
    int64_t last_osd_update_;
};
