#pragma once
#include <cstdint>
#include <string>
#include <memory>

namespace hft {

enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT,
    ICEBERG
};

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    PENDING_NEW,
    NEW,
    FILLED,
    PARTIALLY_FILLED,
    CANCELLED,
    REJECTED,
    EXPIRED
};

struct Order {
    uint64_t orderId;
    std::string symbol;
    OrderType type;
    OrderSide side;
    uint64_t quantity;
    double price;
    double stopPrice;
    uint64_t displayQuantity;  // For iceberg orders
    OrderStatus status;
    uint64_t filledQuantity;
    double avgFillPrice;
    uint64_t timestamp;
    std::string exchange;

    Order() : orderId(0), quantity(0), price(0), stopPrice(0), displayQuantity(0),
              status(OrderStatus::PENDING_NEW), filledQuantity(0), avgFillPrice(0), timestamp(0) {}
};

typedef std::shared_ptr<Order> OrderPtr;

} // namespace hft