/**
 * @file VideoEncoder.h
 * @author chensong
 * @date 2026-01-11
 * @brief 视频编码器模块（Video Encoder Module）
 * 
 * 该模块从拼接输出队列获取 GPU NV12 帧，使用 NVENC 硬件编码或软件编码，
 * 并将编码后的数据推送到 RTSP/RTMP 输出流。
 * 
 * 视频编码流程（Video Encoding Flow）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       VideoEncoder                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   GPU Frame Queue[-1] (来自 VideoStitcher)                    |
 *  |           │                                                   |
 *  |           ▼                                                   |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │                编码路径选择                              │|
 *  |   │                                                          │|
 *  |   │   优先: NVENC (h264_nvenc / hevc_nvenc)                  │|
 *  |   │   回退: libx264 / libx265 (软件编码)                     │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |           │                                                   |
 *  |           ▼                                                   |
 *  |   ┌──────────────────────────────────────────────────────────┐
 *  |   │  NVENC 路径 (零拷贝)                                     │
 *  |   │  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐  │
 *  |   │  │ GpuFrame    │───►│ AVFrame     │───►│ NVENC       │  │
 *  |   │  │ (NV12 GPU)  │    │ (CUDA)      │    │ (GPU 编码)  │  │
 *  |   │  └─────────────┘    └─────────────┘    └─────────────┘  │
 *  |   └──────────────────────────────────────────────────────────┘
 *  |           或                                                  |
 *  |   ┌──────────────────────────────────────────────────────────┐
 *  |   │  软件编码路径                                            │
 *  |   │  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐  │
 *  |   │  │ GpuFrame    │───►│ CPU NV12    │───►│ libx264     │  │
 *  |   │  │ (GPU→CPU)   │    │ (转YUV420P) │    │ (CPU 编码)  │  │
 *  |   │  └─────────────┘    └─────────────┘    └─────────────┘  │
 *  |   └──────────────────────────────────────────────────────────┘
 *  |           │                                                   |
 *  |           ▼                                                   |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │                 AVPacket (H.264/H.265)                   │|
 *  |   └────────────────────────┬────────────────────────────────┘|
 *  |                            │                                  |
 *  |                            ▼                                  |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │              av_interleaved_write_frame()                │|
 *  |   │              输出到 RTSP/RTMP/文件                       │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * NVENC 零拷贝实现（NVENC Zero-Copy Implementation）：
 * 
 *   1. 使用 av_hwframe_get_buffer() 从 FFmpeg 池分配 CUDA surface
 *   2. cudaMemcpy2D 从 GpuFrame 拷贝到 AVFrame（GPU→GPU）
 *   3. NVENC 直接编码 CUDA surface
 * 
 *   数据流：
 *   GpuFrame.gpu_ptr ──[cudaMemcpy2D]──► AVFrame.data[0] (CUDA) ──► NVENC
 * 
 * @note 优先使用 NVENC，失败时自动回退软件编码
 * @see VideoStitcher 输入帧来源
 * @see GpuMemoryManager GPU 帧队列管理
 */

#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include "GpuMemoryManager.h"

/**
 * @struct EncodedPacket
 * @brief 编码后的数据包结构
 * 
 * 封装编码输出的压缩数据和时间戳信息。
 */
struct EncodedPacket {
    uint8_t* data;           ///< 压缩数据指针
    size_t size;             ///< 数据大小（字节）
    int64_t pts;             ///< 显示时间戳
    int64_t dts;             ///< 解码时间戳
    int flags;               ///< AVPacket 标志
    bool is_keyframe;        ///< 是否为关键帧
};

/**
 * @class VideoEncoder
 * @brief 视频编码器
 * 
 * 支持 NVENC 硬件编码和软件编码的统一接口。
 * 
 * 使用示例：
 * @code
 * VideoEncoder encoder;
 * encoder.initialize(1920, 1080, 30, "rtsp://server/stream");
 * encoder.start();  // 自动从队列获取帧并编码
 * // ... 主循环 ...
 * encoder.stop();
 * @endcode
 */
class VideoEncoder {
public:
    /**
     * @brief 构造函数
     */
    VideoEncoder();

    /**
     * @brief 析构函数
     */
    ~VideoEncoder();

