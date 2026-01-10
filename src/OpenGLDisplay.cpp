#ifdef __linux__

#include "OpenGLDisplay.h"
#include "Logger.h"

#include <iostream>
#include <chrono>

static const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

static const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, TexCoord);
}
)";

static const char* nv12_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

static const char* nv12_fragment_shader = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D yTexture;
uniform sampler2D uvTexture;

void main() {
    float y = texture(yTexture, TexCoord).r;
    vec2 uv = texture(uvTexture, TexCoord).rg;

    // NV12 to RGB conversion
    float u = uv.r - 0.5;
    float v = uv.g - 0.5;

    float r = y + 1.402 * v;
    float g = y - 0.344 * u - 0.714 * v;
    float b = y + 1.772 * u;

    FragColor = vec4(r, g, b, 1.0);
}
)";

OpenGLDisplay::OpenGLDisplay() = default;

OpenGLDisplay::~OpenGLDisplay() {
    close();
}

bool OpenGLDisplay::initialize(int window_width, int window_height, const std::string& window_title,
                              bool fullscreen) {
    PlatformDisplay::window_width_ = window_width;
    PlatformDisplay::window_height_ = window_height;
    PlatformDisplay::window_title_ = window_title;

    // 初始化GLFW
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    glfwSetErrorCallback(glfwErrorCallback);

    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // 创建窗口
    GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    window_ = glfwCreateWindow(window_width, window_height, window_title.c_str(),
                              monitor, nullptr);
    if (!window_) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSetKeyCallback(window_, glfwKeyCallback);

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW");
        close();
        return false;
    }

    // 设置OpenGL状态
    glViewport(0, 0, window_width, window_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // 初始化着色器
    if (!createShaders()) {
        LOG_ERROR("Failed to create shaders");
        close();
        return false;
    }

    PlatformDisplay::initialized_ = true;
    LOG_INFO("OpenGL display initialized: " + window_title + " (" +
             std::to_string(window_width) + "x" + std::to_string(window_height) + ")");

    return true;
}

int OpenGLDisplay::addDisplayRegion(int x, int y, int width, int height, const std::string& title) {
    if (!PlatformDisplay::initialized_) return -1;

    DisplayRegion region;
    region.id = PlatformDisplay::next_region_id_++;
    region.x = x;
    region.y = y;
    region.width = width;
    region.height = height;
    region.title = title;

    // 创建纹理
    if (!createTexture(region.texture, width, height, true)) {
        LOG_ERROR("Failed to create texture for region: " + title);
        return -1;
    }

    // 创建纹理并存储
    OpenGLTexture tex;
    if (!createTexture(tex, width, height, true)) {
        LOG_ERROR("Failed to create texture for region: " + title);
        return -1;
    }
    region_textures_.push_back(tex);

    PlatformDisplay::regions_.push_back(region);
    LOG_INFO("Added display region: " + title + " at (" + std::to_string(x) + "," + std::to_string(y) +
             ") size " + std::to_string(width) + "x" + std::to_string(height));

    return region.id;
}

bool OpenGLDisplay::updateRegionFrame(int region_id, std::shared_ptr<GpuFrame> gpu_frame) {
    if (!initialized_) return false;

    std::lock_guard<std::mutex> lock(frame_mutex_);

    for (auto& region : regions_) {
        if (region.id == region_id) {
            region.current_frame = gpu_frame;
            return true;
        }
    }

    return false;
}

void OpenGLDisplay::render() {
    if (!initialized_ || !window_) return;

    glClear(GL_COLOR_BUFFER_BIT);

    std::lock_guard<std::mutex> lock(frame_mutex_);

    // 渲染每个区域
    for (const auto& region : regions_) {
        if (region.current_frame) {
            // 更新纹理
            updateTextureFromGpu(region.texture, region.current_frame);

            // 渲染纹理
            renderTexture(region.texture, region.x, region.y, region.width, region.height);

            // 渲染标题
            renderText(region.title, region.x + 10, region.y + region.height - 30);
        } else {
            // 渲染黑色背景
            glColor3f(0.0f, 0.0f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f((region.x * 2.0f / window_width_ - 1.0f),
                      (region.y * 2.0f / window_height_ - 1.0f));
            glVertex2f(((region.x + region.width) * 2.0f / window_width_ - 1.0f),
                      (region.y * 2.0f / window_height_ - 1.0f));
            glVertex2f(((region.x + region.width) * 2.0f / window_width_ - 1.0f),
                      ((region.y + region.height) * 2.0f / window_height_ - 1.0f));
            glVertex2f((region.x * 2.0f / window_width_ - 1.0f),
                      ((region.y + region.height) * 2.0f / window_height_ - 1.0f));
            glEnd();
        }
    }

    glfwSwapBuffers(window_);
}

void OpenGLDisplay::processEvents() {
    if (window_) {
        glfwPollEvents();
    }
}

bool OpenGLDisplay::shouldClose() const {
    return window_ && glfwWindowShouldClose(window_);
}

void OpenGLDisplay::close() {
    if (!initialized_) return;

    // 销毁纹理
    for (auto& region : regions_) {
        destroyTexture(region.texture);
    }
    regions_.clear();

    // 销毁OpenGL资源
    if (shader_program_) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
    }
    if (nv12_shader_program_) {
        glDeleteProgram(nv12_shader_program_);
        nv12_shader_program_ = 0;
    }
    if (vertex_buffer_) {
        glDeleteBuffers(1, &vertex_buffer_);
        vertex_buffer_ = 0;
    }
    if (vertex_array_) {
        glDeleteVertexArrays(1, &vertex_array_);
        vertex_array_ = 0;
    }

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    glfwTerminate();
    PlatformDisplay::initialized_ = false;
    LOG_INFO("OpenGL display closed");
}

