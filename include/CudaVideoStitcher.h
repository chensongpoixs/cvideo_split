#pragma once

#include <cuda_runtime.h>
#include <vector>
#include <memory>

struct StitchConfig {
    int output_width = 1920;
    int output_height = 1080;
    int num_streams = 4;
    struct {
        int x = 0;
        int y = 0;
        int width = 960;
        int height = 540;
    } stream_positions[4];
};

struct CudaFrame {
    uint8_t* nv12_data = nullptr;
    int width = 0;
    int height = 0;
    cudaStream_t stream = nullptr;
    bool is_gpu_memory = false;  // true if nv12_data is already in GPU memory
};

class CudaVideoStitcher {
public:
    CudaVideoStitcher();
    ~CudaVideoStitcher();

    bool initialize(const StitchConfig& config);
    bool stitchFrames(const std::vector<CudaFrame>& input_frames, CudaFrame& output_frame);
    void cleanup();

private:
    bool allocateBuffers();
    void freeBuffers();

    StitchConfig config_;
    cudaStream_t cuda_stream_;

    // GPU buffers
    uint8_t* d_output_buffer_ = nullptr;
    uint8_t* d_temp_buffers_[4] = {nullptr, nullptr, nullptr, nullptr};
    size_t output_buffer_size_ = 0;
    size_t temp_buffer_size_[4] = {0, 0, 0, 0};

    bool initialized_ = false;
};
