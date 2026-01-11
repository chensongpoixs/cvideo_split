/**
 * @file GpuMemoryManager.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief GPU 内存管理器实现
 * @see GpuMemoryManager.h
 */

#include "GpuMemoryManager.h"
#include "Logger.h"
#include <cstring>
#include <string>
#include <chrono>
#include <thread>

bool GpuMemoryManager::initialize() {
    LOG_INFO("Initializing GPU Memory Manager with Ring Buffer");

    // 初始化环形缓冲区
    frame_ring_buffers_.reserve(MAX_STREAMS);
    for (int i = 0; i < MAX_STREAMS; ++i) {
        frame_ring_buffers_.push_back(std::make_unique<FrameRingBuffer>());
    }

    LOG_INFO("GPU Memory Manager initialized successfully with " + 
             std::to_string(MAX_STREAMS) + " ring buffers, capacity: " + 
             std::to_string(RING_BUFFER_CAPACITY - 1) + " frames each");
    return true;
}

std::shared_ptr<GpuFrame> GpuMemoryManager::allocateFrame(int width, int height, bool is_nv12, int stream_id) {
    LOG_DEBUG("Allocating GPU frame: " + std::to_string(width) + "x" + std::to_string(height) +
              ", NV12: " + (is_nv12 ? "true" : "false") + ", stream_id: " + std::to_string(stream_id));

    // 计算需要的内存大小 (NV12格式: Y + UV/2)
    size_t frame_size = width * height * (is_nv12 ? 3 : 2) / 2;  // NV12: 3/2 bytes per pixel

    // 分配GPU内存
    void* gpu_ptr = nullptr;
    cudaError_t cuda_err = cudaMalloc(&gpu_ptr, frame_size);
    if (cuda_err != cudaSuccess) {
        LOG_ERROR("Failed to allocate GPU memory: " + std::string(cudaGetErrorString(cuda_err)));
        return nullptr;
    }

    // 创建GPU帧对象（用shared_ptr自定义deleter：只有最后一个引用释放时才cudaFree）
    auto gpu_frame = std::shared_ptr<GpuFrame>(new GpuFrame(), [](GpuFrame* f) {
        if (!f) return;
        if (f->owns_memory && f->gpu_ptr) {
            cudaFree(f->gpu_ptr);
            f->gpu_ptr = nullptr;
        }
        delete f;
    });
    gpu_frame->gpu_ptr = gpu_ptr;
    gpu_frame->size = frame_size;
    gpu_frame->width = width;
    gpu_frame->height = height;
    gpu_frame->pts = 0;
    gpu_frame->stream_id = stream_id;
    gpu_frame->is_nv12 = is_nv12;
    gpu_frame->ready = false;
    gpu_frame->owns_memory = true;

    // 创建IPC内存句柄（用于进程间共享）
    cuda_err = cudaIpcGetMemHandle(&gpu_frame->mem_handle, gpu_ptr);
    if (cuda_err != cudaSuccess) {
        LOG_WARNING("Failed to create IPC memory handle: " + std::string(cudaGetErrorString(cuda_err)));
        // 不返回nullptr，继续使用
    }

    LOG_INFO("GPU frame allocated successfully, size: " + std::to_string(frame_size) + " bytes");
    return gpu_frame;
}

void GpuMemoryManager::freeFrame(std::shared_ptr<GpuFrame>& frame) {
    if (!frame) return;
    // 重要：不要在这里cudaFree，否则会提前释放仍被显示/其它模块持有的帧。
    // 正确做法：让shared_ptr引用计数归零时由deleter释放GPU内存。
    LOG_DEBUG("Releasing GPU frame reference (deferred free via shared_ptr deleter), size: " +
              std::to_string(frame->size) + " bytes");
    frame.reset();
}

bool GpuMemoryManager::copyToGpu(std::shared_ptr<GpuFrame> gpu_frame, const uint8_t* cpu_data, size_t size) {
    if (!gpu_frame || !gpu_frame->gpu_ptr || !cpu_data) {
        LOG_ERROR("Invalid parameters for GPU copy");
        return false;
    }

    if (size > gpu_frame->size) {
        LOG_ERROR("Data size exceeds GPU buffer size: " + std::to_string(size) + " > " + std::to_string(gpu_frame->size));
        return false;
    }

    cudaError_t cuda_err = cudaMemcpy(gpu_frame->gpu_ptr, cpu_data, size, cudaMemcpyHostToDevice);
    if (cuda_err != cudaSuccess) {
        LOG_ERROR("Failed to copy data to GPU: " + std::string(cudaGetErrorString(cuda_err)));
        return false;
    }

    LOG_DEBUG("Copied " + std::to_string(size) + " bytes to GPU memory");
    return true;
}

