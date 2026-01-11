/**
 * @file DxgiDisplay.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief Windows DXGI/DirectX11 视频显示实现（CUDA-D3D11 零拷贝）
 * @see DxgiDisplay.h
 */

#ifdef _WIN32

#include "DxgiDisplay.h"
#include "CudaStreamManager.h"
#include "Logger.h"

#include <d3dcompiler.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <thread>

#include <cuda_runtime.h>
#include <cuda_d3d11_interop.h>
#include "Nv12ToRgbaKernel.h"

// 获取共享的 display stream
cudaStream_t DxgiDisplay::getDisplayStream() {
    auto& mgr = CudaStreamManager::getInstance();
    if (mgr.isInitialized()) {
        return mgr.getDisplayStream();
    }
    return nullptr;
}

// 顶点结构：NDC坐标 + 纹理坐标
struct Vertex {
    float pos[2];
    float tex[2];
};

struct ColorCB {
    float color[4];
};

// 顶点着色器HLSL代码
static const char* vertex_shader_code = R"(
struct VS_INPUT {
    float2 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = float4(input.pos, 0.0, 1.0);
    output.tex = input.tex;
    return output;
}
)";

// 像素着色器：采样RGBA纹理（先保证能显示；NV12->RGBA在CPU侧转换）
static const char* pixel_shader_code = R"(
Texture2D tex0 : register(t0);
SamplerState samplerState : register(s0);

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_Target {
    return tex0.Sample(samplerState, input.tex);
}
)";

// 纯色像素着色器：用于画边框/分割线
static const char* color_pixel_shader_code = R"(
cbuffer ColorBuffer : register(b0) {
    float4 color;
};

float4 main(float4 pos : SV_POSITION) : SV_Target {
    return color;
}
)";

static inline uint8_t clamp_u8(int v) {
    return static_cast<uint8_t>(std::max(0, std::min(255, v)));
}

static void nv12ToRgbaCpu(const uint8_t* nv12, std::vector<uint8_t>& rgba, int width, int height) {
    rgba.resize(static_cast<size_t>(width) * height * 4);
    const uint8_t* y_plane = nv12;
    const uint8_t* uv_plane = nv12 + static_cast<size_t>(width) * height;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int y_idx = y * width + x;
            const int uv_idx = (y / 2) * width + (x / 2) * 2;

            const int Y = static_cast<int>(y_plane[y_idx]);
            const int U = static_cast<int>(uv_plane[uv_idx]) - 128;
            const int V = static_cast<int>(uv_plane[uv_idx + 1]) - 128;

            // BT.601 近似
            int R = static_cast<int>(Y + 1.402f * V);
            int G = static_cast<int>(Y - 0.344f * U - 0.714f * V);
            int B = static_cast<int>(Y + 1.772f * U);

            const size_t out = (static_cast<size_t>(y) * width + x) * 4;
            rgba[out + 0] = clamp_u8(R);
            rgba[out + 1] = clamp_u8(G);
            rgba[out + 2] = clamp_u8(B);
            rgba[out + 3] = 255;
        }
    }
}

// 把NV12缩放到目标尺寸（可选择居中裁剪以填满目标区域）
static void nv12ToRgbaCpuScaled(const uint8_t* nv12,
                                std::vector<uint8_t>& rgba,
                                int src_w, int src_h,
                                int dst_w, int dst_h,
                                bool crop_to_fill) {
    if (!nv12 || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) {
        rgba.clear();
        return;
    }

    rgba.resize(static_cast<size_t>(dst_w) * dst_h * 4);
    const uint8_t* y_plane = nv12;
    const uint8_t* uv_plane = nv12 + static_cast<size_t>(src_w) * src_h;

    // 计算裁剪窗口（在源图上取一个与目标同宽高比的居中矩形）
    float src_aspect = static_cast<float>(src_w) / static_cast<float>(src_h);
    float dst_aspect = static_cast<float>(dst_w) / static_cast<float>(dst_h);

    int crop_x = 0, crop_y = 0, crop_w = src_w, crop_h = src_h;
    if (crop_to_fill) {
        if (src_aspect > dst_aspect) {
            // 源更宽，裁剪宽度
            crop_w = static_cast<int>(std::round(dst_aspect * src_h));
            crop_w = std::max(1, std::min(crop_w, src_w));
            crop_x = (src_w - crop_w) / 2;
        } else if (src_aspect < dst_aspect) {
            // 源更高，裁剪高度
            crop_h = static_cast<int>(std::round(src_w / dst_aspect));
            crop_h = std::max(1, std::min(crop_h, src_h));
            crop_y = (src_h - crop_h) / 2;
        }
    }

    // 最近邻采样（显示用足够），避免CPU开销过大
    for (int y = 0; y < dst_h; ++y) {
        float v = (dst_h == 1) ? 0.0f : (static_cast<float>(y) / (dst_h - 1));
        int sy = crop_y + static_cast<int>(v * (crop_h - 1));
        sy = std::max(0, std::min(sy, src_h - 1));

        for (int x = 0; x < dst_w; ++x) {
            float u = (dst_w == 1) ? 0.0f : (static_cast<float>(x) / (dst_w - 1));
            int sx = crop_x + static_cast<int>(u * (crop_w - 1));
            sx = std::max(0, std::min(sx, src_w - 1));

            const int y_idx = sy * src_w + sx;
            const int uv_idx = (sy / 2) * src_w + (sx / 2) * 2;

            const int Y = static_cast<int>(y_plane[y_idx]);
            const int U = static_cast<int>(uv_plane[uv_idx]) - 128;
            const int V = static_cast<int>(uv_plane[uv_idx + 1]) - 128;

            int R = static_cast<int>(Y + 1.402f * V);
            int G = static_cast<int>(Y - 0.344f * U - 0.714f * V);
            int B = static_cast<int>(Y + 1.772f * U);

            const size_t out = (static_cast<size_t>(y) * dst_w + x) * 4;
            rgba[out + 0] = clamp_u8(R);
            rgba[out + 1] = clamp_u8(G);
            rgba[out + 2] = clamp_u8(B);
            rgba[out + 3] = 255;
        }
    }
}

