/**
 * @file HardwareDecoder.h
 * @author chensong
 * @date 2026-01-11
 * @brief CUDA 硬件解码器（CUDA Hardware Decoder - NVDEC）
 * 
 * 该模块封装 FFmpeg 的 NVDEC 硬件解码功能，提供高效的 GPU 视频解码。
 * 解码输出直接在 GPU 内存中，无需 GPU→CPU 拷贝。
 * 
 * NVDEC 解码流程（NVDEC Decoding Flow）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     HardwareDecoder                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   AVPacket (H.264/H.265 压缩数据)                             |
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │                    FFmpeg                                │|
 *  |   │  avcodec_send_packet() ──► NVDEC (GPU 硬件解码)          │|
 *  |   │  avcodec_receive_frame() ◄── 返回 AV_PIX_FMT_CUDA 帧    │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   DecodedFrame (GPU NV12 + metadata)                         |
 *  |   • nv12_data: GPU 内存指针                                   |
 *  |   • linesize_y/linesize_uv: pitch (可能 > width)             |
 *  |   • av_frame_ref: FFmpeg 帧引用（控制 GPU surface 生命周期）  |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * Pitched Memory 说明（Pitched Memory Description）：
 * 
 *   NVDEC 输出的 GPU 内存通常是 pitched（跨距）格式：
 *   
 *   ┌───────────────────────────────────────────────────────────────┐
 *   │                    Y Plane (亮度)                             │
 *   │  Row 0:  [Y0 Y1 Y2 ... Yw-1] [padding...]   ← linesize_y     │
 *   │  Row 1:  [Y0 Y1 Y2 ... Yw-1] [padding...]                    │
 *   │  ...                                                          │
 *   │  Row h-1:[Y0 Y1 Y2 ... Yw-1] [padding...]                    │
 *   ├───────────────────────────────────────────────────────────────┤
 *   │                    UV Plane (色度，高度 = h/2)               │
 *   │  Row 0:  [UV0 UV1 ... UVw/2-1] [padding...]← linesize_uv    │
 *   │  ...                                                          │
 *   └───────────────────────────────────────────────────────────────┘
 *   
 *   linesize_y >= width (例如 4K 可能 pitch=4096, width=3840)
 * 
 * 支持的编解码器（Supported Codecs）：
 *   - H.264 (AVC)  → h264_cuvid
 *   - H.265 (HEVC) → hevc_cuvid
 *   - 回退: 软件解码器 (libavcodec)
 * 
 * @note 需要 NVIDIA GPU 和 CUDA 驱动
 * @note 输出格式固定为 NV12 (AV_PIX_FMT_CUDA)
 * @see VideoDecoder 视频解码器
 * @see GpuMemoryManager GPU 内存管理
 */

#pragma once

#include <cuda_runtime.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_cuda.h>
}

#include <string>
#include <memory>

/**
 * @struct DecodedFrame
 * @brief 解码后的帧数据结构
 * 
 * 包含解码输出的 GPU 内存指针、尺寸、时间戳等元数据。
 * 
 * 内存布局（NV12 格式）：
 * @code
 * Y plane:  nv12_data + 0
 * UV plane: nv12_data + linesize_y * height
 * @endcode
 */
struct DecodedFrame {
    uint8_t* nv12_data = nullptr;       ///< GPU 内存指针（NV12 数据起始地址）
    cudaIpcMemHandle_t mem_handle;       ///< IPC 内存句柄（进程间共享）
    AVFrame* av_frame_ref = nullptr;     ///< FFmpeg 帧引用（控制 GPU surface 生命周期）
    int linesize_y = 0;                  ///< Y plane 行跨距（bytes/row, 可能 > width）
    int linesize_uv = 0;                 ///< UV plane 行跨距（bytes/row）
    int width = 0;                       ///< 帧宽度（像素）
    int height = 0;                      ///< 帧高度（像素）
    int64_t pts = 0;                     ///< 时间戳（presentation timestamp）
    size_t data_size = 0;                ///< 总数据大小（字节）
    bool is_cuda_frame = false;          ///< true = GPU 内存，false = CPU 内存
    bool is_nv12 = true;                 ///< true = NV12，false = YUV420P
    bool owns_memory = true;             ///< true = 拥有内存所有权
};

/**
 * @class HardwareDecoder
 * @brief CUDA 硬件解码器封装
 * 
 * 封装 FFmpeg 的 NVDEC 解码功能，提供简洁的解码接口。
 * 
 * 使用示例：
 * @code
 * HardwareDecoder decoder;
 * decoder.initializeWithCodecParams(codec_params);
 * 
 * DecodedFrame frame;
 * if (decoder.decodePacket(packet, frame)) {
 *     // 使用 frame.nv12_data（GPU 内存）
 *     // 注意：frame.av_frame_ref 需要正确管理生命周期
 * }
 * @endcode
 */
class HardwareDecoder {
public:
    /**
     * @brief 构造函数
     */
    HardwareDecoder();

    /**
     * @brief 析构函数
     * @note 自动调用 cleanup() 释放资源
     */
    ~HardwareDecoder();

    /**
     * @brief 使用编解码器名称初始化
     * 
     * @param codec_name 编解码器名称 ("h264" 或 "h265"/"hevc")
     * @return true 初始化成功
     * @return false 初始化失败
     * 
     * @note 自动选择硬件解码器，失败时回退软件解码
     */
    bool initialize(const std::string& codec_name = "h264");

    /**
     * @brief 使用 FFmpeg 编解码参数初始化
     * 
     * 从 AVCodecParameters 获取编解码信息并初始化。
     * 
     * @param codec_params FFmpeg 编解码参数
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initializeWithCodecParams(const AVCodecParameters* codec_params);

    /**
     * @brief 解码单个视频包
     * 
     * 将压缩的视频包解码为 GPU NV12 帧。
     * 
     * 解码流程：
     * 1. avcodec_send_packet() - 发送压缩数据
     * 2. avcodec_receive_frame() - 接收解码帧
     * 3. 填充 DecodedFrame 结构
     * 4. 创建 av_frame_ref 引用
     * 
     * @param packet 输入视频包
     * @param decoded_frame 输出解码帧
     * @return true 解码成功并产生输出帧
     * @return false 解码失败或需要更多输入（EAGAIN）
     * 
     * @note decoded_frame.av_frame_ref 需要调用者管理（av_frame_free）
     * @note GPU surface 生命周期由 av_frame_ref 控制
     */
    bool decodePacket(const AVPacket* packet, DecodedFrame& decoded_frame);

    /**
     * @brief 刷新解码器
     * 
     * 发送 NULL 包以刷新解码器缓冲区中的剩余帧。
     */
    void flush();

private:
    /**
     * @brief 初始化 CUDA 硬件上下文
     * @return true 成功
     * @return false 失败
     */
    bool initHWContext();

    /**
     * @brief 清理资源
     */
    void cleanup();

    const AVCodec* codec_ = nullptr;           ///< FFmpeg 编解码器
    AVCodecContext* codec_ctx_ = nullptr;      ///< 编解码器上下文
    AVBufferRef* hw_device_ctx_ = nullptr;     ///< CUDA 硬件设备上下文
    AVFrame* hw_frame_ = nullptr;              ///< 硬件帧（GPU 内存）
    AVFrame* sw_frame_ = nullptr;              ///< 软件帧（回退用）

    enum AVHWDeviceType hw_type_ = AV_HWDEVICE_TYPE_CUDA;  ///< 硬件类型
    bool initialized_ = false;                 ///< 初始化状态
};
