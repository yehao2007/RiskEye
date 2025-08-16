#include "DiagnosticCore.h"
#include "../core/Logger.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <regex>

namespace hft {
namespace diagnostics {

// AlertManager 实现
void AlertManager::configure(const AlertConfig& config) {
    std::lock_guard<std::mutex> lock(alert_mutex_);
    config_ = config;
    Logger::info("Alert manager configured with {} rules", config.rules.size());
}

void AlertManager::addRule(const AlertConfig::AlertRule& rule) {
    std::lock_guard<std::mutex> lock(alert_mutex_);
    
    // 检查是否已存在同名规则
    auto it = std::find_if(config_.rules.begin(), config_.rules.end(),
        [&rule](const AlertConfig::AlertRule& existing) {
            return existing.metric == rule.metric;
        });
    
    if (it != config_.rules.end()) {
        *it = rule;  // 更新现有规则
    } else {
        config_.rules.push_back(rule);  // 添加新规则
    }
    
    Logger::info("Alert rule added/updated for metric: {}", rule.metric);
}

void AlertManager::removeRule(const std::string& metric) {
    std::lock_guard<std::mutex> lock(alert_mutex_);
    
    config_.rules.erase(
        std::remove_if(config_.rules.begin(), config_.rules.end(),
            [&metric](const AlertConfig::AlertRule& rule) {
                return rule.metric == metric;
            }),
        config_.rules.end());
    
    Logger::info("Alert rule removed for metric: {}", metric);
}

void AlertManager::processAlert(const DiagnosticIssue& issue, const SystemState& state) {
    std::lock_guard<std::mutex> lock(alert_mutex_);
    
    // 查找匹配的告警规则
    for (const auto& rule : config_.rules) {
        if (!rule.enabled) continue;
        
        // 检查是否应该发送告警
        if (shouldSendAlert(rule, issue)) {
            sendAlert(rule, issue, state);
        }
    }
}

bool AlertManager::shouldSendAlert(const AlertConfig::AlertRule& rule, 
                                  const DiagnosticIssue& issue) {
    
    // 检查严重程度
    if (static_cast<int>(issue.severity) < static_cast<int>(rule.severity)) {
        return false;
    }
    
    // 检查冷却期
    auto now = std::chrono::system_clock::now();
    auto it = last_alert_times_.find(rule.metric);
    if (it != last_alert_times_.end()) {
        auto elapsed = now - it->second;
        if (elapsed < rule.cooldown) {
            return false;
        }
    }
    
    // 检查告警频率限制
    if (config_.enable_aggregation) {
        auto count_it = alert_counts_.find(rule.metric);
        if (count_it != alert_counts_.end() && 
            count_it->second >= config_.max_alerts_per_minute) {
            return false;
        }
    }
    
    return true;
}

void AlertManager::sendAlert(const AlertConfig::AlertRule& rule,
                            const DiagnosticIssue& issue,
                            const SystemState& state) {
    
    for (const auto& method : rule.methods) {
        try {
            switch (method) {
                case AlertConfig::AlertMethod::Email:
                    sendEmailAlert(rule.target, issue, state);
                    break;
                    
                case AlertConfig::AlertMethod::Webhook:
                    sendWebhookAlert(rule.target, issue, state);
                    break;
                    
                case AlertConfig::AlertMethod::Console:
                    Logger::warn("ALERT: {} - {}", issue.description, rule.metric);
                    break;
                    
                case AlertConfig::AlertMethod::Log:
                    logAlert(issue, state);
                    break;
                    
                default:
                    break;
            }
        } catch (const std::exception& e) {
            Logger::error("Failed to send alert via {}: {}", 
                         static_cast<int>(method), e.what());
        }
    }
    
    // 更新告警统计
    last_alert_times_[rule.metric] = std::chrono::system_clock::now();
    alert_counts_[rule.metric]++;
    active_alerts_.push_back(issue.id);
}

void AlertManager::sendEmailAlert(const std::string& target,
                                 const DiagnosticIssue& issue,
                                 const SystemState& state) {
    // 邮件发送实现（需要SMTP库支持）
    std::stringstream subject;
    subject << "[HFT Alert] " << issue.description;
    
    std::stringstream body;
    body << "Alert Details:\n"
         << "- Issue: " << issue.description << "\n"
         << "- Severity: " << static_cast<int>(issue.severity) << "\n"
         << "- Component: " << issue.component << "\n"
         << "- Time: " << std::chrono::system_clock::to_time_t(issue.timestamp) << "\n"
         << "\nSystem State:\n"
         << "- CPU Usage: " << state.resources.cpu_usage << "%\n"
         << "- Memory Usage: " << state.resources.memory_usage << "%\n"
         << "- Latency: " << state.performance.latency << "ms\n";
    
    Logger::info("Email alert sent to {} for issue: {}", target, issue.id);
}

void AlertManager::sendWebhookAlert(const std::string& target,
                                   const DiagnosticIssue& issue,
                                   const SystemState& state) {
    // Webhook发送实现（需要HTTP客户端库支持）
    Logger::info("Webhook alert sent to {} for issue: {}", target, issue.id);
}

void AlertManager::logAlert(const DiagnosticIssue& issue, const SystemState& state) {
    Logger::warn("Alert logged - Issue: {}, Severity: {}, Component: {}",
                issue.description, static_cast<int>(issue.severity), issue.component);
}

// PerformanceAnalyzer 实现
void PerformanceAnalyzer::initialize() {
    history_.reserve(max_history_size_);
    Logger::info("Performance analyzer initialized");
}

void PerformanceAnalyzer::shutdown() {
    std::lock_guard<std::mutex> lock(history_mutex_);
    history_.clear();
    Logger::info("Performance analyzer shut down");
}

void PerformanceAnalyzer::analyzePerformance(const SystemState& state) {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    // 添加到历史记录
    history_.push_back(state);
    
    // 限制历史记录大小
    if (history_.size() > max_history_size_) {
        history_.erase(history_.begin(), 
                      history_.begin() + (history_.size() - max_history_size_));
    }
}

PerformanceTrend::Statistics PerformanceAnalyzer::calculateStatistics(
    const std::vector<double>& data) {
    
    if (data.empty()) {
        return {};
    }
    
    PerformanceTrend::Statistics stats;
    
    // 计算基本统计量
    stats.min = *std::min_element(data.begin(), data.end());
    stats.max = *std::max_element(data.begin(), data.end());
    stats.mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    
    // 计算标准差
    double variance = 0.0;
    for (double value : data) {
        variance += std::pow(value - stats.mean, 2);
    }
    stats.std_dev = std::sqrt(variance / data.size());
    
    // 计算百分位数
    std::vector<double> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());
    
