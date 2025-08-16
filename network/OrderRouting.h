#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include "LowLatencyNetwork.h"
#include "../core/Configuration.h"
#include "../core/EventLoop.h"
#include "../core/Logger.h"

namespace hft {
namespace network {

// 订单类型
enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT,
    ICEBERG
};

// 订单方向
enum class OrderSide {
    BUY,
    SELL
};

// 订单状态
enum class OrderStatus {
    PENDING_NEW,
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    PENDING_CANCEL,
    CANCELED,
    REJECTED
};

// 订单
struct Order {
    std::string orderId;
    std::string symbol;
    OrderType type;
    OrderSide side;
    double quantity;
    double price;
    OrderStatus status;
    std::string timestamp;

    Order(const std::string& id, const std::string& sym, OrderType t, OrderSide s, double qty, double pr)
        : orderId(id), symbol(sym), type(t), side(s), quantity(qty), price(pr),
          status(OrderStatus::PENDING_NEW) {}
};

// 订单路由
class OrderRouting {
public:
    OrderRouting(const std::string& name, const core::Configuration& config,
                 std::shared_ptr<LowLatencyNetwork> network,
                 std::shared_ptr<core::EventLoop> eventLoop);
    ~OrderRouting();

    // 获取路由名称
    std::string getName() const { return m_name; }

    // 初始化路由
    bool initialize();
    // 启动路由
    void start();
    // 停止路由
    void stop();
    // 路由状态
    bool isRunning() const { return m_running; }

    // 发送订单
    std::string sendOrder(const std::string& symbol, OrderType type, OrderSide side, double quantity, double price);
    // 取消订单
    bool cancelOrder(const std::string& orderId);
    // 修改订单
    bool modifyOrder(const std::string& orderId, double quantity, double price);

    // 注册订单状态回调
    using OrderStatusCallback = std::function<void(const Order& order)>;
    void registerStatusCallback(OrderStatusCallback callback);

private:
    std::string m_name;
    core::Configuration m_config;
    std::shared_ptr<LowLatencyNetwork> m_network;
    std::shared_ptr<core::EventLoop> m_eventLoop;
    core::Logger m_logger;
    std::atomic<bool> m_running;

    // 订单集合
    std::vector<Order> m_orders;
    std::mutex m_orderMutex;

    // 回调集合
    std::vector<OrderStatusCallback> m_callbacks;
    std::mutex m_callbackMutex;

    // 连接信息
    std::string m_host;
    uint16_t m_port;

    // 生成订单ID
    std::string generateOrderId();
    // 处理订单响应
    void handleOrderResponse(const std::string& host, uint16_t port, const void* data, size_t size);
    // 更新订单状态
    void updateOrderStatus(const std::string& orderId, OrderStatus status);
};

} // namespace network
} // namespace hft