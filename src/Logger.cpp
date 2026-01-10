#include "Logger.h"

// 说明：
// Logger 当前采用 header-only（`include/Logger.h` 内联实现）以避免在 Windows 下与宏冲突、
// 并减少链接/ODR 问题。本文件仅用于满足构建系统中对 `src/Logger.cpp` 的编译单元引用。