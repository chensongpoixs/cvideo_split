/**
 * @file DxgiDisplay.h
 * @author chensong
 * @date 2026-01-11
 * @brief Windows DXGI/DirectX11 视频显示实现（Windows DXGI/DirectX11 Video Display）
 * 
 * 该模块是 PlatformDisplay 接口的 Windows 实现，使用 DirectX11 进行渲染，
 * 并通过 CUDA-D3D11 互操作实现 GPU 零拷贝显示。
 * 
 * DXGI 显示架构（DXGI Display Architecture）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       DxgiDisplay                             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   零拷贝 GPU 路径（优先）                                     |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │ GpuFrame (NV12, GPU)                                    │|
 *  |   │      │                                                   │|
 *  |   │      ▼                                                   │|
 *  |   │ cudaGraphicsMapResources (映射 D3D11 纹理)              │|
 *  |   │      │                                                   │|
 *  |   │      ▼                                                   │|
 *  |   │ CUDA Kernel: NV12→RGBA (写入 cudaArray)                 │|
 *  |   │      │                                                   │|
 *  |   │      ▼                                                   │|
 *  |   │ cudaGraphicsUnmapResources                              │|
 *  |   │      │                                                   │|
 *  |   │      ▼                                                   │|
 *  |   │ D3D11 纹理直接渲染                                      │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |                                                               |
 *  |   CPU 回退路径（零拷贝失败时）                                |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │ GpuFrame → cudaMemcpy → CPU NV12                        │|
 *  |   │      │                                                   │|
 *  |   │      ▼                                                   │|
 *  |   │ CPU NV12→RGBA 转换 (缩放/裁剪)                          │|
 *  |   │      │                                                   │|
 *  |   │      ▼                                                   │|
 *  |   │ UpdateSubresource → D3D11 纹理                          │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 批量映射优化（Batch Mapping Optimization）：
 * 
 *   单帧映射:  map → kernel → unmap → map → kernel → unmap (开销大)
 *   批量映射:  mapAll → kernel → kernel → unmapAll        (开销小)
 * 
 * DirectX11 渲染管线（DirectX11 Rendering Pipeline）：
 * 
 *   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
 *   │ Vertex      │───►│ Pixel       │───►│ Output      │
 *   │ Shader      │    │ Shader      │    │ Merger      │
 *   └─────────────┘    └─────────────┘    └─────────────┘
 *         │                  │
 *         ▼                  ▼
 *   ┌─────────────┐    ┌─────────────┐
 *   │ NDC 坐标    │    │ RGBA 纹理   │
 *   │ + 纹理坐标  │    │ 采样        │
 *   └─────────────┘    └─────────────┘
 * 
 * @note 仅在 _WIN32 平台编译
 * @note 使用 CudaStreamManager 获取共享 CUDA stream
 * @see PlatformDisplay 抽象基类
 * @see OpenGLDisplay Linux 实现
 */

#pragma once

#include "PlatformDisplay.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <cstdint>
#include <vector>
#include <cuda_runtime.h>
#include <cuda_d3d11_interop.h>

/**
 * @class DxgiDisplay
 * @brief Windows DXGI/DirectX11 视频显示实现
 * 
 * 使用 DirectX11 进行高性能视频渲染，支持：
 * - CUDA-D3D11 互操作零拷贝显示
 * - 多区域布局
 * - 动态缩放和中心裁剪
 * - 边框和分隔线绘制
 * 
 * 使用示例：
 * @code
 * DxgiDisplay display;
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
class DxgiDisplay : public PlatformDisplay {
public:
    /**
     * @brief 构造函数
     */
    DxgiDisplay();

    /**
     * @brief 析构函数
     */
    ~DxgiDisplay() override;

    /**
     * @brief 初始化显示器
     * 
     * 创建 Win32 窗口、初始化 DirectX11 设备和渲染管线。
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
     * 
     * 执行操作：
     * 1. 检查窗口尺寸变化，调整 SwapChain
     * 2. 批量映射所有脏纹理
     * 3. CUDA kernel 转换 NV12→RGBA
     * 4. 批量解除映射
     * 5. 绘制纹理和边框
     * 6. Present
     */
    void render() override;

    /**
     * @brief 处理窗口事件
     * 
     * 处理 Win32 消息队列。
     */
    void processEvents() override;

    /**
     * @brief 检查是否应关闭
     * @return true 收到 WM_CLOSE/WM_DESTROY
     */
    bool shouldClose() const override;

    /**
     * @brief 关闭显示器
     */
    void close() override;

