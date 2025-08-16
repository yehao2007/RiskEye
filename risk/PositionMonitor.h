#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include <mutex>
#include "execution/Order.h"

namespace hft {
namespace risk {

struct Position {
    std::string symbol;
    int64_t quantity;      // 持仓数量，可以为负数（卖空）
    double avgPrice;       // 平均持仓价格
    double currentPrice;   // 当前价格
    double unrealizedPnl;  // 未实现盈亏
    double realizedPnl;    // 已实现盈亏

    Position() : quantity(0), avgPrice(0.0), currentPrice(0.0),
                unrealizedPnl(0.0), realizedPnl(0.0) {}
};

typedef std::shared_ptr<Position> PositionPtr;

class PositionMonitor {
public:
    PositionMonitor();
    ~PositionMonitor();

    // 更新仓位
    void updatePosition(const execution::OrderPtr& order);

    // 更新市场价格
    void updateMarketPrice(const std::string& symbol, double price);

    // 获取单个品种持仓
    PositionPtr getPosition(const std::string& symbol) const;

    // 获取所有持仓
    std::unordered_map<std::string, PositionPtr> getAllPositions() const;

    // 计算总持仓价值
    double calculateTotalPositionValue() const;

    // 计算总未实现盈亏
    double calculateTotalUnrealizedPnl() const;

    // 计算总已实现盈亏
    double calculateTotalRealizedPnl() const;

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, PositionPtr> m_positions;
};

} // namespace risk
} // namespace hft