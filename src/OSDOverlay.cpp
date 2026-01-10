#include "OSDOverlay.h"
#include "Logger.h"
#include <iostream>

OSDOverlay::OSDOverlay() : initialized_(false), filter_graph_(nullptr) {
}

OSDOverlay::~OSDOverlay() {
    cleanup();
}

bool OSDOverlay::initialize(int width, int height, AVPixelFormat format) {
    if (initialized_) return true;

    width_ = width;
    height_ = height;
    format_ = format;

    if (!setupFilterGraph()) {
        LOG_ERROR("Failed to setup filter graph for OSD overlay");
        return false;
    }

    initialized_ = true;
    LOG_INFO("OSD overlay initialized: " + std::to_string(width) + "x" + std::to_string(height));
    return true;
}

bool OSDOverlay::setupFilterGraph() {
    LOG_DEBUG("Setting up FFmpeg filter graph for OSD overlay");

    filter_graph_ = avfilter_graph_alloc();
    if (!filter_graph_) {
        LOG_ERROR("Failed to allocate FFmpeg filter graph");
        return false;
    }

    // 创建buffer源
    char args[512];
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=1/30:pixel_aspect=1/1",
             width_, height_, format_);

    int ret = avfilter_graph_create_filter(&buffersrc_ctx_, avfilter_get_by_name("buffer"),
                                          "in", args, nullptr, filter_graph_);
    if (ret < 0) {
        LOG_ERROR("Failed to create buffer source filter, error code: " + std::to_string(ret));
        return false;
    }

    // 创建buffer sink
    ret = avfilter_graph_create_filter(&buffersink_ctx_, avfilter_get_by_name("buffersink"),
                                      "out", nullptr, nullptr, filter_graph_);
    if (ret < 0) {
        LOG_ERROR("Failed to create buffer sink filter, error code: " + std::to_string(ret));
        return false;
    }

    // 设置输出像素格式
    enum AVPixelFormat pix_fmts[] = { format_, AV_PIX_FMT_NONE };
    ret = av_opt_set_int_list(buffersink_ctx_, "pix_fmts", pix_fmts,
                             AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOG_ERROR("Failed to set output pixel formats, error code: " + std::to_string(ret));
        return false;
    }

    // 连接滤镜
    ret = avfilter_link(buffersrc_ctx_, 0, buffersink_ctx_, 0);
    if (ret < 0) {
        LOG_ERROR("Failed to link filters, error code: " + std::to_string(ret));
        return false;
    }

    // 配置滤镜图
    ret = avfilter_graph_config(filter_graph_, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to configure filter graph, error code: " + std::to_string(ret));
        return false;
    }

    LOG_INFO("FFmpeg filter graph configured successfully for OSD overlay");
    return true;

    return true;
}

bool OSDOverlay::addTextOverlay(const OSDText& text_info) {
    if (!initialized_) return false;

    text_overlays_.push_back(text_info);

    // 重新配置滤镜图以包含文本叠加
    freeFilterGraph();
    return setupFilterGraphWithText();
}

bool OSDOverlay::setupFilterGraphWithText() {
    filter_graph_ = avfilter_graph_alloc();
    if (!filter_graph_) return false;

    // 创建buffer源
    char args[512];
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=1/30:pixel_aspect=1/1",
             width_, height_, format_);

    int ret = avfilter_graph_create_filter(&buffersrc_ctx_, avfilter_get_by_name("buffer"),
                                          "in", args, nullptr, filter_graph_);
    if (ret < 0) return false;

    // 创建buffer sink
    ret = avfilter_graph_create_filter(&buffersink_ctx_, avfilter_get_by_name("buffersink"),
                                      "out", nullptr, nullptr, filter_graph_);
    if (ret < 0) return false;

    // 设置输出像素格式
    enum AVPixelFormat pix_fmts[] = { format_, AV_PIX_FMT_NONE };
    ret = av_opt_set_int_list(buffersink_ctx_, "pix_fmts", pix_fmts,
                             AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) return false;

    AVFilterContext* current_ctx = buffersrc_ctx_;

    // 为每个文本叠加创建drawtext滤镜
    for (size_t i = 0; i < text_overlays_.size(); i++) {
        const OSDText& text = text_overlays_[i];

        char filter_name[32];
        snprintf(filter_name, sizeof(filter_name), "drawtext_%zu", i);

        ret = avfilter_graph_create_filter(&drawtext_ctx_, avfilter_get_by_name("drawtext"),
                                          filter_name, nullptr, nullptr, filter_graph_);
        if (ret < 0) return false;

        // 设置文本参数
        char text_args[1024];
        snprintf(text_args, sizeof(text_args),
                 "text='%s':x=%d:y=%d:fontsize=%d:fontcolor=0x%06X",
                 text.text.c_str(), text.x, text.y, text.font_size, text.color);

        ret = avfilter_init_str(drawtext_ctx_, text_args);
        if (ret < 0) {
            LOG_ERROR("Failed to initialize drawtext filter: " + std::string(text_args) + ", error code: " + std::to_string(ret));
            return false;
        }

        // 连接滤镜
        ret = avfilter_link(current_ctx, 0, drawtext_ctx_, 0);
        if (ret < 0) return false;

        current_ctx = drawtext_ctx_;
    }

    // 连接到sink
    ret = avfilter_link(current_ctx, 0, buffersink_ctx_, 0);
    if (ret < 0) return false;

    // 配置滤镜图
    ret = avfilter_graph_config(filter_graph_, nullptr);
    if (ret < 0) return false;

    return true;
}

bool OSDOverlay::processFrame(AVFrame* input_frame, AVFrame* output_frame) {
    if (!initialized_) return false;

    // 发送帧到滤镜
    int ret = av_buffersrc_add_frame(buffersrc_ctx_, input_frame);
    if (ret < 0) {
        LOG_WARNING("Error feeding frame to OSD filter graph, error code: " + std::to_string(ret));
        return false;
    }

    // 从滤镜接收处理后的帧
    ret = av_buffersink_get_frame(buffersink_ctx_, output_frame);
    if (ret < 0) {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return false;
        }
        LOG_ERROR("Error getting frame from OSD filter graph, error code: " + std::to_string(ret));
        return false;
    }

    return true;
}

void OSDOverlay::cleanup() {
    freeFilterGraph();
    text_overlays_.clear();
    initialized_ = false;
}

void OSDOverlay::freeFilterGraph() {
    if (filter_graph_) {
        avfilter_graph_free(&filter_graph_);
        filter_graph_ = nullptr;
        buffersrc_ctx_ = nullptr;
        buffersink_ctx_ = nullptr;
        drawtext_ctx_ = nullptr;
    }
}
