#include "Config.h"
#include <iostream>
#include <cstring>

bool Config::loadFromArgs(int argc, char* argv[]) {
    // 默认配置
    config_.rtsp_urls = {
            "rtsp://admin:hik@12345@192.168.9.161/streaming/channels/101",
            "rtsp://admin:cs@563519@192.168.9.162/streaming/channels/101",
            
        "rtsp://admin:hik@12345@192.168.9.166/streaming/channels/101",
        
        "rtsp://admin:cs@563519@192.168.9.162/streaming/channels/101"
    };
    config_.num_streams = 4;
    config_.output_width = 1920;
    config_.output_height = 1080;
    config_.fps = 30;
    config_.output_url = "rtsp://127.0.0.1:554/live/test";
    config_.max_frames_to_process = 100;
    // 默认开启显示（可通过参数关闭）
    config_.display_config.enable_decoder_display = true;
    config_.display_config.enable_stitcher_display = true;

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "Video Stitching System Options:" << std::endl;
            std::cout << "  --rtsp-url <url>          RTSP stream URL" << std::endl;
            std::cout << "  --num-streams <n>         Number of input streams" << std::endl;
            std::cout << "  --output-width <w>        Output video width" << std::endl;
            std::cout << "  --output-height <h>       Output video height" << std::endl;
            std::cout << "  --fps <fps>               Output video FPS" << std::endl;
            std::cout << "  --output-url <url>        RTSP output URL" << std::endl;
            std::cout << "  --no-show-decoded         Disable decoded video display" << std::endl;
            std::cout << "  --no-show-stitched        Disable stitched video display" << std::endl;
            std::cout << "  --fullscreen              Enable fullscreen display" << std::endl;
            std::cout << "  --max-frames <n>          Maximum frames to process (0 = unlimited)" << std::endl;
            std::cout << "  --debug                   Enable debug logging" << std::endl;
            return false;
        }
        else if (strcmp(argv[i], "--rtsp-url") == 0 && i + 1 < argc) {
            config_.rtsp_urls.clear();
            config_.rtsp_urls.push_back(argv[++i]);
        }
        else if (strcmp(argv[i], "--num-streams") == 0 && i + 1 < argc) {
            config_.num_streams = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--output-width") == 0 && i + 1 < argc) {
            config_.output_width = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--output-height") == 0 && i + 1 < argc) {
            config_.output_height = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) {
            config_.fps = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--output-url") == 0 && i + 1 < argc) {
            config_.output_url = argv[++i];
        }
        else if (strcmp(argv[i], "--no-show-decoded") == 0) {
            config_.display_config.enable_decoder_display = false;
        }
        else if (strcmp(argv[i], "--no-show-stitched") == 0) {
            config_.display_config.enable_stitcher_display = false;
        }
        else if (strcmp(argv[i], "--fullscreen") == 0) {
            config_.display_config.enable_fullscreen = true;
        }
        else if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) {
            config_.max_frames_to_process = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--debug") == 0) {
            config_.enable_debug_logging = true;
        }
        else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            std::cerr << "Use --help for usage information" << std::endl;
            return false;
        }
    }

    // 如果指定了多个流，但只提供了一个URL，则复制URL
    if (config_.num_streams > 1 && config_.rtsp_urls.size() == 1) {
        std::string base_url = config_.rtsp_urls[0];
        config_.rtsp_urls.clear();
        for (int i = 0; i < config_.num_streams; ++i) {
            config_.rtsp_urls.push_back(base_url);
        }
    }

    return true;
}
