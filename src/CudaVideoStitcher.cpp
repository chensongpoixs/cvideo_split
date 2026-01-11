/**
 * @file CudaVideoStitcher.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief CUDA 视频拼接器实现（GPU Kernel 调用）
 * @see CudaVideoStitcher.h
 */

#include "CudaVideoStitcher.h"
#include "CudaStreamManager.h"
#include "Logger.h"
#include <iostream>
#include <string>

extern "C" {
void launchNv12Resize(
    const uint8_t* d_input, int input_width, int input_height,
    uint8_t* d_output, int output_width, int output_height,
    int offset_x, int offset_y, int target_width, int target_height,
    cudaStream_t stream);

void launchClearOutput(uint8_t* d_output, int width, int height, uint8_t y_value, uint8_t uv_value, cudaStream_t stream);
}

CudaVideoStitcher::CudaVideoStitcher() : initialized_(false), cuda_stream_(nullptr) {
}

CudaVideoStitcher::~CudaVideoStitcher() {
    cleanup();
}

bool CudaVideoStitcher::initialize(const StitchConfig& config) {
    if (initialized_) return true;

    config_ = config;

    // 固定使用GPU0（当前项目默认单卡）。避免多线程/外部库修改device导致stream/ptr不匹配。
    cudaError_t err = cudaSetDevice(0);
    if (err != cudaSuccess) {
        std::cerr << "Failed to cudaSetDevice(0): " << cudaGetErrorString(err) << std::endl;
        return false;
    }
    // 初始化CUDA runtime（创建primary context等）
    (void)cudaFree(0);

    // 使用统一的 CUDA Stream 管理器
    auto& stream_mgr = CudaStreamManager::getInstance();
    if (!stream_mgr.isInitialized()) {
        LOG_ERROR("CudaStreamManager not initialized before CudaVideoStitcher");
        return false;
    }
    cuda_stream_ = stream_mgr.getStitchStream();
    LOG_INFO("CudaVideoStitcher using shared stitch stream from CudaStreamManager");

    // 分配输出缓冲区
    if (!allocateBuffers()) {
        std::cerr << "Failed to allocate CUDA buffers" << std::endl;
        return false;
    }

    initialized_ = true;
    std::cout << "CudaVideoStitcher initialized: " << config.output_width << "x" << config.output_height << std::endl;
    return true;
}

bool CudaVideoStitcher::allocateBuffers() {
    // 计算输出缓冲区大小 (NV12格式)
    output_buffer_size_ = config_.output_width * config_.output_height * 3 / 2;

    cudaError_t err = cudaMalloc(&d_output_buffer_, output_buffer_size_);
    if (err != cudaSuccess) {
        std::cerr << "Failed to allocate output buffer: " << cudaGetErrorString(err) << std::endl;
        return false;
    }

    // 分配临时缓冲区用于每个输入流
    for (int i = 0; i < config_.num_streams; i++) {
        int width = config_.stream_positions[i].width;
        int height = config_.stream_positions[i].height;
        temp_buffer_size_[i] = width * height * 3 / 2;

        err = cudaMalloc(&d_temp_buffers_[i], temp_buffer_size_[i]);
        if (err != cudaSuccess) {
            std::cerr << "Failed to allocate temp buffer " << i << ": " << cudaGetErrorString(err) << std::endl;
            return false;
        }
    }

    return true;
}

