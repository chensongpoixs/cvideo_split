#ifndef CUDA_VIDEO_PROCESSOR_H
#define CUDA_VIDEO_PROCESSOR_H

#include <cuda_runtime.h>
#include <stdint.h>
#include <vector>

/**
 * @brief CUDA视频处理器 - 显存数据合并、裁剪和缩放
 * @author GB28181 Team
 * @date 2026-02-13
 */

namespace cuda {

// 视频格式
enum class VideoFormat {
    NV12,    // YUV 4:2:0, Y平面 + UV交错平面
    I420,    // YUV 4:2:0, Y平面 + U平面 + V平面
    RGBA     // RGBA 8888
};

// 插值方法
enum class InterpolationMethod {
    NEAREST,     // 最近邻插值
    BILINEAR,    // 双线性插值
    BICUBIC,     // 双三次插值
    LANCZOS      // Lanczos插值
};

// 视频帧结构
struct VideoFrame {
    uint8_t* d_data;      // GPU显存指针
    int width;            // 宽度
    int height;           // 高度
    int pitch;            // 行跨度（字节）
    VideoFormat format;   // 格式
    
    VideoFrame() : d_data(nullptr), width(0), height(0), pitch(0), 
                   format(VideoFormat::NV12) {}
};

// ROI区域
struct ROI {
    int x;          // 左上角X坐标
    int y;          // 左上角Y坐标
    int width;      // 宽度
    int height;     // 高度
    
    ROI() : x(0), y(0), width(0), height(0) {}
    ROI(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_) {}
};

// 合并布局
struct MergeLayout {
    int rows;       // 行数
    int cols;       // 列数
    int cellWidth;  // 单元格宽度
    int cellHeight; // 单元格高度
    
    MergeLayout() : rows(0), cols(0), cellWidth(0), cellHeight(0) {}
    MergeLayout(int r, int c, int w, int h) 
        : rows(r), cols(c), cellWidth(w), cellHeight(h) {}
};


/**
 * @brief CUDA视频处理器类
 */
class CudaVideoProcessor {
public:
    CudaVideoProcessor();
    ~CudaVideoProcessor();
    
    /**
     * @brief 初始化CUDA环境
     * @param deviceId GPU设备ID
     * @return true成功，false失败
     */
    bool initialize(int deviceId = 0);
    
    /**
     * @brief 释放资源
     */
    void release();
    
    /**
     * @brief 合并多路视频
     * @param inputs 输入视频帧数组
     * @param numInputs 输入数量
     * @param output 输出视频帧
     * @param layout 合并布局
     * @param stream CUDA流（可选）
     * @return true成功，false失败
     */
    bool mergeVideos(const VideoFrame* inputs, int numInputs,
                     VideoFrame& output, const MergeLayout& layout,
                     cudaStream_t stream = 0);
    
    /**
     * @brief 裁剪视频
     * @param input 输入视频帧
     * @param output 输出视频帧
     * @param roi 裁剪区域
     * @param stream CUDA流（可选）
     * @return true成功，false失败
     */
    bool cropVideo(const VideoFrame& input, VideoFrame& output,
                   const ROI& roi, cudaStream_t stream = 0);
    
    /**
     * @brief 缩放视频
     * @param input 输入视频帧
     * @param output 输出视频帧
     * @param method 插值方法
     * @param stream CUDA流（可选）
     * @return true成功，false失败
     */
    bool scaleVideo(const VideoFrame& input, VideoFrame& output,
                    InterpolationMethod method = InterpolationMethod::BILINEAR,
                    cudaStream_t stream = 0);
    
    /**
     * @brief 分配GPU显存
     * @param frame 视频帧
     * @return true成功，false失败
     */
    bool allocateFrame(VideoFrame& frame);
    
    /**
     * @brief 释放GPU显存
     * @param frame 视频帧
     */
    void freeFrame(VideoFrame& frame);
    
    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    const char* getLastError() const { return m_lastError.c_str(); }
    
private:
    int m_deviceId;
    bool m_initialized;
    std::string m_lastError;
    
    // 设置错误信息
    void setError(const char* error);
    
    // 检查CUDA错误
    bool checkCudaError(cudaError_t err, const char* msg);
};

} // namespace cuda

#endif // CUDA_VIDEO_PROCESSOR_H
