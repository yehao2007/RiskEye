#pragma once

#include <unordered_map>
#include <chrono>
#include <vector>
#include <atomic>
#include "../core/Types.h"

namespace hft {
namespace core {

class PerformanceAnalyzer {
public:
    // 初始化性能分析器
    bool initialize();
    
    // 性能跟踪点类型
    enum class TracePointType {
        MARKET_DATA,        // 市场数据处理
        ORDER_PROCESSING,   // 订单处理
        STRATEGY_EXECUTION, // 策略执行
        RISK_CHECK,        // 风险检查
        NETWORK_IO,        // 网络IO
        DATABASE_ACCESS    // 数据库访问
    };

    // 开始跟踪
    void startTrace(TracePointType type, const std::string& name);
    
    // 结束跟踪
    void endTrace(TracePointType type, const std::string& name);
    
    // 记录延迟
    void recordLatency(TracePointType type, uint64_t latency_ns);
    
    // 记录事件
    void recordEvent(const std::string& event_name, 
                    const std::string& details = "");

    // 性能指标
    struct Metrics {
        // 延迟统计
        struct LatencyStats {
            uint64_t min_ns;
            uint64_t max_ns;
            uint64_t avg_ns;
            uint64_t p50_ns;
            uint64_t p95_ns;
            uint64_t p99_ns;
        };
        
        // 吞吐量统计
        struct ThroughputStats {
            double messages_per_second;
            double bytes_per_second;
            uint64_t total_messages;
            uint64_t total_bytes;
        };
        
        // 资源使用统计
        struct ResourceStats {
            double cpu_usage_percent;
            double memory_usage_mb;
            double network_bandwidth_mbps;
            int active_connections;
        };
        
        std::unordered_map<TracePointType, LatencyStats> latency_stats;
        std::unordered_map<TracePointType, ThroughputStats> throughput_stats;
        ResourceStats resource_stats;
    };

    // 获取性能指标
    Metrics getMetrics() const;
    
    // 设置性能目标
    void setPerformanceTargets(uint64_t max_latency_ns,
                             double min_throughput_mps);
                             
    // 性能告警回调
    using AlertCallback = std::function<void(const std::string&)>;
    void setAlertCallback(AlertCallback callback);
    
    // 导出性能报告
    void exportReport(const std::string& filename);

private:
    // 计算统计指标
    void calculateStats();
    
    // 检查性能目标
    void checkPerformanceTargets();
    
    // 更新滑动窗口
    void updateSlidingWindow();
    
    // 性能数据点
    struct DataPoint {
        uint64_t timestamp;
        uint64_t value;
        TracePointType type;
    };
    
    // 滑动窗口统计
    class SlidingWindowStats {
    public:
        void addSample(uint64_t value);
        void updateWindow();
        
        uint64_t getMin() const { return min_value_; }
        uint64_t getMax() const { return max_value_; }
        uint64_t getAvg() const { return avg_value_; }
        
    private:
        std::vector<uint64_t> samples_;
        uint64_t min_value_{UINT64_MAX};
        uint64_t max_value_{0};
        uint64_t avg_value_{0};
        static constexpr size_t WINDOW_SIZE = 1000;
    };

private:
    // 性能数据存储
    std::unordered_map<TracePointType, SlidingWindowStats> latency_windows_;
    std::unordered_map<std::string, std::vector<DataPoint>> trace_points_;
    
    // 性能目标
    uint64_t target_max_latency_ns_{1000};  // 1微秒
    double target_min_throughput_mps_{1e6}; // 每秒100万消息
    
    // 告警回调
    AlertCallback alert_callback_;
    
    // 原子计数器
    std::atomic<uint64_t> total_messages_{0};
    std::atomic<uint64_t> total_bytes_{0};
    
    // 采样参数
    static constexpr auto SAMPLING_INTERVAL = std::chrono::microseconds(100);
    static constexpr size_t MAX_SAMPLES = 10000;
    
    // 统计窗口
    static constexpr auto STATS_WINDOW = std::chrono::seconds(1);
};

} // namespace core
} // namespace hft
