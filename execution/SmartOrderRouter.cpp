#include "SmartOrderRouter.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace hft {
namespace execution {

SmartOrderRouter::SmartOrderRouter(std::vector<std::shared_ptr<OrderExecution>> executors)
    : m_executors(executors), m_routingStrategy(RoutingStrategy::BALANCED) {
}

uint64_t SmartOrderRouter::routeOrder(const Order& order) {
    // 选择最优执行场所
    std::string bestVenue = selectBestVenue(order);

    // 查找对应的执行器
    for (const auto& executor : m_executors) {
        // 假设executor有获取场所名称的方法
        // if (executor->getVenueName() == bestVenue) {
        //     return executor->sendOrder(order.symbol, order.type, order.side,
        //                               order.quantity, order.price, order.stopPrice,
        //                               order.displayQuantity);
        // }
    }

    // 如果没有找到最优场所，使用第一个执行器
    if (!m_executors.empty()) {
        return m_executors[0]->sendOrder(order.symbol, order.type, order.side,
                                        order.quantity, order.price, order.stopPrice,
                                        order.displayQuantity);
    }

    return 0;
}

std::vector<Order> SmartOrderRouter::splitOrder(const Order& order, double minSize, double maxSize) {
    std::vector<Order> splitOrders;

    if (order.quantity <= maxSize) {
        splitOrders.push_back(order);
        return splitOrders;
    }

    // 计算拆分数量
    int numOrders = static_cast<int>(std::ceil(order.quantity / maxSize));
    uint64_t baseSize = order.quantity / numOrders;
    uint64_t remainder = order.quantity % numOrders;

    for (int i = 0; i < numOrders; i++) {
        Order splitOrder = order;
        splitOrder.quantity = baseSize + (i < remainder ? 1 : 0);
        splitOrder.orderId = 0; // 让执行器生成新的订单ID

        splitOrders.push_back(splitOrder);
    }

    return splitOrders;
}

void SmartOrderRouter::updateVenueInfo(const std::string& venueName, const ExecutionVenue& info) {
    m_venues[venueName] = info;
}

void SmartOrderRouter::setRoutingStrategy(RoutingStrategy strategy) {
    m_routingStrategy = strategy;
}

std::string SmartOrderRouter::selectBestVenue(const Order& order) {
    if (m_venues.empty()) {
        return "default";
    }

    // 选择第一个可用的场所作为默认值
    std::string bestVenue = m_venues.begin()->first;
    ExecutionVenue bestVenueInfo = m_venues.begin()->second;

    for (const auto& pair : m_venues) {
        const std::string& venue = pair.first;
        const ExecutionVenue& info = pair.second;

        if (!info.available) {
            continue;
        }

        switch (m_routingStrategy) {
            case RoutingStrategy::BEST_PRICE:
                // 假设这里有价格信息
                break;
            case RoutingStrategy::LOWEST_COST:
                if (info.fees < bestVenueInfo.fees) {
                    bestVenue = venue;
                    bestVenueInfo = info;
                }
                break;
            case RoutingStrategy::LOWEST_LATENCY:
                if (info.latency < bestVenueInfo.latency) {
                    bestVenue = venue;
                    bestVenueInfo = info;
                }
                break;
            case RoutingStrategy::BALANCED:
                // 综合考虑多个因素
                double bestScore = (bestVenueInfo.liquidity / 100) - bestVenueInfo.fees - (bestVenueInfo.latency / 1000);
                double currentScore = (info.liquidity / 100) - info.fees - (info.latency / 1000);
                if (currentScore > bestScore) {
                    bestVenue = venue;
                    bestVenueInfo = info;
                }
                break;
        }
    }

    return bestVenue;
}

} // namespace execution
} // namespace hft