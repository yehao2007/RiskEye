#pragma once
#include <cstdint>
#include <string>

namespace hft {
namespace synchronization {

class TimeProvider {
public:
    TimeProvider();
    ~TimeProvider();

    // 初始化时间提供器
    void initialize();
    // 更新当前时间（纳秒级）
    void updateCurrentTime(uint64_t nanos);
    // 获取当前时间（纳秒级）
    uint64_t getCurrentNanos() const;
    // 获取当前时间（毫秒级）
    uint64_t getCurrentMillis() const;
    // 获取当前时间（UTC格式字符串）
    std::string getCurrentUtcTime() const;
    // 将纳秒转换为UTC时间字符串
    std::string nanosToUtcTime(uint64_t nanos) const;
    // 将UTC时间字符串转换为纳秒
    uint64_t utcTimeToNanos(const std::string& utcTime) const;

private:
    // 基准时间（纳秒级）
    uint64_t m_baseTimeNanos;
    // 系统启动时间（纳秒级）
    uint64_t m_bootTimeNanos;
};

} // namespace synchronization
} // namespace hft