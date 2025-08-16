#include "MeanReversionStrategy.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "execution/Order.h"
#include "core/Configuration.h"

namespace hft {
namespace strategy {

MeanReversionStrategy::MeanReversionStrategy()
    : Strategy("MeanReversionStrategy", StrategyType::MEAN_REVERSION),
      m_lookback_period(20),
      m_std_dev_threshold(2.0) {
}

bool MeanReversionStrategy::initialize(const std::string& configPath) {
    // 从配置文件加载参数
    core::Configuration config;
    if (!config.loadFromFile(configPath)) {
        std::cerr << "Failed to load config file: " << configPath << std::endl;
        return false;
    }

    m_lookback_period = config.getInt("lookback_period", 20);
    m_std_dev_threshold = config.getDouble("std_dev_threshold", 2.0);

    std::cout << "MeanReversionStrategy initialized with lookback_period=" << m_lookback_period 
              << ", std_dev_threshold=" << m_std_dev_threshold << std::endl;
    return true;
}

void MeanReversionStrategy::start() {
    m_active = true;
    std::cout << "MeanReversionStrategy started" << std::endl;
}

void MeanReversionStrategy::stop() {
    m_active = false;
    std::cout << "MeanReversionStrategy stopped" << std::endl;
}

void MeanReversionStrategy::onMarketData(const network::MarketData& data) {
    if (!data.trades.empty()) {
        double last_price = data.trades.back().price;
        m_price_history.push_back(last_price);

        // 保持历史窗口大小
        if (m_price_history.size() > m_lookback_period) {
            m_price_history.pop_front();
        }
    }
}

void MeanReversionStrategy::onMarketPattern(const pattern::MarketPattern& pattern) {
    // 处理市场模式信号
}

std::vector<execution::Order> MeanReversionStrategy::execute() {
    std::vector<execution::Order> orders;

    if (!m_active || m_price_history.size() < m_lookback_period) {
        return orders;
    }

    double z_score = calculateZScore();
    double current_price = m_price_history.back();

    // 生成交易信号
    if (z_score > m_std_dev_threshold) {
        // 价格高于均值2个标准差，卖出
        execution::Order order;
        order.symbol = "AAPL";
        order.side = execution::OrderSide::SELL;
        order.price = current_price;
        order.size = 100;
        order.type = execution::OrderType::LIMIT;

        orders.push_back(order);
        std::cout << "MeanReversionStrategy: SELL order placed at " << current_price << " (z-score: " << z_score << ")" << std::endl;
    } else if (z_score < -m_std_dev_threshold) {
        // 价格低于均值2个标准差，买入
        execution::Order order;
        order.symbol = "AAPL";
        order.side = execution::OrderSide::BUY;
        order.price = current_price;
        order.size = 100;
        order.type = execution::OrderType::LIMIT;

        orders.push_back(order);
        std::cout << "MeanReversionStrategy: BUY order placed at " << current_price << " (z-score: " << z_score << ")" << std::endl;
    }

    return orders;
}

double MeanReversionStrategy::calculateMovingAverage() const {
    double sum = std::accumulate(m_price_history.begin(), m_price_history.end(), 0.0);
    return sum / m_price_history.size();
}

double MeanReversionStrategy::calculateStdDev() const {
    double mean = calculateMovingAverage();
    double variance = 0.0;

    for (double price : m_price_history) {
        variance += std::pow(price - mean, 2);
    }

    return std::sqrt(variance / m_price_history.size());
}

double MeanReversionStrategy::calculateZScore() const {
    double mean = calculateMovingAverage();
    double std_dev = calculateStdDev();
    double current_price = m_price_history.back();

    return (current_price - mean) / std_dev;
}

// 注册策略
REGISTER_STRATEGY(MeanReversionStrategy);

} // namespace strategy
} // namespace hft