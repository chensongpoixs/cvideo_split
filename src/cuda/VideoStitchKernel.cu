/**
 * @file VideoStitchKernel.cu
 * @author chensong
 * @date 2026-01-11
 * @brief 视频拼接 CUDA Kernel 实现（Video Stitching CUDA Kernels）
 * 
 * 该文件实现多路 NV12 视频帧的 GPU 加速拼接，支持：
 * - 任意分辨率输入到目标分辨率的缩放
 * - 双线性插值 Y 平面
 * - 最近邻插值 UV 平面
 * - 输出缓冲区清空（黑色/灰色背景）
 * 
 * 拼接布局示例（Stitching Layout Example）：
 * 
 *   输入帧（不同分辨率）:
 *   ┌─────────┐  ┌───────┐  ┌───┐  ┌─────────────┐
 *   │1920×1080│  │1280×720│  │640│  │ 3840×2160  │
 *   │         │  │       │  │×480│  │            │
 *   └─────────┘  └───────┘  └───┘  └─────────────┘
 *        │           │        │          │
 *        ▼           ▼        ▼          ▼
 *   ┌──────────────────────────────────────────┐
 *   │             输出帧 1920×1080             │
 *   │  ┌────────────┬────────────┐             │
 *   │  │  Stream 0  │  Stream 1  │             │
 *   │  │  960×540   │  960×540   │             │
 *   │  ├────────────┼────────────┤             │
 *   │  │  Stream 2  │  Stream 3  │             │
 *   │  │  960×540   │  960×540   │             │
 *   │  └────────────┴────────────┘             │
 *   └──────────────────────────────────────────┘
 * 
 * NV12 缩放算法（NV12 Scaling Algorithm）：
 * 
 *   Y 平面（双线性插值）：
 *   ┌───┬───┐
 *   │y00│y01│  weighted_y = (1-fx)(1-fy)×y00 + fx(1-fy)×y01
 *   ├───┼───┤                + (1-fx)fy×y10 + fx×fy×y11
 *   │y10│y11│
 *   └───┴───┘
 *   
 *   UV 平面（最近邻插值）：
 *   直接复制最近的 UV 对
 * 
 * @see CudaVideoStitcher 拼接器封装
 */

#include <cuda_runtime.h>
#include <device_launch_parameters.h>

/**
 * @brief NV12 到 NV12 缩放 kernel
 * 
 * 将输入 NV12 帧缩放后写入输出帧的指定位置。
 * 
 * @param input 输入 NV12 数据（GPU 内存）
 * @param input_width 输入宽度
 * @param input_height 输入高度
 * @param output 输出 NV12 数据（GPU 内存）
 * @param output_width 输出总宽度
 * @param output_height 输出总高度
 * @param offset_x 目标区域 X 偏移
 * @param offset_y 目标区域 Y 偏移
 * @param target_width 目标区域宽度
 * @param target_height 目标区域高度
 */
