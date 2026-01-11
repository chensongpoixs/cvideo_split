/**
 * @file OSDOverlay.h
 * @author chensong
 * @date 2026-01-11
 * @brief OSD 文字叠加模块（On-Screen Display Overlay）
 * 
 * 该模块使用 FFmpeg 的 libavfilter 在视频帧上叠加文字信息，
 * 如时间戳、帧率、流标识等。
 * 
 * OSD 叠加流程（OSD Overlay Flow）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       OSDOverlay                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   输入帧 (AVFrame)                                            |
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │              FFmpeg Filter Graph                         │|
 *  |   │                                                          │|
 *  |   │  buffersrc ──► drawtext ──► buffersink                   │|
 *  |   │                                                          │|
 *  |   │  drawtext 参数:                                          │|
 *  |   │  - text: 显示内容                                        │|
 *  |   │  - x, y: 位置坐标                                        │|
 *  |   │  - fontsize: 字体大小                                    │|
 *  |   │  - fontcolor: 字体颜色 (RGB)                             │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   输出帧 (AVFrame + 文字叠加)                                 |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 使用示例：
 * @code
 * OSDOverlay osd;
 * osd.initialize(1920, 1080, AV_PIX_FMT_YUV420P);
 * 
 * OSDText text;
 * text.text = "Stream 1 - 1080p30";
 * text.x = 10;
 * text.y = 10;
 * text.font_size = 24;
 * text.color = 0xFFFFFF;  // 白色
 * osd.addTextOverlay(text);
 * 
 * osd.processFrame(input_frame, output_frame);
 * @endcode
 * 
 * @note 需要 FFmpeg 编译时启用 libfreetype
 * @see VideoStitcher 拼接后叠加 OSD
 */

#pragma once

extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
}

#include <string>
#include <vector>
#include <memory>

/**
 * @struct OSDText
 * @brief OSD 文字配置
 */
struct OSDText {
    std::string text;            ///< 显示文本
    int x = 0;                   ///< X 坐标
    int y = 0;                   ///< Y 坐标
    int font_size = 24;          ///< 字体大小
    uint32_t color = 0xFFFFFF;   ///< 字体颜色 (RGB 格式)
    std::string font_path;       ///< 字体文件路径（可选）
};

/**
 * @class OSDOverlay
 * @brief OSD 文字叠加器
 * 
 * 使用 FFmpeg libavfilter 实现视频帧上的文字叠加。
 */
class OSDOverlay {
public:
    /**
     * @brief 构造函数
     */
    OSDOverlay();

    /**
     * @brief 析构函数
     */
    ~OSDOverlay();

    /**
     * @brief 初始化叠加器
     * 
     * @param width 视频宽度
     * @param height 视频高度
     * @param format 像素格式
     * @return true 成功
     * @return false 失败
     */
    bool initialize(int width, int height, AVPixelFormat format);

    /**
     * @brief 添加文字叠加
     * 
     * @param text_info 文字配置
     * @return true 成功
     * @return false 失败
     */
    bool addTextOverlay(const OSDText& text_info);

    /**
     * @brief 处理帧（添加叠加）
     * 
     * @param input_frame 输入帧
     * @param output_frame 输出帧
     * @return true 成功
     * @return false 失败
     */
    bool processFrame(AVFrame* input_frame, AVFrame* output_frame);

    /**
     * @brief 清理资源
     */
    void cleanup();

private:
    /**
     * @brief 设置 filter graph
     * @return true 成功
     */
    bool setupFilterGraph();

    /**
     * @brief 设置带文字的 filter graph
     * @return true 成功
     */
    bool setupFilterGraphWithText();

    /**
     * @brief 释放 filter graph
     */
    void freeFilterGraph();

    int width_ = 0;                              ///< 视频宽度
    int height_ = 0;                             ///< 视频高度
    AVPixelFormat format_ = AV_PIX_FMT_NONE;     ///< 像素格式

    AVFilterGraph* filter_graph_ = nullptr;       ///< FFmpeg filter graph
    AVFilterContext* buffersrc_ctx_ = nullptr;    ///< 输入 buffer 上下文
    AVFilterContext* buffersink_ctx_ = nullptr;   ///< 输出 buffer 上下文
    AVFilterContext* drawtext_ctx_ = nullptr;     ///< drawtext filter 上下文

    std::vector<OSDText> text_overlays_;          ///< 文字叠加列表
    bool initialized_ = false;                    ///< 初始化状态
};
