cmake_minimum_required(VERSION 3.20)
project(video_stitch_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找CUDA
find_package(CUDA REQUIRED)
if(CUDA_FOUND)
    message("CUDA found: ${CUDA_VERSION}")
    include_directories(${CUDA_INCLUDE_DIRS})
    # IMPORTANT:
    # The project previously hardcoded sm_75, which will fail on other GPUs with:
    # "no kernel image is available for execution on the device".
    # RTX 5080 is compute capability 12.0 -> need sm_120.
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}
        ;-gencode=arch=compute_120,code=sm_120
        ;-gencode=arch=compute_120,code=compute_120
    )
else()
    message(FATAL_ERROR "CUDA not found")
endif()

# 平台特定的显示库
if(WIN32)
    # Windows: 使用DXGI/DirectX
    message("Windows platform detected - using DXGI for display")
    add_definitions(-DHAVE_DXGI)
elseif(UNIX AND NOT APPLE)
    # Linux: 使用OpenGL
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glfw3 REQUIRED)
    
    if(OpenGL_FOUND AND GLEW_FOUND AND glfw3_FOUND)
        message("Linux platform detected - OpenGL display libraries found")
        include_directories(${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
        add_definitions(-DHAVE_OPENGL)
    else()
        message(WARNING "OpenGL libraries not found - video display will be disabled")
    endif()
endif()

# 添加项目include目录
include_directories(include)

# FFmpeg需要的一些宏定义
add_definitions(-D__STDC_CONSTANT_MACROS)

# 查找FFmpeg
# 用户可以通过以下方式指定FFmpeg路径：
# 1. 设置环境变量：FFMPEG_ROOT
# 2. CMake变量：FFMPEG_ROOT
# 3. CMAKE_PREFIX_PATH包含FFmpeg路径

# 检查用户是否指定了FFmpeg路径
if(DEFINED ENV{FFMPEG_ROOT})
    set(FFMPEG_ROOT $ENV{FFMPEG_ROOT})
    message("Using FFmpeg from environment variable: ${FFMPEG_ROOT}")
elseif(FFMPEG_ROOT)
    message("Using FFmpeg from CMake variable: ${FFMPEG_ROOT}")
else()
    # 尝试从CMAKE_PREFIX_PATH或其他标准位置查找
    message("Searching for FFmpeg in standard locations...")
endif()

# 设置搜索路径
set(FFMPEG_SEARCH_PATHS
    ${FFMPEG_ROOT}
    ${FFMPEG_ROOT}/include
    $ENV{FFMPEG_ROOT}
    $ENV{FFMPEG_ROOT}/include
    /usr/local/include
    /usr/include
    "C:/ffmpeg/include"
    "C:/Program Files/ffmpeg/include"
)

set(FFMPEG_LIBRARY_SEARCH_PATHS
    ${FFMPEG_ROOT}
    ${FFMPEG_ROOT}/lib
    ${FFMPEG_ROOT}/bin
    $ENV{FFMPEG_ROOT}
    $ENV{FFMPEG_ROOT}/lib
    $ENV{FFMPEG_ROOT}/bin
    /usr/local/lib
    /usr/lib
    /usr/lib/x86_64-linux-gnu
    "C:/ffmpeg/lib"
    "C:/ffmpeg/bin"
    "C:/Program Files/ffmpeg/lib"
    "C:/Program Files/ffmpeg/bin"
)

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h
    PATHS ${FFMPEG_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_library(AVCODEC_LIBRARY avcodec
    PATHS ${FFMPEG_LIBRARY_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h
    PATHS ${FFMPEG_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_library(AVFORMAT_LIBRARY avformat
    PATHS ${FFMPEG_LIBRARY_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h
    PATHS ${FFMPEG_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_library(AVUTIL_LIBRARY avutil
    PATHS ${FFMPEG_LIBRARY_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h
    PATHS ${FFMPEG_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_library(SWSCALE_LIBRARY swscale
    PATHS ${FFMPEG_LIBRARY_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h
    PATHS ${FFMPEG_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_library(AVFILTER_LIBRARY avfilter
    PATHS ${FFMPEG_LIBRARY_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h
    PATHS ${FFMPEG_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

find_library(SWRESAMPLE_LIBRARY swresample
    PATHS ${FFMPEG_LIBRARY_SEARCH_PATHS}
    NO_DEFAULT_PATH
)

# 检查FFmpeg是否找到
if(NOT AVCODEC_LIBRARY OR NOT AVFORMAT_LIBRARY OR NOT AVUTIL_LIBRARY)
    message(WARNING "FFmpeg libraries not found")
    message("Please install FFmpeg development libraries:")
    message("- Ubuntu/Debian: sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavfilter-dev")
    message("- Windows: Download from https://ffmpeg.org/download.html")
    message("- Or set CMAKE_PREFIX_PATH to FFmpeg installation directory")
    message("Continuing with build, but linking may fail...")

    # 设置空的FFmpeg库列表，允许继续构建（用户需要手动配置）
    set(FFMPEG_LIBRARIES "")
else()
    message("FFmpeg found: ${AVCODEC_LIBRARY}")
    include_directories(${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR}
                       ${SWSCALE_INCLUDE_DIR} ${AVFILTER_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR})
    set(FFMPEG_LIBRARIES ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY}
                        ${SWSCALE_LIBRARY} ${AVFILTER_LIBRARY} ${SWRESAMPLE_LIBRARY})
endif()

# 设置源文件
set(SOURCES
    src/main.cpp
    src/Logger.cpp
    src/VideoStreamPuller.cpp
    src/HardwareDecoder.cpp
    src/CudaVideoStitcher.cpp
    src/CudaStreamManager.cpp
    src/OSDOverlay.cpp
    src/VideoEncoder.cpp
    src/VideoDecoder.cpp
    src/VideoStitcher.cpp
    src/GpuMemoryManager.cpp
    src/Config.cpp
    src/PlatformDisplay.cpp
    src/DxgiDisplay.cpp
    src/OpenGLDisplay.cpp
    include/Logger.h
    include/VideoStreamPuller.h
    include/HardwareDecoder.h
    include/CudaVideoStitcher.h
    include/CudaStreamManager.h
    include/OSDOverlay.h
    include/VideoEncoder.h
    include/VideoDecoder.h
    include/VideoStitcher.h
    include/GpuMemoryManager.h
    include/Config.h
    include/PlatformDisplay.h
    include/DxgiDisplay.h
    include/OpenGLDisplay.h
)

set(CUDA_SOURCES
    src/cuda/VideoStitchKernel.cu
    src/cuda/Nv12ToRgbaKernel.cu
)

# CUDA编译 - 为Debug和Release模式分别设置运行时库
set(CUDA_NVCC_FLAGS_RELEASE ${CUDA_NVCC_FLAGS_RELEASE} -Xcompiler "/MD")
set(CUDA_NVCC_FLAGS_DEBUG ${CUDA_NVCC_FLAGS_DEBUG} -Xcompiler "/MDd")
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -Xcompiler "/MD")

cuda_add_library(cuda_stitch ${CUDA_SOURCES})
set_target_properties(cuda_stitch PROPERTIES
    CUDA_RUNTIME_LIBRARY Shared
    CUDA_DEBUG_SEPARATE_COMPILATION OFF
)

# 可执行文件
add_executable(video_stitch_system ${SOURCES})

# 链接库
target_link_libraries(video_stitch_system
    cuda_stitch
    ${FFMPEG_LIBRARIES}
    ${CUDA_LIBRARIES}
    ${CUDA_CUFFT_LIBRARIES}
    ${CUDA_CUBLAS_LIBRARIES}
    $<$<PLATFORM_ID:Linux>:OpenGL::GL>
    $<$<PLATFORM_ID:Linux>:GLEW::GLEW>
    $<$<PLATFORM_ID:Linux>:glfw>
    $<$<PLATFORM_ID:Windows>:d3d11>
    $<$<PLATFORM_ID:Windows>:dxgi>
    $<$<PLATFORM_ID:Windows>:d3dcompiler>
)

# 设置运行时库为动态链接 (与FFmpeg一致)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

# CUDA也使用动态运行时库
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -Xcompiler "/MD")

# 设置输出目录
set_target_properties(video_stitch_system PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
