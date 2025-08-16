#pragma once

#include "DiagnosticTool.h"
#include <fstream>
#include <sstream>

namespace hft {
namespace diagnostics {

// 告警配置
struct AlertConfig {
    enum class AlertMethod {
        Email,
        SMS,
        Webhook,
        Console,
        Log
    };

    struct AlertRule {
        std::string metric;              // 监控指标
        double threshold;                // 阈值
        Severity severity;               // 严重程度
        std::vector<AlertMethod> methods; // 告警方式
        std::string target;              // 告警目标地址
        bool enabled{true};              // 是否启用
        std::chrono::seconds cooldown{300}; // 告警冷却期
    };

    std::vector<AlertRule> rules;        // 告警规则
    std::string smtp_server;             // SMTP服务器
    std::string email_from;              // 发送邮箱
    std::string webhook_url;             // Webhook URL
    bool enable_aggregation{true};       // 是否启用告警聚合
    int max_alerts_per_minute{10};      // 每分钟最大告警数
};

// 告警管理器
class AlertManager {
public:
    AlertManager() = default;
    ~AlertManager() = default;

    // 配置管理
    void configure(const AlertConfig& config);
    void addRule(const AlertConfig::AlertRule& rule);
    void removeRule(const std::string& metric);
    void updateRule(const std::string& metric, const AlertConfig::AlertRule& rule);

    // 告警处理
    void processAlert(const DiagnosticIssue& issue, const SystemState& state);
    void sendAlert(const AlertConfig::AlertRule& rule, 
                   const DiagnosticIssue& issue,
                   const SystemState& state);

    // 状态查询
    std::vector<std::string> getActiveAlerts() const;
    std::map<std::string, int> getAlertCounts() const;
    void clearAlertHistory();

private:
    AlertConfig config_;
    std::map<std::string, std::chrono::system_clock::time_point> last_alert_times_;
    std::map<std::string, int> alert_counts_;
    std::vector<std::string> active_alerts_;
    mutable std::mutex alert_mutex_;

    // 内部方法
    bool shouldSendAlert(const AlertConfig::AlertRule& rule, 
                        const DiagnosticIssue& issue);
    void sendEmailAlert(const std::string& target, 
                       const DiagnosticIssue& issue,
                       const SystemState& state);
    void sendWebhookAlert(const std::string& target,
                         const DiagnosticIssue& issue,
                         const SystemState& state);
    void logAlert(const DiagnosticIssue& issue, const SystemState& state);
};

// 性能分析器
class PerformanceAnalyzer {
public:
    PerformanceAnalyzer() = default;
    ~PerformanceAnalyzer() = default;

    // 初始化
    void initialize();
    void shutdown();

    // 性能分析
    void analyzePerformance(const SystemState& state);
    PerformanceTrend::Statistics calculateStatistics(const std::vector<double>& data);
    
    // 趋势分析
    PerformanceTrend getPerformanceTrend(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const;
    
    // 瓶颈检测
    std::vector<std::string> detectBottlenecks(const SystemState& state);
    std::vector<std::string> predictBottlenecks(const PerformanceTrend& trend);
    
    // SLA检查
    std::map<std::string, double> checkSLACompliance(const PerformanceTrend& trend);

private:
    std::vector<SystemState> history_;
    std::mutex history_mutex_;
    size_t max_history_size_{10000};
    
    // 阈值配置
    struct Thresholds {
        double latency_warning{100.0};      // 延迟警告阈值(ms)
        double latency_critical{500.0};     // 延迟严重阈值(ms)
        double throughput_warning{1000.0};  // 吞吐量警告阈值
        double cpu_warning{80.0};           // CPU警告阈值(%)
        double memory_warning{85.0};        // 内存警告阈值(%)
    } thresholds_;
};

// 故障检测器
class FaultDetector {
public:
    FaultDetector() = default;
    ~FaultDetector() = default;

    // 初始化
    void initialize();
    void shutdown();

    // 故障检测
    std::vector<DiagnosticIssue> detectFaults(const SystemState& state);
    
    // 异常检测
    bool detectAnomaly(const std::string& metric, double value);
    void updateBaseline(const std::string& metric, double value);
    
    // 预测故障
    std::vector<DiagnosticIssue> predictFaults(const PerformanceTrend& trend);

private:
    // 基线数据
    std::map<std::string, std::vector<double>> baselines_;
    std::map<std::string, double> thresholds_;
    std::mutex baseline_mutex_;
    
    // 异常检测参数
    struct AnomalyConfig {
        double sensitivity{2.0};        // 敏感度(标准差倍数)
        size_t window_size{100};        // 滑动窗口大小
        double min_confidence{0.8};     // 最小置信度
    } anomaly_config_;
    
    // 内部方法
    double calculateZScore(const std::vector<double>& data, double value);
    bool isOutlier(double z_score);
};

// 日志分析器
class LogAnalyzer {
public:
    LogAnalyzer() = default;
    ~LogAnalyzer() = default;

    // 初始化
    void initialize();
    void shutdown();

    // 日志分析
    std::vector<DiagnosticIssue> analyzeLogs(const std::vector<std::string>& logs);
    std::vector<std::string> extractPatterns(const std::vector<std::string>& logs);
    
    // 错误分类
    std::map<std::string, int> categorizeErrors(const std::vector<std::string>& errors);
    
    // 趋势分析
    std::map<std::string, std::vector<int>> getErrorTrends();

private:
    // 错误模式
    std::vector<std::pair<std::string, std::string>> error_patterns_;
    std::map<std::string, std::vector<int>> error_history_;
    std::mutex analysis_mutex_;
    
    // 分析配置
    struct AnalysisConfig {
        size_t max_log_lines{10000};     // 最大日志行数
        int pattern_min_occurrences{5};  // 模式最小出现次数
        std::chrono::hours retention{24}; // 数据保留时间
    } config_;
    
    // 内部方法
    bool matchesPattern(const std::string& log, const std::string& pattern);
    Severity classifyError(const std::string& error);
};

// 系统监控器
class SystemMonitor {
public:
    SystemMonitor() = default;
    ~SystemMonitor() = default;

    // 初始化
    void initialize();
    void shutdown();
    
    // 系统监控
    SystemState collectSystemState();
    void startMonitoring();
    void stopMonitoring();
    
    // 资源监控
    double getCPUUsage();
    double getMemoryUsage();
    double getDiskUsage();
    double getNetworkUsage();
    
    // 硬件监控
    double getCPUTemperature();
    double getMemoryTemperature();
    std::vector<double> getDiskTemperatures();
    std::vector<double> getNetworkTemperatures();

private:
    std::thread monitoring_thread_;
    std::atomic<bool> monitoring_active_{false};
    SystemState current_state_;
    std::mutex state_mutex_;
    
    // 监控配置
    struct MonitorConfig {
        std::chrono::milliseconds poll_interval{1000}; // 轮询间隔
        bool enable_hardware_monitoring{true};         // 启用硬件监控
        bool enable_network_monitoring{true};          // 启用网络监控
    } config_;
    
    // 内部方法
    void monitoringLoop();
    void updatePerformanceMetrics();
    void updateResourceMetrics();
    void updateHardwareMetrics();
    void updateNetworkMetrics();
};

} // namespace diagnostics
} // namespace hft
