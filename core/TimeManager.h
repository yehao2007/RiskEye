#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

namespace hft {
namespace core {

// 时间管理器
class TimeManager {
public:
    TimeManager();
    ~TimeManager() = default;

    // 初始化时间管理器
    bool initialize();

    // 关闭时间管理器
    void shutdown();

    // 获取当前时间戳（微秒）
    uint64_t getCurrentTimestamp() const;

    // 获取当前时间戳（纳秒）
    uint64_t getCurrentTimestampNs() const;

    // 与GPS同步时间
    bool synchronizeWithGps(const std::string& gps_server);

    // 检查时间同步状态
    bool isTimeSynchronized() const;

    // 获取时间偏差（纳秒）
    int64_t getTimeOffset() const;

private:
    bool m_initialized;
    bool m_time_synchronized;
    int64_t m_time_offset_ns;
    mutable std::mutex m_mutex;

    // 内部更新时间偏差
    void updateTimeOffset(int64_t offset_ns);
};

using TimeManagerPtr = std::shared_ptr<TimeManager>;

} // namespace core
} // namespace hft