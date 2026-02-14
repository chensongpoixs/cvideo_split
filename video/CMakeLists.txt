cmake_minimum_required(VERSION 3.18)
project(CudaVideoProcessor LANGUAGES CXX CUDA)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置CUDA标准
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# 查找CUDA
find_package(CUDA REQUIRED)

# 设置CUDA架构
set(CMAKE_CUDA_ARCHITECTURES 75 80 86)  # Turing, Ampere, Ada

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CUDA_INCLUDE_DIRS})

# 源文件
set(SOURCES
    CudaVideoProcessor.cu
)

set(HEADERS
    CudaVideoProcessor.h
)

# 创建库
add_library(cuda_video_processor STATIC ${SOURCES} ${HEADERS})

# 设置CUDA编译选项
target_compile_options(cuda_video_processor PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:
        --use_fast_math
        -lineinfo
    >
)

# 链接CUDA库
target_link_libraries(cuda_video_processor
    ${CUDA_LIBRARIES}
    ${CUDA_cudart_static_LIBRARY}
)

# 创建示例程序
add_executable(video_processor_example example.cpp)
target_link_libraries(video_processor_example cuda_video_processor)

# 安装
install(TARGETS cuda_video_processor video_processor_example
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES ${HEADERS} DESTINATION include)

# 打印配置信息
message(STATUS "CUDA Version: ${CUDA_VERSION}")
message(STATUS "CUDA Architectures: ${CMAKE_CUDA_ARCHITECTURES}")
message(STATUS "CUDA Include Dirs: ${CUDA_INCLUDE_DIRS}")
