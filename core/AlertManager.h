#pragma once

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include "../core/Logger.h"

namespace hft {
namespace core {

class AlertManager {
public:
    // 告警级别
    enum class Severity {
        CRITICAL,
        HIGH,
        MEDIUM,
        LOW,
        INFO
    };

    // 告警类型
    enum class AlertType {
        SYSTEM_HEALTH,     // 系统健康
        PERFORMANCE,       // 性能问题
        SECURITY,         // 安全问题
        RESOURCE,         // 资源问题
        BUSINESS,         // 业务问题
        NETWORK,          // 网络问题
        DATABASE          // 数据库问题
    };

    // 告警信息
    struct Alert {
        std::string id;
        AlertType type;
        Severity severity;
        std::string source;
        std::string message;
        uint64_t timestamp;
        std::unordered_map<std::string, std::string> labels;
        std::vector<std::string> related_alerts;
        bool is_acknowledged;
        bool is_resolved;
    };

    // 告警规则
    struct AlertRule {
        std::string name;
        std::string condition;
        AlertType type;
        Severity severity;
        std::chrono::seconds evaluation_interval;
        std::function<bool(const Alert&)> suppression_fn;
    };

    // 初始化告警管理器
    bool initialize() {
        try {
            // 初始化规则引擎
            initializeRuleEngine();
            
            // 启动告警处理
            startAlertProcessing();
            
            // 初始化机器学习模型
            initializeMLModel();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Alert manager initialization failed: {}", e.what());
            return false;
        }
    }

    // 创建告警
    void createAlert(const Alert& alert) {
        // 重复检测
        if (isDuplicate(alert)) {
            updateExistingAlert(alert);
            return;
        }
        
        // 智能分类
        classifyAlert(alert);
        
        // 关联分析
        correlateAlerts(alert);
        
        // 优先级评估
        assessPriority(alert);
        
        // 通知分发
        dispatchNotifications(alert);
    }

    // 设置告警规则
    void setAlertRule(const AlertRule& rule) {
        rules_.push_back(rule);
    }

    // 确认告警
    void acknowledgeAlert(const std::string& alert_id);
    
    // 解决告警
    void resolveAlert(const std::string& alert_id);
    
    // 获取活动告警
    std::vector<Alert> getActiveAlerts() const;
    
    // 获取告警统计
    struct AlertStats {
        std::unordered_map<AlertType, uint64_t> by_type;
        std::unordered_map<Severity, uint64_t> by_severity;
        uint64_t total_alerts;
        uint64_t active_alerts;
        double false_positive_rate;
    };
    AlertStats getStats() const;

private:
    // 规则引擎
    class RuleEngine {
    public:
        void addRule(const AlertRule& rule);
        bool evaluateRule(const Alert& alert);
        std::vector<AlertRule> getMatchingRules(const Alert& alert);
        
    private:
        std::vector<AlertRule> rules_;
    };

    // 机器学习模型
    class MLModel {
    public:
        void train(const std::vector<Alert>& history);
        double predictSeverity(const Alert& alert);
        bool isProbablyFalsePositive(const Alert& alert);
        std::vector<std::string> suggestRelatedAlerts(const Alert& alert);
        
    private:
        std::vector<Alert> training_data_;
    };

    // 通知管理器
    class NotificationManager {
    public:
        void notify(const Alert& alert);
        void addChannel(const std::string& channel);
        void setNotificationPolicy(const std::string& policy);
        
    private:
        struct NotificationChannel {
            std::string type;
            std::string endpoint;
            Severity min_severity;
        };
        std::vector<NotificationChannel> channels_;
    };

private:
    // 告警处理方法
    bool isDuplicate(const Alert& alert);
    void updateExistingAlert(const Alert& alert);
    void classifyAlert(const Alert& alert);
    void correlateAlerts(const Alert& alert);
    void assessPriority(const Alert& alert);
    void dispatchNotifications(const Alert& alert);

private:
    std::unique_ptr<RuleEngine> rule_engine_;
    std::unique_ptr<MLModel> ml_model_;
    std::unique_ptr<NotificationManager> notification_manager_;
    
    std::vector<AlertRule> rules_;
    std::unordered_map<std::string, Alert> active_alerts_;
    
    // 统计数据
    struct Stats {
        std::atomic<uint64_t> total_alerts{0};
        std::atomic<uint64_t> false_positives{0};
        std::atomic<uint64_t> correlations{0};
    } stats_;
    
    // 配置参数
    struct Config {
        bool enable_ml{true};
        size_t correlation_window_size{1000};
        std::chrono::seconds cleanup_interval{3600};
        double false_positive_threshold{0.8};
    } config_;
};

} // namespace core
} // namespace hft
