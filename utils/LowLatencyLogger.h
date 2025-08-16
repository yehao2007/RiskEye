#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <chrono>

// 日志级别枚举
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// 日志条目结构体
struct LogEntry {
    LogLevel level;
    std::string message;
    uint64_t timestamp;
};

// 低延迟日志器类
class LowLatencyLogger {
private:
    // 日志文件名
    std::string logFileName;
    // 日志级别
    LogLevel minLevel;
    // 日志队列
    std::queue<LogEntry> logQueue;
    // 互斥锁保护队列
    std::mutex queueMutex;
    // 原子标志控制后台线程
    std::atomic<bool> running;
    // 后台写入线程
    std::thread writerThread;
    // 日志文件流
    std::ofstream logFile;

    // 后台写入函数
    void writerThreadFunc();
    // 将日志级别转换为字符串
    std::string logLevelToString(LogLevel level) const;

public:
    // 构造函数
    LowLatencyLogger(const std::string& fileName, LogLevel level = LogLevel::INFO);
    // 析构函数
    ~LowLatencyLogger();

    // 记录日志
    void log(LogLevel level, const std::string& message);
    // 设置日志级别
    void setLogLevel(LogLevel level);
    // 刷新日志到文件
    void flush();
};