bool OpenGLDisplay::createShaders() {
    // 创建基本着色器
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glLinkProgram(shader_program_);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // 创建NV12着色器
    nv12_shader_program_ = createNV12ToRGBShader();

    // 创建VAO和VBO
    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &vertex_buffer_);

    glBindVertexArray(vertex_array_);

    float vertices[] = {
        // 位置     // 纹理坐标
        -1.0f,  1.0f,  0.0f, 1.0f,  // 左上
         1.0f,  1.0f,  1.0f, 1.0f,  // 右上
         1.0f, -1.0f,  1.0f, 0.0f,  // 右下
        -1.0f, -1.0f,  0.0f, 0.0f   // 左下
    };

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGLDisplay::createTexture(OpenGLTexture& tex, int width, int height, bool is_nv12) {
    tex.width = width;
    tex.height = height;
    tex.is_nv12 = is_nv12;

    if (is_nv12) {
        // 为NV12创建两个纹理（Y和UV）
        glGenTextures(1, &tex.texture_id);
        glBindTexture(GL_TEXTURE_2D, tex.texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

        // 创建PBO用于GPU数据传输
        glGenBuffers(1, &tex.pbo_id);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex.pbo_id);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<size_t>(width) * height * 3 / 2, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    } else {
        // RGB纹理
        glGenTextures(1, &tex.texture_id);
        glBindTexture(GL_TEXTURE_2D, tex.texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    return true;
}

void OpenGLDisplay::destroyTexture(OpenGLTexture& tex) {
    if (tex.texture_id) {
        glDeleteTextures(1, &tex.texture_id);
        tex.texture_id = 0;
    }
    if (tex.pbo_id) {
        glDeleteBuffers(1, &tex.pbo_id);
        tex.pbo_id = 0;
    }
}

bool OpenGLDisplay::updateTextureFromGpu(OpenGLTexture& tex, std::shared_ptr<GpuFrame> gpu_frame) {
    if (!gpu_frame || !gpu_frame->gpu_ptr) return false;

    // 从GPU内存复制数据到PBO
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex.pbo_id);
    // 如果分辨率变化导致size变大，确保PBO容量足够（避免写越界）
    const size_t needed = gpu_frame->size;
    GLint current_size = 0;
    glGetBufferParameteriv(GL_PIXEL_UNPACK_BUFFER, GL_BUFFER_SIZE, &current_size);
    if (current_size < static_cast<GLint>(needed)) {
        glBufferData(GL_PIXEL_UNPACK_BUFFER, needed, nullptr, GL_STREAM_DRAW);
    }
    void* pbo_ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (pbo_ptr) {
        GpuMemoryManager::getInstance().copyFromGpu(gpu_frame, static_cast<uint8_t*>(pbo_ptr), needed);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // 更新纹理
    glBindTexture(GL_TEXTURE_2D, tex.texture_id);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex.pbo_id);
    if (tex.is_nv12) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex.width, tex.height,
                       GL_RED, GL_UNSIGNED_BYTE, nullptr);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex.width, tex.height,
                       GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void OpenGLDisplay::renderTexture(const OpenGLTexture& tex, int x, int y, int width, int height) {
    // 计算标准化设备坐标
    float left = (x * 2.0f / PlatformDisplay::window_width_) - 1.0f;
    float right = ((x + width) * 2.0f / PlatformDisplay::window_width_) - 1.0f;
    float top = 1.0f - (y * 2.0f / PlatformDisplay::window_height_);
    float bottom = 1.0f - ((y + height) * 2.0f / PlatformDisplay::window_height_);

    // 使用NV12着色器
    glUseProgram(nv12_shader_program_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.texture_id);
    glUniform1i(glGetUniformLocation(nv12_shader_program_, "yTexture"), 0);

    // 临时创建UV纹理（这里简化处理，实际应该创建单独的UV纹理）
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex.texture_id);
    glUniform1i(glGetUniformLocation(nv12_shader_program_, "uvTexture"), 1);

    glBindVertexArray(vertex_array_);

    // 更新顶点数据
    float vertices[] = {
        left,  top,    0.0f, 1.0f,
        right, top,    1.0f, 1.0f,
        right, bottom, 1.0f, 0.0f,
        left,  bottom, 0.0f, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLDisplay::renderText(const std::string& text, int x, int y) {
    // 这里简化实现，实际应该使用freetype-gl或类似库渲染文本
    // 暂时跳过文本渲染
}

GLuint OpenGLDisplay::createNV12ToRGBShader() {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &nv12_vertex_shader, nullptr);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &nv12_fragment_shader, nullptr);
    glCompileShader(fragment_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void OpenGLDisplay::glfwErrorCallback(int error, const char* description) {
    LOG_ERROR("GLFW Error " + std::to_string(error) + ": " + description);
}

void OpenGLDisplay::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

#endif // __linux__