DxgiDisplay::DxgiDisplay() {
    hinstance_ = GetModuleHandle(nullptr);
}

DxgiDisplay::~DxgiDisplay() {
    close();
}

LRESULT CALLBACK DxgiDisplay::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    DxgiDisplay* display = reinterpret_cast<DxgiDisplay*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (uMsg) {
        case WM_CLOSE:
            if (display) {
                display->should_close_ = true;
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                if (display) {
                    display->should_close_ = true;
                }
            }
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool DxgiDisplay::initialize(int window_width, int window_height, const std::string& window_title,
                             bool fullscreen) {
    window_width_ = window_width;
    window_height_ = window_height;
    window_title_ = window_title;

    // 注册窗口类
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hinstance_;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"VideoStitchWindow";
    
    if (!RegisterClassExW(&wc)) {
        LOG_ERROR("Failed to register window class");
        return false;
    }

    // 创建窗口
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (fullscreen) {
        style = WS_POPUP | WS_VISIBLE;
    }

    RECT rect = { 0, 0, window_width, window_height };
    AdjustWindowRect(&rect, style, FALSE);

    hwnd_ = CreateWindowExW(
        0,
        L"VideoStitchWindow",
        std::wstring(window_title.begin(), window_title.end()).c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hinstance_,
        nullptr
    );

    if (!hwnd_) {
        LOG_ERROR("Failed to create window");
        return false;
    }

    SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // 初始化DirectX
    if (!initDirectX()) {
        LOG_ERROR("Failed to initialize DirectX");
        close();
        return false;
    }

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);

    initialized_ = true;
    LOG_INFO("DXGI display initialized: " + window_title + " (" +
             std::to_string(window_width) + "x" + std::to_string(window_height) + ")");

    return true;
}

