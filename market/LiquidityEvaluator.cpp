#include "LiquidityEvaluator.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace hft {
namespace market {

LiquidityEvaluator::LiquidityEvaluator()
    : m_depth_levels(5),
      m_liquidity_score(0.0) {
}

void LiquidityEvaluator::initialize(uint32_t depth_levels) {
    m_depth_levels = depth_levels;
    m_liquidity_score = 0.0;
}

 double LiquidityEvaluator::evaluateLiquidity(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        m_liquidity_score = 0.0;
        return m_liquidity_score;
    }

    double spread = getBidAskSpread(order_book);
    double depth = getMarketDepth(order_book, m_depth_levels);

    // 简单的流动性评分：深度/价差
    m_liquidity_score = depth / (spread + 0.0001); // 避免除零
    return m_liquidity_score;
}

 double LiquidityEvaluator::getBidAskSpread(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }

    double best_ask = order_book.getAsks()[0].price;
    double best_bid = order_book.getBids()[0].price;
    return best_ask - best_bid;
}

 double LiquidityEvaluator::getMarketDepth(const OrderBook& order_book, uint32_t levels) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }

    double ask_depth = 0.0;
    double bid_depth = 0.0;

    uint32_t ask_levels = std::min(levels, static_cast<uint32_t>(order_book.getAsks().size()));
    uint32_t bid_levels = std::min(levels, static_cast<uint32_t>(order_book.getBids().size()));

    for (uint32_t i = 0; i < ask_levels; ++i) {
        ask_depth += order_book.getAsks()[i].size;
    }

    for (uint32_t i = 0; i < bid_levels; ++i) {
        bid_depth += order_book.getBids()[i].size;
    }

    return (ask_depth + bid_depth) / 2.0;
}

 double LiquidityEvaluator::getLiquidityScore() const {
    return m_liquidity_score;
}

// AdvancedLiquidityEvaluator 实现
AdvancedLiquidityEvaluator::AdvancedLiquidityEvaluator()
    : LiquidityEvaluator() {
}

void AdvancedLiquidityEvaluator::initialize(uint32_t depth_levels) {
    LiquidityEvaluator::initialize(depth_levels);
}

 double AdvancedLiquidityEvaluator::evaluateLiquidity(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        m_liquidity_score = 0.0;
        return m_liquidity_score;
    }

    double spread = getBidAskSpread(order_book);
    double depth = getMarketDepth(order_book, m_depth_levels);
    double order_flow_ratio = calculateOrderFlowRatio(order_book);

    // 综合评分：深度/价差 * 订单流比率权重
    m_liquidity_score = (depth / (spread + 0.0001)) * (0.5 + order_flow_ratio * 0.5);
    return m_liquidity_score;
}

 double AdvancedLiquidityEvaluator::calculateOrderFlowRatio(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.5; // 中性
    }

    double best_ask = order_book.getAsks()[0].price;
    double best_bid = order_book.getBids()[0].price;
    double mid_price = (best_ask + best_bid) / 2.0;

    double ask_volume = 0.0;
    double bid_volume = 0.0;

    for (const auto& ask : order_book.getAsks()) {
        if (ask.price <= mid_price * 1.01) { // 考虑中间价1%以内的卖单
            ask_volume += ask.size;
        }
    }

    for (const auto& bid : order_book.getBids()) {
        if (bid.price >= mid_price * 0.99) { // 考虑中间价1%以内的买单
            bid_volume += bid.size;
        }
    }

    double total_volume = ask_volume + bid_volume;
    if (total_volume == 0) {
        return 0.5; // 中性
    }

    return bid_volume / total_volume;
}

} // namespace market
} // namespace hft
#include "LiquidityEvaluator.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace hft {
namespace market {

LiquidityEvaluator::LiquidityEvaluator()
    : m_depth_levels(5),
      m_liquidity_score(0.0) {
}

void LiquidityEvaluator::initialize(uint32_t depth_levels) {
    m_depth_levels = depth_levels;
    m_liquidity_score = 0.0;
}

 double LiquidityEvaluator::evaluateLiquidity(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        m_liquidity_score = 0.0;
        return m_liquidity_score;
    }

    double spread = getBidAskSpread(order_book);
    double depth = getMarketDepth(order_book, m_depth_levels);

