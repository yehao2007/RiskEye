#include "PerformanceMonitor.h"
#include "../core/Logger.h"
#include "../core/TimeManager.h"
#include <algorithm>
#include <numeric>

namespace hft {
namespace monitoring {

void PerformanceMonitor::initialize() {
    // 初始化性能指标
    initializeMetrics();
    
    // 设置默认参数
    params_.sampling_interval = std::chrono::microseconds(100);  // 100微秒采样
    params_.metrics_window = std::chrono::seconds(60);          // 1分钟统计窗口
    params_.alert_threshold = 0.9;                              // 90%阈值告警
    params_.critical_threshold = 0.95;                          // 95%阈值严重告警
}

void PerformanceMonitor::startMonitoring() {
    is_running_ = true;
    
    // 启动监控线程
    monitor_thread_ = std::thread([this] {
        while (is_running_) {
            try {
                // 1. 采集性能指标
                auto metrics = collectMetrics();
                
                // 2. 分析性能数据
                analyzePerformance(metrics);
                
                // 3. 检查告警条件
                checkAlerts(metrics);
                
                // 4. 更新统计信息
                updateStatistics(metrics);
                
                // 等待下一个采样周期
                std::this_thread::sleep_for(params_.sampling_interval);
                
            } catch (const std::exception& e) {
                Logger::error("Performance monitoring error: {}", e.what());
            }
        }
    });
}

void PerformanceMonitor::stopMonitoring() {
    is_running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

PerformanceMetrics PerformanceMonitor::collectMetrics() {
    PerformanceMetrics metrics;
    
    // 1. CPU使用率
    metrics.cpu_usage = measureCPUUsage();
    
    // 2. 内存使用情况
    metrics.memory_usage = measureMemoryUsage();
    
    // 3. 网络延迟
    metrics.network_latency = measureNetworkLatency();
    
    // 4. 系统吞吐量
    metrics.throughput = measureThroughput();
    
    // 5. 队列长度
    metrics.queue_lengths = measureQueueLengths();
    
    return metrics;
}

void PerformanceMonitor::analyzePerformance(
    const PerformanceMetrics& metrics) {
    
    // 1. 分析CPU使用率
    analyzeCPUUsage(metrics.cpu_usage);
    
    // 2. 分析内存使用情况
    analyzeMemoryUsage(metrics.memory_usage);
    
    // 3. 分析网络性能
    analyzeNetworkPerformance(metrics.network_latency);
    
    // 4. 分析系统吞吐量
    analyzeThroughput(metrics.throughput);
    
    // 5. 分析队列状态
    analyzeQueueState(metrics.queue_lengths);
}

void PerformanceMonitor::checkAlerts(
    const PerformanceMetrics& metrics) {
    
    // 1. 检查CPU告警
    if (metrics.cpu_usage.utilization > params_.critical_threshold) {
        raiseAlert(AlertType::CPU_CRITICAL, metrics.cpu_usage);
    } else if (metrics.cpu_usage.utilization > params_.alert_threshold) {
        raiseAlert(AlertType::CPU_WARNING, metrics.cpu_usage);
    }
    
    // 2. 检查内存告警
    if (metrics.memory_usage.utilization > params_.critical_threshold) {
        raiseAlert(AlertType::MEMORY_CRITICAL, metrics.memory_usage);
    } else if (metrics.memory_usage.utilization > params_.alert_threshold) {
        raiseAlert(AlertType::MEMORY_WARNING, metrics.memory_usage);
    }
    
    // 3. 检查网络告警
    if (metrics.network_latency.average > network_latency_threshold_) {
        raiseAlert(AlertType::NETWORK_LATENCY, metrics.network_latency);
    }
    
    // 4. 检查吞吐量告警
    if (metrics.throughput.current < throughput_threshold_) {
        raiseAlert(AlertType::LOW_THROUGHPUT, metrics.throughput);
    }
}

void PerformanceMonitor::updateStatistics(
    const PerformanceMetrics& metrics) {
    
    // 1. 更新时间序列数据
    updateTimeSeriesData(metrics);
    
    // 2. 计算统计指标
    calculateStatistics();
    
    // 3. 更新性能报告
    updatePerformanceReport();
    
    // 4. 清理过期数据
    cleanupOldData();
}

void PerformanceMonitor::raiseAlert(
    AlertType type,
    const MetricValue& value) {
    
    Alert alert{
        .type = type,
        .severity = calculateAlertSeverity(type, value),
        .timestamp = TimeManager::getInstance().getCurrentTime(),
        .metric_value = value
    };
    
    // 发送告警
    for (const auto& handler : alert_handlers_) {
        handler(alert);
    }
    
    // 记录告警历史
    alert_history_.push_back(alert);
    
    // 清理过期告警历史
    cleanupAlertHistory();
}

PerformanceReport PerformanceMonitor::generateReport(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) {
    
    PerformanceReport report;
    
    // 1. 汇总性能指标
    report.metrics = aggregateMetrics(start, end);
    
    // 2. 计算统计信息
    report.statistics = calculateReportStatistics(start, end);
    
    // 3. 收集告警信息
    report.alerts = collectAlerts(start, end);
    
    // 4. 生成性能分析
    report.analysis = analyzePerformanceTrends(start, end);
    
    return report;
}

void PerformanceMonitor::registerAlertHandler(
    AlertHandler handler) {
    
    alert_handlers_.push_back(std::move(handler));
}

std::vector<Alert> PerformanceMonitor::getAlertHistory(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) const {
    
    std::vector<Alert> filtered_alerts;
    
    // 过滤指定时间范围内的告警
    std::copy_if(alert_history_.begin(), alert_history_.end(),
                 std::back_inserter(filtered_alerts),
                 [&](const Alert& alert) {
                     return alert.timestamp >= start && 
                            alert.timestamp <= end;
                 });
    
    return filtered_alerts;
}

void PerformanceMonitor::setThresholds(
    const PerformanceThresholds& thresholds) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    thresholds_ = thresholds;
}

} // namespace monitoring
} // namespace hft
