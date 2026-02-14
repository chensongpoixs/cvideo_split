# CUDA视频处理库

基于CUDA的高性能视频处理库，支持显存数据的合并、裁剪和缩放操作。

## 功能特性

- ✅ 多路视频合并（视频墙）
- ✅ 视频裁剪（ROI提取）
- ✅ 视频缩放（双线性插值）
- ✅ NV12格式支持
- ✅ 零拷贝GPU处理
- ✅ 异步流支持

## 系统要求

- CUDA Toolkit 11.0+
- NVIDIA GPU (Compute Capability 7.5+)
- CMake 3.18+
- C++14编译器

## 编译

```bash
cd video
mkdir build && cd build
cmake ..
make -j4
```

## 使用示例

### 1. 初始化

```cpp
#include "CudaVideoProcessor.h"

cuda::CudaVideoProcessor processor;
processor.initialize(0);  // 使用GPU 0
```

### 2. 视频合并

```cpp
// 4路1080p合并成4K
VideoFrame inputs[4];
VideoFrame output;
MergeLayout layout(2, 2, 1920, 1080);

processor.mergeVideos(inputs, 4, output, layout);
```

### 3. 视频裁剪

```cpp
VideoFrame input, output;
ROI roi(100, 100, 800, 600);

processor.cropVideo(input, output, roi);
```

### 4. 视频缩放

```cpp
VideoFrame input, output;

processor.scaleVideo(input, output, InterpolationMethod::BILINEAR);
```

## 性能指标

| 操作 | 分辨率 | 性能 (RTX 3090) |
|------|--------|----------------|
| 合并4路 | 1080p→4K | ~2ms |
| 裁剪 | 1080p | ~0.5ms |
| 缩放 | 1080p→4K | ~1.5ms |

## API文档

详见 [CUDA视频处理方案.md](../CUDA视频处理方案.md)

## 流程图

- [视频合并流程](https://mermaid.ink/svg/...)
- [视频裁剪流程](https://mermaid.ink/svg/...)
- [视频缩放流程](https://mermaid.ink/svg/...)

## 许可证

BSD License