bool CudaVideoStitcher::stitchFrames(const std::vector<CudaFrame>& input_frames, CudaFrame& output_frame) {
    if (!initialized_ || input_frames.size() != (size_t)config_.num_streams) {
        return false;
    }

    // 确保当前线程在同一GPU/device上下文中
    (void)cudaSetDevice(0);
    (void)cudaGetLastError(); // 清理上一次异步错误，避免误报到本次

    // 输出必须指向有效的 GPU NV12 缓冲（优先写到调用者提供的输出帧）
    uint8_t* dst_output = output_frame.nv12_data ? output_frame.nv12_data : d_output_buffer_;
    if (!dst_output) {
        LOG_ERROR("CudaVideoStitcher output buffer is null (output_frame.nv12_data and d_output_buffer_ are both null)");
        return false;
    }

    // 清空输出缓冲区 (黑色背景)
    // 用 cudaMemsetAsync 替代自定义清屏kernel，避免kernel launch 在某些环境下失败并简化问题面。
    const size_t y_size = static_cast<size_t>(config_.output_width) * static_cast<size_t>(config_.output_height);
    const size_t uv_size = y_size / 2;
    cudaError_t clr1 = cudaMemsetAsync(dst_output, 0, y_size, cuda_stream_);
    cudaError_t clr2 = cudaMemsetAsync(dst_output + y_size, 128, uv_size, cuda_stream_);
    if (clr1 != cudaSuccess || clr2 != cudaSuccess) {
        LOG_ERROR("cudaMemsetAsync clear failed: Y=" + std::string(cudaGetErrorString(clr1)) +
                  ", UV=" + std::string(cudaGetErrorString(clr2)));
        return false;
    }

    // 处理每个输入帧
    for (size_t i = 0; i < input_frames.size(); i++) {
        const CudaFrame& input = input_frames[i];

        if (!input.nv12_data || input.width <= 0 || input.height <= 0) {
            continue; // 跳过无效帧
        }

        // 选择输入源（GPU零拷贝 or CPU拷贝到临时缓冲）
        const uint8_t* source_data = nullptr;

        if (input.is_gpu_memory) {
            // 数据已经在GPU上，直接使用
            source_data = input.nv12_data;
            LOG_DEBUG("Using GPU memory directly for stream " + std::to_string(i));
        } else {
            // 数据在CPU上，需要复制到GPU
            cudaError_t err = cudaMemcpyAsync(d_temp_buffers_[i], input.nv12_data,
                                             temp_buffer_size_[i], cudaMemcpyHostToDevice, cuda_stream_);
            if (err != cudaSuccess) {
                std::cerr << "Failed to copy input data to GPU: " << cudaGetErrorString(err) << std::endl;
                continue;
            }
            source_data = d_temp_buffers_[i];
            LOG_DEBUG("Copied CPU data to GPU for stream " + std::to_string(i));
        }

        if (!source_data) {
            LOG_WARNING("Stream " + std::to_string(i) + " source_data is null, skipping");
            continue;
        }

        // 获取目标位置和大小
        int offset_x = config_.stream_positions[i].x;
        int offset_y = config_.stream_positions[i].y;
        int target_width = config_.stream_positions[i].width;
        int target_height = config_.stream_positions[i].height;

        // 启动CUDA内核进行缩放和平移
        launchNv12Resize(
            source_data, input.width, input.height,
            dst_output, config_.output_width, config_.output_height,
            offset_x, offset_y, target_width, target_height,
            cuda_stream_);
        {
            cudaError_t e = cudaGetLastError();
            if (e != cudaSuccess) {
                LOG_ERROR("launchNv12Resize kernel launch failed (stream " + std::to_string(i) + "): " +
                          std::string(cudaGetErrorString(e)));
                return false;
            }
        }
    }

    // 等待所有操作完成
    {
        cudaError_t e = cudaStreamSynchronize(cuda_stream_);
        if (e != cudaSuccess) {
            LOG_ERROR("cudaStreamSynchronize failed in stitchFrames: " + std::string(cudaGetErrorString(e)));
            return false;
        }
    }

    // 设置输出帧
    output_frame.width = config_.output_width;
    output_frame.height = config_.output_height;
    output_frame.stream = cuda_stream_;
    output_frame.is_gpu_memory = true;
    if (!output_frame.nv12_data) {
        // 兼容旧调用方：如果外部没给输出指针，就暴露内部输出缓冲（但更推荐外部提供输出_gpu_frame->gpu_ptr）
        output_frame.nv12_data = d_output_buffer_;
    }

    // 注意：这里我们不复制数据回主机，而是让调用者直接使用GPU缓冲区
    // output_frame.nv12_data 将在后续处理中设置

    return true;
}

void CudaVideoStitcher::cleanup() {
    if (!initialized_) return;

    freeBuffers();

    // 注意：不再销毁 stream，由 CudaStreamManager 统一管理
    cuda_stream_ = nullptr;

    initialized_ = false;
}

void CudaVideoStitcher::freeBuffers() {
    if (d_output_buffer_) {
        cudaFree(d_output_buffer_);
        d_output_buffer_ = nullptr;
    }

    for (int i = 0; i < 4; i++) {
        if (d_temp_buffers_[i]) {
            cudaFree(d_temp_buffers_[i]);
            d_temp_buffers_[i] = nullptr;
        }
    }
}
