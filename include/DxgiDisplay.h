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

// Windows DXGI显示实现
class DxgiDisplay : public PlatformDisplay {
public:
    DxgiDisplay();
    ~DxgiDisplay() override;

    bool initialize(int window_width, int window_height, const std::string& window_title,
                   bool fullscreen = false) override;

    int addDisplayRegion(int x, int y, int width, int height, const std::string& title) override;

    bool updateRegionFrame(int region_id, std::shared_ptr<GpuFrame> gpu_frame) override;

    void render() override;

    void processEvents() override;

    bool shouldClose() const override;

    void close() override;

private:
    // 窗口过程
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // DirectX初始化
    bool initDirectX();
    void cleanupDirectX();

    // 纹理管理
    bool createTexture(int region_id, int width, int height);
    void destroyTexture(int region_id);
    bool updateTextureFromGpu(int region_id, std::shared_ptr<GpuFrame> gpu_frame);

    // 渲染
    void renderTexture(int region_id, int x, int y, int width, int height);

    HWND hwnd_ = nullptr;
    HINSTANCE hinstance_ = nullptr;
    bool should_close_ = false;

    // DirectX资源
    ID3D11Device* d3d_device_ = nullptr;
    ID3D11DeviceContext* d3d_context_ = nullptr;
    IDXGISwapChain1* swap_chain_ = nullptr;
    ID3D11RenderTargetView* render_target_view_ = nullptr;
    ID3D11VertexShader* vertex_shader_ = nullptr;
    ID3D11PixelShader* pixel_shader_ = nullptr;
    ID3D11PixelShader* color_pixel_shader_ = nullptr;  // 用于画边框/分割线
    ID3D11InputLayout* input_layout_ = nullptr;
    ID3D11Buffer* vertex_buffer_ = nullptr;
    ID3D11Buffer* color_constant_buffer_ = nullptr;
    ID3D11SamplerState* sampler_state_ = nullptr;

    // 纹理资源（当前先走“NV12从GPU拷到CPU->转RGBA->上传D3D纹理”保证可显示）
    struct DxgiTexture {
        ID3D11Texture2D* texture = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
        int width = 0;
        int height = 0;
    };
    std::vector<DxgiTexture> textures_;
};

#endif // _WIN32

