#pragma once
#include <string>
#include <chrono>

namespace hft {
namespace types {

// 价格和数量使用定点数以避免浮点误差
using Price = int64_t;  // 价格（放大10000倍）
using Volume = int64_t; // 数量（放大10000倍）
using Timestamp = std::chrono::system_clock::time_point;

// 订单方向
enum class Side {
    BUY,
    SELL
};

// 订单类型
enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT
};

// 订单状态
enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    REJECTED
};

// 交易状态
enum class TradeStatus {
    NORMAL,
    HALTED,
    CLOSED
};

} // namespace types
} // namespace hft
