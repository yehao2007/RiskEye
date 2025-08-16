#include "TimeProvider.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace hft {
namespace synchronization {

TimeProvider::TimeProvider() : m_baseTimeNanos(0), m_bootTimeNanos(0) {
}

TimeProvider::~TimeProvider() {
}

void TimeProvider::initialize() {
    // 获取系统启动时间
    auto now = std::chrono::high_resolution_clock::now();
    m_bootTimeNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    m_baseTimeNanos = m_bootTimeNanos;
}

void TimeProvider::updateCurrentTime(uint64_t nanos) {
    m_baseTimeNanos = nanos;
    m_bootTimeNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

uint64_t TimeProvider::getCurrentNanos() const {
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t currentNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    return m_baseTimeNanos + (currentNanos - m_bootTimeNanos);
}

uint64_t TimeProvider::getCurrentMillis() const {
    return getCurrentNanos() / 1000000;
}

std::string TimeProvider::getCurrentUtcTime() const {
    return nanosToUtcTime(getCurrentNanos());
}

std::string TimeProvider::nanosToUtcTime(uint64_t nanos) const {
    // 将纳秒转换为时间点
    std::chrono::nanoseconds duration(nanos);
    std::chrono::system_clock::time_point timePoint(duration);

    // 转换为tm结构
    std::time_t timeT = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tm = *std::gmtime(&timeT);

    // 构建UTC时间字符串
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

uint64_t TimeProvider::utcTimeToNanos(const std::string& utcTime) const {
    // 解析UTC时间字符串
    std::tm tm = {0};
    std::istringstream ss(utcTime);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

    if (ss.fail()) {
        throw std::runtime_error("Failed to parse UTC time: " + utcTime);
    }

    // 转换为时间点
    std::time_t timeT = std::mktime(&tm);
    std::chrono::system_clock::time_point timePoint(std::chrono::seconds(timeT));

    // 转换为纳秒
    return std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint.time_since_epoch()).count();
}

} // namespace synchronization
} // namespace hft