#include "Logger.h"
#include "VideoDecoder.h"
#include "VideoStitcher.h"
#include "VideoEncoder.h"
#include "GpuMemoryManager.h"
#include "Config.h"
#include "PlatformDisplay.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

std::atomic<bool> running(true);

void signalHandler(int signum) {
    LOG_INFO("Received signal " + std::to_string(signum) + ", stopping...");
    running = false;
}

int main(int argc, char* argv[]) {
    // 加载配置
    Config& config = Config::getInstance();
    if (!config.loadFromArgs(argc, argv)) {
        return 1;  // 配置错误或显示帮助信息
    }

    const VideoSystemConfig& sys_config = config.getConfig();

    // 初始化日志系统
    Logger::getInstance().setLogFile("video_stitch_system.log");
    Logger::getInstance().setLogLevel(sys_config.enable_debug_logging ? LogLevel::DEBUG_LEVEL : LogLevel::INFO_LEVEL);

    LOG_INFO("Starting Multi-Stream Video Stitching System");

    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 初始化FFmpeg
    avformat_network_init();
    LOG_DEBUG("FFmpeg network initialized");

    // 从配置获取参数
    const int NUM_STREAMS = sys_config.num_streams;
    const int OUTPUT_WIDTH = sys_config.output_width;
    const int OUTPUT_HEIGHT = sys_config.output_height;

    LOG_INFO("Number of input streams: " + std::to_string(NUM_STREAMS));
    LOG_INFO("Output resolution: " + std::to_string(OUTPUT_WIDTH) + "x" + std::to_string(OUTPUT_HEIGHT));

    try {
        // 初始化GPU内存管理器
        LOG_INFO("Initializing GPU Memory Manager");
        if (!GpuMemoryManager::getInstance().initialize()) {
            LOG_ERROR("Failed to initialize GPU Memory Manager");
            return -1;
        }

        // 组件声明
        std::vector<std::unique_ptr<VideoDecoder>> video_decoders;
        std::unique_ptr<VideoStitcher> video_stitcher;
        std::unique_ptr<VideoEncoder> video_encoder;
        std::unique_ptr<PlatformDisplay> platform_display;
        // 显示区域ID（用于把各路视频绑定到对应区域）
        std::vector<int> decoder_region_ids;
        int stitcher_region_id = -1;

        // 初始化平台显示器
        if (sys_config.display_config.enable_decoder_display || sys_config.display_config.enable_stitcher_display) {
            LOG_INFO("Initializing Platform Video Display");
            platform_display = createPlatformDisplay();
            if (!platform_display) {
                LOG_ERROR("Failed to create platform display");
                return -1;
            }

            // 显示布局：
            // - 左侧：最多4路解码画面（2x2）
            // - 右侧：1路拼接画面
            const int margin = 10;
            const bool show_decoded = sys_config.display_config.enable_decoder_display;
            const bool show_stitched = sys_config.display_config.enable_stitcher_display;

            // 默认窗口大小：屏幕宽高各一半（若开启fullscreen则忽略）
            int display_width = OUTPUT_WIDTH;
            int display_height = OUTPUT_HEIGHT;
#ifdef _WIN32
            if (!sys_config.display_config.enable_fullscreen) {
                display_width = std::max(640, GetSystemMetrics(SM_CXSCREEN) / 2);
                display_height = std::max(360, GetSystemMetrics(SM_CYSCREEN) / 2);
            }
#else
            // 非Windows平台暂时保持输出分辨率作为窗口尺寸（后续可在各显示后端用原生API取屏幕尺寸）
            (void)show_decoded;
            (void)show_stitched;
#endif

            if (!platform_display->initialize(display_width, display_height, "Video Stitching System",
                                              sys_config.display_config.enable_fullscreen)) {
                LOG_ERROR("Failed to initialize platform display");
                return -1;
            }

            // 添加显示区域
            decoder_region_ids.assign(NUM_STREAMS, -1);
            stitcher_region_id = -1;

            if (show_decoded) {
                const int max_preview = std::min(NUM_STREAMS, 4);
                const int left_x = 0;
                const int left_w = show_stitched ? (display_width / 2) : display_width;
                const int left_h = display_height;

                const int cell_w = (left_w - margin * 3) / 2;
                const int cell_h = (left_h - margin * 3) / 2;

                for (int i = 0; i < max_preview; ++i) {
                    const int row = i / 2;
                    const int col = i % 2;
                    const int x = left_x + margin + col * (cell_w + margin);
                    const int y = margin + row * (cell_h + margin);

                    const std::string title = sys_config.display_config.decoder_window_title + " " + std::to_string(i);
                    int rid = platform_display->addDisplayRegion(x, y, cell_w, cell_h, title);
                    decoder_region_ids[i] = rid;
                    if (rid < 0) {
                        LOG_ERROR("Failed to add decoder display region for stream " + std::to_string(i));
                    } else {
                        LOG_INFO("Decoder region for stream " + std::to_string(i) + " -> region_id=" + std::to_string(rid));
                    }
                }
            }

            if (show_stitched) {
                const int stitched_x = show_decoded ? (display_width / 2) : 0;
                const int stitched_w = show_decoded ? (display_width / 2) : display_width;
                const int stitched_h = display_height;
                const int x = stitched_x + margin;
                const int y = margin;
                const int w = stitched_w - margin * 2;
                const int h = stitched_h - margin * 2;

                stitcher_region_id = platform_display->addDisplayRegion(x, y, w, h,
                                                                        sys_config.display_config.stitcher_window_title);
                if (stitcher_region_id < 0) {
                    LOG_ERROR("Failed to add stitcher display region");
                } else {
                    LOG_INFO("Stitcher region -> region_id=" + std::to_string(stitcher_region_id));
                }
            }

            platform_display->setDisplayFPS(sys_config.display_config.display_fps);
            LOG_INFO("Platform display initialized successfully");

            // 把区域ID绑定到对应解码器/拼接器（注意：先创建区域，再初始化decoder/stitcher会更清晰；
            // 这里先缓存，后面初始化decoder/stitcher时再设置）
        }

        // 初始化视频解码器（每个流一个）
        for (int i = 0; i < NUM_STREAMS; i++) {
            LOG_INFO("Initializing VideoDecoder for stream " + std::to_string(i));
            auto decoder = std::make_unique<VideoDecoder>(i);
            if (!decoder->initialize(sys_config.rtsp_urls[i], platform_display.get())) {
                LOG_ERROR("Failed to initialize VideoDecoder for stream " + std::to_string(i));
                return -1;
            }

            // 设置解码器显示区域ID
            if (platform_display && sys_config.display_config.enable_decoder_display) {
                if (i < static_cast<int>(decoder_region_ids.size()) && decoder_region_ids[i] >= 0) {
                    decoder->setDecoderRegionId(decoder_region_ids[i]);
                }
            }

            video_decoders.push_back(std::move(decoder));
            LOG_INFO("VideoDecoder " + std::to_string(i) + " initialized successfully");
        }

        // 初始化视频拼接器
        LOG_INFO("Initializing VideoStitcher");
        video_stitcher = std::make_unique<VideoStitcher>();
        if (!video_stitcher->initialize(NUM_STREAMS, OUTPUT_WIDTH, OUTPUT_HEIGHT, platform_display.get())) {
            LOG_ERROR("Failed to initialize VideoStitcher");
            return -1;
        }

        // 设置拼接器显示区域ID
        if (platform_display && sys_config.display_config.enable_stitcher_display) {
            if (stitcher_region_id >= 0) {
                video_stitcher->setStitcherRegionId(stitcher_region_id);
            }
        }

        LOG_INFO("VideoStitcher initialized successfully");

        // 初始化视频编码器
        LOG_INFO("Initializing VideoEncoder");
        video_encoder = std::make_unique<VideoEncoder>();
        if (!video_encoder->initialize(OUTPUT_WIDTH, OUTPUT_HEIGHT, sys_config.fps, sys_config.output_url)) {
            LOG_ERROR("Failed to initialize VideoEncoder");
            return -1;
        }
        LOG_INFO("VideoEncoder initialized successfully");

        LOG_INFO("All GPU pipeline components initialized successfully, starting video processing");

        // 启动所有视频解码器
        for (auto& decoder : video_decoders) {
            decoder->start();
        }

        // 启动拼接器
        video_stitcher->start();

        // 平台显示器指针已在initialize阶段传递给decoder/stitcher，这里无需再次设置

        // 启动编码器
        video_encoder->start();
        LOG_INFO("All GPU pipeline threads started");

        // 主监控循环
        int processed_frames = 0;
        const int MAX_FRAMES = 1000000; // 处理10帧用于测试

        while (running && processed_frames < MAX_FRAMES) {
            // 显示处理统计信息
            uint64_t total_decoded = 0;
            uint64_t total_stitched = video_stitcher ? video_stitcher->getFramesProcessed() : 0;
            uint64_t total_encoded = video_encoder ? video_encoder->getFramesEncoded() : 0;

            for (const auto& decoder : video_decoders) {
                if (decoder) {
                    total_decoded += decoder->getFramesDecoded();
                }
            }

            LOG_INFO("Processing status - Decoded: " + std::to_string(total_decoded) +
                     ", Stitched: " + std::to_string(total_stitched) +
                     ", Encoded: " + std::to_string(total_encoded));

            // 更新平台显示
            if (platform_display && platform_display->isInitialized()) {
                platform_display->render();
                platform_display->processEvents();
                if (platform_display->shouldClose()) {
                    LOG_INFO("Display window closed by user");
                    running = false;
                }
            }

            // 检查队列状态
            for (int i = 0; i < NUM_STREAMS; ++i) {
                bool queue_empty = GpuMemoryManager::getInstance().isQueueEmpty(i);
                if (queue_empty) {
                    LOG_DEBUG("Stream " + std::to_string(i) + " queue is empty");
                } else {
                    LOG_DEBUG("Stream " + std::to_string(i) + " has frames in queue");
                }
            }

            bool output_queue_empty = GpuMemoryManager::getInstance().isQueueEmpty(-1);
            if (output_queue_empty) {
                LOG_DEBUG("Output queue is empty");
            } else {
                LOG_DEBUG("Output queue has frames ready for encoding");
            }

            // 渲染显示（如果启用）
            /* 已经在上面更新了平台显示
            if (video_display && video_display->isOpen()) {
                if (!video_display->renderAndDisplay()) {
                    LOG_WARNING("Video display render failed");
                }

                // 检查窗口是否应该关闭
                if (!video_display->isOpen()) {
                    LOG_INFO("Display window closed by user");
                    running = false;
                }
            }
            */

            processed_frames++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30)); // 30 FPS显示更新
        }

        LOG_INFO("Processing completed, stopping all threads");

        // 停止所有组件
        for (auto& decoder : video_decoders) {
            decoder->stop();
        }

        if (video_stitcher) {
            video_stitcher->stop();
        }

        if (video_encoder) {
            video_encoder->stop();
        }
        LOG_INFO("All RTSP stream pullers stopped");

        // 刷新编码器
        if (video_encoder) {
            EncodedPacket flush_packet;
            while (video_encoder->flush(flush_packet)) {
                video_encoder->writePacket(flush_packet);
                delete[] flush_packet.data;
            }
            LOG_INFO("Video encoder flushed");
        }

        LOG_INFO("GPU pipeline processing completed");
    } catch (const std::exception& e) {
        LOG_FATAL("Exception occurred: " + std::string(e.what()));
        return -1;
    }

    LOG_INFO("Video stitching system stopped");
    return 0;
}