bool DxgiDisplay::initDirectX() {
    // 创建设备和上下文
    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    D3D_FEATURE_LEVEL feature_level;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        feature_levels,
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        &d3d_device_,
        &feature_level,
        &d3d_context_
    );

    if (FAILED(hr)) {
        LOG_ERROR("Failed to create D3D11 device, error: " + std::to_string(hr));
        return false;
    }

    // 获取DXGI设备
    IDXGIDevice* dxgi_device = nullptr;
    hr = d3d_device_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get DXGI device");
        return false;
    }

    IDXGIAdapter* dxgi_adapter = nullptr;
    hr = dxgi_device->GetAdapter(&dxgi_adapter);
    dxgi_device->Release();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get DXGI adapter");
        return false;
    }

    IDXGIFactory2* dxgi_factory = nullptr;
    hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgi_factory));
    dxgi_adapter->Release();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get DXGI factory");
        return false;
    }

    // 创建交换链
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Width = window_width_;
    swap_chain_desc.Height = window_height_;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    hr = dxgi_factory->CreateSwapChainForHwnd(
        d3d_device_,
        hwnd_,
        &swap_chain_desc,
        nullptr,
        nullptr,
        &swap_chain_
    );
    dxgi_factory->Release();

    if (FAILED(hr)) {
        LOG_ERROR("Failed to create swap chain, error: " + std::to_string(hr));
        return false;
    }

    // 创建渲染目标视图
    ID3D11Texture2D* back_buffer = nullptr;
    hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
    if (FAILED(hr)) {
        LOG_ERROR("Failed to get back buffer");
        return false;
    }

    hr = d3d_device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_);
    back_buffer->Release();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create render target view");
        return false;
    }

    d3d_context_->OMSetRenderTargets(1, &render_target_view_, nullptr);

    // 设置视口
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(window_width_);
    viewport.Height = static_cast<float>(window_height_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    d3d_context_->RSSetViewports(1, &viewport);

    // 编译/创建着色器
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
    ID3DBlob* cps_blob = nullptr;
    ID3DBlob* err_blob = nullptr;

    hr = D3DCompile(vertex_shader_code, strlen(vertex_shader_code), nullptr, nullptr, nullptr,
                    "main", "vs_5_0", 0, 0, &vs_blob, &err_blob);
    if (FAILED(hr)) {
        std::string err = err_blob ? std::string((const char*)err_blob->GetBufferPointer(), err_blob->GetBufferSize()) : "unknown";
        if (err_blob) err_blob->Release();
        LOG_ERROR("Failed to compile vertex shader: " + err);
        return false;
    }
    if (err_blob) { err_blob->Release(); err_blob = nullptr; }

    hr = D3DCompile(pixel_shader_code, strlen(pixel_shader_code), nullptr, nullptr, nullptr,
                    "main", "ps_5_0", 0, 0, &ps_blob, &err_blob);
    if (FAILED(hr)) {
        std::string err = err_blob ? std::string((const char*)err_blob->GetBufferPointer(), err_blob->GetBufferSize()) : "unknown";
        if (err_blob) err_blob->Release();
        if (vs_blob) vs_blob->Release();
        LOG_ERROR("Failed to compile pixel shader: " + err);
        return false;
    }
    if (err_blob) { err_blob->Release(); err_blob = nullptr; }

    hr = D3DCompile(color_pixel_shader_code, strlen(color_pixel_shader_code), nullptr, nullptr, nullptr,
                    "main", "ps_5_0", 0, 0, &cps_blob, &err_blob);
    if (FAILED(hr)) {
        std::string err = err_blob ? std::string((const char*)err_blob->GetBufferPointer(), err_blob->GetBufferSize()) : "unknown";
        if (err_blob) err_blob->Release();
        if (vs_blob) vs_blob->Release();
        if (ps_blob) ps_blob->Release();
        LOG_ERROR("Failed to compile color pixel shader: " + err);
        return false;
    }
    if (err_blob) { err_blob->Release(); err_blob = nullptr; }

    hr = d3d_device_->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vertex_shader_);
    if (FAILED(hr)) {
        vs_blob->Release();
        ps_blob->Release();
        cps_blob->Release();
        LOG_ERROR("Failed to create vertex shader");
        return false;
    }
    hr = d3d_device_->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &pixel_shader_);
    if (FAILED(hr)) {
        vs_blob->Release();
        ps_blob->Release();
        cps_blob->Release();
        LOG_ERROR("Failed to create pixel shader");
        return false;
    }
    hr = d3d_device_->CreatePixelShader(cps_blob->GetBufferPointer(), cps_blob->GetBufferSize(), nullptr, &color_pixel_shader_);
    if (FAILED(hr)) {
        vs_blob->Release();
        ps_blob->Release();
        cps_blob->Release();
        LOG_ERROR("Failed to create color pixel shader");
        return false;
    }

    // 输入布局
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = d3d_device_->CreateInputLayout(layout, 2, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout_);
    vs_blob->Release();
    ps_blob->Release();
    cps_blob->Release();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create input layout");
        return false;
    }

    // 动态顶点缓冲（每次绘制一个区域更新一次）
    // 注意：我们会用 4 顶点画纹理(TRIANGLESTRIP)，也会用 5 顶点画边框/分区框(LINESTRIP闭合)，
    // 因此缓冲至少要容纳 5 个 Vertex（这里给 6 个留余量）。
    D3D11_BUFFER_DESC vb_desc = {};
    vb_desc.Usage = D3D11_USAGE_DYNAMIC;
    vb_desc.ByteWidth = sizeof(Vertex) * 6;
    vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = d3d_device_->CreateBuffer(&vb_desc, nullptr, &vertex_buffer_);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create vertex buffer");
        return false;
    }

    // 颜色常量缓冲
    D3D11_BUFFER_DESC cb_desc = {};
    cb_desc.Usage = D3D11_USAGE_DYNAMIC;
    cb_desc.ByteWidth = sizeof(ColorCB);
    cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = d3d_device_->CreateBuffer(&cb_desc, nullptr, &color_constant_buffer_);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create color constant buffer");
        return false;
    }

    // 创建采样器状态
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = d3d_device_->CreateSamplerState(&sampler_desc, &sampler_state_);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create sampler state");
        return false;
    }

    return true;
}

int DxgiDisplay::addDisplayRegion(int x, int y, int width, int height, const std::string& title) {
    if (!initialized_) return -1;

    DisplayRegion region;
    region.id = next_region_id_++;
    region.x = x;
    region.y = y;
    region.width = width;
    region.height = height;
    region.title = title;

    // 创建纹理
    if (!createTexture(region.id, width, height)) {
        LOG_ERROR("Failed to create texture for region: " + title);
        return -1;
    }

    regions_.push_back(region);
    LOG_INFO("Added display region: " + title + " at (" + std::to_string(x) + "," + std::to_string(y) +
             ") size " + std::to_string(width) + "x" + std::to_string(height));

    return region.id;
}

