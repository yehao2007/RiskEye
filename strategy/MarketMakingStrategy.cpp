#include "MarketMakingStrategy.h"
#include <algorithm>
#include <cmath>
#include "market/OrderBook.h"
#include "execution/Order.h"
#include "core/TimeManager.h"

namespace hft {
namespace strategy {

MarketMakingStrategy::MarketMakingStrategy() : CustomStrategy("MarketMaking") {
    // 定义参数
    m_parameter_definitions = {
        {"spread", "double", 0.01, 0.001, 0.1, 0.01},
        {"depth", "double", 10.0, 1.0, 100.0, 10.0},
        {"max_position", "int", 100, 10, 1000, 100},
        {"min_profit", "double", 0.005, 0.001, 0.05, 0.005},
        {"inventory_limit", "double", 0.5, 0.1, 1.0, 0.5},
        {"order_lifetime", "double", 5.0, 1.0, 60.0, 5.0},
        {"order_count_limit", "int", 10, 1, 50, 10}
    };

    // 设置默认值
    for (const auto& param : m_parameter_definitions) {
        m_parameters[param.name] = param.default_value;
    }

    m_position = 0;
    m_average_price = 0.0;
    m_volatility = 0.0;
    m_mid_price = 0.0;
}

bool MarketMakingStrategy::initialize(const std::vector<StrategyParameter>& parameters) {
    // 更新参数
    for (const auto& param : parameters) {
        if (!setParameter(param.name, param.value)) {
            std::cerr << "Failed to set parameter: " << param.name << std::endl;
            return false;
        }
    }

    // 初始化成员变量
    m_spread = m_parameters["spread"];
    m_depth = m_parameters["depth"];
    m_max_position = static_cast<int>(m_parameters["max_position"]);
    m_min_profit = m_parameters["min_profit"];
    m_inventory_limit = m_parameters["inventory_limit"];
    m_order_lifetime = m_parameters["order_lifetime"];
    m_order_count_limit = static_cast<int>(m_parameters["order_count_limit"]);

    std::cout << "MarketMakingStrategy initialized successfully" << std::endl;
    return true;
}

void MarketMakingStrategy::onMarketData(const market::MarketData& data) {
    // 更新价格历史
    m_price_history.push_back(data);
    if (m_price_history.size() > 100) {
        m_price_history.pop_front();
    }

    // 更新中间价格
    m_mid_price = data.last_price;

    // 更新统计数据
    updateStatistics();
}

void MarketMakingStrategy::onOrderBook(const market::OrderBook& order_book) {
    // 获取最佳买卖价格
    if (!order_book.getBids().empty() && !order_book.getAsks().empty()) {
        double best_bid = order_book.getBids()[0].price;
        double best_ask = order_book.getAsks()[0].price;
        m_mid_price = (best_bid + best_ask) / 2.0;

        // 更新价差历史
        double current_spread = best_ask - best_bid;
        m_spread_history.push_back(current_spread);
        if (m_spread_history.size() > 100) {
            m_spread_history.pop_front();
        }
    }
}

std::vector<execution::Order> MarketMakingStrategy::execute() {
    std::vector<execution::Order> new_orders;

    // 检查风险约束
    if (!checkRiskConstraints()) {
        return new_orders;
    }

    // 调整仓位
    adjustPosition();

    // 计算买卖报价
    calculateQuotes();

    // 移除过期订单
    auto now = core::TimeManager::getCurrentTimestamp();
    m_active_orders.erase(
        std::remove_if(m_active_orders.begin(), m_active_orders.end(),
                      [now, this](const execution::Order& order) {
                          return (now - order.timestamp) > m_order_lifetime * 1000000;
                      }),
        m_active_orders.end());

    // 生成新订单
    if (m_active_orders.size() < m_order_count_limit) {
        // 卖单
        execution::Order sell_order;
        sell_order.symbol = "AAPL";
        sell_order.type = execution::OrderType::LIMIT;
        sell_order.side = execution::OrderSide::SELL;
        sell_order.price = m_mid_price + m_spread / 2.0;
        sell_order.size = m_depth;
        sell_order.timestamp = now;
        sell_order.id = "sell_" + std::to_string(now);
        new_orders.push_back(sell_order);
        m_active_orders.push_back(sell_order);

        // 买单
        execution::Order buy_order;
        buy_order.symbol = "AAPL";
        buy_order.type = execution::OrderType::LIMIT;
        buy_order.side = execution::OrderSide::BUY;
        buy_order.price = m_mid_price - m_spread / 2.0;
        buy_order.size = m_depth;
        buy_order.timestamp = now;
        buy_order.id = "buy_" + std::to_string(now);
        new_orders.push_back(buy_order);
        m_active_orders.push_back(buy_order);
    }

    return new_orders;
}

void MarketMakingStrategy::calculateQuotes() {
    // 根据波动率调整价差
    double volatility_adjustment = 1.0 + (m_volatility > 0.01 ? (m_volatility - 0.01) * 10 : 0);
    m_spread = m_parameters["spread"] * volatility_adjustment;

    // 根据仓位调整报价
    double position_adjustment = 0.0;
    if (m_position > 0) {
        // 持有多仓，降低卖价
        position_adjustment = -std::min(static_cast<double>(m_position) / m_max_position, m_inventory_limit) * m_spread / 2.0;
    } else if (m_position < 0) {
        // 持有空仓，提高买价
        position_adjustment = std::min(static_cast<double>(-m_position) / m_max_position, m_inventory_limit) * m_spread / 2.0;
    }

    // 调整中间价格
    m_mid_price += position_adjustment;
}

bool MarketMakingStrategy::checkRiskConstraints() {
    // 检查仓位限制
    if (std::abs(m_position) >= m_max_position) {
        std::cerr << "Position limit exceeded: " << m_position << std::endl;
        return false;
    }

    // 检查风险收益比
    if (m_position != 0 && m_average_price != 0.0) {
        double current_profit = m_position * (m_mid_price - m_average_price);
        if (current_profit < -m_max_position * m_min_profit) {
            std::cerr << "Stop loss triggered: " << current_profit << std::endl;
            return false;
        }
    }

    return true;
}

void MarketMakingStrategy::adjustPosition() {
    // 如果仓位过大，减少做市活动
    if (std::abs(m_position) > m_inventory_limit * m_max_position) {
        m_depth = m_parameters["depth"] * (1.0 - std::abs(m_position) / m_max_position);
    } else {
        m_depth = m_parameters["depth"];
    }
}

void MarketMakingStrategy::updateStatistics() {
    // 计算波动率
    if (m_price_history.size() > 20) {
        double sum = 0.0;
        double mean = 0.0;
        for (const auto& data : m_price_history) {
            mean += data.last_price;
        }
        mean /= m_price_history.size();

        for (const auto& data : m_price_history) {
            sum += std::pow(data.last_price - mean, 2);
        }
        m_volatility = std::sqrt(sum / m_price_history.size()) / mean;
    }
}

} // namespace strategy
} // namespace hft