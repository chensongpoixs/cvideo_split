#!/bin/bash

# 多路视频拼接系统构建脚本

echo "开始构建多路视频拼接系统..."

# 创建构建目录
mkdir -p build
cd build

# 运行CMake
echo "运行CMake配置..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 检查CMake是否成功
if [ $? -ne 0 ]; then
    echo "CMake配置失败"
    exit 1
fi

# 编译项目
echo "编译项目..."
make -j$(nproc)

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "编译失败"
    exit 1
fi

echo "构建完成！可执行文件位于 build/bin/video_stitch_system"
