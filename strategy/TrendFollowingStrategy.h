#pragma once
#include "CustomStrategy.h"
#include <vector>

namespace hft {
namespace strategy {

// 趋势跟踪策略
class TrendFollowingStrategy : public CustomStrategy {
public:
    TrendFollowingStrategy();
    ~TrendFollowingStrategy() override = default;

    // 初始化策略
    bool initialize(const std::vector<StrategyParameter>& parameters) override;

    // 处理市场数据
    void onMarketData(const market::MarketData& data) override;

    // 处理订单簿数据
    void onOrderBook(const market::OrderBook& order_book) override;

    // 执行策略
    void execute() override;

private:
    std::vector<double> m_price_history;
    uint32_t m_lookback_period;
    double m_threshold;

    // 计算移动平均线
    double calculateMovingAverage() const;

    // 计算价格趋势
    double calculateTrend() const;
};

} // namespace strategy
} // namespace hft