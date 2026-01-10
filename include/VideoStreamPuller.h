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

extern "C" {
#include <libavcodec/avcodec.h>
}

struct VideoFrame {
    uint8_t* data[4] = {nullptr, nullptr, nullptr, nullptr};
    int linesize[4] = {0, 0, 0, 0};
    int width = 0;
    int height = 0;
    int64_t pts = 0;
    AVPixelFormat format = AV_PIX_FMT_NONE;
};

class VideoStreamPuller {
public:
    VideoStreamPuller(const std::string& rtsp_url, int stream_id);
    ~VideoStreamPuller();

    bool initialize();
    void start();
    void stop();

    // 获取最新帧
    bool getLatestFrame(VideoFrame& frame);
    // 获取原始包用于解码
    bool getLatestPacket(AVPacket** packet);
    // 获取视频编解码器参数
    const AVCodecParameters* getVideoCodecParameters() const;
    int getStreamId() const { return stream_id_; }

private:
    void pullStream();
    void cleanup();
    void resetConnectionState();

    std::string rtsp_url_;
    int stream_id_;
    std::atomic<bool> running_;
    std::thread pull_thread_;

    AVFormatContext* format_ctx_ = nullptr;
    // Note: codec_ctx_ and av_frame_ are no longer used since VideoStreamPuller only pulls packets now
    SwsContext* sws_ctx_ = nullptr;
    int video_stream_index_ = -1;
    std::atomic<uint64_t> reconnect_attempts_{0};

    std::mutex frame_mutex_;
    std::condition_variable frame_cv_;
    std::queue<VideoFrame> frame_queue_;
    std::queue<AVPacket*> packet_queue_;
    VideoFrame latest_frame_;
    bool has_new_frame_ = false;

    static const int MAX_QUEUE_SIZE = 30;
};