bool GpuMemoryManager::copyFromGpu(const std::shared_ptr<GpuFrame> gpu_frame, uint8_t* cpu_data, size_t size) {
    if (!gpu_frame || !gpu_frame->gpu_ptr || !cpu_data) {
        LOG_ERROR("Invalid parameters for GPU copy");
        return false;
    }

    if (size > gpu_frame->size) {
        size = gpu_frame->size;  // 限制大小
    }

    LOG_DEBUG("Starting GPU->CPU copy: " + std::to_string(size) + " bytes from GPU ptr " +
             std::to_string(reinterpret_cast<uintptr_t>(gpu_frame->gpu_ptr)));

    cudaError_t cuda_err = cudaMemcpy(cpu_data, gpu_frame->gpu_ptr, size, cudaMemcpyDeviceToHost);
    if (cuda_err != cudaSuccess) {
        LOG_ERROR("Failed to copy data from GPU: " + std::string(cudaGetErrorString(cuda_err)) +
                 " (size: " + std::to_string(size) + ", gpu_ptr: " +
                 std::to_string(reinterpret_cast<uintptr_t>(gpu_frame->gpu_ptr)) + ")");
        return false;
    }

    LOG_DEBUG("Successfully copied " + std::to_string(size) + " bytes from GPU to CPU");
    return true;
}

bool GpuMemoryManager::copyGpuToGpu(std::shared_ptr<GpuFrame> dst_frame, const std::shared_ptr<GpuFrame> src_frame) {
    if (!dst_frame || !src_frame || !dst_frame->gpu_ptr || !src_frame->gpu_ptr) {
        LOG_ERROR("Invalid parameters for GPU-to-GPU copy");
        return false;
    }

    size_t copy_size = std::min(dst_frame->size, src_frame->size);
    cudaError_t cuda_err = cudaMemcpy(dst_frame->gpu_ptr, src_frame->gpu_ptr, copy_size, cudaMemcpyDeviceToDevice);
    if (cuda_err != cudaSuccess) {
        LOG_ERROR("Failed to copy GPU-to-GPU: " + std::string(cudaGetErrorString(cuda_err)));
        return false;
    }

    // 复制元数据
    dst_frame->width = src_frame->width;
    dst_frame->height = src_frame->height;
    dst_frame->pts = src_frame->pts;
    dst_frame->stream_id = src_frame->stream_id;
    dst_frame->is_nv12 = src_frame->is_nv12;

    LOG_DEBUG("GPU-to-GPU copy completed, size: " + std::to_string(copy_size) + " bytes");
    return true;
}

