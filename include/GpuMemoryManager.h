/**
 * @file GpuMemoryManager.h
 * @author chensong
 * @date 2026-01-11
 * @brief GPU 内存管理器（GPU Memory Manager）
 * 
 * 该模块提供统一的 GPU 内存分配、释放和帧队列管理，实现视频帧在 GPU 内存中的
 * 高效管理，支持零拷贝共享和进程间通信（IPC）。
 * 
 * GPU 内存管理架构（GPU Memory Management Architecture）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                   GpuMemoryManager (单例)                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   GPU 内存池                环形缓冲区管理                    |
 *  |  ┌─────────────┐      ┌───────────────────────────┐          |
 *  |  │ allocateFrame      │  RingBuffer[0] → Stream 0  │          |
 *  |  │ freeFrame   │      │  RingBuffer[1] → Stream 1  │          |
 *  |  │ copyToGpu   │      │  RingBuffer[2] → Stream 2  │          |
 *  |  │ copyFromGpu │      │  RingBuffer[3] → Stream 3  │          |
 *  |  └─────────────┘      │  RingBuffer[16]→ 拼接输出  │          |
 *  |                       └───────────────────────────┘          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 环形缓冲区优势（Ring Buffer Advantages）：
 * 
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  std::queue           │  RingBuffer (无锁 SPSC)             │
 *   ├─────────────────────────────────────────────────────────────┤
 *   │  ✗ 动态内存分配        │  ✓ 预分配固定大小                   │
 *   │  ✗ 每次操作需要锁      │  ✓ 原子操作，无锁设计               │
 *   │  ✗ 缓存不友好          │  ✓ 连续内存，缓存友好               │
 *   │  ✗ 高延迟 (~100ns)     │  ✓ 低延迟 (~10ns)                   │
 *   └─────────────────────────────────────────────────────────────┘
 * 
 * GpuFrame 结构（GpuFrame Structure）：
 * 
 *  ┌─────────────────────────────────────────────────────────────┐
 *  │                         GpuFrame                             │
 *  ├─────────────────────────────────────────────────────────────┤
 *  │ gpu_ptr      │ GPU 内存指针                                  │
 *  │ size         │ 内存大小 (bytes)                              │
 *  │ width/height │ 帧尺寸                                        │
 *  │ pts          │ 时间戳 (presentation timestamp)               │
 *  │ is_nv12      │ NV12 格式标志                                 │
 *  │ mem_handle   │ IPC 句柄（进程间共享）                        │
 *  │ external_ref │ 外部资源引用（如 FFmpeg AVFrame）             │
 *  │ owns_memory  │ 是否拥有内存所有权                            │
 *  └─────────────────────────────────────────────────────────────┘
 * 
 * 内存生命周期管理（Memory Lifecycle Management）：
 * 
 *   分配 (allocateFrame)
 *       │
 *       ▼
 *   ┌───────────┐    pushFrameToQueue    ┌───────────┐
 *   │  GpuFrame │ ────────────────────► │   Queue   │
 *   └───────────┘                        └───────────┘
 *       │                                      │
 *       │ shared_ptr                           │ popFrameFromQueue
 *       │ 引用计数                             │
 *       ▼                                      ▼
 *   ┌───────────┐                        ┌───────────┐
 *   │  消费者1  │                        │  消费者2  │
 *   └───────────┘                        └───────────┘
 *       │                                      │
 *       └──────────────┬───────────────────────┘
 *                      │ 引用计数归零
 *                      ▼
 *                 cudaFree (自动释放)
 * 
 * @note 该类使用单例模式，全局唯一实例
 * @note 使用 shared_ptr 管理 GpuFrame 生命周期
 * @see CudaStreamManager
 * @see VideoDecoder
 * @see VideoStitcher
 */

#pragma once

#include <cuda_runtime.h>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "RingBuffer.h"

/**
 * @struct GpuFrame
 * @brief GPU 视频帧数据结构
 * 
 * 存储一帧视频在 GPU 内存中的所有相关信息，包括内存指针、尺寸、时间戳等。
 * 支持零拷贝共享和 IPC 进程间通信。
 */