bool DxgiDisplay::updateRegionFrame(int region_id, std::shared_ptr<GpuFrame> gpu_frame) {
    if (!initialized_) return false;

    std::lock_guard<std::mutex> lock(frame_mutex_);

    for (auto& region : regions_) {
        if (region.id == region_id) {
            region.current_frame = gpu_frame;
            // 注意：D3D11 immediate context 不是线程安全的。
            // updateTextureFromGpu / 渲染相关的所有 D3D 调用必须在同一线程（主线程 render）中执行，
            // 这里只保存最新帧，真正上传纹理在 render() 里做。
            return true;
        }
    }

    return false;
}

void DxgiDisplay::render() {
    if (!initialized_ || !hwnd_) return;

    // 如果窗口 client 区尺寸发生变化（DPI/WM_SIZE/窗口边框调整），需要 ResizeBuffers 并更新 viewport。
    // 否则会出现“渲染一闪然后消失/错位”等现象。
    RECT rc{};
    if (GetClientRect(hwnd_, &rc)) {
        int client_w = static_cast<int>(rc.right - rc.left);
        int client_h = static_cast<int>(rc.bottom - rc.top);
        if (client_w < 1) client_w = 1;
        if (client_h < 1) client_h = 1;
        if ((client_w != window_width_ || client_h != window_height_) && swap_chain_ && d3d_device_ && d3d_context_) {
            // ResizeBuffers 的关键：必须先解绑 backbuffer（OMSetRenderTargets(nullptr)），否则经常失败/导致后续渲染无效
            ID3D11RenderTargetView* null_rtv = nullptr;
            d3d_context_->OMSetRenderTargets(1, &null_rtv, nullptr);
            d3d_context_->Flush();

            // 先释放旧RTV（但仅在我们即将成功重建时才“真正换新”）
            ID3D11RenderTargetView* old_rtv = render_target_view_;
            render_target_view_ = nullptr;

            HRESULT hr = swap_chain_->ResizeBuffers(0, client_w, client_h, DXGI_FORMAT_UNKNOWN, 0);
            if (FAILED(hr)) {
                // 失败：恢复旧RTV，避免后续帧完全无输出
                render_target_view_ = old_rtv;
                LOG_WARNING("DXGI ResizeBuffers failed, hr=" + std::to_string(static_cast<long long>(hr)) +
                            ", keep old RTV. requested=" + std::to_string(client_w) + "x" + std::to_string(client_h));
            } else {
                // 成功：旧RTV可以释放
                if (old_rtv) {
                    old_rtv->Release();
                    old_rtv = nullptr;
                }

                ID3D11Texture2D* back_buffer = nullptr;
                hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
                if (FAILED(hr) || !back_buffer) {
                    LOG_ERROR("DXGI GetBuffer failed after ResizeBuffers, hr=" + std::to_string(static_cast<long long>(hr)));
                } else {
                    hr = d3d_device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_);
                    back_buffer->Release();
                    if (FAILED(hr) || !render_target_view_) {
                        LOG_ERROR("DXGI CreateRenderTargetView failed after ResizeBuffers, hr=" + std::to_string(static_cast<long long>(hr)));
                    }
                }

                if (render_target_view_) {
                    d3d_context_->OMSetRenderTargets(1, &render_target_view_, nullptr);
                    D3D11_VIEWPORT viewport{};
                    viewport.Width = static_cast<float>(client_w);
                    viewport.Height = static_cast<float>(client_h);
                    viewport.MinDepth = 0.0f;
                    viewport.MaxDepth = 1.0f;
                    d3d_context_->RSSetViewports(1, &viewport);

                    window_width_ = client_w;
                    window_height_ = client_h;
                    LOG_INFO("DXGI resized to client area: " + std::to_string(window_width_) + "x" + std::to_string(window_height_));
                } else {
                    // 没有RTV就别继续渲染，避免无意义调用/潜在错误
                    return;
                }
            }
        }
    }

    if (!render_target_view_) {
        return;
    }

    // 保险：每帧都重新绑定RTV与viewport，避免 ResizeBuffers/系统状态变化导致后续帧“画不出来”
    d3d_context_->OMSetRenderTargets(1, &render_target_view_, nullptr);
    D3D11_VIEWPORT vp{};
    vp.Width = static_cast<float>(window_width_);
    vp.Height = static_cast<float>(window_height_);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    d3d_context_->RSSetViewports(1, &vp);

    // 清除渲染目标
    float clear_color[4] = { 0.05f, 0.05f, 0.15f, 1.0f };
    d3d_context_->ClearRenderTargetView(render_target_view_, clear_color);

    // 拷贝出当前帧指针（避免长时间持锁影响解码线程）
    std::vector<std::pair<int, std::shared_ptr<GpuFrame>>> region_frames;
    struct RegionDraw { int id; int x; int y; int w; int h; };
    std::vector<RegionDraw> draw_list;
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        region_frames.reserve(regions_.size());
        draw_list.reserve(regions_.size());
        for (const auto& region : regions_) {
            if (region.current_frame) {
                region_frames.emplace_back(region.id, region.current_frame);
                draw_list.push_back(RegionDraw{region.id, region.x, region.y, region.width, region.height});
            }
        }
    }

    // zero-copy 批量更新（必要时回退 CPU），然后绘制
    updateTexturesFromGpuBatch(region_frames);
    for (const auto& d : draw_list) {
        renderTexture(d.id, d.x, d.y, d.w, d.h);
    }

    // 画“解码区域/拼接区域”大框与分割线（即使没有视频也能看到边界）
    if (color_pixel_shader_ && vertex_shader_ && input_layout_ && vertex_buffer_ && color_constant_buffer_) {
        auto drawRect = [&](int px, int py, int pw, int ph, float r, float g, float b, float a) {
            float left   = (static_cast<float>(px) / window_width_) * 2.0f - 1.0f;
            float right  = (static_cast<float>(px + pw) / window_width_) * 2.0f - 1.0f;
            float top    = 1.0f - (static_cast<float>(py) / window_height_) * 2.0f;
            float bottom = 1.0f - (static_cast<float>(py + ph) / window_height_) * 2.0f;

            // 设置颜色
            D3D11_MAPPED_SUBRESOURCE cb_mapped = {};
            if (SUCCEEDED(d3d_context_->Map(color_constant_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb_mapped))) {
                ColorCB cb{};
                cb.color[0] = r; cb.color[1] = g; cb.color[2] = b; cb.color[3] = a;
                memcpy(cb_mapped.pData, &cb, sizeof(cb));
                d3d_context_->Unmap(color_constant_buffer_, 0);
            }

            Vertex border[5] = {
                { { left,  top    }, { 0.0f, 0.0f } },
                { { right, top    }, { 0.0f, 0.0f } },
                { { right, bottom }, { 0.0f, 0.0f } },
                { { left,  bottom }, { 0.0f, 0.0f } },
                { { left,  top    }, { 0.0f, 0.0f } },
            };

            D3D11_MAPPED_SUBRESOURCE vb_mapped = {};
            if (SUCCEEDED(d3d_context_->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &vb_mapped))) {
                memcpy(vb_mapped.pData, border, sizeof(border));
                d3d_context_->Unmap(vertex_buffer_, 0);
            }

            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            d3d_context_->IASetInputLayout(input_layout_);
            d3d_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
            d3d_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

            d3d_context_->VSSetShader(vertex_shader_, nullptr, 0);
            d3d_context_->PSSetShader(color_pixel_shader_, nullptr, 0);
            d3d_context_->PSSetConstantBuffers(0, 1, &color_constant_buffer_);
            ID3D11ShaderResourceView* null_srv = nullptr;
            d3d_context_->PSSetShaderResources(0, 1, &null_srv);
            d3d_context_->Draw(5, 0);
        };

        // 判断是否存在左右两块（按窗口一分为二）
        bool has_left = false, has_right = false;
        const int mid = window_width_ / 2;
        for (const auto& region : regions_) {
            const int cx = region.x + region.width / 2;
            if (cx < mid) has_left = true;
            else has_right = true;
        }

        if (has_left) {
            drawRect(0, 0, mid, window_height_, 0.2f, 0.9f, 0.2f, 1.0f); // 绿色：解码区
        }
        if (has_right) {
            drawRect(mid, 0, window_width_ - mid, window_height_, 0.9f, 0.6f, 0.2f, 1.0f); // 橙色：拼接区
        }
    }

    // 交换缓冲区
    swap_chain_->Present(1, 0);  // VSync
}

