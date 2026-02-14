#include "CudaVideoProcessor.h"
#include <stdio.h>
#include <string.h>

namespace cuda {

// ============================================================================
// CUDA Kernels
// ============================================================================

/**
 * @brief NV12格式视频合并Kernel
 * @param inputs 输入视频帧数组
 * @param numInputs 输入数量
 * @param output 输出视频帧
 * @param layout 合并布局
 */
__global__ void mergeNV12Kernel(const uint8_t** inputs, int numInputs,
                                const int* inputWidths, const int* inputHeights,
                                const int* inputPitches,
                                uint8_t* output, int outputWidth, int outputHeight,
                                int outputPitch, int rows, int cols,
                                int cellWidth, int cellHeight, bool isUVPlane) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (isUVPlane) {
        // UV平面处理（高度减半）
        if (x >= outputWidth || y >= outputHeight / 2) return;
    } else {
        // Y平面处理
        if (x >= outputWidth || y >= outputHeight) return;
    }
    
    // 计算当前像素属于哪个输入视频
    int cellX = x / cellWidth;
    int cellY = y / cellHeight;
    int inputIdx = cellY * cols + cellX;
    
    if (inputIdx >= numInputs) {
        // 超出输入数量，填充黑色
        if (isUVPlane) {
            output[y * outputPitch + x] = 128;  // UV中性值
        } else {
            output[y * outputPitch + x] = 0;    // Y黑色
        }
        return;
    }
    
    // 计算在输入视频中的相对坐标
    int localX = x - cellX * cellWidth;
    int localY = y - cellY * cellHeight;
    
    if (isUVPlane) {
        localY = localY / 2;  // UV平面高度减半
    }
    
    // 边界检查
    if (localX >= inputWidths[inputIdx] || localY >= (isUVPlane ? inputHeights[inputIdx] / 2 : inputHeights[inputIdx])) {
        if (isUVPlane) {
            output[y * outputPitch + x] = 128;
        } else {
            output[y * outputPitch + x] = 0;
        }
        return;
    }
    
    // 从输入拷贝像素
    const uint8_t* inputData = inputs[inputIdx];
    if (isUVPlane) {
        // UV平面偏移
        inputData += inputHeights[inputIdx] * inputPitches[inputIdx];
        output[y * outputPitch + x] = inputData[localY * inputPitches[inputIdx] + localX];
    } else {
        // Y平面
        output[y * outputPitch + x] = inputData[localY * inputPitches[inputIdx] + localX];
    }
}


/**
 * @brief NV12格式视频裁剪Kernel
 */
__global__ void cropNV12Kernel(const uint8_t* input, int inputWidth, int inputHeight,
                               int inputPitch, uint8_t* output, int outputWidth,
                               int outputHeight, int outputPitch, int roiX, int roiY,
                               bool isUVPlane) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (isUVPlane) {
        if (x >= outputWidth || y >= outputHeight / 2) return;
        
        // UV平面偏移
        const uint8_t* inputUV = input + inputHeight * inputPitch;
        uint8_t* outputUV = output + outputHeight * outputPitch;
        
        int srcX = roiX + x;
        int srcY = roiY / 2 + y;
        
        if (srcX < inputWidth && srcY < inputHeight / 2) {
            outputUV[y * outputPitch + x] = inputUV[srcY * inputPitch + srcX];
        } else {
            outputUV[y * outputPitch + x] = 128;  // UV中性值
        }
    } else {
        if (x >= outputWidth || y >= outputHeight) return;
        
        int srcX = roiX + x;
        int srcY = roiY + y;
        
        if (srcX < inputWidth && srcY < inputHeight) {
            output[y * outputPitch + x] = input[srcY * inputPitch + srcX];
        } else {
            output[y * outputPitch + x] = 0;  // Y黑色
        }
    }
}

/**
 * @brief NV12格式视频缩放Kernel（双线性插值）
 */
