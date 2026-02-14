#include "CudaVideoProcessor.h"
#include <stdio.h>

using namespace cuda;

/**
 * @brief 示例1: 4路视频合并成2x2视频墙
 */
void example_merge_4videos() {
    printf("\n=== Example 1: Merge 4 Videos (2x2) ===\n");
    
    CudaVideoProcessor processor;
    if (!processor.initialize(0)) {
        printf("Failed to initialize CUDA\n");
        return;
    }
    
    // 创建4路输入视频（1920x1080）
    const int numInputs = 4;
    VideoFrame inputs[numInputs];
    
    for (int i = 0; i < numInputs; i++) {
        inputs[i].width = 1920;
        inputs[i].height = 1080;
        inputs[i].pitch = 1920;
        inputs[i].format = VideoFormat::NV12;
        
        if (!processor.allocateFrame(inputs[i])) {
            printf("Failed to allocate input frame %d\n", i);
            return;
        }
    }
    
    // 创建输出视频（3840x2160, 4K）
    VideoFrame output;
    output.width = 3840;
    output.height = 2160;
    output.pitch = 3840;
    output.format = VideoFormat::NV12;
    
    if (!processor.allocateFrame(output)) {
        printf("Failed to allocate output frame\n");
        return;
    }
    
    // 配置2x2布局
    MergeLayout layout(2, 2, 1920, 1080);
    
    // 执行合并
    if (processor.mergeVideos(inputs, numInputs, output, layout)) {
        printf("Successfully merged 4 videos into 2x2 layout\n");
        printf("Output: %dx%d\n", output.width, output.height);
    } else {
        printf("Merge failed: %s\n", processor.getLastError());
    }
    
    // 释放资源
    for (int i = 0; i < numInputs; i++) {
        processor.freeFrame(inputs[i]);
    }
    processor.freeFrame(output);
}

/**
 * @brief 示例2: 视频裁剪ROI
 */
void example_crop_video() {
    printf("\n=== Example 2: Crop Video ROI ===\n");
    
    CudaVideoProcessor processor;
    if (!processor.initialize(0)) {
        printf("Failed to initialize CUDA\n");
        return;
    }
    
    // 创建输入视频（1920x1080）
    VideoFrame input;
    input.width = 1920;
    input.height = 1080;
    input.pitch = 1920;
    input.format = VideoFormat::NV12;
    
    if (!processor.allocateFrame(input)) {
        printf("Failed to allocate input frame\n");
        return;
    }
    
    // 创建输出视频（800x600）
    VideoFrame output;
    output.width = 800;
    output.height = 600;
    output.pitch = 800;
    output.format = VideoFormat::NV12;
    
    if (!processor.allocateFrame(output)) {
        printf("Failed to allocate output frame\n");
        return;
    }
    
    // 定义ROI区域（从(100,100)开始，800x600大小）
    ROI roi(100, 100, 800, 600);
    
    // 执行裁剪
    if (processor.cropVideo(input, output, roi)) {
        printf("Successfully cropped video\n");
        printf("ROI: (%d,%d) %dx%d\n", roi.x, roi.y, roi.width, roi.height);
        printf("Output: %dx%d\n", output.width, output.height);
    } else {
        printf("Crop failed: %s\n", processor.getLastError());
    }
    
    // 释放资源
    processor.freeFrame(input);
    processor.freeFrame(output);
}

/**
 * @brief 示例3: 视频缩放
 */
void example_scale_video() {
    printf("\n=== Example 3: Scale Video ===\n");
    
    CudaVideoProcessor processor;
    if (!processor.initialize(0)) {
        printf("Failed to initialize CUDA\n");
        return;
    }
    
    // 创建输入视频（1920x1080）
    VideoFrame input;
    input.width = 1920;
    input.height = 1080;
    input.pitch = 1920;
    input.format = VideoFormat::NV12;
    
    if (!processor.allocateFrame(input)) {
        printf("Failed to allocate input frame\n");
        return;
    }
    
    // 创建输出视频（3840x2160, 4K）
    VideoFrame output;
    output.width = 3840;
    output.height = 2160;
    output.pitch = 3840;
    output.format = VideoFormat::NV12;
    
    if (!processor.allocateFrame(output)) {
        printf("Failed to allocate output frame\n");
        return;
    }
    
    // 执行缩放（双线性插值）
    if (processor.scaleVideo(input, output, InterpolationMethod::BILINEAR)) {
        printf("Successfully scaled video\n");
        printf("Input: %dx%d\n", input.width, input.height);
        printf("Output: %dx%d\n", output.width, output.height);
    } else {
        printf("Scale failed: %s\n", processor.getLastError());
    }
    
    // 释放资源
    processor.freeFrame(input);
    processor.freeFrame(output);
}

int main() {
    printf("CUDA Video Processor Examples\n");
    printf("==============================\n");
    
    // 运行示例
    example_merge_4videos();
    example_crop_video();
    example_scale_video();
    
    printf("\nAll examples completed!\n");
    return 0;
}
