#pragma once
#include "RiskLimits.h"
#include "PositionMonitor.h"
#include "../core/EventLoop.h"
#include "../execution/OrderExecution.h"
#include <functional>
#include <vector>

namespace hft {
namespace risk {

enum class RiskLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

enum class RiskAction {
    NO_ACTION,
    WARNING,
    REDUCE_POSITION,
    CLOSE_POSITION,
    STOP_TRADING
};

struct RiskEvent {
    std::string eventId;
    RiskLevel level;
    std::string message;
    RiskAction recommendedAction;
    uint64_t timestamp;
};

typedef std::shared_ptr<RiskEvent> RiskEventPtr;

typedef std::function<void(const RiskEventPtr&)> RiskEventCallback;

typedef std::function<void(RiskAction)> RiskActionCallback;

class RiskManager {
public:
    RiskManager(EventLoop* eventLoop, PositionMonitor* positionMonitor,
               execution::OrderExecution* orderExecution);
    ~RiskManager();

    // 设置风险限额
    void setRiskLimits(const RiskLimitsPtr& limits);

    // 获取风险限额
    RiskLimitsPtr getRiskLimits() const;

    // 评估订单风险
    bool evaluateOrderRisk(const execution::OrderPtr& order);

    // 评估系统风险
    RiskLevel evaluateSystemRisk();

    // 注册风险事件回调
    void registerRiskEventCallback(const RiskEventCallback& callback);

    // 注册风险动作回调
    void registerRiskActionCallback(const RiskActionCallback& callback);

    // 启动风险监控
    void startMonitoring();

    // 停止风险监控
    void stopMonitoring();

private:
    // 执行风险动作
    void executeRiskAction(RiskAction action);

    // 生成风险事件
    void generateRiskEvent(RiskLevel level, const std::string& message, RiskAction action);

    // 定期检查风险
    void periodicRiskCheck();

    EventLoop* m_eventLoop;
    PositionMonitor* m_positionMonitor;
    execution::OrderExecution* m_orderExecution;
    RiskLimitsPtr m_riskLimits;
    RiskEventCallback m_riskEventCallback;
    RiskActionCallback m_riskActionCallback;
    bool m_monitoring;
    uint64_t m_lastCheckTime;
};

} // namespace risk
} // namespace hft
// risk/RiskManager.h 改进
class RiskManager {
public:
    // 添加实时风险计算
    RiskMetrics calculateRealTimeRisk();
    
    // 添加预警机制
    void setRiskAlert(const RiskLevel& level, std::function<void(const RiskEvent&)> callback);
    
    // 添加风险限额动态调整
    void adjustRiskLimits(const MarketCondition& condition);
    
private:
    std::unique_ptr<PositionMonitor> positionMonitor_;
    // ... 其他成员
};