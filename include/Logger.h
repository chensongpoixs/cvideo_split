/**
 * @file Logger.h
 * @author chensong
 * @date 2026-01-11
 * @brief 日志系统（Logging System）
 * 
 * 该模块提供统一的日志记录功能，支持多日志级别、文件输出和控制台输出。
 * 使用单例模式确保全局唯一实例。
 * 
 * 日志系统架构（Logging System Architecture）：
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Logger (单例)                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  |   日志调用 (LOG_INFO, LOG_ERROR, ...)                         |
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │              日志级别过滤                                │|
 *  |   │  DEBUG < INFO < WARNING < ERROR < FATAL                 │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌─────────────────────────────────────────────────────────┐|
 *  |   │              格式化消息                                  │|
 *  |   │  2026-01-11 12:34:56.789 [INFO] file.cpp:123 - message  │|
 *  |   └─────────────────────────────────────────────────────────┘|
 *  |       │                                                       |
 *  |       ▼                                                       |
 *  |   ┌────────────────┬────────────────┐                        |
 *  |   │ 控制台输出     │ 文件输出       │                        |
 *  |   │ (std::cout)    │ (log_file_)    │                        |
 *  |   └────────────────┴────────────────┘                        |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 * 日志格式（Log Format）：
 * 
 *   YYYY-MM-DD HH:MM:SS.mmm [LEVEL] file:line - message
 *   
 *   示例：
 *   2026-01-11 12:34:56.789 [INFO] main.cpp:42 - System started
 *   2026-01-11 12:34:57.123 [ERROR] VideoDecoder.cpp:156 - Decode failed
 * 
 * 使用示例：
 * @code
 * // 设置日志文件
 * Logger::getInstance().setLogFile("app.log");
 * Logger::getInstance().setLogLevel(LogLevel::DEBUG_LEVEL);
 * 
 * // 使用宏记录日志
 * LOG_INFO("Application started");
 * LOG_ERROR("Failed to open file: " + filename);
 * LOG_DEBUG("Debug info: x=" + std::to_string(x));
 * @endcode
 * 
 * @note 线程安全，使用互斥锁保护日志输出
 */

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * @enum LogLevel
 * @brief 日志级别枚举
 * 
 * 级别从低到高：DEBUG < INFO < WARNING < ERROR < FATAL
 */
enum class LogLevel {
    DEBUG_LEVEL,    ///< 调试信息
    INFO_LEVEL,     ///< 一般信息
    WARNING_LEVEL,  ///< 警告信息
    ERROR_LEVEL,    ///< 错误信息
    FATAL_LEVEL     ///< 致命错误
};

/**
 * @class Logger
 * @brief 日志记录器
 * 
 * 单例模式，提供统一的日志记录接口。
 */
class Logger {
public:
    /**
     * @brief 获取单例实例
     * @return Logger& 日志器实例引用
     */
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief 设置日志文件
     * 
     * @param filename 日志文件路径
     * @note 追加模式打开
     */
    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }
        log_file_.open(filename, std::ios::out | std::ios::app);
        if (!log_file_.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    /**
     * @brief 设置日志级别
     * 
     * 低于此级别的日志将被忽略。
     * 
     * @param level 日志级别
     */
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_level_ = level;
    }

    /**
     * @brief 记录日志
     * 
     * @param level 日志级别
     * @param message 日志消息
     * @param file 源文件名（可选）
     * @param line 行号（可选）
     */
    void log(LogLevel level, const std::string& message, const std::string& file = "", int line = 0) {
        if (level < log_level_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::string log_message = formatMessage(level, message, file, line);

        // 输出到控制台
        std::cout << log_message << std::endl;

        // 输出到文件
        if (log_file_.is_open()) {
            log_file_ << log_message << std::endl;
            log_file_.flush();
        }
    }

    /**
     * @brief 记录 DEBUG 级别日志
     */
    void debug(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::DEBUG_LEVEL, message, file, line);
    }

    /**
     * @brief 记录 INFO 级别日志
     */
    void info(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::INFO_LEVEL, message, file, line);
    }

    /**
     * @brief 记录 WARNING 级别日志
     */
    void warning(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::WARNING_LEVEL, message, file, line);
    }

    /**
     * @brief 记录 ERROR 级别日志
     */
    void error(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::ERROR_LEVEL, message, file, line);
    }

    /**
     * @brief 记录 FATAL 级别日志
     */
    void fatal(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::FATAL_LEVEL, message, file, line);
    }

private:
    Logger() : log_level_(LogLevel::INFO_LEVEL) {}
    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 格式化日志消息
     */
    std::string formatMessage(LogLevel level, const std::string& message, const std::string& file, int line) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
           << "." << std::setfill('0') << std::setw(3) << ms.count()
           << " [" << getLevelString(level) << "] ";

        if (!file.empty() && line > 0) {
            ss << file << ":" << line << " - ";
        }

        ss << message;
        return ss.str();
    }

    /**
     * @brief 获取日志级别字符串
     */
    std::string getLevelString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG_LEVEL: return "DEBUG";
            case LogLevel::INFO_LEVEL: return "INFO";
            case LogLevel::WARNING_LEVEL: return "WARNING";
            case LogLevel::ERROR_LEVEL: return "ERROR";
            case LogLevel::FATAL_LEVEL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    LogLevel log_level_;        ///< 当前日志级别
    std::ofstream log_file_;    ///< 日志文件流
    std::mutex mutex_;          ///< 线程安全互斥锁
};

/**
 * @name 日志宏
 * @brief 便捷日志宏，自动包含文件名和行号
 * @{
 */
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::getInstance().info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance().error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg, __FILE__, __LINE__)
/** @} */
