/**
 * @file CudaStreamManager.h
 * @author chensong
 * @date 2026-01-11
 * @brief CUDA Stream 统一管理器（Unified CUDA Stream Manager）
 * 
 * 该模块提供统一的 CUDA Stream 和 Event 管理，实现跨模块的 GPU 资源共享，
 * 减少各模块各自创建/销毁 stream 的开销，并提供精细的跨 stream 同步机制。
 * 
 * CUDA Stream 管理架构（CUDA Stream Management Architecture）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    CudaStreamManager (单例)                   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          |
 *  |   │decode_stream│  │stitch_stream│  │display_stream         |
 *  |   │  (解码打包) │  │  (GPU拼接)  │  │ (NV12→RGBA) │          |
 *  |   └─────────────┘  └─────────────┘  └─────────────┘          |
 *  |          │                │                │                  |
 *  |          ▼                ▼                ▼                  |
 *  |   ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          |
 *  |   │VideoDecoder │  │VideoStitcher│  │ DxgiDisplay │          |
 *  |   └─────────────┘  └─────────────┘  └─────────────┘          |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    CUDA Events (跨stream同步)                 |
 *  |   decode_complete_event_ ──► waitForDecode() ──► stitch      |
 *  |   stitch_complete_event_ ──► waitForStitch() ──► encode      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * Stream 分配策略（Stream Allocation Strategy）：
 * 
 *   Stream 名称        用途                        使用模块
 *  +--------------+------------------------+----------------------+
 *  | decode_stream  | 解码后 pitch 内存打包   | VideoDecoder        |
 *  | stitch_stream  | CUDA 视频拼接操作       | CudaVideoStitcher   |
 *  | display_stream | NV12→RGBA 转换/显示    | DxgiDisplay         |
 *  | encode_stream  | GPU→NVENC 数据传输     | VideoEncoder        |
 *  +--------------+------------------------+----------------------+
 * 
 * 同步策略（Synchronization Strategy）：
 * - 使用 CUDA Events 做跨 stream 依赖，避免全局同步
 * - 仅在必要时调用 cudaStreamSynchronize
 * - 支持流水线并行处理
 * 
 * @note 该类使用单例模式，全局唯一实例
 * @note 必须在 GpuMemoryManager 初始化后调用 initialize()
 * @see GpuMemoryManager
 * @see CudaVideoStitcher
 * @see DxgiDisplay
 */

#pragma once

#include <cuda_runtime.h>
#include <mutex>
#include <string>

/**
 * @class CudaStreamManager
 * @brief 统一的 CUDA Stream 管理器
 * 
 * 提供以下功能：
 * - 统一创建和管理多个 CUDA stream
 * - 提供跨模块的 stream 共享机制
 * - 使用 CUDA Events 实现精细的依赖管理
 * - 支持异步流水线处理
 * 
 * 使用示例：
 * @code
 * // 初始化
 * CudaStreamManager& mgr = CudaStreamManager::getInstance();
 * mgr.initialize();
 * 
 * // 获取 stream 执行操作
 * cudaStream_t stream = mgr.getStitchStream();
 * myKernel<<<grid, block, 0, stream>>>(...);
 * 
 * // 记录完成事件，让其他 stream 等待
 * mgr.recordStitchComplete();
 * mgr.waitForStitch(mgr.getEncodeStream());
 * @endcode
 */
class CudaStreamManager {
public:
    /**
     * @brief 获取单例实例
     * @return CudaStreamManager& 管理器实例引用
     * @note 线程安全（C++11 静态局部变量保证）
     */
    static CudaStreamManager& getInstance() {
        static CudaStreamManager instance;
        return instance;
    }

    /**
     * @brief 初始化管理器
     * 
     * 创建所有 CUDA streams 和 events。
     * 
     * 初始化流程（Initialization Flow）：
     *   1. cudaSetDevice(0) - 确保正确的 GPU 上下文
     *   2. 创建 4 个非阻塞 stream (cudaStreamNonBlocking)
     *   3. 创建 2 个同步 event (cudaEventDisableTiming)
     * 
     * @return true 初始化成功
     * @return false 初始化失败（CUDA 错误）
     * @note 必须在使用其他方法前调用
     */
    bool initialize();

    /**
     * @brief 清理所有资源
     * 
     * 同步并销毁所有 streams 和 events。
     * 
     * @note 析构函数会自动调用
     */
    void cleanup();

    /**
     * @brief 获取解码用 stream
     * @return cudaStream_t 解码 stream（用于 pitch 内存打包）
     */
    cudaStream_t getDecodeStream()  { return decode_stream_; }

    /**
     * @brief 获取拼接用 stream
     * @return cudaStream_t 拼接 stream（用于 CUDA 视频拼接）
     */
    cudaStream_t getStitchStream()  { return stitch_stream_; }

    /**
     * @brief 获取显示用 stream
     * @return cudaStream_t 显示 stream（用于 NV12→RGBA 转换）
     */
    cudaStream_t getDisplayStream() { return display_stream_; }

    /**
     * @brief 获取编码用 stream
     * @return cudaStream_t 编码 stream（用于 GPU→NVENC 传输）
     */
    cudaStream_t getEncodeStream()  { return encode_stream_; }