private:
    /**
     * @brief Win32 窗口过程
     * @param hwnd 窗口句柄
     * @param uMsg 消息类型
     * @param wParam 消息参数
     * @param lParam 消息参数
     * @return LRESULT 处理结果
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 初始化 DirectX11
     * @return true 成功
     */
    bool initDirectX();

    /**
     * @brief 清理 DirectX 资源
     */
    void cleanupDirectX();

    /**
     * @brief 创建区域纹理
     * 
     * 创建 D3D11 纹理并注册 CUDA 互操作资源。
     * 
     * @param region_id 区域 ID
     * @param width 纹理宽度
     * @param height 纹理高度
     * @return true 成功
     */
    bool createTexture(int region_id, int width, int height);

    /**
     * @brief 销毁区域纹理
     * @param region_id 区域 ID
     */
    void destroyTexture(int region_id);

    /**
     * @brief 从 GPU 帧更新纹理
     * 
     * 优先使用零拷贝路径，失败回退 CPU 路径。
     * 
     * @param region_id 区域 ID
     * @param gpu_frame GPU 帧
     * @param allow_zero_copy 是否允许零拷贝
     * @return true 成功
     */
    bool updateTextureFromGpu(int region_id, std::shared_ptr<GpuFrame> gpu_frame, bool allow_zero_copy = true);

    /**
     * @brief 批量更新纹理
     * 
     * 单次 map/unmap 处理多个区域，减少互操作开销。
     * 
     * @param region_frames 区域-帧对
     */
    void updateTexturesFromGpuBatch(const std::vector<std::pair<int, std::shared_ptr<GpuFrame>>>& region_frames);

    /**
     * @brief 渲染单个纹理
     * @param region_id 区域 ID
     * @param x 目标 X
     * @param y 目标 Y
     * @param width 目标宽度
     * @param height 目标高度
     */
    void renderTexture(int region_id, int x, int y, int width, int height);

    HWND hwnd_ = nullptr;                    ///< Win32 窗口句柄
    HINSTANCE hinstance_ = nullptr;          ///< 应用实例
    bool should_close_ = false;              ///< 关闭标志

    // DirectX11 资源
    ID3D11Device* d3d_device_ = nullptr;                ///< D3D11 设备
    ID3D11DeviceContext* d3d_context_ = nullptr;        ///< 设备上下文
    IDXGISwapChain1* swap_chain_ = nullptr;             ///< 交换链
    ID3D11RenderTargetView* render_target_view_ = nullptr; ///< 渲染目标视图
    ID3D11VertexShader* vertex_shader_ = nullptr;       ///< 顶点着色器
    ID3D11PixelShader* pixel_shader_ = nullptr;         ///< 像素着色器（纹理）
    ID3D11PixelShader* color_pixel_shader_ = nullptr;   ///< 像素着色器（纯色边框）
    ID3D11InputLayout* input_layout_ = nullptr;         ///< 输入布局
    ID3D11Buffer* vertex_buffer_ = nullptr;             ///< 顶点缓冲区
    ID3D11Buffer* color_constant_buffer_ = nullptr;     ///< 颜色常量缓冲区
    ID3D11SamplerState* sampler_state_ = nullptr;       ///< 采样器状态

    /**
     * @struct DxgiTexture
     * @brief 区域纹理资源
     */
    struct DxgiTexture {
        ID3D11Texture2D* texture = nullptr;           ///< D3D11 纹理
        ID3D11ShaderResourceView* srv = nullptr;      ///< 着色器资源视图
        cudaGraphicsResource_t cuda_resource = nullptr; ///< CUDA 互操作资源
        int width = 0;                                 ///< 纹理宽度
        int height = 0;                                ///< 纹理高度
        int64_t last_uploaded_pts = INT64_MIN;         ///< 上次上传的 PTS
        uintptr_t last_uploaded_ptr = 0;               ///< 上次上传的 GPU 指针

        /// CPU 回退用 NV12 缓冲区（复用避免分配）
        std::vector<uint8_t> cpu_nv12;
        /// CPU 回退用 RGBA 缓冲区（复用避免分配）
        std::vector<uint8_t> cpu_rgba;
    };

    std::vector<DxgiTexture> textures_;  ///< 区域纹理数组

    /**
     * @brief 获取显示用 CUDA stream
     * @return cudaStream_t 共享 stream
     */
    cudaStream_t getDisplayStream();
};

#endif // _WIN32
