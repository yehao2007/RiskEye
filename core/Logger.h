#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <iostream>
#include <ctime>

namespace hft {
namespace core {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    Logger(const std::string& moduleName);
    ~Logger();

    // 设置日志级别
    static void setLogLevel(LogLevel level);
    // 设置日志文件
    static void setLogFile(const std::string& filename);
    // 启用控制台输出
    static void enableConsoleOutput(bool enable);

    // 日志输出方法
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

private:
    std::string m_moduleName;
    static LogLevel m_logLevel;
    static std::ofstream m_logFile;
    static bool m_consoleOutput;
    static std::mutex m_mutex;

    // 获取当前时间字符串
    std::string getCurrentTime() const;
    // 日志输出实现
    void log(LogLevel level, const std::string& message);
    // 获取日志级别字符串
    std::string logLevelToString(LogLevel level) const;
};

} // namespace core
} // namespace hft