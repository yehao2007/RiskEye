#include "RiskAlertSystem.h"
#include "../core/Logger.h"
#include "../core/TimeManager.h"
#include "../risk/RiskManager.h"
#include <algorithm>

namespace hft {
namespace risk {

void RiskAlertSystem::initialize() {
    // 初始化风险监控器
    initializeMonitors();
    
    // 设置默认参数
    params_.check_interval = std::chrono::milliseconds(100);  // 100ms检查间隔
    params_.alert_window = std::chrono::minutes(5);          // 5分钟告警窗口
    params_.max_alerts_per_window = 10;                      // 每窗口最大告警数
    params_.correlation_threshold = 0.7;                     // 相关性阈值
}

void RiskAlertSystem::startMonitoring() {
    is_running_ = true;
    
    // 启动监控线程
    monitor_thread_ = std::thread([this] {
        while (is_running_) {
            try {
                // 1. 收集风险指标
                auto metrics = collectRiskMetrics();
                
                // 2. 分析风险状态
                analyzeRiskStatus(metrics);
                
                // 3. 检查风险限额
                checkRiskLimits(metrics);
                
                // 4. 更新风险统计
                updateRiskStatistics(metrics);
                
                // 等待下一个检查周期
                std::this_thread::sleep_for(params_.check_interval);
                
            } catch (const std::exception& e) {
                Logger::error("Risk monitoring error: {}", e.what());
            }
        }
    });
}

void RiskAlertSystem::stopMonitoring() {
    is_running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

RiskMetrics RiskAlertSystem::collectRiskMetrics() {
    RiskMetrics metrics;
    
    // 1. 市场风险指标
    metrics.market_risk = calculateMarketRisk();
    
    // 2. 信用风险指标
    metrics.credit_risk = calculateCreditRisk();
    
    // 3. 流动性风险指标
    metrics.liquidity_risk = calculateLiquidityRisk();
    
    // 4. 操作风险指标
    metrics.operational_risk = calculateOperationalRisk();
    
    // 5. 系统风险指标
    metrics.systemic_risk = calculateSystemicRisk();
    
    return metrics;
}

void RiskAlertSystem::analyzeRiskStatus(
    const RiskMetrics& metrics) {
    
    // 1. 分析市场风险
    analyzeMarketRisk(metrics.market_risk);
    
    // 2. 分析信用风险
    analyzeCreditRisk(metrics.credit_risk);
    
    // 3. 分析流动性风险
    analyzeLiquidityRisk(metrics.liquidity_risk);
    
    // 4. 分析操作风险
    analyzeOperationalRisk(metrics.operational_risk);
    
    // 5. 分析系统风险
    analyzeSystemicRisk(metrics.systemic_risk);
}

void RiskAlertSystem::checkRiskLimits(
    const RiskMetrics& metrics) {
    
    // 1. 检查VaR限额
    checkVaRLimits(metrics.market_risk.var);
    
    // 2. 检查敞口限额
    checkExposureLimits(metrics.credit_risk.exposures);
    
    // 3. 检查流动性限额
    checkLiquidityLimits(metrics.liquidity_risk);
    
    // 4. 检查操作限额
    checkOperationalLimits(metrics.operational_risk);
    
    // 5. 检查系统风险限额
    checkSystemicRiskLimits(metrics.systemic_risk);
}

void RiskAlertSystem::raiseAlert(
    RiskAlertType type,
    const RiskMetricValue& value) {
    
    RiskAlert alert{
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
    
    // 触发风险控制措施
    triggerRiskControl(alert);
    
    // 清理过期告警历史
    cleanupAlertHistory();
}

void RiskAlertSystem::triggerRiskControl(
    const RiskAlert& alert) {
    
    // 根据告警类型和严重程度采取相应措施
    switch (alert.type) {
        case RiskAlertType::VAR_BREACH:
            handleVaRBreach(alert);
            break;
            
        case RiskAlertType::EXPOSURE_BREACH:
            handleExposureBreach(alert);
            break;
            
        case RiskAlertType::LIQUIDITY_SHORTAGE:
            handleLiquidityShortage(alert);
            break;
            
        case RiskAlertType::OPERATIONAL_FAILURE:
            handleOperationalFailure(alert);
            break;
            
        case RiskAlertType::SYSTEMIC_RISK:
            handleSystemicRisk(alert);
            break;
    }
}

void RiskAlertSystem::handleVaRBreach(const RiskAlert& alert) {
    // 1. 降低仓位
    reducePositions(alert.metric_value);
    
    // 2. 调整风险限额
    adjustRiskLimits(alert.type);
    
    // 3. 通知风险管理员
    notifyRiskManagers(alert);
}

void RiskAlertSystem::handleExposureBreach(const RiskAlert& alert) {
    // 1. 冻结相关交易
    freezeTrading(alert.metric_value);
    
    // 2. 启动对手方审查
    reviewCounterparty(alert.metric_value);
    
    // 3. 准备清算计划
    prepareLiquidationPlan(alert.metric_value);
}

void RiskAlertSystem::handleLiquidityShortage(const RiskAlert& alert) {
    // 1. 激活流动性储备
    activateLiquidityReserves(alert.metric_value);
    
    // 2. 暂停新开仓
    suspendNewPositions();
    
    // 3. 启动紧急筹资程序
    initiateEmergencyFunding();
}

void RiskAlertSystem::handleOperationalFailure(const RiskAlert& alert) {
    // 1. 启动备份系统
    activateBackupSystems(alert.metric_value);
    
    // 2. 隔离故障组件
    isolateFailedComponents(alert.metric_value);
    
    // 3. 启动恢复程序
    initiateRecoveryProcedure(alert.metric_value);
}

void RiskAlertSystem::handleSystemicRisk(const RiskAlert& alert) {
    // 1. 全面降低风险敞口
    reduceSystemicExposure(alert.metric_value);
    
    // 2. 增加系统冗余
    increaseSystemRedundancy();
    
    // 3. 准备市场退出计划
    prepareMarketExitPlan();
}

RiskReport RiskAlertSystem::generateReport(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) {
    
    RiskReport report;
    
    // 1. 汇总风险指标
    report.metrics = aggregateRiskMetrics(start, end);
    
    // 2. 计算统计信息
    report.statistics = calculateRiskStatistics(start, end);
    
    // 3. 收集告警信息
    report.alerts = collectRiskAlerts(start, end);
    
    // 4. 生成风险分析
    report.analysis = analyzeRiskTrends(start, end);
    
    return report;
}

void RiskAlertSystem::registerAlertHandler(
    RiskAlertHandler handler) {
    
    alert_handlers_.push_back(std::move(handler));
}

void RiskAlertSystem::setRiskLimits(
    const RiskLimits& limits) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    risk_limits_ = limits;
}

} // namespace risk
} // namespace hft
