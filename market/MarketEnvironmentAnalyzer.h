#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "MarketData.h"
#include "OrderBook.h"

namespace hft {
namespace market {

// 市场状态
enum class MarketState {
    STATE_NORMAL,
    STATE_VOLATILE,
    STATE_TRENDING,
    STATE_RANGE_BOUND,
    STATE_UNKNOWN
};

// 市场环境分析器
class MarketEnvironmentAnalyzer {
public:
    MarketEnvironmentAnalyzer();
    ~MarketEnvironmentAnalyzer() = default;

    // 初始化分析器
    void initialize(double volatility_threshold, double trend_threshold);

    // 更新市场数据
    void updateMarketData(const MarketData& data);

    // 更新订单簿数据
    void updateOrderBook(const OrderBook& order_book);

    // 分析市场环境
    MarketState analyzeMarketEnvironment();

    // 获取当前市场状态
    MarketState getCurrentMarketState() const;

    // 获取市场状态描述
    std::string getMarketStateDescription(MarketState state) const;

    // 获取市场波动率
    double getVolatility() const;

    // 获取市场趋势
    double getTrend() const;

private:
    std::vector<double> m_price_history;
    std::vector<double> m_volume_history;
    double m_volatility_threshold;
    double m_trend_threshold;
    MarketState m_current_state;
    uint32_t m_history_window;

    // 计算波动率
    double calculateVolatility() const;

    // 计算趋势
    double calculateTrend() const;

    // 计算订单流不平衡
    double calculateOrderFlowImbalance(const OrderBook& order_book) const;
};

using MarketEnvironmentAnalyzerPtr = std::shared_ptr<MarketEnvironmentAnalyzer>;

} // namespace market
} // namespace hft