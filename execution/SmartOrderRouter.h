#pragma once
#include "Order.h"
#include "OrderExecution.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace hft {
namespace execution {

// 执行场所信息
struct Execution venue {
    std::string name;
    double liquidity;
    double fees;
    double latency;
    bool available;
};

class SmartOrderRouter {
public:
    SmartOrderRouter(std::vector<std::shared_ptr<OrderExecution>> executors);

    // 路由订单到最优场所
    uint64_t routeOrder(const Order& order);

    // 订单分割策略
    std::vector<Order> splitOrder(const Order& order, double minSize, double maxSize);

    // 更新执行场所信息
    void updateVenueInfo(const std::string& venueName, const ExecutionVenue& info);

    // 设置路由策略
    enum class RoutingStrategy { BEST_PRICE, LOWEST_COST, LOWEST_LATENCY, BALANCED };
    void setRoutingStrategy(RoutingStrategy strategy);

private:
    std::vector<std::shared_ptr<OrderExecution>> m_executors;
    std::unordered_map<std::string, ExecutionVenue> m_venues;
    RoutingStrategy m_routingStrategy;

    // 选择最优执行场所
    std::string selectBestVenue(const Order& order);
};

} // namespace execution
} // namespace hft