#include "AlertManager.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <unordered_map>
#include <mutex>

namespace hft {
namespace core {

// 初始化告警管理器
bool AlertManager::initialize() {
    try {
        rule_engine_ = std::make_unique<RuleEngine>();
        ml_model_ = std::make_unique<MLModel>();
        notification_manager_ = std::make_unique<NotificationManager>();

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
void AlertManager::createAlert(const Alert& alert) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 增加总告警计数
    stats_.total_alerts++;

    // 重复检测
    if (isDuplicate(alert)) {
        updateExistingAlert(alert);
        return;
    }
    
    // 智能分类
    Alert classified_alert = alert;
    classifyAlert(classified_alert);
    
    // 关联分析
    correlateAlerts(classified_alert);
    
    // 优先级评估
    assessPriority(classified_alert);
    
    // 检查是否可能是误报
    if (config_.enable_ml && ml_model_->isProbablyFalsePositive(classified_alert)) {
        stats_.false_positives++;
        Logger::info("Alert {} is likely a false positive", classified_alert.id);
        return;
    }
    
    // 添加到活动告警
    active_alerts_[classified_alert.id] = classified_alert;
    
    // 通知分发
    dispatchNotifications(classified_alert);
}

// 设置告警规则
void AlertManager::setAlertRule(const AlertRule& rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    rules_.push_back(rule);
    rule_engine_->addRule(rule);
}

// 确认告警
void AlertManager::acknowledgeAlert(const std::string& alert_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = active_alerts_.find(alert_id);
    if (it != active_alerts_.end()) {
        it->second.is_acknowledged = true;
        Logger::info("Alert {} acknowledged", alert_id);
    }
}

// 解决告警
void AlertManager::resolveAlert(const std::string& alert_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = active_alerts_.find(alert_id);
    if (it != active_alerts_.end()) {
        it->second.is_resolved = true;
        Logger::info("Alert {} resolved", alert_id);
    }
}

// 获取活动告警
std::vector<AlertManager::Alert> AlertManager::getActiveAlerts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Alert> result;
    for (const auto& pair : active_alerts_) {
        if (!pair.second.is_resolved) {
            result.push_back(pair.second);
        }
    }
    return result;
}

// 获取告警统计
AlertManager::AlertStats AlertManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    AlertStats stats;
    stats.total_alerts = stats_.total_alerts;
    stats.active_alerts = active_alerts_.size();
    stats.false_positive_rate = stats_.total_alerts > 0 ? 
        static_cast<double>(stats_.false_positives) / stats_.total_alerts : 0.0;

    // 按类型统计
    for (const auto& pair : active_alerts_) {
        if (!pair.second.is_resolved) {
            stats.by_type[pair.second.type]++;
            stats.by_severity[pair.second.severity]++;
        }
    }

