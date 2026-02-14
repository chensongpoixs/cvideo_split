#include "CudaVideoProcessor.h"
#include <stdio.h>
#include <string.h>

namespace cuda {

// ============================================================================
// CUDA Kernels
// ============================================================================

/**
 * @brief NV12格式视频合并Kernel - 将多个视频帧合并到一个输出帧中
 * 
 * 功能说明：
 * 该kernel将多个输入视频按照网格布局（rows x cols）合并到一个输出视频帧中。
 * 每个输入视频占据输出帧的一个单元格（cell），支持不同尺寸的输入视频。
 * 
 * NV12格式说明：
 * - Y平面：存储亮度信息，分辨率为 width x height
 * - UV平面：存储色度信息，分辨率为 width x (height/2)，UV交错存储
 * 
 * 处理流程图：
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 1. 计算当前线程处理的输出像素坐标 (x, y)                      │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 2. 边界检查：判断坐标是否在输出范围内                         │
 * │    - Y平面：x < outputWidth && y < outputHeight              │
 * │    - UV平面：x < outputWidth && y < outputHeight/2           │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 3. 计算像素所属的单元格位置                                   │
 * │    cellX = x / cellWidth                                    │
 * │    cellY = y / cellHeight                                   │
 * │    inputIdx = cellY * cols + cellX                          │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 *              ┌───────────┴───────────┐
 *              ↓                       ↓
 *    ┌──────────────────┐    ┌──────────────────┐
 *    │ inputIdx >= num? │    │ inputIdx < num?  │
 *    │ 填充默认值：      │    │ 从输入拷贝像素   │
 *    │ Y=0, UV=128      │    └──────────────────┘
 *    └──────────────────┘              ↓
 *                          ┌─────────────────────────────────────┐
 *                          │ 4. 计算在输入视频中的局部坐标        │
 *                          │    localX = x - cellX * cellWidth   │
 *                          │    localY = y - cellY * cellHeight  │
 *                          │    (UV平面：localY /= 2)            │
 *                          └─────────────────────────────────────┘
 *                                        ↓
 *                          ┌─────────────────────────────────────┐
 *                          │ 5. 边界检查：局部坐标是否超出输入   │
 *                          │    范围？超出则填充默认值            │
 *                          └─────────────────────────────────────┘
 *                                        ↓
 *                          ┌─────────────────────────────────────┐
 *                          │ 6. 从输入帧读取像素值并写入输出      │
 *                          │    - Y平面：直接从输入Y平面读取      │
 *                          │    - UV平面：从输入UV平面读取        │
 *                          │      (偏移 = inputHeight * pitch)   │
 *                          └─────────────────────────────────────┘
 * 
 * @param inputs 输入视频帧数据指针数组（GPU内存）
 * @param numInputs 输入视频数量
 * @param inputWidths 每个输入视频的宽度数组
 * @param inputHeights 每个输入视频的高度数组
 * @param inputPitches 每个输入视频的pitch（行字节数）数组
 * @param output 输出视频帧数据指针（GPU内存）
 * @param outputWidth 输出视频宽度
 * @param outputHeight 输出视频高度
 * @param outputPitch 输出视频pitch
 * @param rows 网格布局行数
 * @param cols 网格布局列数
 * @param cellWidth 每个单元格宽度
 * @param cellHeight 每个单元格高度
 * @param isUVPlane 是否处理UV平面（false表示处理Y平面）
 */
__global__ void mergeNV12Kernel(const uint8_t** inputs, int numInputs,
                                const int* inputWidths, const int* inputHeights,
                                const int* inputPitches,
                                uint8_t* output, int outputWidth, int outputHeight,
                                int outputPitch, int rows, int cols,
                                int cellWidth, int cellHeight, bool isUVPlane) {
    // ========== 步骤1：计算当前线程处理的输出像素坐标 ==========
    int x = blockIdx.x * blockDim.x + threadIdx.x;  // 输出帧的x坐标
    int y = blockIdx.y * blockDim.y + threadIdx.y;  // 输出帧的y坐标
    
    // ========== 步骤2：边界检查 ==========
    if (isUVPlane) {
        // UV平面处理（NV12格式UV平面高度为Y平面的一半）
        if (x >= outputWidth || y >= outputHeight / 2) return;
    } else {
        // Y平面处理（亮度平面）
        if (x >= outputWidth || y >= outputHeight) return;
    }
    
    // ========== 步骤3：计算当前像素属于哪个输入视频 ==========
    // 根据输出坐标和单元格尺寸，确定该像素属于哪个网格单元
    int cellX = x / cellWidth;  // 单元格列索引
    int cellY = y / cellHeight; // 单元格行索引
    int inputIdx = cellY * cols + cellX;  // 输入视频索引（行优先）
    
    // 检查是否超出输入数量（网格中可能有空单元格）
    if (inputIdx >= numInputs) {
        // 超出输入数量，填充默认值（黑色背景）
        if (isUVPlane) {
            output[y * outputPitch + x] = 128;  // UV中性值（灰色）
        } else {
            output[y * outputPitch + x] = 0;    // Y黑色
        }
        return;
    }
    
    // ========== 步骤4：计算在输入视频中的相对坐标 ==========
    // 将输出坐标转换为输入视频内的局部坐标
    int localX = x - cellX * cellWidth;   // 在单元格内的x偏移
    int localY = y - cellY * cellHeight;  // 在单元格内的y偏移
    
    if (isUVPlane) {
        localY = localY / 2;  // UV平面高度减半，需要调整y坐标
    }
    
    // ========== 步骤5：边界检查（输入视频可能小于单元格） ==========
    // 检查局部坐标是否超出输入视频的实际尺寸
    if (localX >= inputWidths[inputIdx] || 
        localY >= (isUVPlane ? inputHeights[inputIdx] / 2 : inputHeights[inputIdx])) {
        // 超出输入视频范围，填充默认值
        if (isUVPlane) {
            output[y * outputPitch + x] = 128;  // UV中性值
        } else {
            output[y * outputPitch + x] = 0;    // Y黑色
        }
        return;
    }
    
    // ========== 步骤6：从输入拷贝像素到输出 ==========
    const uint8_t* inputData = inputs[inputIdx];
    
    if (isUVPlane) {
        // UV平面：需要偏移到UV数据起始位置
        // NV12格式中，UV平面紧跟在Y平面之后
        inputData += inputHeights[inputIdx] * inputPitches[inputIdx];
        output[y * outputPitch + x] = inputData[localY * inputPitches[inputIdx] + localX];
    } else {
        // Y平面：直接从输入Y平面读取
        output[y * outputPitch + x] = inputData[localY * inputPitches[inputIdx] + localX];
    }
}


/**
 * @brief NV12格式视频裁剪Kernel - 从输入视频中裁剪指定区域
 * 
 * 功能说明：
 * 该kernel从输入视频帧中裁剪出一个矩形区域（ROI - Region of Interest），
 * 并将裁剪结果写入输出帧。支持NV12格式的Y平面和UV平面分别处理。
 * 
 * ROI说明：
 * - roiX, roiY：裁剪区域在输入帧中的起始坐标（左上角）
 * - outputWidth, outputHeight：裁剪区域的尺寸
 * - 对于NV12格式，ROI坐标和尺寸必须是偶数对齐
 * 
 * 处理流程图：
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 1. 计算当前线程处理的输出像素坐标 (x, y)                      │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 2. 边界检查：判断坐标是否在输出范围内                         │
 * │    - Y平面：x < outputWidth && y < outputHeight              │
 * │    - UV平面：x < outputWidth && y < outputHeight/2           │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 *              ┌───────────┴───────────┐
 *              ↓                       ↓
 *    ┌──────────────────┐    ┌──────────────────┐
 *    │   处理Y平面      │    │   处理UV平面     │
 *    └──────────────────┘    └──────────────────┘
 *              ↓                       ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 3. 计算源坐标（输入帧中的位置）                               │
 * │    Y平面：srcX = roiX + x, srcY = roiY + y                  │
 * │    UV平面：srcX = roiX + x, srcY = roiY/2 + y               │
 * │    (UV平面高度减半，所以roiY需要除以2)                       │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 4. 边界检查：源坐标是否在输入帧范围内                         │
 * └─────────────────────────────────────────────────────────────┘
 *              ↓                       ↓
 *    ┌──────────────────┐    ┌──────────────────┐
 *    │ 在范围内：       │    │ 超出范围：       │
 *    │ 从输入拷贝像素   │    │ 填充默认值       │
 *    │                  │    │ Y=0, UV=128      │
 *    └──────────────────┘    └──────────────────┘
 *              ↓                       ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 5. 写入输出帧                                                 │
 * │    output[y * outputPitch + x] = pixel_value                │
 * └─────────────────────────────────────────────────────────────┘
 * 
 * 内存布局示意：
 * 输入帧：[Y平面: inputHeight * inputPitch][UV平面: inputHeight/2 * inputPitch]
 * 输出帧：[Y平面: outputHeight * outputPitch][UV平面: outputHeight/2 * outputPitch]
 * 
 * @param input 输入视频帧数据指针（GPU内存）
 * @param inputWidth 输入视频宽度
 * @param inputHeight 输入视频高度
 * @param inputPitch 输入视频pitch（行字节数）
 * @param output 输出视频帧数据指针（GPU内存）
 * @param outputWidth 输出视频宽度（裁剪区域宽度）
 * @param outputHeight 输出视频高度（裁剪区域高度）
 * @param outputPitch 输出视频pitch
 * @param roiX ROI区域左上角x坐标（在输入帧中）
 * @param roiY ROI区域左上角y坐标（在输入帧中）
 * @param isUVPlane 是否处理UV平面（false表示处理Y平面）
 */
__global__ void cropNV12Kernel(const uint8_t* input, int inputWidth, int inputHeight,
                               int inputPitch, uint8_t* output, int outputWidth,
                               int outputHeight, int outputPitch, int roiX, int roiY,
                               bool isUVPlane) {
    // ========== 步骤1：计算当前线程处理的输出像素坐标 ==========
    int x = blockIdx.x * blockDim.x + threadIdx.x;  // 输出帧的x坐标
    int y = blockIdx.y * blockDim.y + threadIdx.y;  // 输出帧的y坐标
    
    if (isUVPlane) {
        // ========== UV平面处理分支 ==========
        // 步骤2：边界检查（UV平面高度为Y平面的一半）
        if (x >= outputWidth || y >= outputHeight / 2) return;
        
        // 定位到UV平面的起始位置
        // NV12格式：UV平面紧跟在Y平面之后
        const uint8_t* inputUV = input + inputHeight * inputPitch;
        uint8_t* outputUV = output + outputHeight * outputPitch;
        
        // 步骤3：计算源坐标（UV平面坐标系）
        int srcX = roiX + x;        // UV平面x坐标与Y平面相同
        int srcY = roiY / 2 + y;    // UV平面y坐标为Y平面的一半
        
        // 步骤4：边界检查并步骤5：写入输出
        if (srcX < inputWidth && srcY < inputHeight / 2) {
            // 在输入范围内，从输入UV平面拷贝像素
            outputUV[y * outputPitch + x] = inputUV[srcY * inputPitch + srcX];
        } else {
            // 超出输入范围，填充UV中性值（128表示无色度偏移）
            outputUV[y * outputPitch + x] = 128;
        }
    } else {
        // ========== Y平面处理分支 ==========
        // 步骤2：边界检查
        if (x >= outputWidth || y >= outputHeight) return;
        
        // 步骤3：计算源坐标（Y平面坐标系）
        int srcX = roiX + x;  // 源x坐标 = ROI起始x + 输出偏移x
        int srcY = roiY + y;  // 源y坐标 = ROI起始y + 输出偏移y
        
        // 步骤4：边界检查并步骤5：写入输出
        if (srcX < inputWidth && srcY < inputHeight) {
            // 在输入范围内，从输入Y平面拷贝像素
            output[y * outputPitch + x] = input[srcY * inputPitch + srcX];
        } else {
            // 超出输入范围，填充黑色（Y=0表示黑色）
            output[y * outputPitch + x] = 0;
        }
    }
}

/**
 * @brief NV12格式视频缩放Kernel - 使用双线性插值算法进行图像缩放
 * 
 * 功能说明：
 * 该kernel使用双线性插值（Bilinear Interpolation）算法对NV12格式视频进行缩放。
 * 双线性插值通过对源图像中最近的4个像素进行加权平均，生成目标像素值，
 * 可以实现平滑的图像放大或缩小效果。
 * 
 * 双线性插值原理：
 * 对于目标像素(x,y)，首先映射到源图像的浮点坐标(srcX, srcY)，
 * 然后找到周围4个整数坐标的像素：
 *   (x0,y0)-----(x1,y0)
 *      |    P     |
 *   (x0,y1)-----(x1,y1)
 * 
 * 插值公式：
 * P = (1-dx)*(1-dy)*v00 + dx*(1-dy)*v10 + (1-dx)*dy*v01 + dx*dy*v11
 * 其中：dx = srcX - x0, dy = srcY - y0
 * 
 * 处理流程图：
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 1. 计算当前线程处理的输出像素坐标 (x, y)                      │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 2. 边界检查：判断坐标是否在输出范围内                         │
 * │    actualHeight = isUVPlane ? outputHeight/2 : outputHeight │
 * │    if (x >= outputWidth || y >= actualHeight) return        │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 3. 计算缩放比例                                               │
 * │    scaleX = inputWidth / outputWidth                        │
 * │    scaleY = inputHeight / outputHeight                      │
 * │    (UV平面的高度需要除以2)                                   │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 4. 计算源图像中的浮点坐标（像素中心对齐）                     │
 * │    srcX = (x + 0.5) * scaleX - 0.5                          │
 * │    srcY = (y + 0.5) * scaleY - 0.5                          │
 * │    (+0.5和-0.5是为了对齐像素中心，提高插值质量)              │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 5. 边界裁剪：确保源坐标在有效范围内                           │
 * │    srcX = clamp(srcX, 0, inputWidth-1)                      │
 * │    srcY = clamp(srcY, 0, inputHeight-1)                     │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 6. 计算周围4个整数坐标点                                      │
 * │    x0 = floor(srcX),  x1 = x0 + 1                           │
 * │    y0 = floor(srcY),  y1 = y0 + 1                           │
 * │    确保x1, y1不超出边界                                       │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 7. 计算插值权重                                               │
 * │    dx = srcX - x0  (x方向的小数部分)                         │
 * │    dy = srcY - y0  (y方向的小数部分)                         │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 8. 定位到正确的平面（Y平面或UV平面）                          │
 * │    如果是UV平面：                                             │
 * │      inputData += inputHeight * inputPitch                  │
 * │      outputData += outputHeight * outputPitch               │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 9. 读取4个邻近像素值                                          │
 * │    v00 = input[y0][x0]  (左上)                              │
 * │    v10 = input[y0][x1]  (右上)                              │
 * │    v01 = input[y1][x0]  (左下)                              │
 * │    v11 = input[y1][x1]  (右下)                              │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 10. 双线性插值计算                                            │
 * │     value = (1-dx)*(1-dy)*v00 + dx*(1-dy)*v10 +             │
 * │             (1-dx)*dy*v01 + dx*dy*v11                       │
 * │     权重说明：                                                │
 * │     - v00权重最大当dx=0,dy=0（目标点在左上角）               │
 * │     - 4个权重之和始终为1                                      │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↓
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 11. 写入输出（四舍五入到最近的整数）                          │
 * │     output[y][x] = (uint8_t)(value + 0.5)                   │
 * └─────────────────────────────────────────────────────────────┘
 * 
 * 算法优势：
 * - 平滑过渡：避免最近邻插值的锯齿效果
 * - 计算效率：只需4个像素参与计算，适合GPU并行
 * - 质量平衡：在速度和质量之间取得良好平衡
 * 
 * @param input 输入视频帧数据指针（GPU内存）
 * @param inputWidth 输入视频宽度
 * @param inputHeight 输入视频高度
 * @param inputPitch 输入视频pitch（行字节数）
 * @param output 输出视频帧数据指针（GPU内存）
 * @param outputWidth 输出视频宽度
 * @param outputHeight 输出视频高度
 * @param outputPitch 输出视频pitch
 * @param isUVPlane 是否处理UV平面（false表示处理Y平面）
 */
__global__ void scaleNV12BilinearKernel(const uint8_t* input, int inputWidth,
                                        int inputHeight, int inputPitch,
                                        uint8_t* output, int outputWidth,
                                        int outputHeight, int outputPitch,
                                        bool isUVPlane) {
    // ========== 步骤1：计算当前线程处理的输出像素坐标 ==========
    int x = blockIdx.x * blockDim.x + threadIdx.x;  // 输出帧的x坐标
    int y = blockIdx.y * blockDim.y + threadIdx.y;  // 输出帧的y坐标
    
    // ========== 步骤2：边界检查 ==========
    // UV平面高度为Y平面的一半
    int actualHeight = isUVPlane ? outputHeight / 2 : outputHeight;
    if (x >= outputWidth || y >= actualHeight) return;
    
    // ========== 步骤3：计算缩放比例 ==========
    // scaleX/Y表示输入像素与输出像素的比例关系
    float scaleX = (float)inputWidth / outputWidth;
    float scaleY = (float)(isUVPlane ? inputHeight / 2 : inputHeight) / actualHeight;
    
    // ========== 步骤4：计算源图像中的浮点坐标 ==========
    // 使用像素中心对齐策略：+0.5移到像素中心，*scale映射到源图像，-0.5回到左上角
    // 这样可以确保缩放时像素中心对齐，避免半像素偏移
    float srcX = (x + 0.5f) * scaleX - 0.5f;
    float srcY = (y + 0.5f) * scaleY - 0.5f;
    
    // ========== 步骤5：边界裁剪 ==========
    // 确保源坐标不会超出输入图像范围
    srcX = fmaxf(0.0f, fminf(srcX, inputWidth - 1.0f));
    srcY = fmaxf(0.0f, fminf(srcY, (isUVPlane ? inputHeight / 2 : inputHeight) - 1.0f));
    
    // ========== 步骤6：计算周围4个整数坐标点 ==========
    int x0 = (int)srcX;  // 左边界（向下取整）
    int y0 = (int)srcY;  // 上边界（向下取整）
    int x1 = min(x0 + 1, inputWidth - 1);  // 右边界（确保不越界）
    int y1 = min(y0 + 1, (isUVPlane ? inputHeight / 2 : inputHeight) - 1);  // 下边界
    
    // ========== 步骤7：计算插值权重（小数部分） ==========
    float dx = srcX - x0;  // x方向的插值系数 [0, 1)
    float dy = srcY - y0;  // y方向的插值系数 [0, 1)
    
    // ========== 步骤8：定位到正确的平面 ==========
    const uint8_t* inputData = input;
    uint8_t* outputData = output;
    
    if (isUVPlane) {
        // UV平面紧跟在Y平面之后，需要偏移整个Y平面的大小
        inputData += inputHeight * inputPitch;
        outputData += outputHeight * outputPitch;
    }
    
    // ========== 步骤9：读取4个邻近像素值 ==========
    // 双线性插值需要2x2的像素块
    float v00 = inputData[y0 * inputPitch + x0];  // 左上角像素
    float v10 = inputData[y0 * inputPitch + x1];  // 右上角像素
    float v01 = inputData[y1 * inputPitch + x0];  // 左下角像素
    float v11 = inputData[y1 * inputPitch + x1];  // 右下角像素
    
    // ========== 步骤10：双线性插值计算 ==========
    // 公式：f(x,y) = f(0,0)(1-x)(1-y) + f(1,0)x(1-y) + f(0,1)(1-x)y + f(1,1)xy
    // 权重解释：
    // - 当dx=0, dy=0时，完全取v00（左上角）
    // - 当dx=1, dy=0时，完全取v10（右上角）
    // - 当dx=0, dy=1时，完全取v01（左下角）
    // - 当dx=1, dy=1时，完全取v11（右下角）
    // - 其他情况按距离加权平均
    float value = (1 - dx) * (1 - dy) * v00 +  // 左上角权重
                  dx * (1 - dy) * v10 +        // 右上角权重
                  (1 - dx) * dy * v01 +        // 左下角权重
                  dx * dy * v11;               // 右下角权重
    
    // ========== 步骤11：写入输出（四舍五入） ==========
    // +0.5实现四舍五入，然后转换为uint8_t
    outputData[y * outputPitch + x] = (uint8_t)(value + 0.5f);
}


// ============================================================================
// CudaVideoProcessor Implementation
// ============================================================================

CudaVideoProcessor::CudaVideoProcessor()
    : m_deviceId(0), m_initialized(false) {
}

CudaVideoProcessor::~CudaVideoProcessor() {
    release();
}

bool CudaVideoProcessor::initialize(int deviceId) {
    if (m_initialized) {
        return true;
    }
    
    m_deviceId = deviceId;
    
    // 设置CUDA设备
    cudaError_t err = cudaSetDevice(m_deviceId);
    if (!checkCudaError(err, "Failed to set CUDA device")) {
        return false;
    }
    
    // 获取设备属性
    cudaDeviceProp prop;
    err = cudaGetDeviceProperties(&prop, m_deviceId);
    if (!checkCudaError(err, "Failed to get device properties")) {
        return false;
    }
    
    printf("CUDA Device: %s\n", prop.name);
    printf("Compute Capability: %d.%d\n", prop.major, prop.minor);
    printf("Total Global Memory: %.2f GB\n", prop.totalGlobalMem / 1024.0 / 1024.0 / 1024.0);
    
    m_initialized = true;
    return true;
}

void CudaVideoProcessor::release() {
    if (m_initialized) {
        cudaDeviceReset();
        m_initialized = false;
    }
}

bool CudaVideoProcessor::mergeVideos(const VideoFrame* inputs, int numInputs,
                                     VideoFrame& output, const MergeLayout& layout,
                                     cudaStream_t stream) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    if (numInputs <= 0 || numInputs > layout.rows * layout.cols) {
        setError("Invalid number of inputs");
        return false;
    }
    
