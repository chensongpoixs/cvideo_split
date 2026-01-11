/**
 * @file Nv12ToRgbaKernel.cu
 * @author chensong
 * @date 2026-01-11
 * @brief NV12 到 RGBA 转换 CUDA Kernel 实现
 * 
 * 该文件实现 NV12 到 RGBA 的 GPU 加速转换，支持缩放和中心裁剪填充。
 * 用于 CUDA-D3D11 互操作的零拷贝显示渲染。
 * 
 * Kernel 实现细节（Kernel Implementation Details）：
 * 
 *   线程布局:
 *   - Block: 16×16 threads
 *   - Grid: ceil(dst_w/16) × ceil(dst_h/16) blocks
 *   - 每个线程处理一个目标像素
 * 
 *   计算流程:
 *   1. 根据中心裁剪计算源坐标 (src_fx, src_fy)
 *   2. 最近邻采样获取 Y 值
 *   3. 计算 UV 坐标（2×2 子采样）
 *   4. 获取 U、V 值
 *   5. YUV → RGB 转换
 *   6. 使用 surf2Dwrite 写入 RGBA
 * 
 * @see Nv12ToRgbaKernel.h 接口声明
 */

#include "Nv12ToRgbaKernel.h"

#include <cuda_runtime.h>

/**
 * @brief 将整数值 clamp 到 [0, 255] 范围
 * @param v 输入值
 * @return unsigned char clamp 后的值
 */
static __device__ __forceinline__ unsigned char clamp_u8(int v) {
    return static_cast<unsigned char>(v < 0 ? 0 : (v > 255 ? 255 : v));
}

/**
 * @brief YUV 到 RGBA 转换
 * 
 * 使用 BT.601 full-range 近似公式：
 *   R = Y + 1.402 × (V - 128)
 *   G = Y - 0.344 × (U - 128) - 0.714 × (V - 128)
 *   B = Y + 1.772 × (U - 128)
 * 
 * @param y Y 分量 (亮度)
 * @param u U 分量 (蓝色色度)
 * @param v V 分量 (红色色度)
 * @return uchar4 RGBA 像素值 (A = 255)
 */
static __device__ __forceinline__ uchar4 yuvToRgba(unsigned char y, unsigned char u, unsigned char v) {
    int Y = static_cast<int>(y);
    int U = static_cast<int>(u) - 128;
    int V = static_cast<int>(v) - 128;

    int r = Y + static_cast<int>(1.402f * V);
    int g = Y - static_cast<int>(0.344f * U + 0.714f * V);
    int b = Y + static_cast<int>(1.772f * U);

    uchar4 out;
    out.x = clamp_u8(r);  // R
    out.y = clamp_u8(g);  // G
    out.z = clamp_u8(b);  // B
    out.w = 255;          // A (不透明)
    return out;
}

/**
 * @brief NV12 到 RGBA 缩放转换 kernel
 * 
 * 每个线程处理目标图像中的一个像素。
 * 
 * @param nv12 源 NV12 数据
 * @param src_w 源宽度
 * @param src_h 源高度
 * @param dst_surf 目标 surface object
 * @param dst_w 目标宽度
 * @param dst_h 目标高度
 * @param center_crop_fill 是否中心裁剪填充
 */
static __global__ void nv12ToRgbaResizeKernel(
    const uint8_t* __restrict__ nv12,
    int src_w,
    int src_h,
    cudaSurfaceObject_t dst_surf,
    int dst_w,
    int dst_h,
    bool center_crop_fill)
{
    int x = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    int y = static_cast<int>(blockIdx.y * blockDim.y + threadIdx.y);
    if (x >= dst_w || y >= dst_h) return;

    // 计算缩放因子
    // center_crop_fill: 使用较大缩放因子，填满目标区域
    // !center_crop_fill: 使用较小缩放因子，保持宽高比
    float scale_x = static_cast<float>(dst_w) / static_cast<float>(src_w);
    float scale_y = static_cast<float>(dst_h) / static_cast<float>(src_h);
    float scale = center_crop_fill ? (scale_x > scale_y ? scale_x : scale_y)
                                   : (scale_x < scale_y ? scale_x : scale_y);

    // 计算缩放后图像在目标区域中的偏移
    float scaled_w = static_cast<float>(src_w) * scale;
    float scaled_h = static_cast<float>(src_h) * scale;
    float off_x = (static_cast<float>(dst_w) - scaled_w) * 0.5f;
    float off_y = (static_cast<float>(dst_h) - scaled_h) * 0.5f;

    // 反向映射到源图像坐标
    float src_fx = (static_cast<float>(x) - off_x) / scale;
    float src_fy = (static_cast<float>(y) - off_y) / scale;

    // 最近邻采样（可改为双线性插值以提高质量）
    int sx = static_cast<int>(src_fx + 0.5f);
    int sy = static_cast<int>(src_fy + 0.5f);
    if (sx < 0) sx = 0;
    if (sy < 0) sy = 0;
    if (sx >= src_w) sx = src_w - 1;
    if (sy >= src_h) sy = src_h - 1;

    // 获取 Y 值
    const uint8_t* y_plane = nv12;
    const uint8_t* uv_plane = nv12 + static_cast<size_t>(src_w) * src_h;

    unsigned char Y = y_plane[sy * src_w + sx];

    // 获取 UV 值（NV12 格式：UV 交替存储，每 2×2 像素共享一组）
    int uv_x = (sx / 2) * 2;  // 对齐到偶数
    int uv_y = (sy / 2);
    unsigned char U = uv_plane[uv_y * src_w + uv_x + 0];
    unsigned char V = uv_plane[uv_y * src_w + uv_x + 1];

    // YUV → RGBA 转换
    uchar4 rgba = yuvToRgba(Y, U, V);

    // 写入目标 surface（x 以字节为单位）
    surf2Dwrite(rgba, dst_surf, x * static_cast<int>(sizeof(uchar4)), y);
}

bool launchNv12ToRgbaResizeToCudaArray(
    const uint8_t* src_nv12,
    int src_w,
    int src_h,
    cudaArray_t dst_rgba_array,
    int dst_w,
    int dst_h,
    bool center_crop_fill,
    cudaStream_t stream)
{
    // 参数校验
    if (!src_nv12 || !dst_rgba_array) return false;
    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return false;

    // 创建 surface object
    cudaResourceDesc rd{};
    rd.resType = cudaResourceTypeArray;
    rd.res.array.array = dst_rgba_array;

    cudaSurfaceObject_t surf = 0;
    cudaError_t ce = cudaCreateSurfaceObject(&surf, &rd);
    if (ce != cudaSuccess) return false;

    // 配置线程块和网格
    dim3 block(16, 16);
    dim3 grid((dst_w + block.x - 1) / block.x, (dst_h + block.y - 1) / block.y);

    // 启动 kernel
    nv12ToRgbaResizeKernel<<<grid, block, 0, stream>>>(
        src_nv12, src_w, src_h, surf, dst_w, dst_h, center_crop_fill);

    // 检查 kernel 启动错误
    cudaError_t le = cudaGetLastError();

    // 销毁 surface object
    cudaDestroySurfaceObject(surf);

    return le == cudaSuccess;
}
