/**
 * @file Nv12ToRgbaKernel.h
 * @author chensong
 * @date 2026-01-11
 * @brief NV12 到 RGBA 转换 CUDA Kernel 接口（NV12 to RGBA Conversion CUDA Kernel）
 * 
 * 该模块提供高效的 NV12 到 RGBA 颜色空间转换，支持缩放和中心裁剪填充，
 * 用于 CUDA-D3D11 互操作的零拷贝显示渲染。
 * 
 * NV12 格式说明（NV12 Format Description）：
 * 
 *   NV12 是 NVIDIA 硬件解码器 (NVDEC) 的默认输出格式：
 *   
 *   内存布局:
 *   ┌─────────────────────────────────────────────────┐
 *   │                    Y Plane                       │
 *   │  (width × height bytes, 每像素 1 字节亮度)       │
 *   ├─────────────────────────────────────────────────┤
 *   │                   UV Plane                       │
 *   │  (width × height/2 bytes, U 和 V 交替存储)      │
 *   │  每 2×2 像素共享一组 UV                          │
 *   └─────────────────────────────────────────────────┘
 *   
 *   总大小: width × height × 1.5 字节
 * 
 * 中心裁剪填充（Center-Crop Fill）：
 * 
 *   当源和目标宽高比不同时，使用较大的缩放因子，
 *   使源图像完全覆盖目标区域，多余部分裁剪：
 *   
 *   ┌───────────────┐     ┌─────────────────┐
 *   │   Source      │     │     Target      │
 *   │   16:9        │ ──► │     4:3         │
 *   │               │     │   [裁剪左右]    │
 *   └───────────────┘     └─────────────────┘
 * 
 * @note 输出写入 cudaArray（通常来自 D3D11 纹理映射）
 * @note 使用 surface object 进行写入
 * @see DxgiDisplay 零拷贝显示实现
 */

#pragma once

#include <cuda_runtime.h>

/**
 * @brief 将 NV12 转换为 RGBA 并写入 cudaArray
 * 
 * 执行以下操作：
 * 1. 计算缩放因子（中心裁剪或等比缩放）
 * 2. 双线性采样 NV12 数据
 * 3. YUV → RGB 颜色转换（BT.601 近似）
 * 4. 写入 RGBA 到目标 cudaArray
 * 
 * 颜色转换公式（Color Conversion Formula）：
 * @code
 * R = Y + 1.402 × (V - 128)
 * G = Y - 0.344 × (U - 128) - 0.714 × (V - 128)
 * B = Y + 1.772 × (U - 128)
 * @endcode
 * 
 * @param src_nv12 源 NV12 数据（GPU 内存，紧密布局）
 * @param src_w 源图像宽度
 * @param src_h 源图像高度
 * @param dst_rgba_array 目标 RGBA cudaArray（来自 D3D11 纹理映射）
 * @param dst_w 目标图像宽度
 * @param dst_h 目标图像高度
 * @param center_crop_fill true=中心裁剪填充，false=等比缩放留黑边
 * @param stream CUDA stream
 * @return true kernel 启动成功
 * @return false 参数错误或 kernel 启动失败
 * 
 * @note 不进行强制同步，调用者需自行管理同步
 * @note 线程块大小: 16×16
 * 
 * 使用示例：
 * @code
 * cudaGraphicsMapResources(1, &cuda_resource, stream);
 * cudaArray_t array;
 * cudaGraphicsSubResourceGetMappedArray(&array, cuda_resource, 0, 0);
 * 
 * launchNv12ToRgbaResizeToCudaArray(
 *     gpu_frame->gpu_ptr,
 *     gpu_frame->width, gpu_frame->height,
 *     array,
 *     display_width, display_height,
 *     true,  // center-crop fill
 *     stream
 * );
 * 
 * cudaGraphicsUnmapResources(1, &cuda_resource, stream);
 * @endcode
 */
bool launchNv12ToRgbaResizeToCudaArray(
    const uint8_t* src_nv12,
    int src_w,
    int src_h,
    cudaArray_t dst_rgba_array,
    int dst_w,
    int dst_h,
    bool center_crop_fill,
    cudaStream_t stream);
