#include "VideoStitcher.h"
#include "Logger.h"
#include <chrono>
#include <sstream>

VideoStitcher::VideoStitcher()
    : platform_display_(nullptr)
    , gpu_memory_manager_(GpuMemoryManager::getInstance())
    , running_(false)
    , frames_processed_(0)
    , last_osd_update_(0) {
}

VideoStitcher::~VideoStitcher() {
    stop();
}

bool VideoStitcher::initialize(int num_streams, int output_width, int output_height, PlatformDisplay* display) {
    LOG_INFO("Initializing VideoStitcher with " + std::to_string(num_streams) + " streams, output: " +
             std::to_string(output_width) + "x" + std::to_string(output_height));

    // 当前 StitchConfig / CudaVideoStitcher 的内部数组是固定 4 路（stream_positions[4], d_temp_buffers_[4]）
    if (num_streams > 4) {
        LOG_ERROR("VideoStitcher currently supports up to 4 streams, got: " + std::to_string(num_streams));
        return false;
    }

    // NV12 要求宽高为偶数，否则UV平面会出问题
    if ((output_width % 2) != 0 || (output_height % 2) != 0) {
        LOG_WARNING("Output size is not even for NV12, rounding down: " +
                    std::to_string(output_width) + "x" + std::to_string(output_height));
        output_width &= ~1;
        output_height &= ~1;
    }

    num_streams_ = num_streams;
    output_width_ = output_width;
    output_height_ = output_height;
    platform_display_ = display;
    stitcher_region_id_ = -1;  // 默认不显示

    // 初始化CUDA拼接器
    cuda_stitcher_ = std::make_unique<CudaVideoStitcher>();
    stitch_config_.output_width = output_width;
    stitch_config_.output_height = output_height;
    stitch_config_.num_streams = num_streams;

    setupStitchLayout();

    if (!cuda_stitcher_->initialize(stitch_config_)) {
        LOG_ERROR("Failed to initialize CUDA video stitcher");
        return false;
    }

    // 初始化OSD叠加（暂时禁用，因为GPU内存处理复杂）
    // osd_overlay_ = std::make_unique<OSDOverlay>();
    // if (!osd_overlay_->initialize(output_width, output_height, AV_PIX_FMT_NV12)) {
    //     LOG_WARNING("OSD overlay initialization failed, continuing without OSD");
    // }

    // 设置初始OSD文本
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "Video Stitching System - %Y-%m-%d %H:%M:%S");
    osd_text_ = ss.str();

    LOG_INFO("VideoStitcher initialized successfully");
    return true;
}

void VideoStitcher::setupStitchLayout() {
    LOG_INFO("Setting up stitch layout for " + std::to_string(num_streams_) + " streams");

    if (num_streams_ == 1) {
        // 单路直接输出
        stitch_config_.stream_positions[0] = {0, 0, output_width_, output_height_};
    } else if (num_streams_ == 2) {
        // 双路左右布局
        int half_width = output_width_ / 2;
        stitch_config_.stream_positions[0] = {0, 0, half_width, output_height_};
        stitch_config_.stream_positions[1] = {half_width, 0, half_width, output_height_};
    } else if (num_streams_ == 4) {
        // 四路2x2布局
        int half_width = output_width_ / 2;
        int half_height = output_height_ / 2;
        stitch_config_.stream_positions[0] = {0, 0, half_width, half_height};              // 左上
        stitch_config_.stream_positions[1] = {half_width, 0, half_width, half_height};     // 右上
        stitch_config_.stream_positions[2] = {0, half_height, half_width, half_height};    // 左下
        stitch_config_.stream_positions[3] = {half_width, half_height, half_width, half_height}; // 右下
    } else {
        // 默认布局：尽量方形排列
        int cols = static_cast<int>(std::ceil(std::sqrt(num_streams_)));
        int rows = (num_streams_ + cols - 1) / cols;

        int cell_width = output_width_ / cols;
        int cell_height = output_height_ / rows;

        for (int i = 0; i < num_streams_; ++i) {
            int row = i / cols;
            int col = i % cols;
            stitch_config_.stream_positions[i] = {
                col * cell_width,
                row * cell_height,
                cell_width,
                cell_height
            };
        }
    }

    LOG_INFO("Stitch layout configured");
}

void VideoStitcher::start() {
    if (running_) return;

    LOG_INFO("Starting VideoStitcher thread");
    running_ = true;
    stitch_thread_ = std::thread(&VideoStitcher::stitchThread, this);
}

void VideoStitcher::stop() {
    if (!running_) return;

    LOG_INFO("Stopping VideoStitcher");
    running_ = false;

    if (stitch_thread_.joinable()) {
        stitch_thread_.join();
    }

    LOG_INFO("VideoStitcher stopped, processed " + std::to_string(frames_processed_.load()) + " frames");
}

