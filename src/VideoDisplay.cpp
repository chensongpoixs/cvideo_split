/**
 * @file VideoDisplay.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief 视频显示器实现（历史遗留模块）
 * @see VideoDisplay.h
 * @deprecated 建议使用 PlatformDisplay 系统替代
 */

#include "VideoDisplay.h"
#include "Logger.h"

#ifdef HAVE_OPENGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

VideoDisplay::VideoDisplay(const std::string& window_title)
    : window_title_(window_title)
    , window_width_(0)
    , window_height_(0)
    , display_fps_(30)
    , initialized_(false)
    , fullscreen_(false)
    , running_(false)
#ifdef HAVE_OPENGL
    , window_(nullptr)
#endif
{
}

VideoDisplay::~VideoDisplay() {
    close();
}

bool VideoDisplay::initialize(int window_width, int window_height, bool fullscreen) {
#ifdef HAVE_OPENGL
    window_width_ = window_width;
    window_height_ = window_height;
    fullscreen_ = fullscreen;

    // 初始化GLFW
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    // 设置GLFW窗口提示
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // 创建窗口
    if (fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        window_ = glfwCreateWindow(mode->width, mode->height, window_title_.c_str(), monitor, nullptr);
        window_width_ = mode->width;
        window_height_ = mode->height;
    } else {
        window_ = glfwCreateWindow(window_width, window_height, window_title_.c_str(), nullptr, nullptr);
    }

    if (!window_) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    // 设置窗口为当前上下文
    glfwMakeContextCurrent(window_);

    // 设置键盘回调
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            VideoDisplay* display = static_cast<VideoDisplay*>(glfwGetWindowUserPointer(window));
            display->close();
        }
    });

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW");
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }

    // 设置OpenGL状态
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);

    initialized_ = true;
    LOG_INFO("Video display initialized with OpenGL: " + window_title_ + " (" +
             std::to_string(window_width_) + "x" + std::to_string(window_height_) +
             (fullscreen ? ", fullscreen" : "") + ")");

    return true;
#else
    LOG_WARNING("OpenGL not available - video display disabled");
    return false;
#endif
}

void VideoDisplay::addDisplayFrame(std::shared_ptr<GpuFrame> gpu_frame, int x, int y,
                                  int display_width, int display_height, const std::string& label) {
#ifdef HAVE_OPENGL
    std::lock_guard<std::mutex> lock(frames_mutex_);
    DisplayFrame frame;
    frame.gpu_frame = gpu_frame;
    frame.x = x;
    frame.y = y;
    frame.display_width = display_width;
    frame.display_height = display_height;
    frame.label = label;
    display_frames_.push_back(frame);
#endif
}

bool VideoDisplay::renderAndDisplay() {
#ifdef HAVE_OPENGL
    if (!initialized_ || !window_) {
        return false;
    }

    // 检查窗口是否应该关闭
    if (glfwWindowShouldClose(window_)) {
        close();
        return false;
    }

    // 设置当前上下文
    glfwMakeContextCurrent(window_);

    // 渲染场景
    renderScene();

    // 交换缓冲区
    glfwSwapBuffers(window_);

    // 处理事件
    glfwPollEvents();

    return true;
#else
    return false;
#endif
}

void VideoDisplay::clearDisplayFrames() {
#ifdef HAVE_OPENGL
    std::lock_guard<std::mutex> lock(frames_mutex_);
    display_frames_.clear();
#endif
}

void VideoDisplay::close() {
    if (!initialized_) return;

#ifdef HAVE_OPENGL
    if (window_) {
        // 清理纹理
        for (GLuint texture : textures_) {
            glDeleteTextures(1, &texture);
        }
        textures_.clear();

        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
#endif

    initialized_ = false;
    LOG_INFO("Video display closed: " + window_title_);
}

bool VideoDisplay::isOpen() const {
#ifdef HAVE_OPENGL
    return initialized_ && window_ && !glfwWindowShouldClose(window_);
#else
    return false;
#endif
}

#ifdef HAVE_OPENGL
bool VideoDisplay::initOpenGL() {
    // OpenGL初始化已在initialize()中完成
    return true;
}

GLuint VideoDisplay::createTexture(int width, int height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 分配纹理内存 (RGB格式)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    textures_.push_back(texture);
    return texture;
}

bool VideoDisplay::updateTexture(GLuint texture, std::shared_ptr<GpuFrame> gpu_frame) {
    if (!gpu_frame || !gpu_frame->gpu_ptr) {
        return false;
    }

    // 从GPU复制数据到CPU，然后转换为RGB
    // 不要写死 width*height*3/2，优先使用gpu_frame->size（上游可能有对齐/打包差异）
    size_t nv12_size = gpu_frame->size;
    const size_t expected = static_cast<size_t>(gpu_frame->width) * gpu_frame->height * 3 / 2;
    if (nv12_size > expected) {
        nv12_size = expected;
    }
    std::vector<uint8_t> nv12_data(nv12_size);

    if (!GpuMemoryManager::getInstance().copyFromGpu(gpu_frame, nv12_data.data(), nv12_size)) {
        LOG_ERROR("Failed to copy frame data from GPU for display");
        return false;
    }

    // 转换为RGB
    std::vector<uint8_t> rgb_data;
    if (!nv12ToRgbCpu(nv12_data.data(), rgb_data, gpu_frame->width, gpu_frame->height)) {
        LOG_ERROR("Failed to convert NV12 to RGB for display");
        return false;
    }

    // 更新纹理
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gpu_frame->width, gpu_frame->height,
                    GL_RGB, GL_UNSIGNED_BYTE, rgb_data.data());

    return true;
}

