/**
 * @file VideoDisplay.h
 * @author chensong
 * @date 2026-01-11
 * @brief 视频显示器（Video Display - 历史遗留模块）
 * 
 * 该模块是早期的视频显示实现，使用 OpenGL 进行渲染。
 * 现已被 PlatformDisplay 系统（DxgiDisplay/OpenGLDisplay）替代。
 * 
 * @note 此模块为历史遗留代码，建议使用 PlatformDisplay 系统
 * @see PlatformDisplay 新的跨平台显示接口
 * @see DxgiDisplay Windows 实现（推荐）
 * @see OpenGLDisplay Linux 实现（推荐）
 * 
 * 显示架构（Display Architecture）：
 * 
 *   ┌─────────────────────────────────────────────────────────────────┐
 *   │                      VideoDisplay (Legacy)                      │
 *   ├─────────────────────────────────────────────────────────────────┤
 *   │  GPU Frame → cudaMemcpy → CPU NV12 → CPU RGB → glTexImage2D    │
 *   │                                                                 │
 *   │  注意: 此路径涉及 GPU→CPU 拷贝，性能不如零拷贝实现              │
 *   └─────────────────────────────────────────────────────────────────┘
 */

#pragma once

#include "GpuMemoryManager.h"
#include "Config.h"

#ifdef HAVE_OPENGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#include <thread>
#include <atomic>
#include <string>
#include <memory>
#include <vector>
#include <mutex>

/**
 * @struct DisplayFrame
 * @brief 待显示的帧信息
 */
struct DisplayFrame {
    std::shared_ptr<GpuFrame> gpu_frame;  ///< GPU 帧指针
    int x, y;                              ///< 显示位置
    int display_width, display_height;     ///< 显示尺寸
    std::string label;                     ///< 帧标签（可选）
};

/**
 * @class VideoDisplay
 * @brief 视频显示器（历史遗留实现）
 * 
 * @deprecated 建议使用 PlatformDisplay 系统替代
 * 
 * 使用示例：
 * @code
 * VideoDisplay display("Video Window");
 * display.initialize(1920, 1080);
 * display.addDisplayFrame(gpu_frame, 0, 0, 960, 540, "Stream 0");
 * display.renderAndDisplay();
 * @endcode
 */
class VideoDisplay {
public:
    /**
     * @brief 构造函数
     * @param window_title 窗口标题
     */
    VideoDisplay(const std::string& window_title);

    /**
     * @brief 析构函数
     */
    ~VideoDisplay();

    /**
     * @brief 初始化显示器
     * @param window_width 窗口宽度
     * @param window_height 窗口高度
     * @param fullscreen 是否全屏
     * @return true 成功
     * @return false 失败
     */
    bool initialize(int window_width, int window_height, bool fullscreen = false);

    /**
     * @brief 添加要显示的帧
     * @param gpu_frame GPU 帧
     * @param x 显示 X 坐标
     * @param y 显示 Y 坐标
     * @param display_width 显示宽度
     * @param display_height 显示高度
     * @param label 帧标签
     */
    void addDisplayFrame(std::shared_ptr<GpuFrame> gpu_frame, int x, int y,
                        int display_width, int display_height, const std::string& label = "");

    /**
     * @brief 渲染并显示所有帧
     * @return true 成功
     * @return false 失败
     */
    bool renderAndDisplay();

    /**
     * @brief 清除所有显示帧
     */
    void clearDisplayFrames();

    /**
     * @brief 关闭显示窗口
     */
    void close();

    /**
     * @brief 检查窗口是否打开
     * @return true 窗口打开
     * @return false 窗口已关闭
     */
    bool isOpen() const;

    /**
     * @brief 设置显示帧率
     * @param fps 目标帧率
     */
    void setDisplayFPS(int fps) { display_fps_ = fps; }

    /**
     * @brief 获取窗口大小
     * @param width 输出宽度
     * @param height 输出高度
     */
    void getWindowSize(int& width, int& height) const {
        width = window_width_;
        height = window_height_;
    }

private:
    std::string window_title_;   ///< 窗口标题
    int window_width_;           ///< 窗口宽度
    int window_height_;          ///< 窗口高度
    int display_fps_;            ///< 目标帧率
    bool initialized_;           ///< 初始化状态
    bool fullscreen_;            ///< 全屏模式

#ifdef HAVE_OPENGL
    GLFWwindow* window_;                      ///< GLFW 窗口
    std::vector<GLuint> textures_;            ///< OpenGL 纹理列表
    std::vector<DisplayFrame> display_frames_; ///< 待显示帧列表
    std::mutex frames_mutex_;                  ///< 帧列表互斥锁

    /**
     * @brief 初始化 OpenGL
     * @return true 成功
     */
    bool initOpenGL();

    /**
     * @brief 创建 OpenGL 纹理
     * @param width 纹理宽度
     * @param height 纹理高度
     * @return GLuint 纹理 ID
     */
    GLuint createTexture(int width, int height);

    /**
     * @brief 更新纹理数据
     * @param texture 纹理 ID
     * @param gpu_frame GPU 帧
     * @return true 成功
     */
    bool updateTexture(GLuint texture, std::shared_ptr<GpuFrame> gpu_frame);

    /**
     * @brief 渲染场景
     */
    void renderScene();

    /**
     * @brief NV12 到 RGB 转换（CPU）
     * @param nv12_data NV12 数据
     * @param rgb_data RGB 输出
     * @param width 宽度
     * @param height 高度
     * @return true 成功
     */
    bool nv12ToRgbCpu(const uint8_t* nv12_data, std::vector<uint8_t>& rgb_data, int width, int height);

    /**
     * @brief 绘制纹理
     * @param texture 纹理 ID
     * @param x X 坐标
     * @param y Y 坐标
     * @param width 宽度
     * @param height 高度
     */
    void drawTexture(GLuint texture, int x, int y, int width, int height);

    /**
     * @brief 绘制标签文本
     * @param x X 坐标
     * @param y Y 坐标
     * @param text 文本内容
     */
    void drawLabel(int x, int y, const std::string& text);
#endif

    std::thread display_thread_;  ///< 显示线程
    std::atomic<bool> running_;   ///< 运行状态
};
