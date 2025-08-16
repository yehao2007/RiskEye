#include "PerformanceMonitor.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iostream>
#include "core/TimeManager.h"

namespace hft {
namespace utils {

PerformanceMonitor::PerformanceMonitor() {
    // 初始化阈值
    m_thresholds = {
        {"order_execution_time", 100.0}, // 订单执行时间阈值 (ms)
        {"market_data_latency", 50.0},   // 市场数据延迟阈值 (ms)
        {"cpu_usage", 80.0},             // CPU使用率阈值 (%)
        {"memory_usage", 90.0}           // 内存使用率阈值 (%)
    };
}

void PerformanceMonitor::recordSample(const std::string& metric_name, PerformanceMetricType type, double value) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 记录样本
    m_metrics[metric_name].push_back({value, std::chrono::system_clock::now()});
    m_metric_types[metric_name] = type;

    // 保持样本数量在合理范围内
    if (m_metrics[metric_name].size() > 10000) {
        m_metrics[metric_name].erase(m_metrics[metric_name].begin(), m_metrics[metric_name].begin() + 5000);
    }
}

double PerformanceMonitor::getAverage(const std::string& metric_name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_metrics.find(metric_name);
    if (it == m_metrics.end() || it->second.empty()) {
        return 0.0;
    }

    double sum = std::accumulate(it->second.begin(), it->second.end(), 0.0, 
                                [](double acc, const PerformanceSample& sample) {
                                    return acc + sample.value;
                                });

    return sum / it->second.size();
}

double PerformanceMonitor::getMaximum(const std::string& metric_name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_metrics.find(metric_name);
    if (it == m_metrics.end() || it->second.empty()) {
        return 0.0;
    }

    auto max_it = std::max_element(it->second.begin(), it->second.end(), 
                                  [](const PerformanceSample& a, const PerformanceSample& b) {
                                      return a.value < b.value;
                                  });

    return max_it->value;
}

double PerformanceMonitor::getMinimum(const std::string& metric_name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_metrics.find(metric_name);
    if (it == m_metrics.end() || it->second.empty()) {
        return 0.0;
    }

    auto min_it = std::min_element(it->second.begin(), it->second.end(), 
                                  [](const PerformanceSample& a, const PerformanceSample& b) {
                                      return a.value < b.value;
                                  });

    return min_it->value;
}

double PerformanceMonitor::getPercentile(const std::string& metric_name, double percentile) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_metrics.find(metric_name);
    if (it == m_metrics.end() || it->second.empty()) {
        return 0.0;
    }

    // 复制并排序样本
    std::vector<double> values;
    values.reserve(it->second.size());
    for (const auto& sample : it->second) {
        values.push_back(sample.value);
    }

    std::sort(values.begin(), values.end());

    // 计算百分位数
    int index = static_cast<int>(values.size() * percentile / 100.0);
    index = std::max(0, std::min(index, static_cast<int>(values.size() - 1)));

    return values[index];
}

std::string PerformanceMonitor::generateReport() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::stringstream report;
    report << "========== Performance Report ==========\n";
    report << "Time: " << core::TimeManager::getCurrentTime() << "\n\n";

    for (const auto& [name, samples] : m_metrics) {
        if (samples.empty()) {
            continue;
        }

        report << "Metric: " << name << "\n";
        report << "  Type: ";
        switch (m_metric_types[name]) {
            case PerformanceMetricType::LATENCY:
                report << "Latency (ms)";
                break;
            case PerformanceMetricType::THROUGHPUT:
                report << "Throughput";
                break;
            case PerformanceMetricType::CPU_USAGE:
                report << "CPU Usage (%)";
                break;
            case PerformanceMetricType::MEMORY_USAGE:
                report << "Memory Usage (%)";
                break;
            case PerformanceMetricType::NETWORK_LATENCY:
                report << "Network Latency (ms)";
                break;
            case PerformanceMetricType::ORDER_EXECUTION_TIME:
                report << "Order Execution Time (ms)";
                break;
            default:
                report << "Unknown";
                break;
        }
        report << "\n";
        report << "  Samples: " << samples.size() << "\n";
        report << "  Average: " << getAverage(name) << "\n";
        report << "  Minimum: " << getMinimum(name) << "\n";
        report << "  Maximum: " << getMaximum(name) << "\n";
        report << "  95th Percentile: " << getPercentile(name, 95.0) << "\n";
        report << "  99th Percentile: " << getPercentile(name, 99.0) << "\n";

        // 检查阈值
        auto threshold_it = m_thresholds.find(name);
        if (threshold_it != m_thresholds.end()) {
            double threshold = threshold_it->second;
            double p95 = getPercentile(name, 95.0);
            report << "  Threshold: " << threshold << "\n";
            report << "  Status: " << (p95 <= threshold ? "OK" : "WARNING") << "\n";
        }

        report << "\n";
    }

    report << "======================================\n";
    return report.str();
}

bool PerformanceMonitor::checkThresholds() {
    std::lock_guard<std::mutex> lock(m_mutex);

    bool all_ok = true;

    for (const auto& [name, threshold] : m_thresholds) {
        auto it = m_metrics.find(name);
        if (it != m_metrics.end() && !it->second.empty()) {
            double p95 = getPercentile(name, 95.0);
            if (p95 > threshold) {
                std::cerr << "Performance warning: " << name << " 95th percentile (" << p95 
                          << ") exceeds threshold (" << threshold << ")" << std::endl;
                all_ok = false;
            }
        }
    }

    return all_ok;
}

void PerformanceMonitor::setThreshold(const std::string& metric_name, double threshold) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_thresholds[metric_name] = threshold;
}

} // namespace utils
} // namespace hft