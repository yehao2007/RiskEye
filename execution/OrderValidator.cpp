#include "OrderValidator.h"
#include <iostream>
#include <chrono>
#include "utils/NumberUtils.h"

namespace hft {
namespace execution {

OrderValidator::OrderValidator() {
}

void OrderValidator::setRiskLimits(const risk::RiskLimits& limits) {
    m_risk_limits = limits;
}

bool OrderValidator::validateOrder(const Order& order) {
    // 检查订单大小
    if (!checkOrderSize(order)) {
        return false;
    }

    // 检查订单价格
    if (!checkOrderPrice(order)) {
        return false;
    }

    return true;
}

std::string OrderValidator::getLastError() const {
    return m_last_error;
}

bool OrderValidator::checkOrderSize(const Order& order) {
    if (order.size < m_risk_limits.min_order_size) {
        m_last_error = "Order size too small";
        return false;
    }

    if (order.size > m_risk_limits.max_order_size) {
        m_last_error = "Order size too large";
        return false;
    }

    return true;
}

bool OrderValidator::checkOrderPrice(const Order& order) {
    if (order.price <= 0) {
        m_last_error = "Invalid price";
        return false;
    }

    return true;
}

// AdvancedOrderValidator 实现
AdvancedOrderValidator::AdvancedOrderValidator(const risk::RiskLimits& limits) {
    m_risk_limits = limits;
}

void AdvancedOrderValidator::setLiquidityEvaluator(market::LiquidityEvaluatorPtr liquidity_evaluator) {
    m_liquidity_evaluator = liquidity_evaluator;
}

bool AdvancedOrderValidator::validateOrder(const Order& order) {
    // 基础验证
    if (!OrderValidator::validateOrder(order)) {
        return false;
    }

    // 检查订单频率
    if (!checkOrderFrequency(order)) {
        return false;
    }

    // 检查流动性
    if (!checkLiquidity(order)) {
        return false;
    }

    // 检查订单模式
    if (!checkOrderPattern(order)) {
        return false;
    }

    return true;
}

bool AdvancedOrderValidator::checkOrderFrequency(const Order& order) {
    // 实际应用中会跟踪订单频率
    // 这里简单实现
    static std::chrono::high_resolution_clock::time_point last_order_time;
    static uint32_t order_count = 0;

    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_order_time).count();

    order_count++;

    // 检查每秒订单数
    if (duration < 1000 && order_count > m_risk_limits.max_orders_per_second) {
        m_last_error = "Order frequency too high";
        return false;
    }

    // 每秒重置计数
    if (duration >= 1000) {
        last_order_time = now;
        order_count = 1;
    }

    return true;
}

bool AdvancedOrderValidator::checkLiquidity(const Order& order) {
    if (!m_liquidity_evaluator) {
        return true; // 没有流动性评估器，跳过检查
    }

    if (order.liquidity_score < m_risk_limits.min_liquidity_score) {
        m_last_error = "Insufficient liquidity for order";
        return false;
    }

    return true;
}

bool AdvancedOrderValidator::checkOrderPattern(const Order& order) {
    // 检测异常订单模式
    // 1. 检查价格是否偏离市场价格太多
    if (order.price_deviation > m_risk_limits.max_price_deviation) {
        m_last_error = "Order price deviates too much from market price";
        return false;
    }

    // 2. 检查是否存在翻转订单（快速买入卖出）
    // 实际应用中会跟踪订单历史

    return true;
}

} // namespace execution
} // namespace hft