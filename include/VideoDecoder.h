/**
 * @file VideoDecoder.h
 * @author chensong
 * @date 2026-01-11
 * @brief 视频解码器模块（Video Decoder Module）
 * 
 * 该模块负责从 RTSP 流拉取视频数据并使用 CUDA 硬件加速解码，
 * 将解码后的帧推送到 GPU 内存队列供下游处理。
 * 
 * 视频解码流程（Video Decoding Flow）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      VideoDecoder                             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   RTSP URL ──► VideoStreamPuller ──► [Packet Queue]          |
 *  |                      │                     │                  |
 *  |                      │                     ▼                  |
 *  |                      │            HardwareDecoder (NVDEC)     |
 *  |                      │                     │                  |
 *  |                      │                     ▼                  |
 *  |                      │            GPU NV12 (pitched memory)   |
 *  |                      │                     │                  |
 *  |                      │             pitch打包 (如需要)         |
 *  |                      │                     │                  |
 *  |                      │                     ▼                  |
 *  |                      │            [GPU Frame Queue]           |
 *  |                      │                     │                  |
 *  |                      ▼                     ▼                  |
 *  |              ┌──────────────┐      ┌──────────────┐          |
 *  |              │ 显示 (可选)  │      │ 拼接/编码    │          |
 *  |              └──────────────┘      └──────────────┘          |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 线程模型（Threading Model）：
 * 
 *   ┌─────────────────┐         ┌─────────────────┐
 *   │  pull_thread_   │ ──────► │  decode_thread_ │
 *   │ (网络拉流)      │         │ (硬件解码)      │
 *   └─────────────────┘         └─────────────────┘
 *           │                            │
 *           ▼                            ▼
 *   [Packet Queue]               [GPU Frame Queue]
 * 
 * Pitched Memory 处理（Pitched Memory Handling）：
 * 
 *   NVDEC 输出可能是 pitched memory（每行有额外填充）：
 *   
 *   Pitched:                    Tight (打包后):
 *   [数据][padding]             [数据]
 *   [数据][padding]   ──────►   [数据]
 *   [数据][padding]             [数据]
 *   
 *   使用 cudaMemcpy2D 进行 GPU→GPU 打包拷贝
 * 
 * @note 每个输入流需要一个 VideoDecoder 实例
 * @see VideoStreamPuller RTSP 流拉取
 * @see HardwareDecoder NVDEC 硬件解码
 * @see GpuMemoryManager GPU 帧队列管理
 */

#pragma once

#include "VideoStreamPuller.h"
#include "HardwareDecoder.h"
#include "GpuMemoryManager.h"
#include "PlatformDisplay.h"
#include <thread>
#include <atomic>
#include <memory>

/**
 * @class VideoDecoder
 * @brief 视频解码器
 * 
 * 集成 RTSP 流拉取和 CUDA 硬件解码功能，提供完整的视频解码管线。
 * 
 * 使用示例：
 * @code
 * VideoDecoder decoder(0);  // 创建流 0 的解码器
 * decoder.initialize("rtsp://camera1/stream", display);
 * decoder.setDecoderRegionId(0);  // 设置显示区域
 * decoder.start();  // 启动解码线程
 * // ... 主循环 ...
 * decoder.stop();   // 停止
 * @endcode
 */
class VideoDecoder {
public:
    /**
     * @brief 构造函数
     * @param stream_id 流 ID（0, 1, 2, ...）
     */
    VideoDecoder(int stream_id);

    /**
     * @brief 析构函数
     * @note 自动调用 stop() 停止解码线程
     */
    ~VideoDecoder();

    /**
     * @brief 初始化解码器
     * 
     * 创建 VideoStreamPuller 和 HardwareDecoder，建立 RTSP 连接。
     * 
     * 初始化流程：
     * 1. 创建 VideoStreamPuller 并连接 RTSP
     * 2. 获取视频编解码参数
     * 3. 初始化 NVDEC 硬件解码器
     * 
     * @param rtsp_url RTSP 流 URL
     * @param display 平台显示器指针（可选，用于预览）
     * @return true 初始化成功
     * @return false 初始化失败
     * 
     * @note 如果连接失败，会自动进入重连模式
     */
    bool initialize(const std::string& rtsp_url, PlatformDisplay* display = nullptr);

    /**
     * @brief 启动解码线程
     * 
     * 同时启动：
     * - VideoStreamPuller 的拉流线程
     * - 本类的解码处理线程
     */
    void start();

    /**
     * @brief 停止解码线程
     * 
     * 停止所有子线程并等待完成。
     */
    void stop();

    /**
     * @brief 获取流 ID
     * @return int 流 ID
     */
    int getStreamId() const { return stream_id_; }

    /**
     * @brief 检查是否正在运行
     * @return true 解码线程正在运行
     * @return false 已停止
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief 设置平台显示器
     * @param display 显示器指针
     */
    void setPlatformDisplay(PlatformDisplay* display) { platform_display_ = display; }

    /**
     * @brief 设置解码器显示区域 ID
     * @param region_id 显示区域 ID
     * @note 设置后，每帧解码完成会自动更新到该区域
     */
    void setDecoderRegionId(int region_id) { decoder_region_id_ = region_id; }

    /**
     * @brief 获取已解码帧数
     * @return uint64_t 帧数
     */
    uint64_t getFramesDecoded() const { return frames_decoded_.load(); }

    /**
     * @brief 获取已处理包数
     * @return uint64_t 包数
     */
    uint64_t getPacketsProcessed() const { return packets_processed_.load(); }

private:
    /**
     * @brief 解码线程函数
     * 
     * 主循环：从 Packet Queue 获取包 → 解码 → 推入 GPU Frame Queue
     */
    void decodeThread();

    /**
     * @brief 处理单个视频包
     * 
     * 处理流程：
     * 1. 调用 HardwareDecoder 解码
     * 2. 检查是否需要 pitch 打包
     * 3. 创建 GpuFrame 并推入队列
     * 4. 更新显示（如启用）
     * 
     * @param packet AVPacket 视频包
     * @return true 解码成功
     * @return false 解码失败或无输出帧
     */
    bool processPacket(AVPacket* packet);

    int stream_id_;                 ///< 流 ID
    std::string rtsp_url_;          ///< RTSP URL

    /// RTSP 流拉取器
    std::unique_ptr<VideoStreamPuller> stream_puller_;
    /// NVDEC 硬件解码器
    std::unique_ptr<HardwareDecoder> hardware_decoder_;

    /// GPU 内存管理器引用
    GpuMemoryManager& gpu_memory_manager_;

    /// 解码线程
    std::thread decode_thread_;
    /// 运行状态标志
    std::atomic<bool> running_;

    /// 平台显示器（用于预览）
    PlatformDisplay* platform_display_;
    /// 显示区域 ID
    int decoder_region_id_;

    /// 统计：已解码帧数
    std::atomic<uint64_t> frames_decoded_;
    /// 统计：已处理包数
    std::atomic<uint64_t> packets_processed_;
};