void DxgiDisplay::processEvents() {
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool DxgiDisplay::shouldClose() const {
    return should_close_;
}

void DxgiDisplay::close() {
    if (!initialized_) return;

    cleanupDirectX();

    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }

    UnregisterClassW(L"VideoStitchWindow", hinstance_);
    initialized_ = false;
    LOG_INFO("DXGI display closed");
}

void DxgiDisplay::cleanupDirectX() {
    if (color_constant_buffer_) {
        color_constant_buffer_->Release();
        color_constant_buffer_ = nullptr;
    }
    if (sampler_state_) {
        sampler_state_->Release();
        sampler_state_ = nullptr;
    }
    if (vertex_buffer_) {
        vertex_buffer_->Release();
        vertex_buffer_ = nullptr;
    }
    if (input_layout_) {
        input_layout_->Release();
        input_layout_ = nullptr;
    }
    if (pixel_shader_) {
        pixel_shader_->Release();
        pixel_shader_ = nullptr;
    }
    if (color_pixel_shader_) {
        color_pixel_shader_->Release();
        color_pixel_shader_ = nullptr;
    }
    if (vertex_shader_) {
        vertex_shader_->Release();
        vertex_shader_ = nullptr;
    }
    if (render_target_view_) {
        render_target_view_->Release();
        render_target_view_ = nullptr;
    }
    if (swap_chain_) {
        swap_chain_->Release();
        swap_chain_ = nullptr;
    }
    if (d3d_context_) {
        d3d_context_->Release();
        d3d_context_ = nullptr;
    }
    if (d3d_device_) {
        d3d_device_->Release();
        d3d_device_ = nullptr;
    }

    // 清理纹理
    for (int i = 0; i < static_cast<int>(textures_.size()); ++i) {
        destroyTexture(i);
    }
    textures_.clear();

    // 注意：CUDA stream 由 CudaStreamManager 统一管理，不在此销毁
}

