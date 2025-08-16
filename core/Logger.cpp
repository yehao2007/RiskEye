#include "Logger.h"
#include <iomanip>

namespace hft {
namespace core {

// 静态成员初始化
LogLevel Logger::m_logLevel = LogLevel::INFO;
std::ofstream Logger::m_logFile;
bool Logger::m_consoleOutput = true;
std::mutex Logger::m_mutex;

Logger::Logger(const std::string& moduleName) : m_moduleName(moduleName) {
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logFile.flush();
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logLevel = level;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    m_logFile.open(filename, std::ios::out | std::ios::app);
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

void Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enable;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::FATAL, message);
}

std::string Logger::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&time_t_now);

    std::stringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S.");

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - std::chrono::system_clock::from_time_t(time_t_now));
    ss << std::setw(3) << std::setfill('0') << ms.count();

    return ss.str();
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_logLevel) {
        return;
    }

    std::string timeStr = getCurrentTime();
    std::string levelStr = logLevelToString(level);
    std::string logMessage = timeStr + " [" + levelStr + "] [" + m_moduleName + "]: " + message + "\n";

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_consoleOutput) {
        // 根据日志级别设置控制台颜色
        switch (level) {
            case LogLevel::DEBUG: std::cout << "\033[37m" << logMessage << "\033[0m";
                break;
            case LogLevel::INFO: std::cout << "\033[32m" << logMessage << "\033[0m";
                break;
            case LogLevel::WARNING: std::cout << "\033[33m" << logMessage << "\033[0m";
                break;
            case LogLevel::ERROR: std::cout << "\033[31m" << logMessage << "\033[0m";
                break;
            case LogLevel::FATAL: std::cout << "\033[41;37m" << logMessage << "\033[0m";
                break;
        }
    }

    if (m_logFile.is_open()) {
        m_logFile << logMessage;
        m_logFile.flush();
    }
}

std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

} // namespace core
} // namespace hft