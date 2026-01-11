/**
 * @file PlatformDisplay.cpp
 * @author chensong
 * @date 2026-01-11
 * @brief 跨平台显示接口实现和工厂函数
 * @see PlatformDisplay.h
 */

#include "PlatformDisplay.h"

#ifdef _WIN32
#include "DxgiDisplay.h"
#elif __linux__
#include "OpenGLDisplay.h"
#endif

PlatformDisplay::PlatformDisplay() = default;

PlatformDisplay::~PlatformDisplay() = default;

// 工厂函数：根据平台创建相应的显示实现
std::unique_ptr<PlatformDisplay> createPlatformDisplay() {
#ifdef _WIN32
    return std::make_unique<DxgiDisplay>();
#elif __linux__
    return std::make_unique<OpenGLDisplay>();
#else
    return nullptr;
#endif
}

