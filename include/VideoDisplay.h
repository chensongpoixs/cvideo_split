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

struct DisplayFrame {
    std::shared_ptr<GpuFrame> gpu_frame;
    int x, y;  // 显示位置
    int display_width, display_height;  // 显示尺寸
    std::string label;
};

class VideoDisplay {
public:
    VideoDisplay(const std::string& window_title);
    ~VideoDisplay();

    // 初始化显示器
    bool initialize(int window_width, int window_height, bool fullscreen = false);

    // 添加要显示的帧
    void addDisplayFrame(std::shared_ptr<GpuFrame> gpu_frame, int x, int y,
                        int display_width, int display_height, const std::string& label = "");

    // 渲染并显示所有帧
    bool renderAndDisplay();

    // 清除所有显示帧
    void clearDisplayFrames();

    // 关闭显示窗口
    void close();

    // 检查窗口是否打开
    bool isOpen() const;

    // 设置显示FPS
    void setDisplayFPS(int fps) { display_fps_ = fps; }

    // 获取窗口大小
    void getWindowSize(int& width, int& height) const {
        width = window_width_;
        height = window_height_;
    }

private:
    std::string window_title_;
    int window_width_;
    int window_height_;
    int display_fps_;
    bool initialized_;
    bool fullscreen_;

#ifdef HAVE_OPENGL
    GLFWwindow* window_;
    std::vector<GLuint> textures_;  // OpenGL纹理
    std::vector<DisplayFrame> display_frames_;
    std::mutex frames_mutex_;

    // OpenGL初始化
    bool initOpenGL();

    // 创建纹理
    GLuint createTexture(int width, int height);

    // 更新纹理数据
    bool updateTexture(GLuint texture, std::shared_ptr<GpuFrame> gpu_frame);

    // 渲染场景
    void renderScene();

    // NV12到RGB转换（在CPU上进行简单转换）
    bool nv12ToRgbCpu(const uint8_t* nv12_data, std::vector<uint8_t>& rgb_data, int width, int height);

    // 绘制纹理
    void drawTexture(GLuint texture, int x, int y, int width, int height);

    // 绘制标签
    void drawLabel(int x, int y, const std::string& text);
#endif

    // 显示线程
    std::thread display_thread_;
    std::atomic<bool> running_;
};