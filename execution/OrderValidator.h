#pragma once
#include <memory>
#include <string>
#include "Order.h"
#include "risk/RiskLimits.h"
#include "market/LiquidityEvaluator.h"

namespace hft {
namespace execution {

// 订单验证器
class OrderValidator {
public:
    OrderValidator();
    virtual ~OrderValidator() = default;

    // 设置风险限额
    virtual void setRiskLimits(const risk::RiskLimits& limits);

    // 验证订单
    virtual bool validateOrder(const Order& order);

    // 获取验证错误
    virtual std::string getLastError() const;

protected:
    risk::RiskLimits m_risk_limits;
    std::string m_last_error;

    // 检查订单大小
    virtual bool checkOrderSize(const Order& order);

    // 检查订单价格
    virtual bool checkOrderPrice(const Order& order);
};

// 高级订单验证器
class AdvancedOrderValidator : public OrderValidator {
public:
    AdvancedOrderValidator(const risk::RiskLimits& limits);
    ~AdvancedOrderValidator() override = default;

    // 设置流动性评估器
    void setLiquidityEvaluator(market::LiquidityEvaluatorPtr liquidity_evaluator);

    // 验证订单
    bool validateOrder(const Order& order) override;

private:
    market::LiquidityEvaluatorPtr m_liquidity_evaluator;

    // 检查订单频率
    bool checkOrderFrequency(const Order& order);

    // 检查流动性
    bool checkLiquidity(const Order& order);

    // 检查异常订单模式
    bool checkOrderPattern(const Order& order);
};

using OrderValidatorPtr = std::shared_ptr<OrderValidator>;

} // namespace execution
} // namespace hft