    // 准备输入数据
    std::vector<const uint8_t*> h_inputs(numInputs);
    std::vector<int> h_widths(numInputs);
    std::vector<int> h_heights(numInputs);
    std::vector<int> h_pitches(numInputs);
    
    for (int i = 0; i < numInputs; i++) {
        h_inputs[i] = inputs[i].d_data;
        h_widths[i] = inputs[i].width;
        h_heights[i] = inputs[i].height;
        h_pitches[i] = inputs[i].pitch;
    }
    
    // 拷贝到GPU
    const uint8_t** d_inputs;
    int* d_widths;
    int* d_heights;
    int* d_pitches;
    
    cudaMalloc(&d_inputs, numInputs * sizeof(uint8_t*));
    cudaMalloc(&d_widths, numInputs * sizeof(int));
    cudaMalloc(&d_heights, numInputs * sizeof(int));
    cudaMalloc(&d_pitches, numInputs * sizeof(int));
    
    cudaMemcpy(d_inputs, h_inputs.data(), numInputs * sizeof(uint8_t*), cudaMemcpyHostToDevice);
    cudaMemcpy(d_widths, h_widths.data(), numInputs * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_heights, h_heights.data(), numInputs * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_pitches, h_pitches.data(), numInputs * sizeof(int), cudaMemcpyHostToDevice);
    
