#include "LowLatencyLogger.h"
#include <iostream>
#include <sstream>

LowLatencyLogger::LowLatencyLogger(const std::string& fileName, LogLevel level) 
    : logFileName(fileName), minLevel(level), running(true) {
    // 打开日志文件
    logFile.open(fileName, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << fileName << std::endl;
    }

    // 启动后台写入线程
    writerThread = std::thread(&LowLatencyLogger::writerThreadFunc, this);
}

LowLatencyLogger::~LowLatencyLogger() {
    // 停止后台线程
    running = false;
    if (writerThread.joinable()) {
        writerThread.join();
    }

    // 刷新剩余日志
    flush();

    // 关闭日志文件
    if (logFile.is_open()) {
        logFile.close();
    }
}

void LowLatencyLogger::writerThreadFunc() {
    while (running || !logQueue.empty()) {
        if (!logQueue.empty()) {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!logQueue.empty()) {
                LogEntry entry = logQueue.front();
                logQueue.pop();

                // 格式化日志条目
                std::string levelStr = logLevelToString(entry.level);
                auto timePoint = std::chrono::system_clock::from_time_t(entry.timestamp / 1000000000);
                auto time = std::chrono::system_clock::to_time_t(timePoint);
                std::string timeStr = std::ctime(&time);
                timeStr.pop_back(); // 移除换行符

                // 写入日志文件
                if (logFile.is_open()) {
                    logFile << "[" << timeStr << "." << (entry.timestamp % 1000000000) << "] [" 
                           << levelStr << "] " << entry.message << std::endl;
                } else {
                    std::cerr << "[" << timeStr << "." << (entry.timestamp % 1000000000) << "] [" 
                              << levelStr << "] " << entry.message << std::endl;
                }
            }
        } else {
            // 短暂休眠以避免CPU占用过高
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
}

std::string LowLatencyLogger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

void LowLatencyLogger::log(LogLevel level, const std::string& message) {
    // 检查日志级别
    if (level < minLevel) {
        return;
    }

    // 创建日志条目
    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // 添加到队列
    std::lock_guard<std::mutex> lock(queueMutex);
    logQueue.push(entry);
}

void LowLatencyLogger::setLogLevel(LogLevel level) {
    minLevel = level;
}

void LowLatencyLogger::flush() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (logFile.is_open()) {
        logFile.flush();
    }
}