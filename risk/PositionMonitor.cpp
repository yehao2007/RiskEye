#include "PositionMonitor.h"
#include <algorithm>

namespace hft {
namespace risk {

PositionMonitor::PositionMonitor() {
}

PositionMonitor::~PositionMonitor() {
}

void PositionMonitor::updatePosition(const execution::OrderPtr& order) {
    if (!order) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    std::string symbol = order->symbol;
    int64_t quantity = (order->side == execution::OrderSide::BUY) ? 
                        static_cast<int64_t>(order->filledQuantity) : 
                        -static_cast<int64_t>(order->filledQuantity);

    // 如果没有该品种的持仓，则创建新的持仓
    if (m_positions.find(symbol) == m_positions.end()) {
        m_positions[symbol] = std::make_shared<Position>();
        m_positions[symbol]->symbol = symbol;
    }

    PositionPtr position = m_positions[symbol];

    // 更新已实现盈亏
    if (quantity != 0) {
        // 计算本次交易的已实现盈亏
        if (position->quantity > 0 && quantity < 0) {
            // 多头平仓
            double pnl = static_cast<double>(-quantity) * (order->avgFillPrice - position->avgPrice);
            position->realizedPnl += pnl;
        } else if (position->quantity < 0 && quantity > 0) {
            // 空头平仓
            double pnl = static_cast<double>(quantity) * (position->avgPrice - order->avgFillPrice);
            position->realizedPnl += pnl;
        }

        // 更新平均持仓价格
        if (position->quantity + quantity == 0) {
            // 完全平仓
            position->avgPrice = 0.0;
        } else {
            position->avgPrice = (
                position->avgPrice * static_cast<double>(position->quantity) + 
                order->avgFillPrice * static_cast<double>(quantity)
            ) / static_cast<double>(position->quantity + quantity);
        }

        // 更新持仓数量
        position->quantity += quantity;
    }

    // 更新当前价格
    position->currentPrice = order->avgFillPrice;

    // 更新未实现盈亏
    position->unrealizedPnl = static_cast<double>(position->quantity) * 
                             (position->currentPrice - position->avgPrice);
}

void PositionMonitor::updateMarketPrice(const std::string& symbol, double price) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_positions.find(symbol);
    if (it != m_positions.end()) {
        PositionPtr position = it->second;
        position->currentPrice = price;
        position->unrealizedPnl = static_cast<double>(position->quantity) * 
                                 (position->currentPrice - position->avgPrice);
    }
}

PositionPtr PositionMonitor::getPosition(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_positions.find(symbol);
    if (it != m_positions.end()) {
        return it->second;
    }
    return nullptr;
}

std::unordered_map<std::string, PositionPtr> PositionMonitor::getAllPositions() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_positions;
}

double PositionMonitor::calculateTotalPositionValue() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    double totalValue = 0.0;
    for (const auto& pair : m_positions) {
        PositionPtr position = pair.second;
        totalValue += std::abs(static_cast<double>(position->quantity) * position->currentPrice);
    }
    return totalValue;
}

double PositionMonitor::calculateTotalUnrealizedPnl() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    double totalPnl = 0.0;
    for (const auto& pair : m_positions) {
        totalPnl += pair.second->unrealizedPnl;
    }
    return totalPnl;
}

double PositionMonitor::calculateTotalRealizedPnl() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    double totalPnl = 0.0;
    for (const auto& pair : m_positions) {
        totalPnl += pair.second->realizedPnl;
    }
    return totalPnl;
}

} // namespace risk
} // namespace hft