__global__ void scaleNV12BilinearKernel(const uint8_t* input, int inputWidth,
                                        int inputHeight, int inputPitch,
                                        uint8_t* output, int outputWidth,
                                        int outputHeight, int outputPitch,
                                        bool isUVPlane) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    int actualHeight = isUVPlane ? outputHeight / 2 : outputHeight;
    if (x >= outputWidth || y >= actualHeight) return;
    
    // 计算缩放比例
    float scaleX = (float)inputWidth / outputWidth;
    float scaleY = (float)(isUVPlane ? inputHeight / 2 : inputHeight) / actualHeight;
    
    // 计算源坐标
    float srcX = (x + 0.5f) * scaleX - 0.5f;
    float srcY = (y + 0.5f) * scaleY - 0.5f;
    
    // 边界处理
    srcX = fmaxf(0.0f, fminf(srcX, inputWidth - 1.0f));
    srcY = fmaxf(0.0f, fminf(srcY, (isUVPlane ? inputHeight / 2 : inputHeight) - 1.0f));
    
    int x0 = (int)srcX;
    int y0 = (int)srcY;
    int x1 = min(x0 + 1, inputWidth - 1);
    int y1 = min(y0 + 1, (isUVPlane ? inputHeight / 2 : inputHeight) - 1);
    
    float dx = srcX - x0;
    float dy = srcY - y0;
    
    const uint8_t* inputData = input;
    uint8_t* outputData = output;
    
    if (isUVPlane) {
        inputData += inputHeight * inputPitch;
        outputData += outputHeight * outputPitch;
    }
    
    // 双线性插值
    float v00 = inputData[y0 * inputPitch + x0];
    float v10 = inputData[y0 * inputPitch + x1];
    float v01 = inputData[y1 * inputPitch + x0];
    float v11 = inputData[y1 * inputPitch + x1];
    
    float value = (1 - dx) * (1 - dy) * v00 +
                  dx * (1 - dy) * v10 +
                  (1 - dx) * dy * v01 +
                  dx * dy * v11;
    
    outputData[y * outputPitch + x] = (uint8_t)(value + 0.5f);
}


// ============================================================================
// CudaVideoProcessor Implementation
// ============================================================================

CudaVideoProcessor::CudaVideoProcessor()
    : m_deviceId(0), m_initialized(false) {
}

CudaVideoProcessor::~CudaVideoProcessor() {
    release();
}

bool CudaVideoProcessor::initialize(int deviceId) {
    if (m_initialized) {
        return true;
    }
    
    m_deviceId = deviceId;
    
    // 设置CUDA设备
    cudaError_t err = cudaSetDevice(m_deviceId);
    if (!checkCudaError(err, "Failed to set CUDA device")) {
        return false;
    }
    
    // 获取设备属性
    cudaDeviceProp prop;
    err = cudaGetDeviceProperties(&prop, m_deviceId);
    if (!checkCudaError(err, "Failed to get device properties")) {
        return false;
    }
    
    printf("CUDA Device: %s\n", prop.name);
    printf("Compute Capability: %d.%d\n", prop.major, prop.minor);
    printf("Total Global Memory: %.2f GB\n", prop.totalGlobalMem / 1024.0 / 1024.0 / 1024.0);
    
    m_initialized = true;
    return true;
}

void CudaVideoProcessor::release() {
    if (m_initialized) {
        cudaDeviceReset();
        m_initialized = false;
    }
}

bool CudaVideoProcessor::mergeVideos(const VideoFrame* inputs, int numInputs,
                                     VideoFrame& output, const MergeLayout& layout,
                                     cudaStream_t stream) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    if (numInputs <= 0 || numInputs > layout.rows * layout.cols) {
        setError("Invalid number of inputs");
        return false;
    }
    
    // 准备输入数据
    std::vector<const uint8_t*> h_inputs(numInputs);
    std::vector<int> h_widths(numInputs);
    std::vector<int> h_heights(numInputs);
    std::vector<int> h_pitches(numInputs);
    
    for (int i = 0; i < numInputs; i++) {
        h_inputs[i] = inputs[i].d_data;
        h_widths[i] = inputs[i].width;
        h_heights[i] = inputs[i].height;
        h_pitches[i] = inputs[i].pitch;
    }
    
    // 拷贝到GPU
    const uint8_t** d_inputs;
    int* d_widths;
    int* d_heights;
    int* d_pitches;
    
    cudaMalloc(&d_inputs, numInputs * sizeof(uint8_t*));
    cudaMalloc(&d_widths, numInputs * sizeof(int));
    cudaMalloc(&d_heights, numInputs * sizeof(int));
    cudaMalloc(&d_pitches, numInputs * sizeof(int));
    
    cudaMemcpy(d_inputs, h_inputs.data(), numInputs * sizeof(uint8_t*), cudaMemcpyHostToDevice);
    cudaMemcpy(d_widths, h_widths.data(), numInputs * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_heights, h_heights.data(), numInputs * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_pitches, h_pitches.data(), numInputs * sizeof(int), cudaMemcpyHostToDevice);
    
    // 配置线程块和网格
    dim3 blockSize(16, 16);
    dim3 gridSize((output.width + blockSize.x - 1) / blockSize.x,
                  (output.height + blockSize.y - 1) / blockSize.y);
    
    // 处理Y平面
    mergeNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        d_inputs, numInputs, d_widths, d_heights, d_pitches,
        output.d_data, output.width, output.height, output.pitch,
        layout.rows, layout.cols, layout.cellWidth, layout.cellHeight, false);
    
    // 处理UV平面
    mergeNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        d_inputs, numInputs, d_widths, d_heights, d_pitches,
        output.d_data, output.width, output.height, output.pitch,
        layout.rows, layout.cols, layout.cellWidth, layout.cellHeight, true);
    
    // 同步
    cudaError_t err = cudaStreamSynchronize(stream);
    
    // 释放临时内存
    cudaFree(d_inputs);
    cudaFree(d_widths);
    cudaFree(d_heights);
    cudaFree(d_pitches);
    
    return checkCudaError(err, "Merge kernel failed");
}