    size_t p95_index = static_cast<size_t>(0.95 * sorted_data.size());
    size_t p99_index = static_cast<size_t>(0.99 * sorted_data.size());
    
    stats.percentile_95 = sorted_data[std::min(p95_index, sorted_data.size() - 1)];
    stats.percentile_99 = sorted_data[std::min(p99_index, sorted_data.size() - 1)];
    
    return stats;
}

PerformanceTrend PerformanceAnalyzer::getPerformanceTrend(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) const {
    
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    PerformanceTrend trend;
    
    // 筛选时间范围内的数据
    for (const auto& state : history_) {
        if (state.timestamp >= start && state.timestamp <= end) {
            trend.latencies.push_back(state.performance.latency);
            trend.throughputs.push_back(state.performance.throughput);
            trend.success_rates.push_back(state.performance.success_rate);
            trend.cpu_usages.push_back(state.resources.cpu_usage);
            trend.memory_usages.push_back(state.resources.memory_usage);
            trend.timestamps.push_back(state.timestamp);
        }
    }
    
    // 计算统计信息
    trend.latency_stats = calculateStatistics(trend.latencies);
    trend.throughput_stats = calculateStatistics(trend.throughputs);
    trend.cpu_stats = calculateStatistics(trend.cpu_usages);
    trend.memory_stats = calculateStatistics(trend.memory_usages);
    
    return trend;
}

std::vector<std::string> PerformanceAnalyzer::detectBottlenecks(const SystemState& state) {
    std::vector<std::string> bottlenecks;
    
    // CPU瓶颈检测
    if (state.resources.cpu_usage > thresholds_.cpu_warning) {
        bottlenecks.push_back("High CPU usage: " + 
                             std::to_string(state.resources.cpu_usage) + "%");
    }
    
    // 内存瓶颈检测
    if (state.resources.memory_usage > thresholds_.memory_warning) {
        bottlenecks.push_back("High memory usage: " + 
                             std::to_string(state.resources.memory_usage) + "%");
    }
    
    // 延迟瓶颈检测
    if (state.performance.latency > thresholds_.latency_warning) {
        bottlenecks.push_back("High latency: " + 
                             std::to_string(state.performance.latency) + "ms");
    }
    
    // 吞吐量瓶颈检测
    if (state.performance.throughput < thresholds_.throughput_warning) {
        bottlenecks.push_back("Low throughput: " + 
                             std::to_string(state.performance.throughput) + " ops/sec");
    }
    
    return bottlenecks;
}

// FaultDetector 实现
void FaultDetector::initialize() {
    // 初始化异常检测参数
    Logger::info("Fault detector initialized");
}

void FaultDetector::shutdown() {
    std::lock_guard<std::mutex> lock(baseline_mutex_);
    baselines_.clear();
    Logger::info("Fault detector shut down");
}

std::vector<DiagnosticIssue> FaultDetector::detectFaults(const SystemState& state) {
    std::vector<DiagnosticIssue> issues;
    
    // 检测CPU异常
    if (detectAnomaly("cpu_usage", state.resources.cpu_usage)) {
        DiagnosticIssue issue;
        issue.id = "cpu_anomaly_" + std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        issue.description = "CPU usage anomaly detected";
        issue.severity = Severity::Warning;
        issue.type = RuleType::Hardware;
        issue.component = "CPU";
        issue.timestamp = std::chrono::system_clock::now();
        issues.push_back(issue);
    }
    
    // 检测内存异常
    if (detectAnomaly("memory_usage", state.resources.memory_usage)) {
        DiagnosticIssue issue;
        issue.id = "memory_anomaly_" + std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        issue.description = "Memory usage anomaly detected";
        issue.severity = Severity::Warning;
        issue.type = RuleType::Hardware;
        issue.component = "Memory";
        issue.timestamp = std::chrono::system_clock::now();
        issues.push_back(issue);
    }
    
    // 检测延迟异常
    if (detectAnomaly("latency", state.performance.latency)) {
        DiagnosticIssue issue;
        issue.id = "latency_anomaly_" + std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        issue.description = "Latency anomaly detected";
        issue.severity = Severity::Error;
        issue.type = RuleType::Performance;
        issue.component = "Network";
        issue.timestamp = std::chrono::system_clock::now();
        issues.push_back(issue);
    }
    
    return issues;
}

bool FaultDetector::detectAnomaly(const std::string& metric, double value) {
    std::lock_guard<std::mutex> lock(baseline_mutex_);
    
    auto it = baselines_.find(metric);
    if (it == baselines_.end() || it->second.size() < anomaly_config_.window_size) {
        // 基线数据不足，更新基线
        updateBaseline(metric, value);
        return false;
    }
    
    // 计算Z分数
    double z_score = calculateZScore(it->second, value);
    
    // 判断是否为异常
    bool is_anomaly = isOutlier(z_score);
    
    // 更新基线
    updateBaseline(metric, value);
    
    return is_anomaly;
}

void FaultDetector::updateBaseline(const std::string& metric, double value) {
    auto& baseline = baselines_[metric];
    baseline.push_back(value);
    
    // 保持滑动窗口大小
    if (baseline.size() > anomaly_config_.window_size) {
        baseline.erase(baseline.begin());
    }
}

double FaultDetector::calculateZScore(const std::vector<double>& data, double value) {
    if (data.empty()) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    
    double variance = 0.0;
    for (double v : data) {
        variance += std::pow(v - mean, 2);
    }
    double std_dev = std::sqrt(variance / data.size());
    
    if (std_dev == 0.0) return 0.0;
    
    return (value - mean) / std_dev;
}

bool FaultDetector::isOutlier(double z_score) {
    return std::abs(z_score) > anomaly_config_.sensitivity;
}

// LogAnalyzer 实现
void LogAnalyzer::initialize() {
    // 初始化错误模式
    error_patterns_ = {
        {"connection_timeout", R"(connection.*timeout)"},
        {"memory_error", R"(out of memory|memory.*error)"},
        {"network_error", R"(network.*error|connection.*failed)"},
        {"disk_error", R"(disk.*error|io.*error)"},
        {"authentication_error", R"(auth.*failed|permission.*denied)"}
    };
    
    Logger::info("Log analyzer initialized with {} patterns", error_patterns_.size());
}

void LogAnalyzer::shutdown() {
    std::lock_guard<std::mutex> lock(analysis_mutex_);
    error_history_.clear();
    Logger::info("Log analyzer shut down");
}

std::vector<DiagnosticIssue> LogAnalyzer::analyzeLogs(
    const std::vector<std::string>& logs) {
    
    std::vector<DiagnosticIssue> issues;
    std::map<std::string, int> pattern_counts;
    
    // 分析日志模式
    for (const auto& log : logs) {
        for (const auto& [pattern_name, pattern_regex] : error_patterns_) {
            if (matchesPattern(log, pattern_regex)) {
                pattern_counts[pattern_name]++;
            }
        }
    }
    
    // 生成诊断问题
    for (const auto& [pattern, count] : pattern_counts) {
        if (count >= config_.pattern_min_occurrences) {
            DiagnosticIssue issue;
            issue.id = pattern + "_" + std::to_string(
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
            issue.description = "Pattern detected: " + pattern + 
                               " (occurrences: " + std::to_string(count) + ")";
            issue.severity = classifyError(pattern);
            issue.type = RuleType::Custom;
            issue.component = "Logs";
            issue.timestamp = std::chrono::system_clock::now();
            issue.metadata["pattern"] = pattern;
            issue.metadata["count"] = std::to_string(count);
            issues.push_back(issue);
        }
    }
    
    return issues;
}

bool LogAnalyzer::matchesPattern(const std::string& log, const std::string& pattern) {
    try {
        std::regex regex_pattern(pattern, std::regex_constants::icase);
        return std::regex_search(log, regex_pattern);
    } catch (const std::exception& e) {
        Logger::error("Error matching pattern {}: {}", pattern, e.what());
        return false;
    }
}

Severity LogAnalyzer::classifyError(const std::string& error) {
    if (error.find("memory") != std::string::npos ||
        error.find("disk") != std::string::npos) {
        return Severity::Critical;
    } else if (error.find("network") != std::string::npos ||
               error.find("connection") != std::string::npos) {
        return Severity::Error;
    } else if (error.find("auth") != std::string::npos) {
        return Severity::Warning;
    }
    return Severity::Info;
}

} // namespace diagnostics
} // namespace hft
