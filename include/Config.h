#pragma once

#include <string>
#include <vector>

struct VideoDisplayConfig {
    bool enable_decoder_display = true;    // 是否显示解码后的视频
    bool enable_stitcher_display = true;   // 是否显示拼接后的视频
    std::string decoder_window_title = "Decoded Video";     // 解码器窗口标题
    std::string stitcher_window_title = "Stitched Video";   // 拼接器窗口标题
    int display_fps = 30;                   // 显示帧率
    bool enable_fullscreen = false;         // 是否全屏显示
};

struct VideoSystemConfig {
    // RTSP配置
    std::vector<std::string> rtsp_urls;
    int num_streams = 1;

    // 视频参数
    int output_width = 1920;
    int output_height = 1080;
    int fps = 30;

    // 输出配置
    std::string output_url = "rtsp://127.0.0.1:554/live/test";

    // 显示配置
    VideoDisplayConfig display_config;

    // 调试配置
    bool enable_debug_logging = true;
    int max_frames_to_process = 100;  // 最多处理帧数，用于测试
};

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    // 加载配置（可以从文件或命令行参数）
    bool loadFromArgs(int argc, char* argv[]);

    // 获取配置
    const VideoSystemConfig& getConfig() const { return config_; }

    // 设置配置
    void setConfig(const VideoSystemConfig& config) { config_ = config; }

private:
    Config() = default;
    ~Config() = default;

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    VideoSystemConfig config_;
};
