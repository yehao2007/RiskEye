#pragma once

#include <atomic>
#include <chrono>
#include "../core/Types.h"

namespace hft {
namespace core {

class HardwareTimer {
public:
    // 初始化硬件时钟
    bool initialize(const char* device = nullptr);
    
    // 获取硬件时间戳
    uint64_t getTimestamp() const;
    
    // 获取校准后的时间戳
    uint64_t getCalibratedTimestamp() const;
    
    // 计算时间差(纳秒)
    int64_t calculateDeltaNs(uint64_t start, uint64_t end) const;
    
    // 设置时钟源
    enum class ClockSource {
        TSC,        // 时间戳计数器
        HPET,       // 高精度事件定时器
        PTP,        // 精确时间协议
        SYSTEM      // 系统时钟
    };
    bool setClockSource(ClockSource source);
    
    // 同步状态
    struct SyncStatus {
        bool is_synchronized;      // 是否同步
        int64_t offset_ns;        // 时钟偏移(纳秒)
        double drift_ppb;         // 漂移率(ppb)
        uint64_t last_sync_time;  // 上次同步时间
    };
    SyncStatus getSyncStatus() const;

private:
    // 校准TSC
    void calibrateTSC();
    
    // 测量TSC频率
    double measureTSCFrequency();
    
    // 校准HPET
    void calibrateHPET();
    
    // PTP同步
    void synchronizePTP();
    
    // 监控时钟漂移
    void monitorClockDrift();
    
    // 计算Allan方差
    double calculateAllanVariance(const std::vector<uint64_t>& timestamps);

private:
    std::atomic<uint64_t> base_tsc_{0};        // 基准TSC值
    std::atomic<uint64_t> tsc_frequency_{0};   // TSC频率
    std::atomic<int64_t> clock_offset_{0};     // 时钟偏移
    std::atomic<double> drift_rate_{0.0};      // 漂移率
    ClockSource current_source_{ClockSource::TSC}; // 当前时钟源
    
    // 性能参数
    static constexpr int64_t MAX_OFFSET_NS = 100;    // 最大允许偏移
    static constexpr double MAX_DRIFT_PPB = 50.0;    // 最大允许漂移
    static constexpr uint64_t SYNC_INTERVAL_NS = 1000000; // 同步间隔(1ms)
    
    class TSCHelper {
    public:
        static inline uint64_t rdtsc() {
            #ifdef _MSC_VER
                return __rdtsc();
            #else
                uint32_t hi, lo;
                __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
                return (static_cast<uint64_t>(hi) << 32) | lo;
            #endif
        }
        
        static inline uint64_t rdtscp() {
            unsigned int aux;
            #ifdef _MSC_VER
                return __rdtscp(&aux);
            #else
                uint32_t hi, lo;
                __asm__ __volatile__ ("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
                return (static_cast<uint64_t>(hi) << 32) | lo;
            #endif
        }
    };
};

} // namespace core
} // namespace hft
