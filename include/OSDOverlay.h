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

struct OSDText {
    std::string text;
    int x = 0;
    int y = 0;
    int font_size = 24;
    uint32_t color = 0xFFFFFF; // RGB格式
    std::string font_path;
};

class OSDOverlay {
public:
    OSDOverlay();
    ~OSDOverlay();

    bool initialize(int width, int height, AVPixelFormat format);
    bool addTextOverlay(const OSDText& text_info);
    bool processFrame(AVFrame* input_frame, AVFrame* output_frame);
    void cleanup();

private:
    bool setupFilterGraph();
    bool setupFilterGraphWithText();
    void freeFilterGraph();

    int width_ = 0;
    int height_ = 0;
    AVPixelFormat format_ = AV_PIX_FMT_NONE;

    AVFilterGraph* filter_graph_ = nullptr;
    AVFilterContext* buffersrc_ctx_ = nullptr;
    AVFilterContext* buffersink_ctx_ = nullptr;
    AVFilterContext* drawtext_ctx_ = nullptr;

    std::vector<OSDText> text_overlays_;
    bool initialized_ = false;
};
