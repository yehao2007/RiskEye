#include "TimeManager.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace hft {
namespace core {

TimeManager::TimeManager()
    : m_initialized(false),
      m_time_synchronized(false),
      m_time_offset_ns(0) {
}

bool TimeManager::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    m_time_synchronized = false;
    m_time_offset_ns = 0;

    std::cout << "TimeManager initialized" << std::endl;
    return true;
}

void TimeManager::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized) {
        m_initialized = false;
        std::cout << "TimeManager shutdown" << std::endl;
    }
}

uint64_t TimeManager::getCurrentTimestamp() const {
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_time_synchronized) {
        timestamp += m_time_offset_ns / 1000;
    }

    return timestamp;
}

uint64_t TimeManager::getCurrentTimestampNs() const {
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_time_synchronized) {
        timestamp_ns += m_time_offset_ns;
    }

    return timestamp_ns;
}

bool TimeManager::synchronizeWithGps(const std::string& gps_server) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized) {
        std::cerr << "TimeManager not initialized" << std::endl;
        return false;
    }

    // 模拟GPS同步过程
    std::cout << "Synchronizing time with GPS server: " << gps_server << std::endl;

    // 模拟网络延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 假设同步成功，设置一个随机的时间偏差（实际应用中会从GPS信号获取）
    int64_t offset_ns = 100000; // 100微秒
    updateTimeOffset(offset_ns);

    m_time_synchronized = true;
    std::cout << "Time synchronized with GPS, offset: " << offset_ns << " ns" << std::endl;
    return true;
}

bool TimeManager::isTimeSynchronized() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_time_synchronized;
}

int64_t TimeManager::getTimeOffset() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_time_offset_ns;
}

void TimeManager::updateTimeOffset(int64_t offset_ns) {
    m_time_offset_ns = offset_ns;
}

} // namespace core
} // namespace hft