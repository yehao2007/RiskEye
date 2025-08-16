#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>

namespace hft {
namespace utils {

// 性能指标类型
enum class PerformanceMetricType {
    LATENCY,
    THROUGHPUT,
    CPU_USAGE,
    MEMORY_USAGE,
    NETWORK_LATENCY,
    ORDER_EXECUTION_TIME
};

// 性能样本
struct PerformanceSample {
    double value;
    std::chrono::system_clock::time_point timestamp;
};

// 性能监控器
class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor() = default;

    // 记录性能样本
    void recordSample(const std::string& metric_name, PerformanceMetricType type, double value);

    // 获取指标的统计信息
    double getAverage(const std::string& metric_name);
    double getMaximum(const std::string& metric_name);
    double getMinimum(const std::string& metric_name);
    double getPercentile(const std::string& metric_name, double percentile);

    // 生成性能报告
    std::string generateReport();

    // 检查性能阈值
    bool checkThresholds();

    // 设置阈值
    void setThreshold(const std::string& metric_name, double threshold);

private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<PerformanceSample>> m_metrics;
    std::unordered_map<std::string, PerformanceMetricType> m_metric_types;
    std::unordered_map<std::string, double> m_thresholds;
};

} // namespace utils
} // namespace hft
// utils/PerformanceMonitor.h 改进
class PerformanceMonitor {
public:
    // 添加延迟分析
    LatencyMetrics analyzeLatency();
    
    // 添加系统资源监控
    SystemResourceMetrics getResourceUsage();
    
    // 添加性能预警
    void setPerformanceAlert(const PerformanceThreshold& threshold);
    
private:
    CircularBuffer<PerformanceMetric> metrics_;
    // ... 其他成员
};