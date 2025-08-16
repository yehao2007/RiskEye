#include "TrendFollowingStrategy.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include "execution/Order.h"
#include "utils/NumberUtils.h"

namespace hft {
namespace strategy {

TrendFollowingStrategy::TrendFollowingStrategy()
    : CustomStrategy("TrendFollowingStrategy"),
      m_lookback_period(20),
      m_threshold(0.02) {
}

bool TrendFollowingStrategy::initialize(const std::vector<StrategyParameter>& parameters) {
    // 调用基类初始化
    if (!CustomStrategy::initialize(parameters)) {
        return false;
    }

    // 定义默认参数（如果未提供）
    if (m_parameter_definitions.empty()) {
        m_parameter_definitions = {
            {"lookback_period", "int", 20, 5, 100, 20},
            {"threshold", "double", 0.02, 0.001, 0.1, 0.02}
        };

        // 设置默认值
        for (const auto& param : m_parameter_definitions) {
            m_parameters[param.name] = param.default_value;
        }
    }

    // 从参数中获取配置
    m_lookback_period = static_cast<uint32_t>(getParameter("lookback_period"));
    m_threshold = getParameter("threshold");

    std::cout << "TrendFollowingStrategy initialized with lookback_period=" << m_lookback_period 
              << ", threshold=" << m_threshold << std::endl;
    return true;
}

void TrendFollowingStrategy::onMarketData(const market::MarketData& data) {
    // 保存市场数据
    CustomStrategy::onMarketData(data);

    if (!data.trades.empty()) {
        double last_price = data.trades.back().price;
        m_price_history.push_back(last_price);

        // 保持历史窗口大小
        if (m_price_history.size() > m_lookback_period) {
            m_price_history.erase(m_price_history.begin());
        }
    }
}

void TrendFollowingStrategy::onOrderBook(const market::OrderBook& order_book) {
    // 保存订单簿数据
    CustomStrategy::onOrderBook(order_book);
}

void TrendFollowingStrategy::execute() {
    if (m_price_history.size() < m_lookback_period) {
        return; // 数据不足
    }

    double trend = calculateTrend();

    // 生成交易信号
    if (trend > m_threshold) {
        // 上升趋势，买入
        execution::Order order;
        order.symbol = "AAPL";
        order.side = execution::OrderSide::BUY;
        order.price = m_price_history.back();
        order.size = 100; // 固定大小，实际应用中会根据风险和流动性调整
        order.type = execution::OrderType::MARKET;

        std::cout << "TrendFollowingStrategy: Placing BUY order for " << order.size << " shares of " 
                  << order.symbol << " at " << order.price << std::endl;

        // 发送订单
        if (m_order_executor) {
            m_order_executor->executeOrder(order);
        }
    } else if (trend < -m_threshold) {
        // 下降趋势，卖出
        execution::Order order;
        order.symbol = "AAPL";
        order.side = execution::OrderSide::SELL;
        order.price = m_price_history.back();
        order.size = 100; // 固定大小
        order.type = execution::OrderType::MARKET;

        std::cout << "TrendFollowingStrategy: Placing SELL order for " << order.size << " shares of " 
                  << order.symbol << " at " << order.price << std::endl;

        // 发送订单
        if (m_order_executor) {
            m_order_executor->executeOrder(order);
        }
    }
    // 否则，不操作
}

 double TrendFollowingStrategy::calculateMovingAverage() const {
    if (m_price_history.empty()) {
        return 0.0;
    }

    double sum = std::accumulate(m_price_history.begin(), m_price_history.end(), 0.0);
    return sum / m_price_history.size();
}

 double TrendFollowingStrategy::calculateTrend() const {
    if (m_price_history.size() < 2) {
        return 0.0;
    }

    // 简单趋势计算：最新价格与移动平均线的差值比率
    double ma = calculateMovingAverage();
    double last_price = m_price_history.back();

    return (last_price - ma) / ma;
}

} // namespace strategy
} // namespace hft