bool DxgiDisplay::createTexture(int region_id, int width, int height) {
    if (region_id < 0 || region_id >= static_cast<int>(textures_.size())) {
        textures_.resize(region_id + 1);
    }

    DxgiTexture& tex = textures_[region_id];
    tex.width = width;
    tex.height = height;

    // 创建RGBA纹理
    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width = width;
    tex_desc.Height = height;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = 0;

    HRESULT hr = d3d_device_->CreateTexture2D(&tex_desc, nullptr, &tex.texture);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create D3D11 texture, error: " + std::to_string(hr));
        return false;
    }

    // 创建着色器资源视图
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    hr = d3d_device_->CreateShaderResourceView(tex.texture, &srv_desc, &tex.srv);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create shader resource view");
        tex.texture->Release();
        tex.texture = nullptr;
        return false;
    }

    // CUDA-D3D11 互操作：注册纹理用于零CPU拷贝更新
    tex.cuda_resource = nullptr;
    tex.last_uploaded_pts = INT64_MIN;
    tex.last_uploaded_ptr = 0;
    tex.cpu_nv12.clear();
    tex.cpu_rgba.clear();

    // 使用统一的 CUDA Stream 管理器
    cudaStream_t display_stream = getDisplayStream();
    if (display_stream) {
        cudaError_t ce = cudaGraphicsD3D11RegisterResource(&tex.cuda_resource, tex.texture, cudaGraphicsRegisterFlagsNone);
        if (ce != cudaSuccess) {
            tex.cuda_resource = nullptr;
            LOG_WARNING("cudaGraphicsD3D11RegisterResource failed, fallback to CPU upload. err=" + std::string(cudaGetErrorString(ce)));
        } else {
            LOG_INFO("DXGI zero-copy enabled for region " + std::to_string(region_id) +
                     " texture " + std::to_string(width) + "x" + std::to_string(height));
        }
    } else {
        LOG_WARNING("CudaStreamManager not available, DXGI display will use CPU fallback");
    }

    return true;
}

void DxgiDisplay::destroyTexture(int region_id) {
    if (region_id < 0 || region_id >= static_cast<int>(textures_.size())) {
        return;
    }

    DxgiTexture& tex = textures_[region_id];
    
    if (tex.cuda_resource) {
        cudaGraphicsUnregisterResource(tex.cuda_resource);
        tex.cuda_resource = nullptr;
    }
    if (tex.srv) {
        tex.srv->Release();
        tex.srv = nullptr;
    }
    if (tex.texture) {
        tex.texture->Release();
        tex.texture = nullptr;
    }
    tex.last_uploaded_pts = INT64_MIN;
    tex.last_uploaded_ptr = 0;
    tex.cpu_nv12.clear();
    tex.cpu_rgba.clear();
}