bool GpuMemoryManager::waitFrameReady(std::shared_ptr<GpuFrame> frame, int timeout_ms) {
    if (!frame) return false;

    // 简单的轮询等待（实际应用中可能需要更复杂的同步机制）
    auto start_time = std::chrono::steady_clock::now();
    while (!frame->ready.load()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time).count();
        if (elapsed > timeout_ms) {
            LOG_WARNING("Timeout waiting for frame to be ready");
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return true;
}

void GpuMemoryManager::setFrameReady(std::shared_ptr<GpuFrame> frame, bool ready) {
    if (frame) {
        frame->ready.store(ready);
        LOG_DEBUG("Frame ready status set to: " + std::string(ready ? "true" : "false"));
    }
}

bool GpuMemoryManager::pushFrameToQueue(int stream_id, std::shared_ptr<GpuFrame> frame) {
    int array_index = mapStreamIdToIndex(stream_id);
    if (array_index < 0 || array_index >= MAX_STREAMS || !frame) {
        LOG_ERROR("Invalid stream ID or frame for queue push: " + std::to_string(stream_id));
        return false;
    }

    // 尝试推入环形缓冲区（非阻塞）
    if (!frame_ring_buffers_[array_index]->push(std::move(frame), false)) {
        // 缓冲区已满，弹出最老的帧再推入
        LOG_WARNING("Ring buffer full for stream " + std::to_string(stream_id) + ", dropping oldest frame");
        std::shared_ptr<GpuFrame> old_frame;
        if (frame_ring_buffers_[array_index]->pop(old_frame, false)) {
            freeFrame(old_frame);
        }
        // 再次尝试推入
        if (!frame_ring_buffers_[array_index]->push(std::move(frame), false)) {
            LOG_ERROR("Failed to push frame to ring buffer after drop");
            return false;
        }
    }

    LOG_DEBUG("Frame pushed to ring buffer for stream " + std::to_string(stream_id) +
              ", buffer size: " + std::to_string(frame_ring_buffers_[array_index]->size()));
    return true;
}

std::shared_ptr<GpuFrame> GpuMemoryManager::popFrameFromQueue(int stream_id) {
    int array_index = mapStreamIdToIndex(stream_id);
    if (array_index < 0 || array_index >= MAX_STREAMS) {
        LOG_ERROR("Invalid stream ID for queue pop: " + std::to_string(stream_id));
        return nullptr;
    }

    std::shared_ptr<GpuFrame> frame;
    if (!frame_ring_buffers_[array_index]->pop(frame, false)) {
        return nullptr; // 缓冲区为空
    }

    LOG_DEBUG("Frame popped from ring buffer for stream " + std::to_string(stream_id) +
              ", remaining buffer size: " + std::to_string(frame_ring_buffers_[array_index]->size()));

    return frame;
}

bool GpuMemoryManager::isQueueEmpty(int stream_id) {
    int array_index = mapStreamIdToIndex(stream_id);
    if (array_index < 0 || array_index >= MAX_STREAMS) {
        return true;
    }

    return frame_ring_buffers_[array_index]->empty();
}

size_t GpuMemoryManager::getQueueSize(int stream_id) {
    int array_index = mapStreamIdToIndex(stream_id);
    if (array_index < 0 || array_index >= MAX_STREAMS) {
        return 0;
    }

    return frame_ring_buffers_[array_index]->size();
}

std::string GpuMemoryManager::getBufferStats() {
    std::string stats = "Ring Buffer Stats:\n";
    for (int i = 0; i < MAX_STREAMS; ++i) {
        size_t sz = frame_ring_buffers_[i]->size();
        if (sz > 0 || i == MAX_INPUT_STREAMS) {
            std::string label = (i == MAX_INPUT_STREAMS) ? "Output" : "Stream " + std::to_string(i);
            stats += "  " + label + ": " + std::to_string(sz) + "/" + 
                     std::to_string(RING_BUFFER_CAPACITY - 1) + "\n";
        }
    }
    return stats;
}

std::shared_ptr<GpuFrame> GpuMemoryManager::createFrameFromPointer(void* gpu_ptr, cudaIpcMemHandle_t mem_handle, size_t size,
                                                                             int width, int height, bool is_nv12,
                                                                             int stream_id, int64_t pts, bool owns_memory) {
    LOG_DEBUG("Creating GPU frame from pointer, size: " + std::to_string(size) +
              ", resolution: " + std::to_string(width) + "x" + std::to_string(height) +
              ", owns_memory: " + (owns_memory ? "true" : "false"));

    // 创建GPU帧对象（外部指针通常不由我们释放；若 owns_memory=true 则最后一个引用释放时cudaFree）
    auto gpu_frame = std::shared_ptr<GpuFrame>(new GpuFrame(), [](GpuFrame* f) {
        if (!f) return;
        if (f->owns_memory && f->gpu_ptr) {
            cudaFree(f->gpu_ptr);
            f->gpu_ptr = nullptr;
        }
        delete f;
    });
    gpu_frame->gpu_ptr = gpu_ptr;
    gpu_frame->mem_handle = mem_handle;
    gpu_frame->size = size;
    gpu_frame->width = width;
    gpu_frame->height = height;
    gpu_frame->pts = pts;
    gpu_frame->stream_id = stream_id;
    gpu_frame->is_nv12 = is_nv12;
    gpu_frame->ready = true;
    gpu_frame->owns_memory = owns_memory;

    LOG_DEBUG("GPU frame created from pointer successfully, GPU ptr: " +
             std::to_string(reinterpret_cast<uintptr_t>(gpu_ptr)));
    return gpu_frame;
}

void GpuMemoryManager::cleanup() {
    LOG_INFO("Cleaning up GPU Memory Manager");

    // 清理所有环形缓冲区：shared_ptr 引用计数归零时自动释放 GPU 内存
    for (int i = 0; i < MAX_STREAMS; i++) {
        if (frame_ring_buffers_[i]) {
            std::shared_ptr<GpuFrame> frame;
            while (frame_ring_buffers_[i]->pop(frame, false)) {
                // shared_ptr 离开作用域时自动释放
            }
            frame_ring_buffers_[i]->clear();
        }
    }

    frame_ring_buffers_.clear();

    LOG_INFO("GPU Memory Manager cleanup completed");
}
