#include "RiskManager.h"
#include "../core/Logger.h"
#include <chrono>
#include <sstream>
#include <algorithm>

namespace hft {
namespace risk {

RiskManager::RiskManager(EventLoop* eventLoop, PositionMonitor* positionMonitor,
                       execution::OrderExecution* orderExecution)
    : m_eventLoop(eventLoop), m_positionMonitor(positionMonitor),
      m_orderExecution(orderExecution), m_monitoring(false), m_lastCheckTime(0) {
    m_riskLimits = std::make_shared<RiskLimits>();
}

RiskManager::~RiskManager() {
    stopMonitoring();
}

void RiskManager::setRiskLimits(const RiskLimitsPtr& limits) {
    if (limits) {
        m_riskLimits = limits;
    }
}

RiskLimitsPtr RiskManager::getRiskLimits() const {
    return m_riskLimits;
}

bool RiskManager::evaluateOrderRisk(const execution::OrderPtr& order) {
    if (!order || !m_riskLimits) {
        return true;
    }

    // 检查订单大小
    if (order->quantity > m_riskLimits->maxOrderSize) {
        generateRiskEvent(RiskLevel::HIGH,
                         "Order size exceeds maximum limit: " + std::to_string(order->quantity) + 
                         " > " + std::to_string(m_riskLimits->maxOrderSize),
                         RiskAction::STOP_TRADING);
        return false;
    }

    // 检查仓位限额
    PositionPtr position = m_positionMonitor->getPosition(order->symbol);
    int64_t currentPosition = (position) ? position->quantity : 0;
    int64_t newPosition = currentPosition + 
                         ((order->side == execution::OrderSide::BUY) ? 
                          static_cast<int64_t>(order->quantity) : 
                          -static_cast<int64_t>(order->quantity));

    // 检查单个品种持仓比例
    double totalPositionValue = m_positionMonitor->calculateTotalPositionValue();
    double newPositionValue = std::abs(static_cast<double>(newPosition) * order->price);

    if (totalPositionValue > 0 && newPositionValue / totalPositionValue > m_riskLimits->maxSinglePosition) {
        generateRiskEvent(RiskLevel::HIGH,
                         "Position concentration exceeds limit for symbol: " + order->symbol,
                         RiskAction::REDUCE_POSITION);
        return false;
    }

    // 检查总持仓价值
    if (newPositionValue + (totalPositionValue - 
            std::abs(static_cast<double>(currentPosition) * order->price)) > m_riskLimits->maxTotalValue) {
        generateRiskEvent(RiskLevel::HIGH,
                         "Total position value exceeds maximum limit",
                         RiskAction::REDUCE_POSITION);
        return false;
    }

    // 检查总持仓数量
    if (std::abs(newPosition) > static_cast<int64_t>(m_riskLimits->maxTotalPosition)) {
        generateRiskEvent(RiskLevel::HIGH,
                         "Total position quantity exceeds maximum limit",
                         RiskAction::REDUCE_POSITION);
        return false;
    }

    return true;
}

RiskLevel RiskManager::evaluateSystemRisk() {
    if (!m_riskLimits) {
        return RiskLevel::LOW;
    }

    // 检查总未实现亏损
    double totalUnrealizedPnl = m_positionMonitor->calculateTotalUnrealizedPnl();
    if (totalUnrealizedPnl < -m_riskLimits->maxDailyLoss) {
        generateRiskEvent(RiskLevel::CRITICAL,
                         "Daily loss exceeds limit: " + std::to_string(totalUnrealizedPnl) + 
                         " < -" + std::to_string(m_riskLimits->maxDailyLoss),
                         RiskAction::STOP_TRADING);
        return RiskLevel::CRITICAL;
    }

    // 检查总持仓价值
    double totalPositionValue = m_positionMonitor->calculateTotalPositionValue();
    if (totalPositionValue > m_riskLimits->maxTotalValue) {
        generateRiskEvent(RiskLevel::HIGH,
                         "Total position value exceeds limit: " + std::to_string(totalPositionValue) + 
                         " > " + std::to_string(m_riskLimits->maxTotalValue),
                         RiskAction::REDUCE_POSITION);
        return RiskLevel::HIGH;
    }

    // 检查是否有单一品种持仓比例过高
    auto allPositions = m_positionMonitor->getAllPositions();
    for (const auto& pair : allPositions) {
        PositionPtr position = pair.second;
        double positionValue = std::abs(static_cast<double>(position->quantity) * position->currentPrice);
        if (totalPositionValue > 0 && positionValue / totalPositionValue > m_riskLimits->maxSinglePosition) {
            generateRiskEvent(RiskLevel::MEDIUM,
                             "Position concentration exceeds limit for symbol: " + position->symbol,
                             RiskAction::REDUCE_POSITION);
            return RiskLevel::MEDIUM;
        }
    }

    return RiskLevel::LOW;
}

void RiskManager::registerRiskEventCallback(const RiskEventCallback& callback) {
    m_riskEventCallback = callback;
}

void RiskManager::registerRiskActionCallback(const RiskActionCallback& callback) {
    m_riskActionCallback = callback;
}

void RiskManager::startMonitoring() {
    if (m_monitoring) {
        return;
    }

    m_monitoring = true;
    m_lastCheckTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // 立即执行一次检查
    periodicRiskCheck();
}

void RiskManager::stopMonitoring() {
    m_monitoring = false;
}

void RiskManager::executeRiskAction(RiskAction action) {
    if (m_riskActionCallback) {
        m_riskActionCallback(action);
    }

    switch (action) {
        case RiskAction::NO_ACTION:
            break;
        case RiskAction::WARNING:
            // 记录警告日志
            Logger::info("Risk warning issued");
            break;
        case RiskAction::REDUCE_POSITION:
            // 这里应该实现减仓逻辑
            Logger::info("Reducing position due to risk");
            break;
        case RiskAction::CLOSE_POSITION:
            // 这里应该实现平仓逻辑
            Logger::info("Closing all positions due to risk");
            break;
        case RiskAction::STOP_TRADING:
            // 这里应该实现停止交易逻辑
            Logger::info("Stopping trading due to risk");
            break;
    }
}

void RiskManager::generateRiskEvent(RiskLevel level, const std::string& message, RiskAction action) {
    RiskEventPtr event = std::make_shared<RiskEvent>();
    event->eventId = "risk_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    event->level = level;
    event->message = message;
    event->recommendedAction = action;
    event->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // 记录风险事件
    switch (level) {
        case RiskLevel::LOW:
            Logger::info("Risk event (LOW): " + message);
            break;
        case RiskLevel::MEDIUM:
            Logger::warning("Risk event (MEDIUM): " + message);
            break;
        case RiskLevel::HIGH:
            Logger::error("Risk event (HIGH): " + message);
            break;
        case RiskLevel::CRITICAL:
            Logger::critical("Risk event (CRITICAL): " + message);
            break;
    }

    // 执行推荐动作
    executeRiskAction(action);

    // 通知回调
    if (m_riskEventCallback) {
        m_riskEventCallback(event);
    }
}

void RiskManager::periodicRiskCheck() {
    if (!m_monitoring) {
        return;
    }

    // 评估系统风险
    evaluateSystemRisk();

    // 安排下一次检查（100ms后）
    m_eventLoop->postDelayed([this]() {
        periodicRiskCheck();
    }, 100);
}

} // namespace risk
} // namespace hft