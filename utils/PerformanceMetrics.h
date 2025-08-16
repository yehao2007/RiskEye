#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace hft {
namespace utils {

class PerformanceMetrics {
public:
    PerformanceMetrics();
    ~PerformanceMetrics() = default;

    // 开始计时
    void startTimer(const std::string& metric_name);
    // 结束计时并记录
    double endTimer(const std::string& metric_name);
    // 记录值
    void recordValue(const std::string& metric_name, double value);
    // 获取平均值
    double getAverage(const std::string& metric_name) const;
    // 获取最大值
    double getMax(const std::string& metric_name) const;
    // 获取最小值
    double getMin(const std::string& metric_name) const;
    // 获取计数
    uint64_t getCount(const std::string& metric_name) const;
    // 重置特定指标
    void resetMetric(const std::string& metric_name);
    // 重置所有指标
    void resetAll();
    // 打印所有指标
    void printMetrics() const;

private:
    struct MetricData {
        double sum;         // 总和
        double min;         // 最小值
        double max;         // 最大值
        uint64_t count;     // 计数
        std::chrono::high_resolution_clock::time_point start_time; // 开始时间
    };

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, MetricData> m_metrics;
};

} // namespace utils
} // namespace hft