    /**
     * @brief 获取主 stream
     * @return cudaStream_t 主 stream（默认使用 stitch_stream）
     * @note 用于不需要特定 stream 的通用场景
     */
    cudaStream_t getMainStream() { return stitch_stream_; }

    /**
     * @brief 记录解码完成事件
     * 
     * 在 decode_stream 上记录一个 event，其他 stream 可以等待此事件。
     * 
     * @return cudaEvent_t 解码完成事件
     * @see waitForDecode()
     */
    cudaEvent_t recordDecodeComplete();

    /**
     * @brief 记录拼接完成事件
     * 
     * 在 stitch_stream 上记录一个 event，其他 stream 可以等待此事件。
     * 
     * @return cudaEvent_t 拼接完成事件
     * @see waitForStitch()
     */
    cudaEvent_t recordStitchComplete();

    /**
     * @brief 让指定 stream 等待解码完成
     * @param waiting_stream 需要等待的 stream
     * @note 使用 cudaStreamWaitEvent 实现非阻塞等待
     */
    void waitForDecode(cudaStream_t waiting_stream);

    /**
     * @brief 让指定 stream 等待拼接完成
     * @param waiting_stream 需要等待的 stream
     * @note 使用 cudaStreamWaitEvent 实现非阻塞等待
     */
    void waitForStitch(cudaStream_t waiting_stream);

    /**
     * @brief 同步解码 stream
     * @note 阻塞直到 decode_stream 上所有操作完成
     */
    void syncDecode();

    /**
     * @brief 同步拼接 stream
     * @note 阻塞直到 stitch_stream 上所有操作完成
     */
    void syncStitch();

    /**
     * @brief 同步显示 stream
     * @note 阻塞直到 display_stream 上所有操作完成
     */
    void syncDisplay();

    /**
     * @brief 同步编码 stream
     * @note 阻塞直到 encode_stream 上所有操作完成
     */
    void syncEncode();

    /**
     * @brief 同步所有 streams
     * @note 阻塞直到所有 stream 上的操作完成
     */
    void syncAll();

    /**
     * @brief 检查是否已初始化
     * @return true 已初始化
     * @return false 未初始化
     */
    bool isInitialized() const { return initialized_; }

    // ===== 异步管道扩展 =====

    /**
     * @brief 记录显示完成事件
     * @return cudaEvent_t 显示完成事件
     */
    cudaEvent_t recordDisplayComplete();

    /**
     * @brief 记录编码完成事件
     * @return cudaEvent_t 编码完成事件
     */
    cudaEvent_t recordEncodeComplete();

    /**
     * @brief 让指定 stream 等待显示完成
     * @param waiting_stream 等待的 stream
     */
    void waitForDisplay(cudaStream_t waiting_stream);

    /**
     * @brief 让指定 stream 等待编码完成
     * @param waiting_stream 等待的 stream
     */
    void waitForEncode(cudaStream_t waiting_stream);

    /**
     * @brief 非阻塞查询解码是否完成
     * @return true 解码已完成
     * @return false 解码仍在进行
     */
    bool isDecodeComplete();

    /**
     * @brief 非阻塞查询拼接是否完成
     * @return true 拼接已完成
     * @return false 拼接仍在进行
     */
    bool isStitchComplete();

    /**
     * @brief 非阻塞查询显示是否完成
     * @return true 显示已完成
     * @return false 显示仍在进行
     */
    bool isDisplayComplete();

    /**
     * @brief 非阻塞查询编码是否完成
     * @return true 编码已完成
     * @return false 编码仍在进行
     */
    bool isEncodeComplete();

    /**
     * @brief 在指定 stream 上记录事件并返回
     * 
     * 用于创建帧级别的依赖关系。
     * 
     * @param stream 要记录的 stream
     * @return cudaEvent_t 新创建的事件（调用者需要调用 cudaEventDestroy 释放）
     * @note 调用者负责管理返回的 event 生命周期
     */
    cudaEvent_t createAndRecordEvent(cudaStream_t stream);

    /**
     * @brief 让 stream 等待指定事件
     * @param stream 等待的 stream
     * @param event 要等待的事件
     */
    void streamWaitEvent(cudaStream_t stream, cudaEvent_t event);

private:
    CudaStreamManager() = default;
    ~CudaStreamManager() { cleanup(); }

    // 禁止拷贝和赋值
    CudaStreamManager(const CudaStreamManager&) = delete;
    CudaStreamManager& operator=(const CudaStreamManager&) = delete;

    bool initialized_ = false;          ///< 初始化状态标志
    std::mutex mutex_;                   ///< 线程安全互斥锁

    // CUDA Streams
    cudaStream_t decode_stream_  = nullptr;  ///< 解码用 stream
    cudaStream_t stitch_stream_  = nullptr;  ///< 拼接用 stream
    cudaStream_t display_stream_ = nullptr;  ///< 显示用 stream
    cudaStream_t encode_stream_  = nullptr;  ///< 编码用 stream

    // CUDA Events（用于跨 stream 同步）
    cudaEvent_t decode_complete_event_ = nullptr;   ///< 解码完成事件
    cudaEvent_t stitch_complete_event_ = nullptr;   ///< 拼接完成事件
    cudaEvent_t display_complete_event_ = nullptr;  ///< 显示完成事件
    cudaEvent_t encode_complete_event_ = nullptr;   ///< 编码完成事件
};