bool DxgiDisplay::updateTextureFromGpu(int region_id, std::shared_ptr<GpuFrame> gpu_frame, bool allow_zero_copy) {
    if (!gpu_frame || !gpu_frame->gpu_ptr) return false;
    if (region_id < 0 || region_id >= static_cast<int>(textures_.size())) return false;

    DxgiTexture& tex = textures_[region_id];
    if (!tex.texture) return false;

    // 查找该 region 的目标显示尺寸（解码区域方块大小固定，输入分辨率可能不同）
    int target_w = gpu_frame->width;
    int target_h = gpu_frame->height;
    for (const auto& r : regions_) {
        if (r.id == region_id) {
            target_w = std::max(1, r.width);
            target_h = std::max(1, r.height);
            break;
        }
    }

    if (tex.width != target_w || tex.height != target_h) {
        destroyTexture(region_id);
        if (!createTexture(region_id, target_w, target_h)) {
            return false;
        }
    }

    // 异步更新：同一帧不重复上传
    const uintptr_t src_ptr = reinterpret_cast<uintptr_t>(gpu_frame->gpu_ptr);
    if (tex.last_uploaded_ptr == src_ptr && tex.last_uploaded_pts == gpu_frame->pts) {
        return true;
    }

    // 零拷贝 GPU 路径：NV12(GPU) -> CUDA kernel -> D3D11 RGBA 纹理（cudaGraphicsResource 映射）
    cudaStream_t display_stream = getDisplayStream();
    if (allow_zero_copy && tex.cuda_resource && display_stream && gpu_frame->is_nv12) {
        cudaError_t me = cudaGraphicsMapResources(1, &tex.cuda_resource, 0);
        if (me == cudaSuccess) {
            cudaArray_t array = nullptr;
            cudaError_t ae = cudaGraphicsSubResourceGetMappedArray(&array, tex.cuda_resource, 0, 0);
            if (ae == cudaSuccess && array) {
                bool ok = launchNv12ToRgbaResizeToCudaArray(
                    reinterpret_cast<const uint8_t*>(gpu_frame->gpu_ptr),
                    gpu_frame->width, gpu_frame->height,
                    array,
                    target_w, target_h,
                    true, // center-crop fill
                    display_stream
                );
                cudaGraphicsUnmapResources(1, &tex.cuda_resource, 0);
                if (ok) {
                    tex.last_uploaded_ptr = src_ptr;
                    tex.last_uploaded_pts = gpu_frame->pts;
                    return true;
                }
            } else {
                cudaGraphicsUnmapResources(1, &tex.cuda_resource, 0);
            }
        }
        LOG_WARNING("DXGI zero-copy update failed for region " + std::to_string(region_id) + ", fallback to CPU path");
    }

    // CPU拷贝 fallback（复用 staging buffer，避免每帧分配）：
    // GPU(NV12) -> CPU(NV12) -> CPU(RGBA) -> D3D纹理
    const size_t nv12_size = gpu_frame->size;
    if (tex.cpu_nv12.size() != nv12_size) {
        tex.cpu_nv12.resize(nv12_size);
    }

    if (!GpuMemoryManager::getInstance().copyFromGpu(gpu_frame, tex.cpu_nv12.data(), nv12_size)) {
        LOG_ERROR("Failed to copy frame data from GPU");
        return false;
    }

    // 关键：将输入NV12动态缩放/裁剪到区域方块大小
    nv12ToRgbaCpuScaled(tex.cpu_nv12.data(), tex.cpu_rgba, gpu_frame->width, gpu_frame->height, target_w, target_h, true);

    if (tex.width != target_w || tex.height != target_h) {
        destroyTexture(region_id);
        if (!createTexture(region_id, target_w, target_h)) {
            return false;
        }
    }

    const UINT row_pitch = static_cast<UINT>(target_w * 4);
    d3d_context_->UpdateSubresource(tex.texture, 0, nullptr, tex.cpu_rgba.data(), row_pitch, 0);
    tex.last_uploaded_ptr = src_ptr;
    tex.last_uploaded_pts = gpu_frame->pts;
    return true;
}

void DxgiDisplay::updateTexturesFromGpuBatch(const std::vector<std::pair<int, std::shared_ptr<GpuFrame>>>& region_frames) {
    // 只批量处理 zero-copy 脏帧；其余/失败的走单个 CPU fallback
    if (region_frames.empty()) return;

    struct ZcItem {
        int region_id;
        std::shared_ptr<GpuFrame> frame;
        int target_w;
        int target_h;
        cudaGraphicsResource_t res;
        uintptr_t src_ptr;
    };

    std::vector<ZcItem> items;
    items.reserve(region_frames.size());

    for (const auto& rf : region_frames) {
        const int region_id = rf.first;
        const auto& gpu_frame = rf.second;
        if (!gpu_frame || !gpu_frame->gpu_ptr) continue;
        if (region_id < 0 || region_id >= static_cast<int>(textures_.size())) continue;

        DxgiTexture& tex = textures_[region_id];
        if (!tex.texture) continue;

        // 目标尺寸 = region 尺寸
        int target_w = gpu_frame->width;
        int target_h = gpu_frame->height;
        for (const auto& r : regions_) {
            if (r.id == region_id) {
                target_w = std::max(1, r.width);
                target_h = std::max(1, r.height);
                break;
            }
        }

        if (tex.width != target_w || tex.height != target_h) {
            destroyTexture(region_id);
            if (!createTexture(region_id, target_w, target_h)) {
                continue;
            }
        }

        const uintptr_t src_ptr = reinterpret_cast<uintptr_t>(gpu_frame->gpu_ptr);
        if (tex.last_uploaded_ptr == src_ptr && tex.last_uploaded_pts == gpu_frame->pts) {
            continue; // 不脏
        }

        cudaStream_t display_stream = getDisplayStream();
        if (tex.cuda_resource && display_stream && gpu_frame->is_nv12) {
            items.push_back(ZcItem{region_id, gpu_frame, target_w, target_h, tex.cuda_resource, src_ptr});
        }
    }

    if (!items.empty()) {
        std::vector<cudaGraphicsResource_t> resources;
        resources.reserve(items.size());
        for (const auto& it : items) resources.push_back(it.res);

        cudaStream_t display_stream = getDisplayStream();
        cudaError_t me = cudaGraphicsMapResources(static_cast<int>(resources.size()), resources.data(), 0);
        if (me == cudaSuccess) {
            for (size_t i = 0; i < items.size(); ++i) {
                const auto& it = items[i];
                cudaArray_t array = nullptr;
                cudaError_t ae = cudaGraphicsSubResourceGetMappedArray(&array, resources[i], 0, 0);
                if (ae == cudaSuccess && array) {
                    bool ok = launchNv12ToRgbaResizeToCudaArray(
                        reinterpret_cast<const uint8_t*>(it.frame->gpu_ptr),
                        it.frame->width, it.frame->height,
                        array,
                        it.target_w, it.target_h,
                        true,
                        display_stream
                    );
                    if (ok) {
                        DxgiTexture& tex = textures_[it.region_id];
                        tex.last_uploaded_ptr = it.src_ptr;
                        tex.last_uploaded_pts = it.frame->pts;
                    }
                }
            }
            cudaGraphicsUnmapResources(static_cast<int>(resources.size()), resources.data(), 0);
        } else {
            LOG_WARNING("DXGI zero-copy batch map failed, fallback to CPU path");
        }
    }

    // 对仍未上传的帧走 CPU fallback（禁用单个 zero-copy，避免重复 map/unmap）
    for (const auto& rf : region_frames) {
        const int region_id = rf.first;
        const auto& gpu_frame = rf.second;
        if (!gpu_frame || !gpu_frame->gpu_ptr) continue;
        if (region_id < 0 || region_id >= static_cast<int>(textures_.size())) continue;
        DxgiTexture& tex = textures_[region_id];
        const uintptr_t src_ptr = reinterpret_cast<uintptr_t>(gpu_frame->gpu_ptr);
        if (tex.last_uploaded_ptr == src_ptr && tex.last_uploaded_pts == gpu_frame->pts) continue;
        (void)updateTextureFromGpu(region_id, gpu_frame, false);
    }
}