    // 配置线程块和网格
    dim3 blockSize(16, 16);
    dim3 gridSize((output.width + blockSize.x - 1) / blockSize.x,
                  (output.height + blockSize.y - 1) / blockSize.y);
    
    // 处理Y平面
    mergeNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        d_inputs, numInputs, d_widths, d_heights, d_pitches,
        output.d_data, output.width, output.height, output.pitch,
        layout.rows, layout.cols, layout.cellWidth, layout.cellHeight, false);
    
    // 处理UV平面
    mergeNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        d_inputs, numInputs, d_widths, d_heights, d_pitches,
        output.d_data, output.width, output.height, output.pitch,
        layout.rows, layout.cols, layout.cellWidth, layout.cellHeight, true);
    
    // 同步
    cudaError_t err = cudaStreamSynchronize(stream);
    
    // 释放临时内存
    cudaFree(d_inputs);
    cudaFree(d_widths);
    cudaFree(d_heights);
    cudaFree(d_pitches);
    
    return checkCudaError(err, "Merge kernel failed");
}


bool CudaVideoProcessor::cropVideo(const VideoFrame& input, VideoFrame& output,
                                   const ROI& roi, cudaStream_t stream) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    // 边界检查
    if (roi.x < 0 || roi.y < 0 ||
        roi.x + roi.width > input.width ||
        roi.y + roi.height > input.height) {
        setError("ROI out of bounds");
        return false;
    }
    
    // 对齐检查（UV平面要求偶数对齐）
    if (roi.x % 2 != 0 || roi.y % 2 != 0 ||
        roi.width % 2 != 0 || roi.height % 2 != 0) {
        setError("ROI must be even-aligned for NV12 format");
        return false;
    }
    
    // 配置线程块和网格
    dim3 blockSize(16, 16);
    dim3 gridSize((output.width + blockSize.x - 1) / blockSize.x,
                  (output.height + blockSize.y - 1) / blockSize.y);
    
    // 处理Y平面
    cropNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch,
        roi.x, roi.y, false);
    
    // 处理UV平面
    cropNV12Kernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch,
        roi.x, roi.y, true);
    
    cudaError_t err = cudaStreamSynchronize(stream);
    return checkCudaError(err, "Crop kernel failed");
}