    /**
     * @brief 初始化编码器
     * 
     * 创建编码器、设置参数、打开输出流。
     * 
     * 初始化流程：
     * 1. 尝试创建 NVENC 硬件编码器 (h264_nvenc)
     * 2. 失败则回退 libx264 软件编码器
     * 3. 创建输出格式上下文
     * 4. 写入流头信息
     * 
     * @param width 视频宽度
     * @param height 视频高度
     * @param fps 目标帧率
     * @param output_url 输出 URL（rtsp://、rtmp://、file://）
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initialize(int width, int height, int fps, const std::string& output_url);

    /**
     * @brief 编码 GPU 帧
     * 
     * 手动编码单帧（通常使用 start() 自动模式）。
     * 
     * @param gpu_frame GPU 帧
     * @param packet 输出编码包
     * @return true 编码成功
     * @return false 编码失败或需要更多输入
     */
    bool encodeGpuFrame(std::shared_ptr<GpuFrame> gpu_frame, EncodedPacket& packet);

    /**
     * @brief 刷新编码器
     * 
     * 输出编码器缓冲区中的剩余帧。
     * 
     * @param packet 输出编码包
     * @return true 有剩余帧输出
     * @return false 无剩余帧
     */
    bool flush(EncodedPacket& packet);

    /**
     * @brief 写入编码包到输出
     * 
     * @param packet 编码包
     * @return true 写入成功
     * @return false 写入失败
     */
    bool writePacket(const EncodedPacket& packet);

    /**
     * @brief 启动编码线程
     * 
     * 自动从 GPU 帧队列获取帧并编码推流。
     */
    void start();

    /**
     * @brief 停止编码线程
     */
    void stop();

    /**
     * @brief 检查是否正在运行
     * @return true 正在运行
     * @return false 已停止
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief 获取已编码帧数
     * @return uint64_t 帧数
     */
    uint64_t getFramesEncoded() const { return frame_count_.load(); }

private:
    /**
     * @brief 清理资源
     */
    void cleanup();

    /**
     * @brief 编码线程函数
     */
    void encodeThread();

    /**
     * @brief 处理 GPU 队列
     * 
     * 从队列获取帧、编码、写入输出。
     * 
     * @return true 成功处理一帧
     * @return false 队列为空或编码失败
     */
    bool processGpuQueue();

    /**
     * @brief 从 GPU 帧创建 AVFrame（软件编码路径）
     * 
     * GPU→CPU 拷贝，NV12→YUV420P 转换。
     * 
     * @param gpu_frame GPU 帧
     * @return AVFrame* 软件帧，失败返回 nullptr
     */
    AVFrame* createFrameFromGpu(std::shared_ptr<GpuFrame> gpu_frame);

    /**
     * @brief 从 GPU 帧创建 AVFrame（NVENC 零拷贝路径）
     * 
     * GPU→GPU 拷贝到 FFmpeg 分配的 CUDA surface。
     * 
     * @param gpu_frame GPU 帧
     * @return AVFrame* 硬件帧，失败返回 nullptr
     */
    AVFrame* createFrameFromGpuDirect(std::shared_ptr<GpuFrame> gpu_frame);

    AVFormatContext* format_ctx_ = nullptr;    ///< FFmpeg 格式上下文
    AVCodecContext* codec_ctx_ = nullptr;      ///< 编解码器上下文
    AVStream* stream_ = nullptr;               ///< 输出流
    const AVCodec* codec_ = nullptr;           ///< 编解码器
    AVBufferRef* hw_device_ctx_ = nullptr;     ///< CUDA 硬件设备上下文

    /// NVENC 硬件帧池（用于 GPU→GPU 零拷贝）
    AVBufferRef* hw_frames_ctx_pool_ = nullptr;
    int hw_frames_width_ = 0;                  ///< 硬件帧池宽度
    int hw_frames_height_ = 0;                 ///< 硬件帧池高度

    int width_ = 0;                            ///< 视频宽度
    int height_ = 0;                           ///< 视频高度
    int fps_ = 0;                              ///< 帧率
    std::string output_url_;                   ///< 输出 URL

    /// GPU 内存管理器引用
    GpuMemoryManager& gpu_memory_manager_;

    /// 编码线程
    std::thread encode_thread_;
    /// 运行状态
    std::atomic<bool> running_;
    /// 统计：编码帧数
    std::atomic<uint64_t> frame_count_;
};
