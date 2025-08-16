#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

namespace hft {
namespace diagnostics {

// 前向声明
class IDiagnosticRule;
class IDiagnosticPlugin;
class IPluginEventListener;
class IDiagnosticDataSource;
class IDataVisualizer;
class RemoteMonitorServer;
class AlertManager;

// 基础数据结构
struct SystemState {
    // 性能指标
    struct {
        double latency{0.0};          // 延迟(ms)
        double throughput{0.0};       // 吞吐量(ops/sec)
        double success_rate{0.0};     // 成功率(%)
        double error_rate{0.0};       // 错误率(%)
    } performance;
    
    // 资源使用
    struct {
        double cpu_usage{0.0};        // CPU使用率(%)
        double memory_usage{0.0};     // 内存使用率(%)
        double disk_usage{0.0};       // 磁盘使用率(%)
        double network_usage{0.0};    // 网络使用率(%)
    } resources;
    
    // 硬件健康
    struct {
        struct {
            double temperature{0.0};   // CPU温度(°C)
            double frequency{0.0};     // CPU频率(GHz)
            std::vector<double> core_loads;  // 各核心负载(%)
        } cpu;
        
        struct {
            double temperature{0.0};   // 内存温度(°C)
            double bandwidth{0.0};     // 内存带宽(GB/s)
            double page_faults{0.0};   // 页面错误率
        } memory;
        
        struct {
            std::vector<double> temperatures;    // 磁盘温度(°C)
            std::vector<double> io_rates;        // IO速率(MB/s)
            std::vector<double> latencies;       // IO延迟(ms)
            std::vector<int> bad_sectors;        // 坏扇区数
        } disk;
        
        struct {
            std::vector<double> temperatures;    // 网卡温度(°C)
            std::vector<double> bandwidths;      // 带宽使用(Mbps)
            std::vector<double> error_rates;     // 错误率(%)
            std::vector<double> packet_losses;   // 丢包率(%)
        } network;
    } hardware;
    
    // 网络状态
    struct {
        int active_connections{0};    // 活动连接数
        int failed_connections{0};    // 失败连接数
        double retry_rate{0.0};       // 重试率(%)
        std::vector<std::string> blocked_ips;     // 被封禁IP
        std::vector<std::string> suspicious_ips;  // 可疑IP
    } network;
    
    // 系统日志和错误
    std::vector<std::string> logs;
    std::vector<std::string> errors;
    std::chrono::system_clock::time_point timestamp;
};

// 诊断问题严重程度
enum class Severity {
    Info,
    Warning,
    Error,
    Critical
};

// 规则类型
enum class RuleType {
    Hardware,      // 硬件相关
    Network,       // 网络相关
    Performance,   // 性能相关
    Security,      // 安全相关
    Custom         // 自定义
};

// 规则优先级
enum class RulePriority {
    Low,
    Medium,
    High,
    Critical
};

#include <memory>
#include <string>
#include <vector>
#include <future>
#include "../core/Types.h"
#include "../core/Logger.h"

namespace hft {
namespace diagnostics {

class DiagnosticTool {
public:
    // 诊断类型
    enum class DiagnosticType {
        PERFORMANCE,    // 性能诊断
        MEMORY,        // 内存诊断
        NETWORK,       // 网络诊断
        DATABASE,      // 数据库诊断
        THREAD,        // 线程诊断
        RESOURCE,      // 资源诊断
        SECURITY       // 安全诊断
    };

    // 诊断级别
    enum class DiagnosticLevel {
        BASIC,        // 基础诊断
        DETAILED,     // 详细诊断
        ADVANCED,     // 高级诊断
        EXPERT        // 专家诊断
    };

    // 诊断结果
    struct DiagnosticResult {
        bool success;
        std::vector<std::string> findings;
        std::vector<std::string> recommendations;
        std::unordered_map<std::string, double> metrics;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

    // 初始化诊断工具
    bool initialize() {
        try {
            // 初始化诊断引擎
            initializeDiagnosticEngine();
            
            // 加载诊断规则
            loadDiagnosticRules();
            
            // 启动监控
            startMonitoring();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Diagnostic tool initialization failed: {}", e.what());
            return false;
        }
    }

