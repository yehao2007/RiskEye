#pragma once

#include <vector>
#include <chrono>
#include <functional>
#include <memory>
#include <atomic>
#include "../core/Logger.h"

namespace hft {
namespace core {

class FaultDetector {
public:
    // 故障类型
    enum class FaultType {
        HARDWARE_ERROR,     // 硬件故障
        NETWORK_FAILURE,    // 网络故障
        PROCESS_CRASH,      // 进程崩溃
        MEMORY_LEAK,       // 内存泄漏
        RESOURCE_EXHAUST,   // 资源耗尽
        DATA_CORRUPTION,    // 数据损坏
        TIMING_VIOLATION    // 时序违规
    };

    // 故障级别
    enum class FaultSeverity {
        CRITICAL,   // 严重故障
        HIGH,       // 高危故障
        MEDIUM,     // 中等故障
        LOW        // 低危故障
    };

    // 故障信息
    struct FaultInfo {
        FaultType type;
        FaultSeverity severity;
        std::string component;
        std::string description;
        uint64_t timestamp;
        std::vector<std::string> affected_systems;
    };

    // 初始化故障检测器
    bool initialize();
    
    // 启动故障检测
    void startDetection();
    
    // 停止故障检测
    void stopDetection();
    
    // 注册故障处理回调
    using FaultCallback = std::function<void(const FaultInfo&)>;
    void registerFaultCallback(FaultCallback callback);
    
    // 获取系统健康状态
    struct HealthStatus {
        bool is_healthy;
        double health_score;
        std::vector<FaultInfo> active_faults;
        std::vector<std::string> warnings;
    };
    HealthStatus getHealthStatus() const;

    // 执行自动恢复
    bool attemptRecovery(const FaultInfo& fault);
    
    // 设置检测策略
    struct DetectionPolicy {
        uint64_t check_interval_ms;
        uint32_t max_retry_attempts;
        bool auto_recovery_enabled;
        std::vector<FaultType> monitored_faults;
    };
    void setDetectionPolicy(const DetectionPolicy& policy);

private:
    // 故障检测方法
    void detectHardwareFaults();
    void detectNetworkFaults();
    void detectProcessFaults();
    void detectMemoryFaults();
    void detectResourceFaults();
    void detectDataCorruption();
    void detectTimingViolations();
    
    // 恢复方法
    bool recoverFromHardwareFault(const FaultInfo& fault);
    bool recoverFromNetworkFault(const FaultInfo& fault);
    bool recoverFromProcessCrash(const FaultInfo& fault);
    bool recoverFromMemoryLeak(const FaultInfo& fault);
    bool recoverFromResourceExhaustion(const FaultInfo& fault);
    bool recoverFromDataCorruption(const FaultInfo& fault);
    bool recoverFromTimingViolation(const FaultInfo& fault);
    
    // 故障分析
    void analyzeFaultPattern();
    void updateHealthScore();
    
    // 预测性维护
    void predictFailures();
    void recommendMaintenance();

private:
    std::atomic<bool> running_{false};
    DetectionPolicy policy_;
    FaultCallback fault_callback_;
    std::vector<FaultInfo> fault_history_;
    
    // 健康状态
    std::atomic<double> health_score_{1.0};
    std::atomic<uint32_t> active_fault_count_{0};
    
    // 检测器组件
    class HardwareMonitor;
    class NetworkMonitor;
    class ProcessMonitor;
    class MemoryMonitor;
    class ResourceMonitor;
    class DataValidator;
    class TimingAnalyzer;
    
    std::unique_ptr<HardwareMonitor> hw_monitor_;
    std::unique_ptr<NetworkMonitor> net_monitor_;
    std::unique_ptr<ProcessMonitor> proc_monitor_;
    std::unique_ptr<MemoryMonitor> mem_monitor_;
    std::unique_ptr<ResourceMonitor> res_monitor_;
    std::unique_ptr<DataValidator> data_validator_;
    std::unique_ptr<TimingAnalyzer> timing_analyzer_;
    
    // 故障模式分析
    class FaultPatternAnalyzer {
    public:
        void addFault(const FaultInfo& fault);
        std::vector<std::string> analyzePatterns();
        double predictNextFailure();
    private:
        std::vector<FaultInfo> fault_database_;
    };
    std::unique_ptr<FaultPatternAnalyzer> pattern_analyzer_;
    
    // 监控参数
    static constexpr auto CHECK_INTERVAL = std::chrono::milliseconds(100);
    static constexpr uint32_t MAX_FAULT_HISTORY = 1000;
    static constexpr double MIN_HEALTH_SCORE = 0.0;
    static constexpr double HEALTH_RECOVERY_RATE = 0.01;
};

} // namespace core
} // namespace hft
