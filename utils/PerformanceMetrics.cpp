#include "PerformanceMetrics.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace hft {
namespace utils {

PerformanceMetrics::PerformanceMetrics() {
}

void PerformanceMetrics::startTimer(const std::string& metric_name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& metric = m_metrics[metric_name];
    metric.start_time = std::chrono::high_resolution_clock::now();
}

double PerformanceMetrics::endTimer(const std::string& metric_name) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_metrics.find(metric_name) == m_metrics.end()) {
        throw std::runtime_error("Timer not started for metric: " + metric_name);
    }

    auto& metric = m_metrics[metric_name];
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - metric.start_time).count() / 1000.0; // 转换为微秒

    // 更新统计数据
    metric.sum += duration;
    metric.count++;
    if (duration < metric.min || metric.count == 1) {
        metric.min = duration;
    }
    if (duration > metric.max || metric.count == 1) {
        metric.max = duration;
    }

    return duration;
}

void PerformanceMetrics::recordValue(const std::string& metric_name, double value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& metric = m_metrics[metric_name];

    metric.sum += value;
    metric.count++;
    if (value < metric.min || metric.count == 1) {
        metric.min = value;
    }
    if (value > metric.max || metric.count == 1) {
        metric.max = value;
    }
}

double PerformanceMetrics::getAverage(const std::string& metric_name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_metrics.find(metric_name);

    if (it == m_metrics.end() || it->second.count == 0) {
        return 0.0;
    }

    return it->second.sum / it->second.count;
}

double PerformanceMetrics::getMax(const std::string& metric_name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_metrics.find(metric_name);

    if (it == m_metrics.end() || it->second.count == 0) {
        return 0.0;
    }

    return it->second.max;
}

double PerformanceMetrics::getMin(const std::string& metric_name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_metrics.find(metric_name);

    if (it == m_metrics.end() || it->second.count == 0) {
        return 0.0;
    }

    return it->second.min;
}

uint64_t PerformanceMetrics::getCount(const std::string& metric_name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_metrics.find(metric_name);

    if (it == m_metrics.end()) {
        return 0;
    }

    return it->second.count;
}

void PerformanceMetrics::resetMetric(const std::string& metric_name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_metrics.erase(metric_name);
}

void PerformanceMetrics::resetAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_metrics.clear();
}

void PerformanceMetrics::printMetrics() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << std::left << std::setw(30) << "Metric Name" 
              << std::right << std::setw(15) << "Average" 
              << std::setw(15) << "Min" 
              << std::setw(15) << "Max" 
              << std::setw(10) << "Count" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    for (const auto& [name, data] : m_metrics) {
        if (data.count > 0) {
            std::cout << std::left << std::setw(30) << name 
                      << std::right << std::setw(15) << std::fixed << std::setprecision(3) << (data.sum / data.count) 
                      << std::setw(15) << std::fixed << std::setprecision(3) << data.min 
                      << std::setw(15) << std::fixed << std::setprecision(3) << data.max 
                      << std::setw(10) << data.count << std::endl;
        }
    }
}

} // namespace utils
} // namespace hft