void DxgiDisplay::renderTexture(int region_id, int x, int y, int width, int height) {
    if (region_id < 0 || region_id >= static_cast<int>(textures_.size())) return;
    
    DxgiTexture& tex = textures_[region_id];
    if (!tex.srv) return;

    // 计算NDC坐标
    float left   = (static_cast<float>(x) / window_width_) * 2.0f - 1.0f;
    float right  = (static_cast<float>(x + width) / window_width_) * 2.0f - 1.0f;
    float top    = 1.0f - (static_cast<float>(y) / window_height_) * 2.0f;
    float bottom = 1.0f - (static_cast<float>(y + height) / window_height_) * 2.0f;

    Vertex verts[4] = {
        { { left,  top    }, { 0.0f, 0.0f } },
        { { right, top    }, { 1.0f, 0.0f } },
        { { left,  bottom }, { 0.0f, 1.0f } },
        { { right, bottom }, { 1.0f, 1.0f } },
    };

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = d3d_context_->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr)) {
        memcpy(mapped.pData, verts, sizeof(verts));
        d3d_context_->Unmap(vertex_buffer_, 0);
    }

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    d3d_context_->IASetInputLayout(input_layout_);
    d3d_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
    d3d_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    d3d_context_->VSSetShader(vertex_shader_, nullptr, 0);
    d3d_context_->PSSetShader(pixel_shader_, nullptr, 0);
    d3d_context_->PSSetShaderResources(0, 1, &tex.srv);
    d3d_context_->PSSetSamplers(0, 1, &sampler_state_);

    d3d_context_->Draw(4, 0);

    // 画边框（白色）
    d3d_context_->PSSetShader(color_pixel_shader_, nullptr, 0);
    ID3D11ShaderResourceView* null_srv = nullptr;
    d3d_context_->PSSetShaderResources(0, 1, &null_srv);
    if (color_constant_buffer_) {
        D3D11_MAPPED_SUBRESOURCE cb_mapped = {};
        if (SUCCEEDED(d3d_context_->Map(color_constant_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb_mapped))) {
            ColorCB cb{};
            cb.color[0] = 1.0f; cb.color[1] = 1.0f; cb.color[2] = 1.0f; cb.color[3] = 1.0f;
            memcpy(cb_mapped.pData, &cb, sizeof(cb));
            d3d_context_->Unmap(color_constant_buffer_, 0);
        }
        d3d_context_->PSSetConstantBuffers(0, 1, &color_constant_buffer_);
    }

    Vertex border[5] = {
        { { left,  top    }, { 0.0f, 0.0f } },
        { { right, top    }, { 0.0f, 0.0f } },
        { { right, bottom }, { 0.0f, 0.0f } },
        { { left,  bottom }, { 0.0f, 0.0f } },
        { { left,  top    }, { 0.0f, 0.0f } },
    };
    if (SUCCEEDED(d3d_context_->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, border, sizeof(border));
        d3d_context_->Unmap(vertex_buffer_, 0);
    }
    d3d_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    d3d_context_->Draw(5, 0);
}

#endif // _WIN32


