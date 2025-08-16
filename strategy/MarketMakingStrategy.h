#pragma once
#include "CustomStrategy.h"
#include <vector>
#include <deque>

namespace hft {
namespace strategy {

// 做市策略
class MarketMakingStrategy : public CustomStrategy {
public:
    MarketMakingStrategy();
    ~MarketMakingStrategy() override = default;

    bool initialize(const std::vector<StrategyParameter>& parameters) override;
    void onMarketData(const market::MarketData& data) override;
    void onOrderBook(const market::OrderBook& order_book) override;
    std::vector<execution::Order> execute() override;

private:
    // 计算买卖报价
    void calculateQuotes();
    // 风险管理
    bool checkRiskConstraints();
    // 调整仓位
    void adjustPosition();
    // 更新统计数据
    void updateStatistics();

    double m_spread;               // 买卖价差
    double m_depth;                // 下单深度
    int m_max_position;            // 最大仓位
    double m_min_profit;           // 最小利润
    double m_inventory_limit;      // 库存限制
    double m_order_lifetime;       // 订单生命周期
    int m_order_count_limit;       // 订单数量限制

    std::deque<market::MarketData> m_price_history; // 价格历史
    std::deque<double> m_spread_history;            // 价差历史
    int m_position;                                 // 当前仓位
    double m_average_price;                         // 平均持仓价格
    std::vector<execution::Order> m_active_orders;  // 活动订单
    double m_volatility;                            // 波动率
    double m_mid_price;                             // 中间价格
};

} // namespace strategy
} // namespace hft