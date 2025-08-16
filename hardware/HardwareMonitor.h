#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include "../core/Logger.h"

namespace hft {
namespace hardware {

class HardwareMonitor {
public:
    // 初始化硬件监控
    bool initialize();
    
    // 启动监控
    void startMonitoring();
    
    // 停止监控
    void stopMonitoring();
    
    // CPU状态
    struct CPUStats {
        double temperature_c;        // 温度(摄氏度)
        double frequency_ghz;        // 频率(GHz)
        double utilization_percent;  // 使用率(%)
        int active_cores;           // 活动核心数
        bool turbo_enabled;         // 睿频状态
    };
    CPUStats getCPUStats() const;
    
    // 内存状态
    struct MemoryStats {
        size_t total_bytes;         // 总内存
        size_t used_bytes;          // 已用内存
        size_t cached_bytes;        // 缓存大小
        double bandwidth_gbps;      // 带宽(GB/s)
        int numa_nodes;            // NUMA节点数
    };
    MemoryStats getMemoryStats() const;
    
    // 热管理状态
    struct ThermalStats {
        std::vector<double> core_temps;  // 核心温度
        double package_temp;             // 包温度
        int fan_speed_rpm;              // 风扇转速
        double power_draw_watts;        // 功耗(瓦特)
    };
    ThermalStats getThermalStats() const;

    // 设置温度阈值
    void setTemperatureThresholds(double warning_c, double critical_c);
    
    // 设置功耗限制
    void setPowerLimit(double watts);
    
    // 注册告警回调
    using AlertCallback = std::function<void(const std::string&)>;
    void registerAlertCallback(AlertCallback callback);

private:
    // 监控CPU状态
    void monitorCPU();
    
    // 监控内存状态
    void monitorMemory();
    
    // 监控温度
    void monitorTemperature();
    
    // 调整CPU频率
    void adjustCPUFrequency();
    
    // 控制风扇速度
    void controlFanSpeed();
    
    // 检查热警告
    void checkThermalAlerts();
    
    // DVFS(动态电压频率调节)
    void performDVFS();
    
    // 读取MSR寄存器
    uint64_t readMSR(int cpu, uint32_t reg);
    
    // 写入MSR寄存器
    void writeMSR(int cpu, uint32_t reg, uint64_t value);

private:
    std::atomic<bool> running_{false};          // 运行标志
    std::thread monitor_thread_;                // 监控线程
    AlertCallback alert_callback_;              // 告警回调
    
    // 温度阈值
    double warning_temp_c_{70.0};              // 警告温度
    double critical_temp_c_{85.0};             // 临界温度
    
    // 功耗限制
    double power_limit_watts_{95.0};           // 功耗限制
    
    // 监控间隔
    static constexpr auto MONITOR_INTERVAL = std::chrono::milliseconds(100);
    
    // MSR寄存器
    static constexpr uint32_t MSR_TEMPERATURE_TARGET = 0x1A2;
    static constexpr uint32_t MSR_PERF_STATUS = 0x198;
    static constexpr uint32_t MSR_POWER_CTRL = 0x1FC;
    
    // RAPL(运行时平均功率限制)域
    enum class RAPLDomain {
        PACKAGE,
        CORE,
        UNCORE,
        DRAM
    };
    
    // 获取RAPL能耗
    double getRAPLEnergy(RAPLDomain domain);
};

} // namespace hardware
} // namespace hft