bool CudaVideoProcessor::cropVideo(const VideoFrame& input, VideoFrame& output,
                                   const ROI& roi, cudaStream_t stream) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    // 边界检查
    if (roi.x < 0 || roi.y < 0 ||
        roi.x + roi.width > input.width ||
        roi.y + roi.height > input.height) {
        setError("ROI out of bounds");
        return false;
    }
    
    // 对齐检查（UV平面要求偶数对齐）
    if (roi.x % 2 != 0 || roi.y % 2 != 0 ||
        roi.width % 2 != 0 || roi.height % 2 != 0) {
        setError("ROI must be even-aligned for NV12 format");
        return false;
    }
    
    // 配置线程块和网格
    dim3 blockSize(16, 16);
    dim3 gridSize((output.width + blockSize.x - 1) / blockSize.x,
                  (output.height + blockSize.y - 1) / blockSize.y);
    
    // 处理Y平面
    cropNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch,
        roi.x, roi.y, false);
    
    // 处理UV平面
    cropNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch,
        roi.x, roi.y, true);
    
    cudaError_t err = cudaStreamSynchronize(stream);
    return checkCudaError(err, "Crop kernel failed");
}

bool CudaVideoProcessor::scaleVideo(const VideoFrame& input, VideoFrame& output,
                                    InterpolationMethod method, cudaStream_t stream) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    // 配置线程块和网格
    dim3 blockSize(16, 16);
    dim3 gridSize((output.width + blockSize.x - 1) / blockSize.x,
                  (output.height + blockSize.y - 1) / blockSize.y);
    
    // 目前只实现双线性插值
    if (method != InterpolationMethod::BILINEAR) {
        setError("Only bilinear interpolation is currently supported");
        return false;
    }
    
    // 处理Y平面
    scaleNV12BilinearKernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch, false);
    
    // 处理UV平面
    scaleNV12BilinearKernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch, true);
    
    cudaError_t err = cudaStreamSynchronize(stream);
    return checkCudaError(err, "Scale kernel failed");
}

bool CudaVideoProcessor::allocateFrame(VideoFrame& frame) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    // 计算NV12格式所需内存大小
    size_t ySize = frame.height * frame.pitch;
    size_t uvSize = (frame.height / 2) * frame.pitch;
    size_t totalSize = ySize + uvSize;
    
    cudaError_t err = cudaMalloc(&frame.d_data, totalSize);
    if (!checkCudaError(err, "Failed to allocate frame memory")) {
        return false;
    }
    
    // 初始化为黑色
    cudaMemset(frame.d_data, 0, ySize);
    cudaMemset(frame.d_data + ySize, 128, uvSize);
    
    return true;
}

void CudaVideoProcessor::freeFrame(VideoFrame& frame) {
    if (frame.d_data) {
        cudaFree(frame.d_data);
        frame.d_data = nullptr;
    }
}

void CudaVideoProcessor::setError(const char* error) {
    m_lastError = error;
    fprintf(stderr, "CudaVideoProcessor Error: %s\n", error);
}

bool CudaVideoProcessor::checkCudaError(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s: %s", msg, cudaGetErrorString(err));
        setError(buffer);
        return false;
    }
    return true;
}

} // namespace cuda
