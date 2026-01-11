/**
 * @file CudaVideoStitcher.h
 * @author chensong
 * @date 2026-01-11
 * @brief CUDA 视频拼接器（CUDA Video Stitcher）
 * 
 * 该模块使用 CUDA kernel 实现高效的多路视频拼接，支持任意分辨率输入
 * 和自定义布局输出。所有操作在 GPU 上完成，实现零 CPU 拷贝。
 * 
 * CUDA 拼接流程（CUDA Stitching Flow）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    CudaVideoStitcher                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   输入帧 (NV12, GPU)                                          |
 *  |   ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐            |
 *  |   │ Frame 0 │ │ Frame 1 │ │ Frame 2 │ │ Frame 3 │            |
 *  |   │ 1920x1080│ │1280x720 │ │ 640x480 │ │3840x2160│            |
 *  |   └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘            |
 *  |        │           │           │           │                  |
 *  |        ▼           ▼           ▼           ▼                  |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │              CUDA Kernel: launchNv12Resize              │|
 *  |   │  • 双线性缩放/裁剪                                       │|
 *  |   │  • 写入目标位置                                          │|
 *  |   │  • 处理 Y 和 UV plane                                    │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |        │                                                      |
 *  |        ▼                                                      |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │              输出帧 (NV12, GPU)                          │|
 *  |   │  ┌─────────┬─────────┐                                  │|
 *  |   │  │ Stream 0│ Stream 1│                                  │|
 *  |   │  ├─────────┼─────────┤  output_width x output_height    │|
 *  |   │  │ Stream 2│ Stream 3│                                  │|
 *  |   │  └─────────┴─────────┘                                  │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 拼接布局配置（Stitch Layout Configuration）：
 * 
 *   StitchConfig.stream_positions[i] = {x, y, width, height}
 *   
 *   2x2 布局示例:
 *   ┌────────────────────────────────────┐
 *   │  pos[0]         │  pos[1]         │
 *   │  (0,0,w/2,h/2)  │  (w/2,0,w/2,h/2)│
 *   ├────────────────────────────────────┤
 *   │  pos[2]         │  pos[3]         │
 *   │  (0,h/2,w/2,h/2)│  (w/2,h/2,w/2,h/2)
 *   └────────────────────────────────────┘
 * 
 * @note 使用统一的 CudaStreamManager 管理 CUDA stream
 * @note 输入输出均为 NV12 格式
 * @see VideoStitcher 上层封装
 * @see launchNv12Resize CUDA 缩放 kernel
 */

#pragma once

#include <cuda_runtime.h>
#include <vector>
#include <memory>

/**
 * @struct StitchConfig
 * @brief 拼接配置结构
 * 
 * 定义输出尺寸和各输入流在输出画面中的位置。
 */
struct StitchConfig {
    int output_width = 1920;     ///< 输出宽度（像素）
    int output_height = 1080;    ///< 输出高度（像素）
    int num_streams = 4;         ///< 输入流数量

    /**
     * @brief 各流在输出中的位置和尺寸
     * 
     * 每个流会被缩放到指定尺寸并放置在指定位置。
     */
    struct {
        int x = 0;               ///< 左上角 X 坐标
        int y = 0;               ///< 左上角 Y 坐标
        int width = 960;         ///< 目标宽度
        int height = 540;        ///< 目标高度
    } stream_positions[4];       ///< 最多 4 路流
};

/**
 * @struct CudaFrame
 * @brief CUDA 帧数据结构
 * 
 * 用于 CUDA 拼接操作的帧数据描述。
 */
struct CudaFrame {
    uint8_t* nv12_data = nullptr;  ///< NV12 数据指针
    int width = 0;                  ///< 帧宽度
    int height = 0;                 ///< 帧高度
    cudaStream_t stream = nullptr;  ///< 关联的 CUDA stream
    bool is_gpu_memory = false;     ///< true = GPU 内存，false = CPU 内存
};

/**
 * @class CudaVideoStitcher
 * @brief CUDA 视频拼接器
 * 
 * 使用 CUDA kernel 实现高效的多路视频拼接。
 * 
 * 使用示例：
 * @code
 * CudaVideoStitcher stitcher;
 * 
 * StitchConfig config;
 * config.output_width = 1920;
 * config.output_height = 1080;
 * config.num_streams = 4;
 * // 设置各流位置...
 * 
 * stitcher.initialize(config);
 * 
 * std::vector<CudaFrame> inputs(4);
 * // 填充输入帧...
 * 
 * CudaFrame output;
 * output.nv12_data = output_gpu_ptr;
 * stitcher.stitchFrames(inputs, output);
 * @endcode
 */
class CudaVideoStitcher {
public:
    /**
     * @brief 构造函数
     */
    CudaVideoStitcher();

    /**
     * @brief 析构函数
     */
    ~CudaVideoStitcher();

    /**
     * @brief 初始化拼接器
     * 
     * 分配 GPU 缓冲区，获取共享 CUDA stream。
     * 
     * @param config 拼接配置
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initialize(const StitchConfig& config);

    /**
     * @brief 执行拼接操作
     * 
     * 将多路输入帧拼接到一个输出帧。
     * 
     * 处理流程：
     * 1. 清空输出缓冲区（黑色背景）
     * 2. 对每个输入帧调用 CUDA 缩放 kernel
     * 3. 写入到输出对应位置
     * 4. 同步 stream
     * 
     * @param input_frames 输入帧数组
     * @param output_frame 输出帧（nv12_data 需预先分配）
     * @return true 拼接成功
     * @return false 拼接失败
     * 
     * @note 输入帧数量必须等于 config.num_streams
     * @note 输出 nv12_data 如为空，使用内部缓冲区
     */
    bool stitchFrames(const std::vector<CudaFrame>& input_frames, CudaFrame& output_frame);

    /**
     * @brief 清理资源
     */
    void cleanup();

private:
    /**
     * @brief 分配 GPU 缓冲区
     * @return true 成功
     * @return false 失败
     */
    bool allocateBuffers();

    /**
     * @brief 释放 GPU 缓冲区
     */
    void freeBuffers();

    StitchConfig config_;            ///< 拼接配置
    cudaStream_t cuda_stream_;       ///< CUDA stream（从 CudaStreamManager 获取）

    /// GPU 输出缓冲区（内部备用）
    uint8_t* d_output_buffer_ = nullptr;
    /// GPU 临时缓冲区（用于 CPU→GPU 拷贝）
    uint8_t* d_temp_buffers_[4] = {nullptr, nullptr, nullptr, nullptr};
    size_t output_buffer_size_ = 0;          ///< 输出缓冲区大小
    size_t temp_buffer_size_[4] = {0, 0, 0, 0}; ///< 临时缓冲区大小

    bool initialized_ = false;       ///< 初始化状态
};
