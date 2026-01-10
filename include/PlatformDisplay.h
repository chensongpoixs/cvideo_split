#pragma once

#include "GpuMemoryManager.h"
#include "Config.h"

#include <string>
#include <memory>
#include <vector>
#include <mutex>

// 跨平台显示接口
class PlatformDisplay {
public:
    PlatformDisplay();
    virtual ~PlatformDisplay();

    // 初始化显示器
    virtual bool initialize(int window_width, int window_height, const std::string& window_title,
                           bool fullscreen = false) = 0;

    // 添加显示区域
    virtual int addDisplayRegion(int x, int y, int width, int height, const std::string& title) = 0;

    // 更新显示区域的GPU帧
    virtual bool updateRegionFrame(int region_id, std::shared_ptr<GpuFrame> gpu_frame) = 0;

    // 渲染一帧
    virtual void render() = 0;

    // 处理事件
    virtual void processEvents() = 0;

    // 检查窗口是否应该关闭
    virtual bool shouldClose() const = 0;

    // 关闭显示窗口
    virtual void close() = 0;

    // 设置显示FPS
    void setDisplayFPS(int fps) { target_fps_ = fps; }

    // 检查是否已初始化
    bool isInitialized() const { return initialized_; }

protected:
    bool initialized_ = false;
    int window_width_ = 0;
    int window_height_ = 0;
    std::string window_title_;
    int target_fps_ = 30;

    // 显示区域
    struct DisplayRegion {
        int id;
        int x, y;
        int width, height;
        std::string title;
        std::shared_ptr<GpuFrame> current_frame;
    };
    std::vector<DisplayRegion> regions_;
    int next_region_id_ = 0;
    std::mutex frame_mutex_;
};

// 工厂函数：根据平台创建相应的显示实现
std::unique_ptr<PlatformDisplay> createPlatformDisplay();

