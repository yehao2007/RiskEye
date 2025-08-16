#pragma once

#include "DiagnosticPlugin.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace hft {
namespace diagnostics {

// 硬件健康状态
struct HardwareHealth {
    struct {
        double temperature;      // CPU温度
        double utilization;      // CPU使用率
        double frequency;        // CPU频率
        std::vector<double> core_loads;  // 各核心负载
    } cpu;

    struct {
        double temperature;      // 内存温度
        double usage;           // 内存使用率
        double bandwidth;       // 内存带宽
        double page_faults;     // 页面错误率
    } memory;

    struct {
        std::vector<double> temperatures;  // 磁盘温度
        std::vector<double> utilizations;  // 磁盘使用率
        std::vector<double> io_rates;      // IO速率
        std::vector<double> latencies;     // IO延迟
    } disk;

    struct {
        std::vector<double> temperatures;  // 网卡温度
        std::vector<double> bandwidths;    // 带宽使用率
        std::vector<double> error_rates;   // 错误率
        std::vector<double> packet_losses;  // 丢包率
    } network;
};

// 网络状态
struct NetworkState {
    struct {
        double latency;          // 网络延迟
        double bandwidth;        // 带宽使用
        double packet_loss;      // 丢包率
        double error_rate;       // 错误率
    } connection_quality;

    struct {
        int active_connections;  // 活动连接数
        int failed_connections;  // 失败连接数
        double retry_rate;       // 重试率
    } connectivity;

    struct {
        std::vector<std::string> blocked_ips;     // 被封禁IP
        std::vector<std::string> suspicious_ips;   // 可疑IP
    } security;
};

// 磁盘状态
struct DiskState {
    struct {
        std::vector<double> free_space;     // 剩余空间
        std::vector<double> write_speed;    // 写入速度
        std::vector<double> read_speed;     // 读取速度
        std::vector<double> iops;           // IOPS
    } performance;

    struct {
        std::vector<int> bad_sectors;       // 坏扇区数
        std::vector<double> temperatures;    // 温度
        std::vector<double> health_scores;   // 健康评分
    } health;
};

// 性能趋势数据
struct PerformanceTrend {
    std::vector<double> latencies;          // 延迟趋势
    std::vector<double> throughputs;        // 吞吐量趋势
    std::vector<double> success_rates;      // 成功率趋势
    std::vector<std::chrono::system_clock::time_point> timestamps;  // 时间戳
};

// 告警配置
struct AlertConfig {
    enum class Severity {
        Info,
        Warning,
        Error,
        Critical
    };

    enum class AlertMethod {
        Email,
        SMS,
        Webhook,
        Console
    };

    struct AlertRule {
        std::string metric;          // 监控指标
        double threshold;            // 阈值
        Severity severity;           // 严重程度
        std::vector<AlertMethod> methods;  // 告警方式
    };

    std::vector<AlertRule> rules;    // 告警规则
    std::string alert_endpoint;      // 告警接收端点
    bool enable_aggregation;         // 是否启用告警聚合
    int cooldown_period;             // 告警冷却期(秒)
};

// 远程监控配置
struct RemoteMonitoringConfig {
    std::string endpoint_url;        // 监控端点URL
    int report_interval;             // 上报间隔(秒)
    bool enable_compression;         // 是否启用压缩
    bool enable_encryption;          // 是否启用加密
    std::string auth_token;          // 认证令牌
};

// 自定义诊断规则基类
class CustomRule : public IDiagnosticRule {
public:
    CustomRule(const std::string& name, 
              const std::string& description,
              std::function<bool(const SystemState&)> evaluator)
        : name_(name)
        , description_(description)
        , evaluator_(evaluator) {}

    bool evaluate(const SystemState& state) override {
        return evaluator_(state);
    }

    std::string getRuleName() const override {
        return name_;
    }

    std::string getDescription() const override {
        return description_;
    }

private:
    std::string name_;
    std::string description_;
    std::function<bool(const SystemState&)> evaluator_;
};

// 扩展的诊断报告格式
struct EnhancedDiagnosticReport : public DiagnosticReport {
    HardwareHealth hardware_health;
    NetworkState network_state;
    DiskState disk_state;
    PerformanceTrend performance_trend;
    
    // 导出为JSON格式
    std::string toJson() const;
    
    // 导出为HTML格式
    std::string toHtml() const;
    
    // 导出为CSV格式
    std::string toCsv() const;
};

} // namespace diagnostics
} // namespace hft
