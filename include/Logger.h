#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    DEBUG_LEVEL,
    INFO_LEVEL,
    WARNING_LEVEL,
    ERROR_LEVEL,
    FATAL_LEVEL
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

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

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_level_ = level;
    }

    void log(LogLevel level, const std::string& message, const std::string& file = "", int line = 0) {
        if (level < log_level_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::string log_message = formatMessage(level, message, file, line);

        // Output to console
        std::cout << log_message << std::endl;

        // Output to file if open
        if (log_file_.is_open()) {
            log_file_ << log_message << std::endl;
            log_file_.flush();
        }
    }

    // Convenience methods
    void debug(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::DEBUG_LEVEL, message, file, line);
    }

    void info(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::INFO_LEVEL, message, file, line);
    }

    void warning(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::WARNING_LEVEL, message, file, line);
    }

    void error(const std::string& message, const std::string& file = "", int line = 0) {
        log(LogLevel::ERROR_LEVEL, message, file, line);
    }

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

    LogLevel log_level_;
    std::ofstream log_file_;
    std::mutex mutex_;
};

// Convenience macros for logging
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::getInstance().info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance().error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg, __FILE__, __LINE__)
