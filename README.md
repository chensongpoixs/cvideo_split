# 多路视频拼接系统

## 项目简介

这是一个基于FFmpeg和CUDA的多路视频拼接系统，能够实时拉取多路RTSP视频流，进行硬件解码、CUDA加速的视频拼接处理、OSD字幕叠加，最终重新编码并推流输出。

## 技术栈

- **FFmpeg**: 音视频处理框架，用于RTSP流拉取、编解码、滤镜处理
- **CUDA**: NVIDIA GPU加速，用于视频拼接、缩放等计算密集型操作
- **H.264/H.265**: 支持硬件解码的视频编解码标准
- **RTSP**: 实时流媒体传输协议

## 主要功能

1. **多路RTSP流拉取**: 支持同时拉取多路RTSP视频流
2. **硬件解码**: 利用GPU硬件解码H.264/H.265视频流
3. **CUDA视频拼接**: 基于CUDA对显存数据进行合并、裁剪与缩放
4. **OSD字幕叠加**: 在拼接后的视频上添加文字信息（如时间戳）
5. **重新编码推流**: 将处理后的视频重新编码并推送到RTSP服务器

## 系统架构

```
RTSP流1 ──┐
          ├── VideoStreamPuller ── HardwareDecoder ──┐
RTSP流2 ──┤                                            │
          ├── VideoStreamPuller ── HardwareDecoder ──┼── CudaVideoStitcher ── OSDOverlay ── VideoEncoder ── RTSP输出
RTSP流3 ──┤                                            │
          ├── VideoStreamPuller ── HardwareDecoder ──┘
RTSP流4 ──┘
```

## 构建要求

### 系统要求
- Linux/Windows操作系统
- NVIDIA GPU (支持CUDA)
- FFmpeg开发库
- CUDA Toolkit

### 依赖库
- FFmpeg >= 4.0 (包含libavcodec, libavformat, libavutil, libswscale, libavfilter)
- CUDA >= 10.0
- CMake >= 3.20

### 安装依赖 (Ubuntu)
```bash
# 安装FFmpeg
sudo apt update
sudo apt install ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavfilter-dev

# 安装CUDA (如果还没有安装)
# 请参考NVIDIA官方文档安装CUDA Toolkit
```

### 安装依赖 (Windows)
1. 下载FFmpeg：访问 https://ffmpeg.org/download.html 下载Windows版本
2. 解压到任意目录，例如 `C:\ffmpeg`
3. 将 `C:\ffmpeg\bin` 添加到系统PATH环境变量
4. 或者在CMake配置时指定FFmpeg路径（见下文）

## 构建步骤

1. 克隆或下载项目代码

2. **设置FFmpeg路径**（如果FFmpeg不在标准位置）：

   **方法1：环境变量**
   ```bash
   export FFMPEG_ROOT=/path/to/ffmpeg
   ```

   **方法2：CMake变量**
   ```bash
   cmake -D FFMPEG_ROOT=/path/to/ffmpeg ..
   ```

   **方法3：CMAKE_PREFIX_PATH**
   ```bash
   export CMAKE_PREFIX_PATH=/path/to/ffmpeg:$CMAKE_PREFIX_PATH
   ```

   **Windows示例：**
   ```cmd
   set FFMPEG_ROOT=C:\ffmpeg
   cmake -D FFMPEG_ROOT=C:\ffmpeg ..
   ```

3. 创建构建目录并编译：
```bash
chmod +x build.sh
./build.sh
```

或者手动构建：
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Windows构建示例：**
```cmd
mkdir build
cd build
cmake -D FFMPEG_ROOT=C:\ffmpeg .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 使用方法

### 基本用法

修改 `src/main.cpp` 中的RTSP URL配置：

```cpp
std::vector<std::string> rtsp_urls = {
    "rtsp://your_camera1_ip:554/stream",
    "rtsp://your_camera2_ip:554/stream",
    "rtsp://your_camera3_ip:554/stream",
    "rtsp://your_camera4_ip:554/stream"
};
```

修改输出RTSP URL：
```cpp
const std::string OUTPUT_URL = "rtsp://0.0.0.0:8554/live";
```

### 运行程序

```bash
cd build/bin
./video_stitch_system
```

### 配置选项

在 `src/main.cpp` 中可以配置：

- `NUM_STREAMS`: 输入视频流数量 (默认4)
- `OUTPUT_WIDTH/OUTPUT_HEIGHT`: 输出视频分辨率 (默认1920x1080)
- `FPS`: 输出帧率 (默认30)
- 拼接布局: 在 `stitch_config.stream_positions` 中配置各路视频的位置和大小

## 模块说明

### VideoStreamPuller
负责拉取RTSP视频流，进行软件解码并缓存最新帧。

### HardwareDecoder
利用GPU硬件加速解码H.264/H.265视频流。

### CudaVideoStitcher
基于CUDA进行视频拼接、缩放和位置调整，支持4宫格布局。

### OSDOverlay
使用FFmpeg滤镜在视频上叠加文字信息。

### VideoEncoder
将处理后的视频重新编码为H.264并推流到RTSP服务器。

## 性能优化

- 使用GPU硬件解码减少CPU负载
- CUDA加速视频处理操作
- 多线程并行处理各路视频流
- 零拷贝数据传输优化

## 注意事项

1. 确保输入RTSP流稳定可用
2. GPU内存充足以支持多路高清视频处理
3. 根据网络条件调整缓冲区大小
4. 监控系统资源使用情况

## 许可证

本项目采用MIT许可证。  
