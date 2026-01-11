/**
 * @file VideoStitcher.h
 * @author chensong
 * @date 2026-01-11
 * @brief 视频拼接器模块（Video Stitcher Module）
 * 
 * 该模块从多个解码队列获取帧，调用 CUDA 拼接器合成一帧，
 * 并将结果推送到拼接输出队列供编码器使用。
 * 
 * 视频拼接流程（Video Stitching Flow）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      VideoStitcher                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   GPU Frame Queues (来自 VideoDecoder)                        |
 *  |   ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐            |
 *  |   │Queue[0] │ │Queue[1] │ │Queue[2] │ │Queue[3] │            |
 *  |   └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘            |
 *  |        │           │           │           │                  |
 *  |        ▼           ▼           ▼           ▼                  |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │               CudaVideoStitcher                          │|
 *  |   │  • 收集各路最新帧                                        │|
 *  |   │  • 调用 CUDA kernel 缩放/合成                            │|
 *  |   │  • 输出 NV12 拼接帧                                      │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |                           │                                   |
 *  |                           ▼                                   |
 *  |                   ┌─────────────┐                            |
 *  |                   │  OSDOverlay │ (可选: 时间戳、帧率等)     |
 *  |                   └──────┬──────┘                            |
 *  |                          │                                    |
 *  |                          ▼                                    |
 *  |                   ┌─────────────┐                            |
 *  |                   │ Queue[-1]   │ (拼接输出队列)             |
 *  |                   └─────────────┘                            |
 *  |                          │                                    |
 *  |           ┌──────────────┴──────────────┐                    |
 *  |           ▼                              ▼                    |
 *  |   ┌─────────────┐              ┌─────────────┐               |
 *  |   │PlatformDisplay             │ VideoEncoder│               |
 *  |   │  (预览)     │              │  (推流)     │               |
 *  |   └─────────────┘              └─────────────┘               |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 帧同步策略（Frame Synchronization Strategy）：
 * 
 *   轮询模式：每轮尝试从各队列获取最新帧
 *   - 如果某队列为空，使用上一帧（保持画面连续）
 *   - 丢弃旧帧，只使用最新帧（低延迟优先）
 * 
 * @note 拼接输出队列 ID 为 -1（映射到 Queue[16]）
 * @see CudaVideoStitcher CUDA 拼接实现
 * @see VideoDecoder 输入帧来源
 * @see VideoEncoder 输出帧消费者
 */

#pragma once

#include "GpuMemoryManager.h"
#include "CudaVideoStitcher.h"
#include "OSDOverlay.h"
#include "PlatformDisplay.h"
#include <thread>
#include <atomic>
#include <vector>
#include <memory>

/**
 * @class VideoStitcher
 * @brief 视频拼接器
 * 
 * 管理拼接线程，协调多路输入帧的收集和合成。
 * 
 * 使用示例：
 * @code
 * VideoStitcher stitcher;
 * stitcher.initialize(4, 1920, 1080, display);
 * stitcher.setStitcherRegionId(4);  // 设置显示区域
 * stitcher.start();  // 启动拼接线程
 * // ... 主循环 ...
 * stitcher.stop();   // 停止
 * @endcode
 */
class VideoStitcher {
public:
    /**
     * @brief 构造函数
     */
    VideoStitcher();

    /**
     * @brief 析构函数
     */
    ~VideoStitcher();

    /**
     * @brief 初始化拼接器
     * 
     * 配置拼接参数，初始化 CUDA 拼接器和 OSD 叠加器。
     * 
     * @param num_streams 输入流数量（1-4）
     * @param output_width 输出宽度
     * @param output_height 输出高度
     * @param display 平台显示器（可选，用于预览）
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initialize(int num_streams, int output_width, int output_height, PlatformDisplay* display = nullptr);

    /**
     * @brief 启动拼接线程
     */
    void start();

    /**
     * @brief 停止拼接线程
     */
    void stop();

    /**
     * @brief 检查是否正在运行
     * @return true 正在运行
     * @return false 已停止
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief 获取已处理帧数
     * @return uint64_t 帧数
     */
    uint64_t getFramesProcessed() const { return frames_processed_.load(); }

    /**
     * @brief 设置拼接输出显示区域 ID
     * @param region_id 显示区域 ID
     */
    void setStitcherRegionId(int region_id) { stitcher_region_id_ = region_id; }

    /**
     * @brief 设置平台显示器
     * @param display 显示器指针
     */
    void setPlatformDisplay(PlatformDisplay* display) { platform_display_ = display; }

private:
    /**
     * @brief 拼接线程函数
     * 
     * 主循环：收集帧 → 拼接 → 推入输出队列 → 更新显示
     */
    void stitchThread();

    /**
     * @brief 执行单次拼接处理
     * 
     * @return true 成功生成拼接帧
     * @return false 输入帧不足或拼接失败
     */
    bool processStitching();

    /**
     * @brief 设置拼接布局
     * 
     * 根据 num_streams 计算各流在输出中的位置。
     */
    void setupStitchLayout();

    int num_streams_;              ///< 输入流数量
    int output_width_;             ///< 输出宽度
    int output_height_;            ///< 输出高度

    /// CUDA 拼接器
    std::unique_ptr<CudaVideoStitcher> cuda_stitcher_;
    /// OSD 叠加器（时间戳、帧率等）
    std::unique_ptr<OSDOverlay> osd_overlay_;
    /// GPU 内存管理器引用
    GpuMemoryManager& gpu_memory_manager_;

    /// 平台显示器
    PlatformDisplay* platform_display_;
    /// 拼接输出显示区域 ID
    int stitcher_region_id_;

    /// 拼接配置
    StitchConfig stitch_config_;

    /// 拼接线程
    std::thread stitch_thread_;
    /// 运行状态标志
    std::atomic<bool> running_;

    /// 统计：已处理帧数
    std::atomic<uint64_t> frames_processed_;

    /// OSD 文本
    std::string osd_text_;
    /// 上次 OSD 更新时间戳
    int64_t last_osd_update_;
};
