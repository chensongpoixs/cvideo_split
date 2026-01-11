/**
 * @file PlatformDisplay.h
 * @author chensong
 * @date 2026-01-11
 * @brief 跨平台视频显示接口（Cross-Platform Video Display Interface）
 * 
 * 该模块定义统一的视频显示抽象接口，支持 Windows (DXGI/DirectX11) 和 Linux (OpenGL)
 * 两种平台实现。通过工厂模式根据编译平台自动选择正确的实现。
 * 
 * 平台显示架构（Platform Display Architecture）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    PlatformDisplay (抽象基类)                 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   ┌─────────────────────┐    ┌─────────────────────┐         |
 *  |   │    DxgiDisplay      │    │   OpenGLDisplay     │         |
 *  |   │   (Windows/D3D11)   │    │    (Linux/GL)       │         |
 *  |   ├─────────────────────┤    ├─────────────────────┤         |
 *  |   │ • CUDA-D3D11 interop│    │ • CUDA-GL interop   │         |
 *  |   │ • 零拷贝 GPU→纹理   │    │ • PBO 异步传输      │         |
 *  |   │ • NV12→RGBA 转换    │    │ • NV12→RGBA 转换    │         |
 *  |   └─────────────────────┘    └─────────────────────┘         |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 显示区域布局（Display Region Layout）：
 * 
 *  ┌───────────────────────────────────────────────────────────────┐
 *  │                         显示窗口                              │
 *  │  ┌─────────────┬─────────────┬─────────────────────────────┐ │
 *  │  │  Region 0   │  Region 1   │                             │ │
 *  │  │ (解码流 0)  │ (解码流 1)  │                             │ │
 *  │  ├─────────────┼─────────────┤       Region 4              │ │
 *  │  │  Region 2   │  Region 3   │      (拼接输出)             │ │
 *  │  │ (解码流 2)  │ (解码流 3)  │                             │ │
 *  │  └─────────────┴─────────────┴─────────────────────────────┘ │
 *  │     解码区域 (2x2)                    拼接区域               │
 *  └───────────────────────────────────────────────────────────────┘
 * 
 * 渲染流程（Rendering Flow）：
 * 
 *   updateRegionFrame()    render()           processEvents()
 *        │                    │                     │
 *        ▼                    ▼                     ▼
 *   ┌─────────┐         ┌─────────┐           ┌─────────┐
 *   │ 更新帧  │         │ GPU转换 │           │ 窗口事件│
 *   │ 指针    │ ──────► │ 绘制纹理│ ────────► │ 处理    │
 *   │ (异步)  │         │ Present │           │ (WM_*)  │
 *   └─────────┘         └─────────┘           └─────────┘
 * 
 * @note 使用 createPlatformDisplay() 工厂函数创建平台相关实例
 * @see DxgiDisplay (Windows 实现)
 * @see OpenGLDisplay (Linux 实现)
 */

#pragma once

#include "GpuMemoryManager.h"
#include "Config.h"

#include <string>
#include <memory>
#include <vector>
#include <mutex>

/**
 * @class PlatformDisplay
 * @brief 跨平台视频显示抽象基类
 * 
 * 定义视频显示的统一接口，包括：
 * - 窗口初始化和管理
 * - 显示区域划分
 * - GPU 帧更新和渲染
 * - 事件处理
 * 
 * 子类需要实现所有纯虚函数以适配特定平台。
 */
class PlatformDisplay {
public:
    /**
     * @brief 构造函数
     */
    PlatformDisplay();

    /**
     * @brief 虚析构函数
     */
    virtual ~PlatformDisplay();

    /**
     * @brief 初始化显示器
     * 
     * 创建窗口、初始化图形 API、设置渲染管线。
     * 
     * @param window_width 窗口宽度（像素）
     * @param window_height 窗口高度（像素）
     * @param window_title 窗口标题
     * @param fullscreen 是否全屏模式
     * @return true 初始化成功
     * @return false 初始化失败
     */
    virtual bool initialize(int window_width, int window_height, const std::string& window_title,
                           bool fullscreen = false) = 0;