    // 执行诊断
    DiagnosticResult runDiagnostic(
        DiagnosticType type,
        DiagnosticLevel level = DiagnosticLevel::DETAILED) {
        
        // 准备诊断上下文
        auto context = prepareDiagnosticContext();
        
        // 执行诊断
        switch (type) {
            case DiagnosticType::PERFORMANCE:
                return diagnosePerformance(level);
            case DiagnosticType::MEMORY:
                return diagnoseMemory(level);
            case DiagnosticType::NETWORK:
                return diagnoseNetwork(level);
            case DiagnosticType::THREAD:
                return diagnoseThreads(level);
            default:
                return DiagnosticResult{false, {}, {}, {}, {}, 
                    {"Unsupported diagnostic type"}};
        }
    }

    // 异步诊断
    std::future<DiagnosticResult> runDiagnosticAsync(
        DiagnosticType type,
        DiagnosticLevel level = DiagnosticLevel::DETAILED) {
        return std::async(std::launch::async,
            [this, type, level]() {
                return runDiagnostic(type, level);
            });
    }

    // 持续监控
    void startContinuousMonitoring(
        const std::vector<DiagnosticType>& types,
        std::chrono::seconds interval);

    // 获取诊断历史
    std::vector<DiagnosticResult> getDiagnosticHistory() const;

private:
    // 性能分析器
    class PerformanceAnalyzer {
    public:
        struct PerformanceMetrics {
            double cpu_usage;
            double memory_usage;
            double io_throughput;
            double network_latency;
            std::vector<double> thread_latencies;
        };
        
        PerformanceMetrics analyze();
        std::vector<std::string> getBottlenecks();
        std::vector<std::string> getOptimizationSuggestions();
    
    private:
        void analyzeCPUUsage();
        void analyzeMemoryUsage();
        void analyzeIOPerformance();
        void analyzeNetworkPerformance();
    };

    // 内存分析器
    class MemoryAnalyzer {
    public:
        struct MemoryMetrics {
            size_t total_allocated;
            size_t peak_usage;
            size_t fragmentation;
            std::vector<std::string> leak_suspects;
        };
        
        MemoryMetrics analyze();
        std::vector<std::string> findLeaks();
        std::vector<std::string> optimizeAllocation();
    
    private:
        void trackAllocations();
        void analyzeHeap();
        void detectLeaks();
    };

    // 线程分析器
    class ThreadAnalyzer {
    public:
        struct ThreadMetrics {
            int active_threads;
            int blocked_threads;
            double context_switches;
            std::vector<std::string> deadlock_risks;
        };
        
        ThreadMetrics analyze();
        std::vector<std::string> findDeadlocks();
        std::vector<std::string> optimizeThreading();
    
    private:
        void analyzeThreadStates();
        void detectDeadlocks();
        void measureContextSwitches();
    };

private:
    // 诊断方法
    DiagnosticResult diagnosePerformance(DiagnosticLevel level);
    DiagnosticResult diagnoseMemory(DiagnosticLevel level);
    DiagnosticResult diagnoseNetwork(DiagnosticLevel level);
    DiagnosticResult diagnoseThreads(DiagnosticLevel level);
    
    // 诊断工具
    void* prepareDiagnosticContext();
    void analyzeDiagnosticResults(DiagnosticResult& result);
    void generateRecommendations(DiagnosticResult& result);
    
    // 监控功能
    void startMonitoring();
    void processMonitoringData();
    void detectAnomalies();

private:
    std::unique_ptr<PerformanceAnalyzer> perf_analyzer_;
    std::unique_ptr<MemoryAnalyzer> mem_analyzer_;
    std::unique_ptr<ThreadAnalyzer> thread_analyzer_;
    
    // 诊断历史
    std::vector<DiagnosticResult> diagnostic_history_;
    
    // 监控状态
    struct MonitoringState {
        bool is_active{false};
        std::chrono::system_clock::time_point last_check;
        std::vector<DiagnosticType> monitored_types;
    } monitoring_state_;
};

} // namespace diagnostics
} // namespace hft
