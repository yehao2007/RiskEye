#pragma once

#include <cstdint>
#include <string>

namespace hft {
namespace utils {

// 时间工具类
class TimeUtils {
public:
    // 将时间戳转换为字符串
    static std::string timestampToString(uint64_t timestamp_us, const std::string& format = "%Y-%m-%d %H:%M:%S.%f");

    // 将字符串转换为时间戳
    static uint64_t stringToTimestamp(const std::string& time_str, const std::string& format = "%Y-%m-%d %H:%M:%S.%f");

    // 获取当前时间字符串
    static std::string getCurrentTimeString(const std::string& format = "%Y-%m-%d %H:%M:%S.%f");

    // 睡眠指定的毫秒数
    static void sleepMs(uint32_t ms);

    // 睡眠指定的微秒数
    static void sleepUs(uint32_t us);
};

} // namespace utils
} // namespace hft