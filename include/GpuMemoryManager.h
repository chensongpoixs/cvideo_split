#pragma once

#include <cuda_runtime.h>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct GpuFrame {
    cudaIpcMemHandle_t mem_handle;  // IPC内存句柄，用于进程间共享
    void* gpu_ptr;                  // GPU内存指针
    size_t size;                    // 内存大小
    int width;                      // 帧宽度
    int height;                     // 帧高度
    int64_t pts;                    // 时间戳
    int stream_id;                  // 流ID
    bool is_nv12;                   // 是否为NV12格式
    std::atomic<bool> ready;        // 帧是否准备好
    std::shared_ptr<void> external_ref; // 持有外部资源（例如 FFmpeg AVFrame 引用），保证零拷贝内存生命周期
    bool owns_memory = false;       // 是否由本管理器分配并负责释放gpu_ptr（通过shared_ptr deleter）
};

class GpuMemoryManager {
public:
    static GpuMemoryManager& getInstance() {
        static GpuMemoryManager instance;
        return instance;
    }

    // 初始化GPU内存管理器
    bool initialize();

    // 分配GPU帧内存
    std::shared_ptr<GpuFrame> allocateFrame(int width, int height, bool is_nv12 = true, int stream_id = -1);

    // 释放GPU帧内存
    void freeFrame(std::shared_ptr<GpuFrame>& frame);

    // 从CPU内存复制到GPU内存
    bool copyToGpu(std::shared_ptr<GpuFrame> gpu_frame, const uint8_t* cpu_data, size_t size);

    // 从GPU内存复制到CPU内存
    bool copyFromGpu(const std::shared_ptr<GpuFrame> gpu_frame, uint8_t* cpu_data, size_t size);

    // GPU内存到GPU内存复制
    bool copyGpuToGpu(std::shared_ptr<GpuFrame> dst_frame, const std::shared_ptr<GpuFrame> src_frame);

    // 从GPU内存指针创建GPU帧（用于共享GPU内存）
    std::shared_ptr<GpuFrame> createFrameFromPointer(void* gpu_ptr, cudaIpcMemHandle_t mem_handle, size_t size,
                                                   int width, int height, bool is_nv12,
                                                   int stream_id, int64_t pts, bool owns_memory = true);

    // 等待帧准备就绪
    bool waitFrameReady(std::shared_ptr<GpuFrame> frame, int timeout_ms = 1000);

    // 标记帧准备就绪
    void setFrameReady(std::shared_ptr<GpuFrame> frame, bool ready = true);

    // 获取帧队列操作
    bool pushFrameToQueue(int stream_id, std::shared_ptr<GpuFrame> frame);
    std::shared_ptr<GpuFrame> popFrameFromQueue(int stream_id);
    bool isQueueEmpty(int stream_id);

    // 清理资源
    void cleanup();

private:
    GpuMemoryManager() = default;
    ~GpuMemoryManager() { cleanup(); }

    GpuMemoryManager(const GpuMemoryManager&) = delete;
    GpuMemoryManager& operator=(const GpuMemoryManager&) = delete;

    // 帧队列管理
    std::vector<std::queue<std::shared_ptr<GpuFrame>>> frame_queues_;
    std::vector<std::unique_ptr<std::mutex>> queue_mutexes_;
    std::vector<std::unique_ptr<std::condition_variable>> queue_cvs_;

    // 配置
    static const int MAX_STREAMS = 17;  // 16个输入流 + 1个输出流（ID -1映射到16）
    static const int MAX_FRAMES_PER_STREAM = 4;
};
