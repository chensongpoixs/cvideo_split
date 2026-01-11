/**
 * @file Config.h
 * @author chensong
 * @date 2026-01-11
 * @brief 系统配置管理（System Configuration Management）
 * 
 * 该模块提供视频拼接系统的配置管理，支持从命令行参数加载配置。
 * 使用单例模式确保全局配置一致性。
 * 
 * 配置结构（Configuration Structure）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    VideoSystemConfig                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  rtsp_urls[]     : RTSP 输入流 URL 列表                       |
 *  |  num_streams     : 输入流数量                                 |
 *  |  output_width    : 输出视频宽度                               |
 *  |  output_height   : 输出视频高度                               |
 *  |  fps             : 输出帧率                                   |
 *  |  output_url      : RTSP 输出流 URL                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    VideoDisplayConfig                         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  enable_decoder_display  : 是否显示解码后的视频               |
 *  |  enable_stitcher_display : 是否显示拼接后的视频               |
 *  |  display_fps             : 显示帧率                           |
 *  |  enable_fullscreen       : 是否全屏                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 使用示例：
 * @code
 * // 加载配置
 * Config::getInstance().loadFromArgs(argc, argv);
 * 
 * // 获取配置
 * const auto& config = Config::getInstance().getConfig();
 * 
 * // 使用配置
 * for (const auto& url : config.rtsp_urls) {
 *     createDecoder(url);
 * }
 * @endcode
 * 
 * @note 该类使用单例模式
 */

#pragma once

#include <string>
#include <vector>

/**
 * @struct VideoDisplayConfig
 * @brief 视频显示配置
 */
struct VideoDisplayConfig {
    bool enable_decoder_display = true;     ///< 是否显示解码后的视频
    bool enable_stitcher_display = true;    ///< 是否显示拼接后的视频
    std::string decoder_window_title = "Decoded Video";   ///< 解码器窗口标题
    std::string stitcher_window_title = "Stitched Video"; ///< 拼接器窗口标题
    int display_fps = 30;                   ///< 显示帧率
    bool enable_fullscreen = false;         ///< 是否全屏显示
};

/**
 * @struct VideoSystemConfig
 * @brief 视频系统主配置
 */
struct VideoSystemConfig {
    /// @name RTSP 输入配置
    /// @{
    std::vector<std::string> rtsp_urls;     ///< RTSP 输入流 URL 列表
    int num_streams = 1;                     ///< 输入流数量
    /// @}

    /// @name 视频参数
    /// @{
    int output_width = 1920;                ///< 输出视频宽度
    int output_height = 1080;               ///< 输出视频高度
    int fps = 30;                           ///< 输出帧率
    /// @}

    /// @name 输出配置
    /// @{
    std::string output_url = "rtsp://127.0.0.1:554/live/test"; ///< 输出流 URL
    /// @}

    /// @name 显示配置
    /// @{
    VideoDisplayConfig display_config;      ///< 显示相关配置
    /// @}

    /// @name 调试配置
    /// @{
    bool enable_debug_logging = true;       ///< 是否启用调试日志
    int max_frames_to_process = 100;        ///< 最大处理帧数（调试用）
    /// @}
};

/**
 * @class Config
 * @brief 配置管理器
 * 
 * 单例模式，提供系统配置的加载和访问。
 */
class Config {
public:
    /**
     * @brief 获取单例实例
     * @return Config& 配置管理器实例引用
     */
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    /**
     * @brief 从命令行参数加载配置
     * 
     * 支持的参数格式：
     * - --rtsp <url>      : 添加 RTSP 输入流
     * - --output <url>    : 设置输出流 URL
     * - --width <n>       : 设置输出宽度
     * - --height <n>      : 设置输出高度
     * - --fps <n>         : 设置帧率
     * 
     * @param argc 参数数量
     * @param argv 参数数组
     * @return true 加载成功
     * @return false 加载失败
     */
    bool loadFromArgs(int argc, char* argv[]);

    /**
     * @brief 获取配置
     * @return const VideoSystemConfig& 配置引用
     */
    const VideoSystemConfig& getConfig() const { return config_; }

    /**
     * @brief 设置配置
     * @param config 新配置
     */
    void setConfig(const VideoSystemConfig& config) { config_ = config; }

private:
    Config() = default;
    ~Config() = default;

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    VideoSystemConfig config_;  ///< 系统配置
};
