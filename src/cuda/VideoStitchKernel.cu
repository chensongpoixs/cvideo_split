#include <cuda_runtime.h>
#include <device_launch_parameters.h>

__global__ void nv12ToNv12ResizeKernel(
    const uint8_t* input, int input_width, int input_height,
    uint8_t* output, int output_width, int output_height,
    int offset_x, int offset_y, int target_width, int target_height) {

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= target_width || y >= target_height) return;

    // 计算输出位置
    int out_x = offset_x + x;
    int out_y = offset_y + y;

    if (out_x >= output_width || out_y >= output_height) return;

    // 计算输入位置（线性插值缩放）
    float scale_x = (float)input_width / target_width;
    float scale_y = (float)input_height / target_height;

    float in_x = x * scale_x;
    float in_y = y * scale_y;

    int in_x0 = (int)in_x;
    int in_y0 = (int)in_y;
    int in_x1 = min(in_x0 + 1, input_width - 1);
    int in_y1 = min(in_y0 + 1, input_height - 1);

    float fx = in_x - in_x0;
    float fy = in_y - in_y0;

    // Y平面处理
    int y_size = input_width * input_height;
    int input_y_offset = in_y0 * input_width + in_x0;
    int output_y_offset = out_y * output_width + out_x;

    uint8_t y00 = input[input_y_offset];
    uint8_t y01 = input[in_y0 * input_width + in_x1];
    uint8_t y10 = input[in_y1 * input_width + in_x0];
    uint8_t y11 = input[in_y1 * input_width + in_x1];

    // 双线性插值
    float y_val = (1 - fx) * (1 - fy) * y00 +
                  fx * (1 - fy) * y01 +
                  (1 - fx) * fy * y10 +
                  fx * fy * y11;

    output[output_y_offset] = (uint8_t)y_val;

    // UV平面处理 (NV12格式，UV是Y的一半)
    if (x % 2 == 0 && y % 2 == 0) {
        int uv_input_width = input_width;
        int uv_input_height = input_height / 2;
        int uv_output_width = output_width;
        int uv_output_height = output_height / 2;

        // UV输入位置
        int uv_in_x = in_x0 & ~1; // 偶数对齐
        int uv_in_y = in_y0 / 2;

        // UV输出位置
        int uv_out_x = out_x & ~1;
        int uv_out_y = out_y / 2;

        // 写入UV需要保证 +1 不越界
        if (uv_out_x + 1 < uv_output_width && uv_out_y < uv_output_height &&
            uv_in_x + 1 < uv_input_width && uv_in_y < uv_input_height) {
            int uv_input_offset = y_size + uv_in_y * uv_input_width + uv_in_x;
            int uv_output_offset = output_width * output_height + uv_out_y * uv_output_width + uv_out_x;

            // 最近邻插值用于UV
            output[uv_output_offset] = input[uv_input_offset];
            output[uv_output_offset + 1] = input[uv_input_offset + 1];
        }
    }
}

__global__ void clearOutputKernel(uint8_t* output, int width, int height, uint8_t y_value, uint8_t uv_value) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    int y_size = width * height;
    int offset = y * width + x;

    if (y < height) {
        output[offset] = y_value;
    }

    // UV平面（NV12: 每2个水平像素对应一组UV，避免 uv_offset+1 越界）
    if (y < height / 2 && (x % 2 == 0) && (x + 1 < width)) {
        int uv_offset = y_size + y * width + x;
        output[uv_offset] = uv_value;
        output[uv_offset + 1] = uv_value;
    }
}

extern "C" {

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

void launchClearOutput(uint8_t* d_output, int width, int height, uint8_t y_value, uint8_t uv_value, cudaStream_t stream) {
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);

    clearOutputKernel<<<grid, block, 0, stream>>>(d_output, width, height, y_value, uv_value);
}

}