    /**
     * @brief 添加显示区域
     * 
     * 在窗口中划分一个矩形区域用于显示视频帧。
     * 
     * @param x 区域左上角 X 坐标
     * @param y 区域左上角 Y 坐标
     * @param width 区域宽度
     * @param height 区域高度
     * @param title 区域标题（用于日志和调试）
     * @return int 区域 ID（>=0 成功，<0 失败）
     */
    virtual int addDisplayRegion(int x, int y, int width, int height, const std::string& title) = 0;

    /**
     * @brief 更新显示区域的 GPU 帧
     * 
     * 异步更新：仅交换帧指针，实际 GPU 操作在 render() 中执行。
     * 
     * @param region_id 区域 ID
     * @param gpu_frame GPU 帧指针
     * @return true 更新成功
     * @return false 区域 ID 无效或帧为空
     * 
     * @note 线程安全，可从任意线程调用
     * @note 帧数据必须是 NV12 格式且在 GPU 内存中
     */
    virtual bool updateRegionFrame(int region_id, std::shared_ptr<GpuFrame> gpu_frame) = 0;

    /**
     * @brief 渲染一帧
     * 
     * 执行以下操作：
     * 1. 检查窗口尺寸变化，必要时调整缓冲区
     * 2. 清除渲染目标
     * 3. 对每个区域执行 NV12→RGBA 转换（GPU）
     * 4. 绘制纹理到对应位置
     * 5. 绘制边框和分隔线
     * 6. 交换缓冲区（Present）
     * 
     * @note 必须在主线程调用
     * @note 帧率由 VSync 或 setDisplayFPS() 控制
     */
    virtual void render() = 0;

    /**
     * @brief 处理窗口事件
     * 
     * 处理操作系统窗口消息（如关闭、调整大小等）。
     * 
     * @note 必须在主线程调用
     */
    virtual void processEvents() = 0;

    /**
     * @brief 检查窗口是否应该关闭
     * 
     * @return true 收到关闭请求
     * @return false 窗口正常运行
     */
    virtual bool shouldClose() const = 0;

    /**
     * @brief 关闭显示窗口
     * 
     * 释放所有图形资源和窗口句柄。
     */
    virtual void close() = 0;

    /**
     * @brief 设置目标显示帧率
     * 
     * @param fps 目标帧率
     * @note 实际帧率可能受 VSync 限制
     */
    void setDisplayFPS(int fps) { target_fps_ = fps; }

    /**
     * @brief 检查是否已初始化
     * 
     * @return true 已初始化
     * @return false 未初始化
     */
    bool isInitialized() const { return initialized_; }

protected:
    bool initialized_ = false;      ///< 初始化状态
    int window_width_ = 0;          ///< 窗口宽度
    int window_height_ = 0;         ///< 窗口高度
    std::string window_title_;      ///< 窗口标题
    int target_fps_ = 30;           ///< 目标帧率

    /**
     * @struct DisplayRegion
     * @brief 显示区域描述
     */
    struct DisplayRegion {
        int id;                              ///< 区域唯一 ID
        int x, y;                            ///< 左上角坐标
        int width, height;                   ///< 区域尺寸
        std::string title;                   ///< 区域标题
        std::shared_ptr<GpuFrame> current_frame; ///< 当前显示帧
    };

    std::vector<DisplayRegion> regions_;  ///< 所有显示区域
    int next_region_id_ = 0;              ///< 下一个区域 ID
    std::mutex frame_mutex_;               ///< 帧更新互斥锁
};

/**
 * @brief 创建平台相关的显示实现
 * 
 * 工厂函数，根据编译平台返回正确的实现：
 * - Windows: DxgiDisplay (DirectX 11)
 * - Linux: OpenGLDisplay (OpenGL + GLFW)
 * 
 * @return std::unique_ptr<PlatformDisplay> 显示实例
 * 
 * 使用示例：
 * @code
 * auto display = createPlatformDisplay();
 * display->initialize(1920, 1080, "Video Display");
 * int region = display->addDisplayRegion(0, 0, 960, 540, "Stream 0");
 * // ... 主循环 ...
 * display->render();
 * display->processEvents();
 * @endcode
 */
std::unique_ptr<PlatformDisplay> createPlatformDisplay();
