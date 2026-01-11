/**
 * @file VideoStreamPuller.h
 * @author chensong
 * @date 2026-01-11
 * @brief RTSP 视频流拉取器（RTSP Video Stream Puller）
 * 
 * 该模块负责从 RTSP 流拉取视频数据包，支持自动重连机制。
 * 拉取的 AVPacket 供 HardwareDecoder 进行解码。
 * 
 * 视频流拉取流程（Video Stream Pulling Flow）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    VideoStreamPuller                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   RTSP URL: rtsp://camera_ip:554/stream                       |
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │                    FFmpeg                                │|
 *  |   │  avformat_open_input() ──► RTSP 连接建立                 │|
 *  |   │  av_read_frame() ──► 读取 AVPacket                       │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │              Packet Queue (线程安全)                     │|
 *  |   │  ┌─────┬─────┬─────┬─────┬─────┐                        │|
 *  |   │  │Pkt 1│Pkt 2│Pkt 3│ ... │Pkt N│ (最大30个)             │|
 *  |   │  └─────┴─────┴─────┴─────┴─────┘                        │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼ getLatestPacket()                                    |
 *  |   VideoDecoder ──► HardwareDecoder (NVDEC)                   |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 自动重连机制（Auto-Reconnect Mechanism）：
 * 
 *   ┌─────────────┐
 *   │  正常拉流   │◄───────────────────────────────────────────┐
 *   └──────┬──────┘                                             │
 *          │ 错误/断开                                          │
 *          ▼                                                    │
 *   ┌─────────────┐                                             │
 *   │ cleanup()   │ 清理资源                                    │
 *   └──────┬──────┘                                             │
 *          │                                                    │
 *          ▼                                                    │
 *   ┌─────────────┐                                             │
 *   │ 等待 3 秒   │                                             │
 *   └──────┬──────┘                                             │
 *          │                                                    │
 *          ▼                                                    │
 *   ┌─────────────┐     成功                                    │
 *   │ initialize()│ ────────────────────────────────────────────┘
 *   └──────┬──────┘
 *          │ 失败
 *          ▼
 *       重试...
 * 
 * @note 拉流线程独立运行，通过 Packet Queue 与解码器通信
 * @see VideoDecoder 使用此模块拉取视频包
 * @see HardwareDecoder 解码拉取的视频包
 */

#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

/**
 * @struct VideoFrame
 * @brief CPU 视频帧结构（历史遗留，主要使用 AVPacket）
 */
struct VideoFrame {
    uint8_t* data[4] = {nullptr, nullptr, nullptr, nullptr};  ///< 平面数据指针
    int linesize[4] = {0, 0, 0, 0};                            ///< 行跨距
    int width = 0;                                              ///< 宽度
    int height = 0;                                             ///< 高度
    int64_t pts = 0;                                            ///< 时间戳
    AVPixelFormat format = AV_PIX_FMT_NONE;                    ///< 像素格式
};

/**
 * @class VideoStreamPuller
 * @brief RTSP 视频流拉取器
 * 
 * 从 RTSP 流拉取 AVPacket，支持自动重连。
 * 
 * 使用示例：
 * @code
 * VideoStreamPuller puller("rtsp://camera/stream", 0);
 * if (puller.initialize()) {
 *     puller.start();  // 启动拉流线程
 *     
 *     AVPacket* pkt = nullptr;
 *     if (puller.getLatestPacket(&pkt)) {
 *         // 处理 pkt
 *         av_packet_free(&pkt);
 *     }
 *     
 *     puller.stop();
 * }
 * @endcode
 */
class VideoStreamPuller {
public:
    /**
     * @brief 构造函数
     * @param rtsp_url RTSP 流 URL
     * @param stream_id 流 ID
     */
    VideoStreamPuller(const std::string& rtsp_url, int stream_id);

    /**
     * @brief 析构函数
     */
    ~VideoStreamPuller();

    /**
     * @brief 初始化连接
     * 
     * 建立 RTSP 连接，获取流信息。
     * 
     * @return true 连接成功
     * @return false 连接失败
     */
    bool initialize();

    /**
     * @brief 启动拉流线程
     */
    void start();

    /**
     * @brief 停止拉流线程
     */
    void stop();

    /**
     * @brief 获取最新视频帧（历史接口）
     * @param frame 输出帧
     * @return true 成功
     * @return false 无帧
     */
    bool getLatestFrame(VideoFrame& frame);

    /**
     * @brief 获取最新视频包
     * 
     * 从 Packet Queue 弹出一个包。
     * 
     * @param packet 输出包指针的指针
     * @return true 成功
     * @return false 队列为空
     * 
     * @note 调用者负责 av_packet_free()
     */
    bool getLatestPacket(AVPacket** packet);

    /**
     * @brief 获取视频编解码参数
     * 
     * 用于初始化 HardwareDecoder。
     * 
     * @return const AVCodecParameters* 编解码参数
     */
    const AVCodecParameters* getVideoCodecParameters() const;

    /**
     * @brief 获取流 ID
     * @return int 流 ID
     */
    int getStreamId() const { return stream_id_; }

private:
    /**
     * @brief 拉流线程函数
     * 
     * 主循环：av_read_frame() → 推入队列 → 错误时重连
     */
    void pullStream();

    /**
     * @brief 清理资源
     */
    void cleanup();

    /**
     * @brief 重置连接状态
     */
    void resetConnectionState();

    std::string rtsp_url_;                    ///< RTSP URL
    int stream_id_;                           ///< 流 ID
    std::atomic<bool> running_;               ///< 运行标志
    std::thread pull_thread_;                 ///< 拉流线程

    AVFormatContext* format_ctx_ = nullptr;   ///< FFmpeg 格式上下文
    SwsContext* sws_ctx_ = nullptr;           ///< 格式转换上下文（历史）
    int video_stream_index_ = -1;             ///< 视频流索引
    std::atomic<uint64_t> reconnect_attempts_{0}; ///< 重连尝试次数

    std::mutex frame_mutex_;                  ///< 队列互斥锁
    std::condition_variable frame_cv_;         ///< 队列条件变量
    std::queue<VideoFrame> frame_queue_;       ///< 帧队列（历史）
    std::queue<AVPacket*> packet_queue_;       ///< 包队列
    VideoFrame latest_frame_;                  ///< 最新帧（历史）
    bool has_new_frame_ = false;               ///< 新帧标志

    static const int MAX_QUEUE_SIZE = 30;      ///< 队列最大容量
};
