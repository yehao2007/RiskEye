#pragma once

#include <cstdint>
#include <chrono>
#include "../core/Types.h"

namespace hft {
namespace core {

class TimeSync {
public:
    // 初始化PTP同步
    bool initializePTP(const char* interface);
    
    // 获取硬件时间戳
    uint64_t getHardwareTimestamp();
    
    // 获取软件时间戳
    uint64_t getSoftwareTimestamp();
    
    // 计算时钟偏移
    int64_t calculateOffset();
    
    // 同步时钟
    void synchronizeClock();
    
    // 获取同步状态
    struct SyncStatus {
        bool is_synchronized;
        int64_t current_offset_ns;
        double drift_ppb;  // parts per billion
    };
    SyncStatus getStatus() const;

private:
    // 配置硬件时钟
    void configureHardwareClock();
    
    // 校准时钟源
    void calibrateClockSource();
    
    // 监控时钟漂移
    void monitorClockDrift();
    
    // 调整时钟频率
    void adjustClockFrequency(double ppb);

private:
    void* ptp_clock_{nullptr};    // PTP时钟句柄
    uint64_t base_time_{0};       // 基准时间
    int64_t last_offset_{0};      // 上次偏移
    bool initialized_{false};      // 初始化标志
    
    static constexpr int64_t MAX_OFFSET_NS = 100;  // 最大允许偏移(ns)
    static constexpr double MAX_DRIFT_PPB = 50.0;  // 最大允许漂移(ppb)
};

} // namespace core
} // namespace hft