    return stats;
}

// 初始化规则引擎
void AlertManager::initializeRuleEngine() {
    // 默认规则
    AlertRule critical_rule = {
        "critical_system_failure",
        "severity == CRITICAL",
        AlertType::SYSTEM_HEALTH,
        Severity::CRITICAL,
        std::chrono::seconds(10),
        [](const Alert&) { return false; }
    };
    rule_engine_->addRule(critical_rule);
}

// 启动告警处理
void AlertManager::startAlertProcessing() {
    // 启动后台线程处理告警
    processing_thread_ = std::thread([this]() {
        while (running_) {
            // 处理告警队列
            processAlertQueue();
            // 清理过期告警
            cleanupExpiredAlerts();
            // 短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

// 初始化机器学习模型
void AlertManager::initializeMLModel() {
    if (config_.enable_ml) {
        // 加载预训练模型
        ml_model_->train({}); // 空训练集，实际应用中应该加载历史数据
    }
}

// 检查是否重复告警
bool AlertManager::isDuplicate(const Alert& alert) {
    // 简单实现：检查相同类型和源的告警在短时间内是否出现
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    for (const auto& pair : active_alerts_) {
        const Alert& existing = pair.second;
        if (existing.type == alert.type && 
            existing.source == alert.source && 
            !existing.is_resolved && 
            (now_ms - existing.timestamp) < 60000) { // 60秒内
            return true;
        }
    }
    return false;
}

// 更新现有告警
void AlertManager::updateExistingAlert(const Alert& alert) {
    // 查找并更新现有告警
    for (auto& pair : active_alerts_) {
        Alert& existing = pair.second;
        if (existing.type == alert.type && 
            existing.source == alert.source && 
            !existing.is_resolved) {
            // 更新告警信息
            existing.message = alert.message;
            existing.timestamp = alert.timestamp;
            existing.labels.insert(alert.labels.begin(), alert.labels.end());
            Logger::info("Updated existing alert {}", existing.id);
            return;
        }
    }
}

// 分类告警
void AlertManager::classifyAlert(Alert& alert) {
    // 使用规则引擎和机器学习模型进行分类
    std::vector<AlertRule> matching_rules = rule_engine_->getMatchingRules(alert);
    if (!matching_rules.empty()) {
        alert.type = matching_rules[0].type;
        alert.severity = matching_rules[0].severity;
    } else if (config_.enable_ml) {
        // 使用机器学习模型预测严重程度
        double predicted_severity = ml_model_->predictSeverity(alert);
        if (predicted_severity > 0.7) {
            alert.severity = Severity::CRITICAL;
        } else if (predicted_severity > 0.5) {
            alert.severity = Severity::HIGH;
        } else if (predicted_severity > 0.3) {
            alert.severity = Severity::MEDIUM;
        } else {
            alert.severity = Severity::LOW;
        }
    }
}

// 关联分析
void AlertManager::correlateAlerts(Alert& alert) {
    if (config_.enable_ml) {
        alert.related_alerts = ml_model_->suggestRelatedAlerts(alert);
        stats_.correlations += alert.related_alerts.size();
    }
}

// 优先级评估
void AlertManager::assessPriority(Alert& alert) {
    // 根据严重程度和类型设置优先级
    // 实际应用中可以更复杂
}

// 通知分发
void AlertManager::dispatchNotifications(const Alert& alert) {
    notification_manager_->notify(alert);
}

// 处理告警队列
void AlertManager::processAlertQueue() {
    // 实际应用中应该有一个队列来处理告警
}

// 清理过期告警
void AlertManager::cleanupExpiredAlerts() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    for (auto it = active_alerts_.begin(); it != active_alerts_.end();) {
        if (it->second.is_resolved && 
            (now_ms - it->second.timestamp) > config_.cleanup_interval.count() * 1000) {
            it = active_alerts_.erase(it);
        } else {
            ++it;
        }
    }
}

// RuleEngine 实现
void AlertManager::RuleEngine::addRule(const AlertRule& rule) {
    rules_.push_back(rule);
}

bool AlertManager::RuleEngine::evaluateRule(const Alert& alert) {
    // 简化实现，实际应用中应该解析条件表达式
    return true;
}

std::vector<AlertManager::AlertRule> AlertManager::RuleEngine::getMatchingRules(const Alert& alert) {
    std::vector<AlertRule> result;
    for (const auto& rule : rules_) {
        if (evaluateRule(alert)) {
            result.push_back(rule);
        }
    }
    return result;
}

// MLModel 实现
void AlertManager::MLModel::train(const std::vector<Alert>& history) {
    training_data_ = history;
    // 实际应用中应该进行模型训练
}

double AlertManager::MLModel::predictSeverity(const Alert& alert) {
    // 简化实现，返回随机值
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen);
}

bool AlertManager::MLModel::isProbablyFalsePositive(const Alert& alert) {
    // 简化实现，基于规则判断
    if (alert.severity == Severity::LOW && alert.type == AlertType::PERFORMANCE) {
        return true;
    }
    return false;
}

std::vector<std::string> AlertManager::MLModel::suggestRelatedAlerts(const Alert& alert) {
    // 简化实现，返回空列表
    return {};
}

// NotificationManager 实现
void AlertManager::NotificationManager::notify(const Alert& alert) {
    // 简化实现，仅记录日志
    Logger::info("Sending notification for alert: {} (severity: {})");
}

void AlertManager::NotificationManager::addChannel(const std::string& channel) {
    // 实现添加通知渠道
}

void AlertManager::NotificationManager::setNotificationPolicy(const std::string& policy) {
    // 实现设置通知策略
}

} // namespace core
} // namespace hft