void VideoStitcher::stitchThread() {
    LOG_INFO("VideoStitcher thread started");

    while (running_) {
        if (processStitching()) {
            frames_processed_++;
        } else {
            // 没有足够的帧进行拼接，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    LOG_INFO("VideoStitcher thread stopped");
}

bool VideoStitcher::processStitching() {
    // 检查是否有足够的输入帧
    bool has_all_frames = true;
    for (int i = 0; i < num_streams_; ++i) {
        if (gpu_memory_manager_.isQueueEmpty(i)) {
            has_all_frames = false;
            break;
        }
    }

    if (!has_all_frames) {
        return false;
    }

    LOG_DEBUG("Starting stitching process for " + std::to_string(num_streams_) + " streams");

    // 从每个流的队列中获取最新帧
    std::vector<std::shared_ptr<GpuFrame>> input_frames;
    int64_t max_pts = 0;

    for (int i = 0; i < num_streams_; ++i) {
        auto frame = gpu_memory_manager_.popFrameFromQueue(i);
        if (frame) {
            input_frames.push_back(frame);
            max_pts = std::max(max_pts, frame->pts);
            LOG_DEBUG("Stream " + std::to_string(i) + " frame: " +
                     std::to_string(frame->width) + "x" + std::to_string(frame->height) +
                     ", PTS: " + std::to_string(frame->pts));
        } else {
            LOG_WARNING("Failed to get frame from stream " + std::to_string(i));
            // 释放已获取的帧
            for (auto& f : input_frames) {
                gpu_memory_manager_.freeFrame(f);
            }
            return false;
        }
    }

    // 执行CUDA拼接
    CudaFrame output_frame;
    output_frame.width = output_width_;
    output_frame.height = output_height_;
    output_frame.is_gpu_memory = true;

    // 分配输出GPU内存
    auto output_gpu_frame = gpu_memory_manager_.allocateFrame(output_width_, output_height_, true, -1);
    if (!output_gpu_frame) {
        LOG_ERROR("Failed to allocate output GPU frame");
        // 释放输入帧
        for (auto& f : input_frames) {
            gpu_memory_manager_.freeFrame(f);
        }
        return false;
    }

    // 关键：让 CUDA 拼接直接写到我们要推入输出队列的那块 GPU 内存里
    output_frame.nv12_data = reinterpret_cast<uint8_t*>(output_gpu_frame->gpu_ptr);

    // 转换输入帧格式
    std::vector<CudaFrame> cuda_input_frames;
    for (const auto& frame : input_frames) {
        CudaFrame cuda_frame;
        cuda_frame.nv12_data = reinterpret_cast<uint8_t*>(frame->gpu_ptr);
        cuda_frame.width = frame->width;
        cuda_frame.height = frame->height;
        cuda_frame.stream = nullptr;  // 使用默认CUDA流
        cuda_frame.is_gpu_memory = true;  // 数据已经在GPU内存中
        cuda_input_frames.push_back(cuda_frame);
    }

    if (cuda_stitcher_->stitchFrames(cuda_input_frames, output_frame)) {
        LOG_DEBUG("CUDA stitching completed successfully");

        // 轻量自检：抽样读取输出Y平面前64字节，判断是否仍为全黑（避免长期“黑屏”不知原因）
        static uint64_t sample_counter = 0;
        sample_counter++;
        if ((sample_counter % 60) == 0) { // 每60帧抽样一次
            uint8_t y_sample[64] = {0};
            if (gpu_memory_manager_.copyFromGpu(output_gpu_frame, y_sample, sizeof(y_sample))) {
                int non_zero = 0;
                for (uint8_t v : y_sample) {
                    if (v != 0) { non_zero++; }
                }
                LOG_INFO("Stitch output Y sample non-zero bytes: " + std::to_string(non_zero) + "/64");
            }
        }

        // 设置输出帧元数据
        output_gpu_frame->pts = max_pts;
        output_gpu_frame->width = output_width_;
        output_gpu_frame->height = output_height_;

        // 将拼接结果推送到输出队列（供编码器使用）
        if (!gpu_memory_manager_.pushFrameToQueue(-1, output_gpu_frame)) {  // -1表示输出队列
            LOG_ERROR("Failed to push stitched frame to output queue");
            gpu_memory_manager_.freeFrame(output_gpu_frame);
        } else {
            gpu_memory_manager_.setFrameReady(output_gpu_frame, true);
            LOG_DEBUG("Stitched frame pushed to output queue, PTS: " + std::to_string(max_pts));

            // 更新OpenGL显示（如果启用）
            if (platform_display_ && stitcher_region_id_ >= 0) {
                platform_display_->updateRegionFrame(stitcher_region_id_, output_gpu_frame);
            }
        }
    } else {
        LOG_ERROR("CUDA stitching failed");
        gpu_memory_manager_.freeFrame(output_gpu_frame);
    }

    // 释放输入帧
    for (auto& frame : input_frames) {
        gpu_memory_manager_.freeFrame(frame);
    }

    return true;
}