struct GpuFrame {
    cudaIpcMemHandle_t mem_handle;  ///< IPC 内存句柄，用于进程间共享
    void* gpu_ptr;                  ///< GPU 内存指针（NV12 数据起始地址）
    size_t size;                    ///< 内存大小（字节）
    int width;                      ///< 帧宽度（像素）
    int height;                     ///< 帧高度（像素）
    int64_t pts;                    ///< 时间戳（Presentation Timestamp）
    int stream_id;                  ///< 所属流 ID（-1 表示拼接输出流）
    bool is_nv12;                   ///< 是否为 NV12 格式（Y + UV 交错）
    std::atomic<bool> ready;        ///< 帧数据是否准备就绪
    std::shared_ptr<void> external_ref; ///< 外部资源引用（如 FFmpeg AVFrame），保证零拷贝内存生命周期
    bool owns_memory = false;       ///< 是否拥有 GPU 内存所有权（通过 shared_ptr deleter 自动释放）
};

/**
 * @class GpuMemoryManager
 * @brief GPU 内存统一管理器
 * 
 * 提供以下核心功能：
 * - GPU 内存分配和释放
 * - CPU ↔ GPU 数据拷贝
 * - 帧队列管理（生产者-消费者模式）
 * - 零拷贝内存共享
 * - IPC 进程间共享支持
 * 
 * 使用示例：
 * @code
 * auto& mgr = GpuMemoryManager::getInstance();
 * mgr.initialize();
 * 
 * // 分配 GPU 帧
 * auto frame = mgr.allocateFrame(1920, 1080, true, 0);
 * 
 * // 推入队列
 * mgr.pushFrameToQueue(0, frame);
 * 
 * // 从队列获取
 * auto f = mgr.popFrameFromQueue(0);
 * @endcode
 */
class GpuMemoryManager {
public:
    /**
     * @brief 获取单例实例
     * @return GpuMemoryManager& 管理器实例引用
     */
    static GpuMemoryManager& getInstance() {
        static GpuMemoryManager instance;
        return instance;
    }

    /**
     * @brief 初始化 GPU 内存管理器
     * 
     * 创建帧队列和同步原语。
     * 
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initialize();

    /**
     * @brief 分配 GPU 帧内存
     * 
     * 分配指定尺寸的 GPU 内存，创建 GpuFrame 并返回 shared_ptr。
     * 
     * @param width 帧宽度（像素）
     * @param height 帧高度（像素）
     * @param is_nv12 是否为 NV12 格式（默认 true）
     * @param stream_id 所属流 ID（默认 -1）
     * @return std::shared_ptr<GpuFrame> GPU 帧指针，失败返回 nullptr
     * 
     * @note NV12 格式：内存大小 = width × height × 1.5
     * @note 使用 shared_ptr 自动管理内存生命周期
     */
    std::shared_ptr<GpuFrame> allocateFrame(int width, int height, bool is_nv12 = true, int stream_id = -1);

    /**
     * @brief 释放 GPU 帧内存
     * 
     * 重置 shared_ptr，当引用计数归零时自动调用 cudaFree。
     * 
     * @param frame GPU 帧引用
     * @note 实际释放由 shared_ptr deleter 处理
     */
    void freeFrame(std::shared_ptr<GpuFrame>& frame);

    /**
     * @brief 从 CPU 拷贝数据到 GPU
     * 
     * @param gpu_frame 目标 GPU 帧
     * @param cpu_data 源 CPU 数据指针
     * @param size 拷贝大小（字节）
     * @return true 拷贝成功
     * @return false 拷贝失败
     */
    bool copyToGpu(std::shared_ptr<GpuFrame> gpu_frame, const uint8_t* cpu_data, size_t size);

    /**
     * @brief 从 GPU 拷贝数据到 CPU
     * 
     * @param gpu_frame 源 GPU 帧
     * @param cpu_data 目标 CPU 数据指针
     * @param size 拷贝大小（字节）
     * @return true 拷贝成功
     * @return false 拷贝失败
     */
    bool copyFromGpu(const std::shared_ptr<GpuFrame> gpu_frame, uint8_t* cpu_data, size_t size);

    /**
     * @brief GPU 到 GPU 内存拷贝
     * 
     * @param dst_frame 目标 GPU 帧
     * @param src_frame 源 GPU 帧
     * @return true 拷贝成功
     * @return false 拷贝失败
     */
    bool copyGpuToGpu(std::shared_ptr<GpuFrame> dst_frame, const std::shared_ptr<GpuFrame> src_frame);

