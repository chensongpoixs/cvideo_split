# FFmpeg路径设置指南

## CMake中设置FFmpeg目录的几种方法

### 方法1：环境变量（推荐）

在运行CMake之前设置环境变量：

**Linux/macOS:**
```bash
export FFMPEG_ROOT=/usr/local/ffmpeg
cmake .. -DCMAKE_BUILD_TYPE=Release
```

**Windows:**
```cmd
set FFMPEG_ROOT=C:\ffmpeg
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### 方法2：CMake命令行参数

直接在CMake命令中指定：

```bash
cmake -D FFMPEG_ROOT=/path/to/ffmpeg .. -DCMAKE_BUILD_TYPE=Release
```

**Windows示例:**
```cmd
cmake -D FFMPEG_ROOT=C:\ffmpeg .. -DCMAKE_BUILD_TYPE=Release
```

### 方法3：CMAKE_PREFIX_PATH

将FFmpeg路径添加到CMAKE_PREFIX_PATH：

**Linux/macOS:**
```bash
export CMAKE_PREFIX_PATH=/path/to/ffmpeg:$CMAKE_PREFIX_PATH
cmake .. -DCMAKE_BUILD_TYPE=Release
```

**Windows:**
```cmd
set CMAKE_PREFIX_PATH=C:\ffmpeg;%CMAKE_PREFIX_PATH%
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## FFmpeg目录结构

确保FFmpeg安装目录包含以下结构：

```
/path/to/ffmpeg/
├── include/
│   ├── libavcodec/
│   ├── libavformat/
│   ├── libavutil/
│   ├── libswscale/
│   └── libavfilter/
└── lib/  (或 bin/)
    ├── avcodec.lib (或 libavcodec.so/.dylib)
    ├── avformat.lib (或 libavformat.so/.dylib)
    ├── avutil.lib (或 libavutil.so/.dylib)
    ├── swscale.lib (或 libswscale.so/.dylib)
    └── avfilter.lib (或 libavfilter.so/.dylib)
```

## 验证设置

运行CMake配置时，应该看到类似以下输出：

```
Using FFmpeg from CMake variable: /path/to/ffmpeg
FFmpeg found: /path/to/ffmpeg/lib/avcodec.lib
```

如果仍然显示"FFmpeg libraries not found"，请检查：
1. FFmpeg路径是否正确
2. 目录结构是否符合上述要求
3. 是否有必要的权限访问FFmpeg文件

## 构建脚本示例

### Linux/macOS
```bash
#!/bin/bash
export FFMPEG_ROOT=/usr/local/ffmpeg
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Windows (PowerShell)
```powershell
$env:FFMPEG_ROOT = "C:\ffmpeg"
New-Item -ItemType Directory -Path build -Force
Set-Location build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Windows (CMD)
```cmd
set FFMPEG_ROOT=C:\ffmpeg
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