    // 简单的流动性评分：深度/价差
    m_liquidity_score = depth / (spread + 0.0001); // 避免除零
    return m_liquidity_score;
}

 double LiquidityEvaluator::getBidAskSpread(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }

    double best_ask = order_book.getAsks()[0].price;
    double best_bid = order_book.getBids()[0].price;
    return best_ask - best_bid;
}

 double LiquidityEvaluator::getMarketDepth(const OrderBook& order_book, uint32_t levels) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }

    double ask_depth = 0.0;
    double bid_depth = 0.0;

    uint32_t ask_levels = std::min(levels, static_cast<uint32_t>(order_book.getAsks().size()));
    uint32_t bid_levels = std::min(levels, static_cast<uint32_t>(order_book.getBids().size()));

    for (uint32_t i = 0; i < ask_levels; ++i) {
        ask_depth += order_book.getAsks()[i].size;
    }

    for (uint32_t i = 0; i < bid_levels; ++i) {
        bid_depth += order_book.getBids()[i].size;
    }

    return (ask_depth + bid_depth) / 2.0;
}

 double LiquidityEvaluator::getLiquidityScore() const {
    return m_liquidity_score;
}

// AdvancedLiquidityEvaluator 实现
AdvancedLiquidityEvaluator::AdvancedLiquidityEvaluator()
    : LiquidityEvaluator() {
}

void AdvancedLiquidityEvaluator::initialize(uint32_t depth_levels) {
    LiquidityEvaluator::initialize(depth_levels);
}

 double AdvancedLiquidityEvaluator::evaluateLiquidity(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        m_liquidity_score = 0.0;
        return m_liquidity_score;
    }

    double spread = getBidAskSpread(order_book);
    double depth = getMarketDepth(order_book, m_depth_levels);
    double order_flow_ratio = calculateOrderFlowRatio(order_book);

    // 综合评分：深度/价差 * 订单流比率权重
    m_liquidity_score = (depth / (spread + 0.0001)) * (0.5 + order_flow_ratio * 0.5);
    return m_liquidity_score;
}

 double AdvancedLiquidityEvaluator::calculateOrderFlowRatio(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.5; // 中性
    }

    double best_ask = order_book.getAsks()[0].price;
    double best_bid = order_book.getBids()[0].price;
    double mid_price = (best_ask + best_bid) / 2.0;

    double ask_volume = 0.0;
    double bid_volume = 0.0;

    for (const auto& ask : order_book.getAsks()) {
        if (ask.price <= mid_price * 1.01) { // 考虑中间价1%以内的卖单
            ask_volume += ask.size;
        }
    }

    for (const auto& bid : order_book.getBids()) {
        if (bid.price >= mid_price * 0.99) { // 考虑中间价1%以内的买单
            bid_volume += bid.size;
        }
    }

    double total_volume = ask_volume + bid_volume;
    if (total_volume == 0) {
        return 0.5; // 中性
    }

    return bid_volume / total_volume;
}

} // namespace market
} // namespace hft
#include "LiquidityEvaluator.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace hft {
namespace market {

LiquidityEvaluator::LiquidityEvaluator()
    : m_depth_levels(5),
      m_liquidity_score(0.0) {
}

void LiquidityEvaluator::initialize(uint32_t depth_levels) {
    m_depth_levels = depth_levels;
    m_liquidity_score = 0.0;
}

 double LiquidityEvaluator::evaluateLiquidity(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        m_liquidity_score = 0.0;
        return m_liquidity_score;
    }

    double spread = getBidAskSpread(order_book);
    double depth = getMarketDepth(order_book, m_depth_levels);

    // 简单的流动性评分：深度/价差
    m_liquidity_score = depth / (spread + 0.0001); // 避免除零
    return m_liquidity_score;
}

 double LiquidityEvaluator::getBidAskSpread(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }

    double best_ask = order_book.getAsks()[0].price;
    double best_bid = order_book.getBids()[0].price;
    return best_ask - best_bid;
}

 double LiquidityEvaluator::getMarketDepth(const OrderBook& order_book, uint32_t levels) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }

    double ask_depth = 0.0;
    double bid_depth = 0.0;

    uint32_t ask_levels = std::min(levels, static_cast<uint32_t>(order_book.getAsks().size()));
    uint32_t bid_levels = std::min(levels, static_cast<uint32_t>(order_book.getBids().size()));

    for (uint32_t i = 0; i < ask_levels; ++i) {
        ask_depth += order_book.getAsks()[i].size;
    }

    for (uint32_t i = 0; i < bid_levels; ++i) {
        bid_depth += order_book.getBids()[i].size;
    }

    return (ask_depth + bid_depth) / 2.0;
}

 double LiquidityEvaluator::getLiquidityScore() const {
    return m_liquidity_score;
}

// AdvancedLiquidityEvaluator 实现
AdvancedLiquidityEvaluator::AdvancedLiquidityEvaluator()
    : LiquidityEvaluator() {
}

