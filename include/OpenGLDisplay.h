/**
 * @file OpenGLDisplay.h
 * @author chensong
 * @date 2026-01-11
 * @brief Linux OpenGL 视频显示实现（Linux OpenGL Video Display）
 * 
 * 该模块是 PlatformDisplay 接口的 Linux 实现，使用 OpenGL 和 GLFW 进行渲染。
 * 支持 NV12 到 RGB 的 GPU 着色器转换。
 * 
 * OpenGL 显示架构（OpenGL Display Architecture）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      OpenGLDisplay                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   GpuFrame (NV12, GPU)                                        |
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │ PBO (Pixel Buffer Object) - 异步传输                    │|
 *  |   │  cudaMemcpy → PBO → glTexSubImage2D (流水线化)          │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │ NV12 to RGB Fragment Shader                             │|
 *  |   │  Y  纹理 + UV 纹理 → RGB 输出                           │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │ Framebuffer → glfwSwapBuffers()                         │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * @note 仅在 __linux__ 平台编译
 * @note 需要 GLFW 和 GLEW 库
 * @see PlatformDisplay 抽象基类
 * @see DxgiDisplay Windows 实现
 */

#pragma once

#include "PlatformDisplay.h"

#ifdef __linux__
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/**
 * @struct OpenGLTexture
 * @brief OpenGL 纹理资源结构
 */
struct OpenGLTexture {
    GLuint texture_id = 0;    ///< OpenGL 纹理 ID
    GLuint pbo_id = 0;        ///< PBO ID（异步 GPU→CPU 传输）
    int width = 0;            ///< 纹理宽度
    int height = 0;           ///< 纹理高度
    bool is_nv12 = true;      ///< 是否 NV12 格式
};

/**
 * @class OpenGLDisplay
 * @brief Linux OpenGL 视频显示实现
 * 
 * 使用 OpenGL 和 GLFW 进行高性能视频渲染，支持：
 * - PBO 异步数据传输
 * - NV12 到 RGB 着色器转换
 * - 多区域布局
 * 
 * 使用示例：
 * @code
 * OpenGLDisplay display;
 * display.initialize(1920, 1080, "Video Display");
 * int region = display.addDisplayRegion(0, 0, 960, 540, "Stream 0");
 * 
 * while (!display.shouldClose()) {
 *     display.updateRegionFrame(region, gpu_frame);
 *     display.render();
 *     display.processEvents();
 * }
 * @endcode
 */
class OpenGLDisplay : public PlatformDisplay {
public:
    /**
     * @brief 构造函数
     */
    OpenGLDisplay();

    /**
     * @brief 析构函数
     */
    ~OpenGLDisplay() override;

    /**
     * @brief 初始化 OpenGL 显示器
     * 
     * 创建 GLFW 窗口，初始化 OpenGL 上下文和着色器。
     * 
     * @param window_width 窗口宽度
     * @param window_height 窗口高度
     * @param window_title 窗口标题
     * @param fullscreen 是否全屏
     * @return true 成功
     * @return false 失败
     */
    bool initialize(int window_width, int window_height, const std::string& window_title,
                   bool fullscreen = false) override;

    /**
     * @brief 添加显示区域
     * @see PlatformDisplay::addDisplayRegion
     */
    int addDisplayRegion(int x, int y, int width, int height, const std::string& title) override;

    /**
     * @brief 更新区域帧
     * @see PlatformDisplay::updateRegionFrame
     */
    bool updateRegionFrame(int region_id, std::shared_ptr<GpuFrame> gpu_frame) override;

    /**
     * @brief 渲染一帧
     */
    void render() override;

    /**
     * @brief 处理事件
     * 
     * 调用 glfwPollEvents() 处理窗口事件。
     */
    void processEvents() override;

    /**
     * @brief 检查是否应关闭
     * @return true 收到关闭请求
     */
    bool shouldClose() const override;

    /**
     * @brief 关闭显示器
     */
    void close() override;

private:
    /**
     * @brief GLFW 错误回调
     */
    static void glfwErrorCallback(int error, const char* description);

    /**
     * @brief GLFW 键盘回调
     */
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * @brief 初始化 OpenGL
     * @return true 成功
     */
    bool initGL();

    /**
     * @brief 创建着色器程序
     * @return true 成功
     */
    bool createShaders();

    /**
     * @brief 创建纹理
     * @param tex 纹理结构
     * @param width 宽度
     * @param height 高度
     * @param is_nv12 是否 NV12
     * @return true 成功
     */
    bool createTexture(OpenGLTexture& tex, int width, int height, bool is_nv12);

    /**
     * @brief 销毁纹理
     * @param tex 纹理结构
     */
    void destroyTexture(OpenGLTexture& tex);

    /**
     * @brief 从 GPU 帧更新纹理
     * @param tex 纹理结构
     * @param gpu_frame GPU 帧
     * @return true 成功
     */
    bool updateTextureFromGpu(OpenGLTexture& tex, std::shared_ptr<GpuFrame> gpu_frame);

    /**
     * @brief 渲染单个纹理
     * @param tex 纹理
     * @param x X 坐标
     * @param y Y 坐标
     * @param width 宽度
     * @param height 高度
     */
    void renderTexture(const OpenGLTexture& tex, int x, int y, int width, int height);

    /**
     * @brief 渲染文本
     * @param text 文本内容
     * @param x X 坐标
     * @param y Y 坐标
     */
    void renderText(const std::string& text, int x, int y);

    /**
     * @brief 创建 NV12 到 RGB 着色器
     * @return GLuint 着色器程序 ID
     */
    GLuint createNV12ToRGBShader();

    GLFWwindow* window_ = nullptr;  ///< GLFW 窗口

    // OpenGL 资源
    GLuint vertex_array_ = 0;       ///< VAO
    GLuint vertex_buffer_ = 0;      ///< VBO
    GLuint shader_program_ = 0;     ///< 基础着色器程序
    GLuint nv12_shader_program_ = 0; ///< NV12→RGB 着色器程序

    /// 区域纹理数组
    std::vector<OpenGLTexture> region_textures_;
};
#endif // __linux__
