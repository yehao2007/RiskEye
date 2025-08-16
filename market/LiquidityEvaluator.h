#pragma once
#include <vector>
#include <cstdint>
#include <memory>
#include "OrderBook.h"

namespace hft {
namespace market {

// 流动性评估器
class LiquidityEvaluator {
public:
    LiquidityEvaluator();
    virtual ~LiquidityEvaluator() = default;

    // 初始化
    virtual void initialize(uint32_t depth_levels);

    // 评估订单簿流动性
    virtual double evaluateLiquidity(const OrderBook& order_book);

    // 获取买卖价差
    virtual double getBidAskSpread(const OrderBook& order_book);

    // 获取市场深度
    virtual double getMarketDepth(const OrderBook& order_book, uint32_t levels);

    // 获取流动性分数
    virtual double getLiquidityScore() const;

protected:
    uint32_t m_depth_levels;
    double m_liquidity_score;
};

// 高级流动性评估器
class AdvancedLiquidityEvaluator : public LiquidityEvaluator {
public:
    AdvancedLiquidityEvaluator();
    ~AdvancedLiquidityEvaluator() override = default;

    // 初始化
    void initialize(uint32_t depth_levels) override;

    // 评估订单簿流动性
    double evaluateLiquidity(const OrderBook& order_book) override;

    // 计算订单流比率
    double calculateOrderFlowRatio(const OrderBook& order_book);

    // 计算流动性波动
    double calculateLiquidityVolatility(const std::vector<double>& liquidity_history);
};

using LiquidityEvaluatorPtr = std::shared_ptr<LiquidityEvaluator>;

} // namespace market
} // namespace hft