bool CudaVideoProcessor::scaleVideo(const VideoFrame& input, VideoFrame& output,
                                    InterpolationMethod method, cudaStream_t stream) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    // 配置线程块和网格
    dim3 blockSize(16, 16);
    dim3 gridSize((output.width + blockSize.x - 1) / blockSize.x,
                  (output.height + blockSize.y - 1) / blockSize.y);
    
    // 目前只实现双线性插值
    if (method != InterpolationMethod::BILINEAR) {
        setError("Only bilinear interpolation is currently supported");
        return false;
    }
    
    // 处理Y平面
    scaleNV12BilinearKernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch, false);
    
    // 处理UV平面
    scaleNV12BilinearKernel<<<gridSize, blockSize, 0, stream>>>(
        input.d_data, input.width, input.height, input.pitch,
        output.d_data, output.width, output.height, output.pitch, true);
    
    cudaError_t err = cudaStreamSynchronize(stream);
    return checkCudaError(err, "Scale kernel failed");
}

bool CudaVideoProcessor::allocateFrame(VideoFrame& frame) {
    if (!m_initialized) {
        setError("CUDA not initialized");
        return false;
    }
    
    // 计算NV12格式所需内存大小
    size_t ySize = frame.height * frame.pitch;
    size_t uvSize = (frame.height / 2) * frame.pitch;
    size_t totalSize = ySize + uvSize;
    
    cudaError_t err = cudaMalloc(&frame.d_data, totalSize);
    if (!checkCudaError(err, "Failed to allocate frame memory")) {
        return false;
    }
    
    // 初始化为黑色
    cudaMemset(frame.d_data, 0, ySize);
    cudaMemset(frame.d_data + ySize, 128, uvSize);
    
    return true;
}

void CudaVideoProcessor::freeFrame(VideoFrame& frame) {
    if (frame.d_data) {
        cudaFree(frame.d_data);
        frame.d_data = nullptr;
    }
}

void CudaVideoProcessor::setError(const char* error) {
    m_lastError = error;
    fprintf(stderr, "CudaVideoProcessor Error: %s\n", error);
}

bool CudaVideoProcessor::checkCudaError(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s: %s", msg, cudaGetErrorString(err));
        setError(buffer);
        return false;
    }
    return true;
}

} // namespace cuda
