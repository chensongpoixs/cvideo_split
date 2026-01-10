#pragma once

#include "PlatformDisplay.h"

#ifdef __linux__
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct OpenGLTexture {
    GLuint texture_id = 0;
    GLuint pbo_id = 0;      // Pixel Buffer Object for GPU-CPU transfer
    int width = 0;
    int height = 0;
    bool is_nv12 = true;
};

// Linux OpenGL显示实现
class OpenGLDisplay : public PlatformDisplay {
public:
    OpenGLDisplay();
    ~OpenGLDisplay() override;

    // 初始化OpenGL显示器
    bool initialize(int window_width, int window_height, const std::string& window_title,
                   bool fullscreen = false) override;

    // 添加显示区域
    int addDisplayRegion(int x, int y, int width, int height, const std::string& title) override;

    // 更新显示区域的GPU帧
    bool updateRegionFrame(int region_id, std::shared_ptr<GpuFrame> gpu_frame) override;

    // 渲染一帧
    void render() override;

    // 处理事件
    void processEvents() override;

    // 检查窗口是否应该关闭
    bool shouldClose() const override;

    // 关闭显示窗口
    void close() override;

private:
    // GLFW回调函数
    static void glfwErrorCallback(int error, const char* description);
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    // OpenGL初始化
    bool initGL();
    bool createShaders();

    // 纹理管理
    bool createTexture(OpenGLTexture& tex, int width, int height, bool is_nv12);
    void destroyTexture(OpenGLTexture& tex);
    bool updateTextureFromGpu(OpenGLTexture& tex, std::shared_ptr<GpuFrame> gpu_frame);

    // 渲染
    void renderTexture(const OpenGLTexture& tex, int x, int y, int width, int height);
    void renderText(const std::string& text, int x, int y);

    // NV12到RGB转换着色器
    GLuint createNV12ToRGBShader();

    GLFWwindow* window_ = nullptr;

    // OpenGL资源
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLuint shader_program_ = 0;
    GLuint nv12_shader_program_ = 0;

    // 显示区域纹理
    std::vector<OpenGLTexture> region_textures_;
};