    /**
     * @brief 从现有 GPU 指针创建帧（零拷贝）
     * 
     * 用于包装外部分配的 GPU 内存（如 NVDEC 输出）。
     * 
     * @param gpu_ptr GPU 内存指针
     * @param mem_handle IPC 内存句柄
     * @param size 内存大小
     * @param width 帧宽度
     * @param height 帧高度
     * @param is_nv12 是否 NV12 格式
     * @param stream_id 流 ID
     * @param pts 时间戳
     * @param owns_memory 是否拥有内存（决定是否在释放时调用 cudaFree）
     * @return std::shared_ptr<GpuFrame> GPU 帧指针
     */
    std::shared_ptr<GpuFrame> createFrameFromPointer(void* gpu_ptr, cudaIpcMemHandle_t mem_handle, size_t size,
                                                   int width, int height, bool is_nv12,
                                                   int stream_id, int64_t pts, bool owns_memory = true);

    /**
     * @brief 等待帧准备就绪
     * 
     * @param frame GPU 帧
     * @param timeout_ms 超时时间（毫秒）
     * @return true 帧已就绪
     * @return false 超时或失败
     */
    bool waitFrameReady(std::shared_ptr<GpuFrame> frame, int timeout_ms = 1000);

    /**
     * @brief 标记帧准备就绪
     * 
     * @param frame GPU 帧
     * @param ready 就绪状态
     */
    void setFrameReady(std::shared_ptr<GpuFrame> frame, bool ready = true);

    /**
     * @brief 将帧推入指定流的队列
     * 
     * @param stream_id 流 ID（-1 映射到拼接输出队列）
     * @param frame GPU 帧
     * @return true 成功
     * @return false 队列已满或参数错误
     */
    bool pushFrameToQueue(int stream_id, std::shared_ptr<GpuFrame> frame);

    /**
     * @brief 从指定流的队列弹出帧
     * 
     * @param stream_id 流 ID
     * @return std::shared_ptr<GpuFrame> GPU 帧，队列为空返回 nullptr
     */
    std::shared_ptr<GpuFrame> popFrameFromQueue(int stream_id);

    /**
     * @brief 检查指定流队列是否为空
     * 
     * @param stream_id 流 ID
     * @return true 队列为空
     * @return false 队列非空
     */
    bool isQueueEmpty(int stream_id);

    /**
     * @brief 清理所有资源
     */
    void cleanup();

    /**
     * @brief 获取队列统计信息
     * 
     * @param stream_id 流 ID
     * @return size_t 当前队列中的帧数
     */
    size_t getQueueSize(int stream_id);

    /**
     * @brief 获取总缓冲区统计
     * 
     * @return std::string 格式化的统计信息
     */
    std::string getBufferStats();

private:
    GpuMemoryManager() = default;
    ~GpuMemoryManager() { cleanup(); }

    GpuMemoryManager(const GpuMemoryManager&) = delete;
    GpuMemoryManager& operator=(const GpuMemoryManager&) = delete;

    /**
     * @brief 将流 ID 映射到内部队列索引
     * @param stream_id 流 ID（-1 映射到 MAX_INPUT_STREAMS）
     * @return int 内部索引
     */
    int mapStreamIdToIndex(int stream_id) const {
        if (stream_id < 0) return MAX_INPUT_STREAMS; // 拼接输出队列
        return stream_id;
    }

    /// 环形缓冲区类型定义（容量为8，2的幂以支持位运算优化）
    static const size_t RING_BUFFER_CAPACITY = 8;
    using FrameRingBuffer = BlockingRingBuffer<std::shared_ptr<GpuFrame>, RING_BUFFER_CAPACITY>;

    /// 帧环形缓冲区（每个流一个）
    std::vector<std::unique_ptr<FrameRingBuffer>> frame_ring_buffers_;

    static const int MAX_INPUT_STREAMS = 16;     ///< 最大输入流数量
    static const int MAX_STREAMS = 17;           ///< 总队列数量（16 输入 + 1 输出）
    static const int MAX_FRAMES_PER_STREAM = 7;  ///< 每个流最大缓存帧数（RING_BUFFER_CAPACITY - 1）
};