void VideoDisplay::renderScene() {
    // 清除屏幕
    glClear(GL_COLOR_BUFFER_BIT);

    // 设置正交投影
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width_, window_height_, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 启用纹理
    glEnable(GL_TEXTURE_2D);

    std::lock_guard<std::mutex> lock(frames_mutex_);

    // 渲染每个显示帧
    for (size_t i = 0; i < display_frames_.size(); ++i) {
        const DisplayFrame& frame = display_frames_[i];

        if (!frame.gpu_frame) continue;

        // 确保纹理存在
        if (i >= textures_.size()) {
            createTexture(frame.gpu_frame->width, frame.gpu_frame->height);
        }

        // 更新纹理数据
        if (updateTexture(textures_[i], frame.gpu_frame)) {
            // 绘制纹理
            drawTexture(textures_[i], frame.x, frame.y, frame.display_width, frame.display_height);

            // 绘制标签
            if (!frame.label.empty()) {
                drawLabel(frame.x, frame.y - 20, frame.label);
            }
        }
    }

    glDisable(GL_TEXTURE_2D);
}

bool VideoDisplay::nv12ToRgbCpu(const uint8_t* nv12_data, std::vector<uint8_t>& rgb_data, int width, int height) {
    // 简单的NV12到RGB转换 (可以优化为GPU版本)
    rgb_data.resize(width * height * 3);

    const uint8_t* y_plane = nv12_data;
    const uint8_t* uv_plane = nv12_data + width * height;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int y_idx = y * width + x;
            int uv_idx = (y / 2) * width + (x / 2) * 2;

            int y_val = y_plane[y_idx];
            int u_val = uv_plane[uv_idx];
            int v_val = uv_plane[uv_idx + 1];

            // YUV到RGB转换
            int r = y_val + 1.402 * (v_val - 128);
            int g = y_val - 0.344 * (u_val - 128) - 0.714 * (v_val - 128);
            int b = y_val + 1.772 * (u_val - 128);

            // 裁剪到0-255
            r = std::max(0, std::min(255, r));
            g = std::max(0, std::min(255, g));
            b = std::max(0, std::min(255, b));

            int rgb_idx = (y * width + x) * 3;
            rgb_data[rgb_idx] = r;
            rgb_data[rgb_idx + 1] = g;
            rgb_data[rgb_idx + 2] = b;
        }
    }

    return true;
}

void VideoDisplay::drawTexture(GLuint texture, int x, int y, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + height);
    glEnd();
}

void VideoDisplay::drawLabel(int x, int y, const std::string& text) {
    // 简单的文本渲染 (可以扩展为更完整的字体渲染)
    glColor3f(1.0f, 1.0f, 1.0f);  // 白色
    glRasterPos2f(x, y);

    // 这里可以添加实际的字体渲染逻辑
    // 暂时只显示一个简单的矩形背景
    glColor3f(0.0f, 0.0f, 0.0f);  // 黑色背景
    glBegin(GL_QUADS);
    glVertex2f(x - 5, y - 15);
    glVertex2f(x + text.length() * 8 + 5, y - 15);
    glVertex2f(x + text.length() * 8 + 5, y + 5);
    glVertex2f(x - 5, y + 5);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);  // 白色文字
    // 简单的字符渲染 (可以扩展)
    for (size_t i = 0; i < text.length(); ++i) {
        // 这里应该调用实际的字符渲染函数
        // 暂时跳过
    }
}
#endif