__global__ void nv12ToNv12ResizeKernel(
    const uint8_t* input, int input_width, int input_height,
    uint8_t* output, int output_width, int output_height,
    int offset_x, int offset_y, int target_width, int target_height) {

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // 边界检查：目标区域
    if (x >= target_width || y >= target_height) return;

    // 计算输出位置
    int out_x = offset_x + x;
    int out_y = offset_y + y;

    // 边界检查：输出缓冲区
    if (out_x >= output_width || out_y >= output_height) return;

    // 计算缩放因子
    float scale_x = (float)input_width / target_width;
    float scale_y = (float)input_height / target_height;

    // 双线性插值坐标
    float in_x = x * scale_x;
    float in_y = y * scale_y;

    // 四个最近邻像素坐标
    int in_x0 = (int)in_x;
    int in_y0 = (int)in_y;
    int in_x1 = min(in_x0 + 1, input_width - 1);
    int in_y1 = min(in_y0 + 1, input_height - 1);

    // 插值权重
    float fx = in_x - in_x0;
    float fy = in_y - in_y0;

    // ===== Y 平面处理（双线性插值）=====
    int y_size = input_width * input_height;
    int input_y_offset = in_y0 * input_width + in_x0;
    int output_y_offset = out_y * output_width + out_x;

    // 获取四个邻域 Y 值
    uint8_t y00 = input[input_y_offset];
    uint8_t y01 = input[in_y0 * input_width + in_x1];
    uint8_t y10 = input[in_y1 * input_width + in_x0];
    uint8_t y11 = input[in_y1 * input_width + in_x1];

    // 双线性插值
    float y_val = (1 - fx) * (1 - fy) * y00 +
                  fx * (1 - fy) * y01 +
                  (1 - fx) * fy * y10 +
                  fx * fy * y11;

    // 写入 Y
    output[output_y_offset] = (uint8_t)y_val;

    // ===== UV 平面处理（最近邻插值）=====
    // NV12 格式：UV 是 Y 的一半高度，每 2×2 像素共享一组
    if (x % 2 == 0 && y % 2 == 0) {
        int uv_input_width = input_width;
        int uv_input_height = input_height / 2;
        int uv_output_width = output_width;
        int uv_output_height = output_height / 2;

        // UV 输入坐标（偶数对齐）
        int uv_in_x = in_x0 & ~1;
        int uv_in_y = in_y0 / 2;

        // UV 输出坐标（偶数对齐）
        int uv_out_x = out_x & ~1;
        int uv_out_y = out_y / 2;

        // 边界检查：确保 +1 不越界
        if (uv_out_x + 1 < uv_output_width && uv_out_y < uv_output_height &&
            uv_in_x + 1 < uv_input_width && uv_in_y < uv_input_height) {

            int uv_input_offset = y_size + uv_in_y * uv_input_width + uv_in_x;
            int uv_output_offset = output_width * output_height + uv_out_y * uv_output_width + uv_out_x;

            // 最近邻插值：直接复制 UV 对
            output[uv_output_offset] = input[uv_input_offset];         // U
            output[uv_output_offset + 1] = input[uv_input_offset + 1]; // V
        }
    }
}

/**
 * @brief 清空输出缓冲区 kernel
 * 
 * 将 NV12 输出缓冲区填充为指定颜色（通常黑色或灰色）。
 * 
 * @param output 输出缓冲区
 * @param width 宽度
 * @param height 高度
 * @param y_value Y 填充值（16=黑色，128=中灰）
 * @param uv_value UV 填充值（128=无色度）
 */
__global__ void clearOutputKernel(uint8_t* output, int width, int height, uint8_t y_value, uint8_t uv_value) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    int y_size = width * height;
    int offset = y * width + x;

    // 清空 Y 平面
    if (y < height) {
        output[offset] = y_value;
    }

    // 清空 UV 平面（NV12: 每 2 个水平像素对应一组 UV）
    if (y < height / 2 && (x % 2 == 0) && (x + 1 < width)) {
        int uv_offset = y_size + y * width + x;
        output[uv_offset] = uv_value;      // U
        output[uv_offset + 1] = uv_value;  // V
    }
}

// ===== C 接口导出 =====
extern "C" {

/**
 * @brief 启动 NV12 缩放 kernel
 * 
 * @param d_input 输入 NV12 数据（GPU 内存）
 * @param input_width 输入宽度
 * @param input_height 输入高度
 * @param d_output 输出 NV12 数据（GPU 内存）
 * @param output_width 输出总宽度
 * @param output_height 输出总高度
 * @param offset_x 目标区域 X 偏移
 * @param offset_y 目标区域 Y 偏移
 * @param target_width 目标区域宽度
 * @param target_height 目标区域高度
 * @param stream CUDA stream
 */
void launchNv12Resize(
    const uint8_t* d_input, int input_width, int input_height,
    uint8_t* d_output, int output_width, int output_height,
    int offset_x, int offset_y, int target_width, int target_height,
    cudaStream_t stream) {

    dim3 block(16, 16);
    dim3 grid((target_width + block.x - 1) / block.x, (target_height + block.y - 1) / block.y);

    nv12ToNv12ResizeKernel<<<grid, block, 0, stream>>>(
        d_input, input_width, input_height,
        d_output, output_width, output_height,
        offset_x, offset_y, target_width, target_height);
}

/**
 * @brief 启动清空输出 kernel
 * 
 * @param d_output 输出缓冲区
 * @param width 宽度
 * @param height 高度
 * @param y_value Y 填充值
 * @param uv_value UV 填充值
 * @param stream CUDA stream
 */
void launchClearOutput(uint8_t* d_output, int width, int height, uint8_t y_value, uint8_t uv_value, cudaStream_t stream) {
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);

    clearOutputKernel<<<grid, block, 0, stream>>>(d_output, width, height, y_value, uv_value);
}

}
