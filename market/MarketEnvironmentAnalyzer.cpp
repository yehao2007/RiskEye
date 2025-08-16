#include "MarketEnvironmentAnalyzer.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace hft {
namespace market {

MarketEnvironmentAnalyzer::MarketEnvironmentAnalyzer()
    : m_volatility_threshold(0.02),
      m_trend_threshold(0.01),
      m_current_state(MarketState::STATE_UNKNOWN),
      m_history_window(20) {
}

void MarketEnvironmentAnalyzer::initialize(double volatility_threshold, double trend_threshold) {
    m_volatility_threshold = volatility_threshold;
    m_trend_threshold = trend_threshold;
    m_current_state = MarketState::STATE_UNKNOWN;
    m_price_history.clear();
    m_volume_history.clear();
}

void MarketEnvironmentAnalyzer::updateMarketData(const MarketData& data) {
    // 保存价格和成交量历史
    if (!data.trades.empty()) {
        double last_price = data.trades.back().price;
        double last_volume = data.trades.back().size;

        m_price_history.push_back(last_price);
        m_volume_history.push_back(last_volume);

        // 保持历史窗口大小
        if (m_price_history.size() > m_history_window) {
            m_price_history.erase(m_price_history.begin());
        }
        if (m_volume_history.size() > m_history_window) {
            m_volume_history.erase(m_volume_history.begin());
        }
    }
}

void MarketEnvironmentAnalyzer::updateOrderBook(const OrderBook& order_book) {
    // 可以根据订单簿数据更新市场状态分析
    // 例如计算订单流不平衡等
}

MarketState MarketEnvironmentAnalyzer::analyzeMarketEnvironment() {
    if (m_price_history.size() < m_history_window) {
        return MarketState::STATE_UNKNOWN;
    }

    double volatility = calculateVolatility();
    double trend = calculateTrend();

    // 确定市场状态
    if (volatility > m_volatility_threshold) {
        m_current_state = MarketState::STATE_VOLATILE;
    } else if (std::abs(trend) > m_trend_threshold) {
        m_current_state = MarketState::STATE_TRENDING;
    } else {
        m_current_state = MarketState::STATE_RANGE_BOUND;
    }

    return m_current_state;
}

MarketState MarketEnvironmentAnalyzer::getCurrentMarketState() const {
    return m_current_state;
}

std::string MarketEnvironmentAnalyzer::getMarketStateDescription(MarketState state) const {
    switch (state) {
        case MarketState::STATE_NORMAL:
            return "Normal market conditions";
        case MarketState::STATE_VOLATILE:
            return "Volatile market conditions";
        case MarketState::STATE_TRENDING:
            return "Trending market conditions";
        case MarketState::STATE_RANGE_BOUND:
            return "Range-bound market conditions";
        default:
            return "Unknown market conditions";
    }
}

double MarketEnvironmentAnalyzer::getVolatility() const {
    return calculateVolatility();
}

double MarketEnvironmentAnalyzer::getTrend() const {
    return calculateTrend();
}

double MarketEnvironmentAnalyzer::calculateVolatility() const {
    if (m_price_history.size() < 2) {
        return 0.0;
    }

    std::vector<double> returns;
    for (size_t i = 1; i < m_price_history.size(); ++i) {
        double ret = log(m_price_history[i] / m_price_history[i-1]);
        returns.push_back(ret);
    }

    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    for (double ret : returns) {
        variance += (ret - mean) * (ret - mean);
    }
    variance /= returns.size();

    return sqrt(variance);
}

double MarketEnvironmentAnalyzer::calculateTrend() const {
    if (m_price_history.size() < 2) {
        return 0.0;
    }

    // 使用线性回归计算趋势
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    for (size_t i = 0; i < m_price_history.size(); ++i) {
        double x = i;
        double y = m_price_history[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    double n = m_price_history.size();
    double denominator = n * sum_x2 - sum_x * sum_x;
    if (denominator == 0.0) {
        return 0.0;
    }

    // 斜率即为趋势
    double slope = (n * sum_xy - sum_x * sum_y) / denominator;
    return slope;
}

double MarketEnvironmentAnalyzer::calculateOrderFlowImbalance(const OrderBook& order_book) const {
    // 计算订单流不平衡 = (买单量 - 卖单量) / (买单量 + 卖单量)
    double bid_volume = 0.0, ask_volume = 0.0;
    const int depth_levels = 5;
    int bid_count = 0, ask_count = 0;

    for (const auto& bid : order_book.getBids()) {
        if (bid_count++ < depth_levels) bid_volume += bid.size;
        else break;
    }
    for (const auto& ask : order_book.getAsks()) {
        if (ask_count++ < depth_levels) ask_volume += ask.size;
        else break;
    }

    double total_volume = bid_volume + ask_volume;
    if (total_volume == 0.0) return 0.0;

    return (bid_volume - ask_volume) / total_volume;
}

} // namespace market
} // namespace hft