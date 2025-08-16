#pragma once
#include "Order.h"
#include "../network/NetworkManager.h"
#include "../core/EventLoop.h"
#include <unordered_map>
#include <mutex>

namespace hft {

class OrderExecution {
public:
    OrderExecution(NetworkManager* networkManager, EventLoop* eventLoop);
    ~OrderExecution();

    // 创建并发送新订单
    uint64_t sendOrder(const std::string& symbol, OrderType type, OrderSide side,
                      uint64_t quantity, double price = 0, double stopPrice = 0,
                      uint64_t displayQuantity = 0);

    // 取消订单
    bool cancelOrder(uint64_t orderId);

    // 修改订单
    bool modifyOrder(uint64_t orderId, uint64_t newQuantity, double newPrice);

    // 获取订单状态
    OrderStatus getOrderStatus(uint64_t orderId) const;

    // 获取订单详情
    OrderPtr getOrder(uint64_t orderId) const;

    // 订单更新回调
    using OrderUpdateCallback = std::function<void(const OrderPtr&)>;
    void registerOrderUpdateCallback(const OrderUpdateCallback& callback);

private:
    // 处理网络响应
    void handleOrderResponse(const std::string& response);

    // 生成唯一订单ID
    uint64_t generateOrderId();

    NetworkManager* m_networkManager;
    EventLoop* m_eventLoop;
    std::unordered_map<uint64_t, OrderPtr> m_orders;
    mutable std::mutex m_ordersMutex;
    OrderUpdateCallback m_orderUpdateCallback;
    uint64_t m_lastOrderId;
};

} // namespace hft
// execution/OrderExecution.h 改进
class OrderExecutor {
public:
    // 添加智能订单路由
    void routeOrder(const Order& order);
    
    // 添加订单分割策略
    std::vector<Order> splitOrder(const Order& order, const SplitStrategy& strategy);
    
    // 添加执行质量分析
    ExecutionQuality analyzeExecution(const Order& order);
    
private:
    std::unique_ptr<ExecutionOptimizer> optimizer_;
    // ... 其他成员
};