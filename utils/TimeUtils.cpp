#include "TimeUtils.h"
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace hft {
namespace utils {

std::string TimeUtils::timestampToString(uint64_t timestamp_us, const std::string& format) {
    std::time_t seconds = timestamp_us / 1000000;
    uint32_t microseconds = timestamp_us % 1000000;

    std::tm* tm = std::localtime(&seconds);
    if (!tm) {
        return "";
    }

    std::stringstream ss;
    ss << std::put_time(tm, format.c_str());

    // 处理微秒部分
    size_t pos = format.find("%f");
    if (pos != std::string::npos) {
        ss.seekp(pos);
        ss << std::setw(6) << std::setfill('0') << microseconds;
    }

    return ss.str();
}

uint64_t TimeUtils::stringToTimestamp(const std::string& time_str, const std::string& format) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, format.c_str());
    if (ss.fail()) {
        return 0;
    }

    std::time_t seconds = std::mktime(&tm);
    if (seconds == -1) {
        return 0;
    }

    // 提取微秒部分
    uint32_t microseconds = 0;
    size_t pos = format.find("%f");
    if (pos != std::string::npos && time_str.length() > pos + 6) {
        try {
            microseconds = std::stoi(time_str.substr(pos, 6));
        } catch (...) {
            microseconds = 0;
        }
    }

    return static_cast<uint64_t>(seconds) * 1000000 + microseconds;
}

std::string TimeUtils::getCurrentTimeString(const std::string& format) {
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
    return timestampToString(timestamp_us, format);
}

void TimeUtils::sleepMs(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void TimeUtils::sleepUs(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

} // namespace utils
} // namespace hft