void AdvancedLiquidityEvaluator::initialize(uint32_t depth_levels) {
    LiquidityEvaluator::initialize(depth_levels);
}

 double AdvancedLiquidityEvaluator::evaluateLiquidity(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        m_liquidity_score = 0.0;
        return m_liquidity_score;
    }

    double spread = getBidAskSpread(order_book);
    double depth = getMarketDepth(order_book, m_depth_levels);
    double order_flow_ratio = calculateOrderFlowRatio(order_book);

    // 综合评分：深度/价差 * 订单流比率权重
    m_liquidity_score = (depth / (spread + 0.0001)) * (0.5 + order_flow_ratio * 0.5);
    return m_liquidity_score;
}

 double AdvancedLiquidityEvaluator::calculateOrderFlowRatio(const OrderBook& order_book) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.5; // 中性
    }

    double best_ask = order_book.getAsks()[0].price;
    double best_bid = order_book.getBids()[0].price;
    double mid_price = (best_ask + best_bid) / 2.0;

    double ask_volume = 0.0;
    double bid_volume = 0.0;

    for (const auto& ask : order_book.getAsks()) {
        if (ask.price <= mid_price * 1.01) { // 考虑中间价1%以内的卖单
            ask_volume += ask.size;
        }
    }

    for (const auto& bid : order_book.getBids()) {
        if (bid.price >= mid_price * 0.99) { // 考虑中间价1%以内的买单
            bid_volume += bid.size;
        }
    }

    double total_volume = ask_volume + bid_volume;
    if (total_volume == 0) {
        return 0.5; // 中性
    }

    return bid_volume / total_volume;
}

} // namespace market
} // namespace hft
#include "LiquidityEvaluator.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace hft {
namespace market {

AdvancedLiquidityEvaluator::AdvancedLiquidityEvaluator()
    : m_history_window_size(100) {
}

LiquidityInfo AdvancedLiquidityEvaluator::evaluate(const OrderBook& order_book) {
    LiquidityInfo info;

    // 计算买卖价差
    if (!order_book.getAsks().empty() && !order_book.getBids().empty()) {
        info.spread = order_book.getAsks()[0].price - order_book.getBids()[0].price;
    } else {
        info.spread = 0.0;
    }

    // 计算市场深度 (前5档订单总量)
    const int depth_levels = 5;
    double bid_depth = 0.0, ask_depth = 0.0;
    int bid_count = 0, ask_count = 0;

    for (const auto& bid : order_book.getBids()) {
        if (bid_count++ < depth_levels) bid_depth += bid.size;
        else break;
    }
    for (const auto& ask : order_book.getAsks()) {
        if (ask_count++ < depth_levels) ask_depth += ask.size;
        else break;
    }
    info.depth = std::min(bid_depth, ask_depth);

    // 计算订单流不平衡
    info.order_flow_imbalance = calculateOrderFlowImbalance(order_book);

    // 记录最新价格用于计算波动率
    double mid_price = (order_book.getAsks()[0].price + order_book.getBids()[0].price) / 2.0;
    m_price_history.push_back(mid_price);

    // 保持历史窗口大小
    if (m_price_history.size() > m_history_window_size) {
        m_price_history.erase(m_price_history.begin());
    }

    // 计算波动率
    if (m_price_history.size() > 1) {
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
        info.volatility = sqrt(variance);
    } else {
        info.volatility = 0.0;
    }

    // 初始化价格冲击
    info.price_impact = 0.0;

    // 交易量 (这里简化处理，实际应该从市场数据获取)
    info.trading_volume = 0;

    return info;
}

void AdvancedLiquidityEvaluator::optimizeOrderExecution(const LiquidityInfo& liquidity_info, Order& order) {
    // 基于流动性优化订单大小和价格
    if (liquidity_info.depth > 0) {
        // 如果流动性充足，可以增大订单 size
        double liquidity_ratio = liquidity_info.depth / order.size;
        if (liquidity_ratio > 5.0) { // 流动性非常充足
            order.size *= 1.2; // 增加 20% 的订单量
        } else if (liquidity_ratio < 1.5) { // 流动性不足
            order.size *= 0.8; // 减少 20% 的订单量
            // 可能需要拆分订单
            order.time_in_force = TimeInForce::TIME_IN_FORCE_GTC;
        }
    }

    // 基于波动率调整订单价格
    if (liquidity_info.volatility > 0.01) { // 高波动
        if (order.side == OrderSide::ORDER_SIDE_BUY) {
            order.price *= (1.0 + liquidity_info.volatility);
        } else {
            order.price *= (1.0 - liquidity_info.volatility);
        }
    }

    // 计算价格冲击
    order.price_impact = calculatePriceImpact(order.size, liquidity_info.depth);
}

double AdvancedLiquidityEvaluator::calculateOrderFlowImbalance(const OrderBook& order_book) {
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

double AdvancedLiquidityEvaluator::calculatePriceImpact(double order_size, double liquidity) {
    // 简化的价格冲击模型: 价格冲击 = 订单大小 / 流动性 * 常数
    const double IMPACT_COEFFICIENT = 0.1;
    return (order_size / liquidity) * IMPACT_COEFFICIENT;
}

} // namespace market
} // namespace hft