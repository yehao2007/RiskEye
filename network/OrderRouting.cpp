#include "OrderRouting.h"
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>

namespace hft {
namespace network {

OrderRouting::OrderRouting(const std::string& name, const core::Configuration& config,
                           std::shared_ptr<LowLatencyNetwork> network,
                           std::shared_ptr<core::EventLoop> eventLoop)
    : m_name(name), m_config(config), m_network(network), m_eventLoop(eventLoop),
      m_logger("OrderRouting[" + name + "]"), m_running(false) {
}

OrderRouting::~OrderRouting() {
    stop();
}

bool OrderRouting::initialize() {
    m_logger.info("Initializing order routing...");

    // 从配置中获取连接信息
    m_host = m_config.getString("order_routing." + m_name + ".host", "127.0.0.1");
    m_port = static_cast<uint16_t>(m_config.getInt("order_routing." + m_name + ".port", 5556));

    m_logger.info("Order routing initialized successfully");
    return true;
}

void OrderRouting::start() {
    if (m_running) {
        m_logger.warning("Order routing is already running");
        return;
    }

    m_logger.info("Starting order routing...");
    m_running = true;

    // 连接到订单路由服务器
    m_network->connect(m_host, m_port);

    // 注册数据回调
    m_network->registerDataCallback(m_host, m_port, [this](const std::string& host, uint16_t port, const void* data, size_t size) {
        handleOrderResponse(host, port, data, size);
    });

    m_logger.info("Order routing started successfully");
}

void OrderRouting::stop() {
    if (!m_running) {
        m_logger.warning("Order routing is not running");
        return;
    }

    m_logger.info("Stopping order routing...");
    m_running = false;

    // 取消所有未完成的订单
    std::lock_guard<std::mutex> lock(m_orderMutex);
    for (const auto& order : m_orders) {
        if (order.status != OrderStatus::FILLED && order.status != OrderStatus::CANCELED && order.status != OrderStatus::REJECTED) {
            cancelOrder(order.orderId);
        }
    }

    // 断开连接
    m_network->disconnect(m_host, m_port);

    m_logger.info("Order routing stopped successfully");
}

std::string OrderRouting::sendOrder(const std::string& symbol, OrderType type, OrderSide side, double quantity, double price) {
    if (!m_running) {
        m_logger.error("Cannot send order, routing is not running");
        return "";
    }

    // 生成订单ID
    std::string orderId = generateOrderId();

    // 创建订单
    Order order(orderId, symbol, type, side, quantity, price);

    // 添加到订单集合
    {
        std::lock_guard<std::mutex> lock(m_orderMutex);
        m_orders.push_back(order);
    }

    // 构建订单消息并发送
    // 注意：这只是一个框架，实际实现需要根据具体的订单协议进行序列化
    std::stringstream ss;
    ss << "NEW_ORDER," << orderId << "," << symbol << "," << static_cast<int>(type) << "," << static_cast<int>(side) << "," << quantity << "," << price;
    std::string message = ss.str();

    if (m_network->send(m_host, m_port, message.c_str(), message.size())) {
        m_logger.info("Sent order: " + orderId + " " + symbol + " " + (side == OrderSide::BUY ? "BUY" : "SELL") + " " + std::to_string(quantity) + " @ " + std::to_string(price));
        return orderId;
    } else {
        m_logger.error("Failed to send order: " + orderId);
        // 从订单集合中移除
        std::lock_guard<std::mutex> lock(m_orderMutex);
        for (auto it = m_orders.begin(); it != m_orders.end(); ++it) {
            if (it->orderId == orderId) {
                m_orders.erase(it);
                break;
            }
        }
        return "";
    }
}

bool OrderRouting::cancelOrder(const std::string& orderId) {
    if (!m_running) {
        m_logger.error("Cannot cancel order, routing is not running");
        return false;
    }

    // 检查订单是否存在
    bool orderExists = false;
    {
        std::lock_guard<std::mutex> lock(m_orderMutex);
        for (const auto& order : m_orders) {
            if (order.orderId == orderId) {
                orderExists = true;
                break;
            }
        }
    }

    if (!orderExists) {
        m_logger.error("Order not found: " + orderId);
        return false;
    }

    // 构建取消订单消息并发送
    std::string message = "CANCEL_ORDER," + orderId;
    if (m_network->send(m_host, m_port, message.c_str(), message.size())) {
        m_logger.info("Sent cancel request for order: " + orderId);
        // 更新订单状态为PENDING_CANCEL
        updateOrderStatus(orderId, OrderStatus::PENDING_CANCEL);
        return true;
    } else {
        m_logger.error("Failed to send cancel request for order: " + orderId);
        return false;
    }
}

bool OrderRouting::modifyOrder(const std::string& orderId, double quantity, double price) {
    if (!m_running) {
        m_logger.error("Cannot modify order, routing is not running");
        return false;
    }

    // 检查订单是否存在
    bool orderExists = false;
    {
        std::lock_guard<std::mutex> lock(m_orderMutex);
        for (const auto& order : m_orders) {
            if (order.orderId == orderId) {
                orderExists = true;
                break;
            }
        }
    }

    if (!orderExists) {
        m_logger.error("Order not found: " + orderId);
        return false;
    }

    // 构建修改订单消息并发送
    std::stringstream ss;
    ss << "MODIFY_ORDER," << orderId << "," << quantity << "," << price;
    std::string message = ss.str();

    if (m_network->send(m_host, m_port, message.c_str(), message.size())) {
        m_logger.info("Sent modify request for order: " + orderId);
        return true;
    } else {
        m_logger.error("Failed to send modify request for order: " + orderId);
        return false;
    }
}

void OrderRouting::registerStatusCallback(OrderStatusCallback callback) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callbacks.push_back(callback);
    m_logger.info("Registered order status callback");
}

std::string OrderRouting::generateOrderId() {
    // 生成唯一订单ID
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distrib(0, 99999999);

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::stringstream ss;
    ss << "ORD" << millis << "-" << distrib(gen);
    return ss.str();
}

void OrderRouting::handleOrderResponse(const std::string& host, uint16_t port, const void* data, size_t size) {
    // 在事件循环中处理订单响应
    m_eventLoop->postTask([this, data, size]() {
        // 解析订单响应
        // 注意：这只是一个框架，实际实现需要根据具体的订单协议进行解析
        std::string response(static_cast<const char*>(data), size);
        m_logger.debug("Received order response: " + response);

        // 假设响应格式: ORDER_STATUS,orderId,status
        size_t pos1 = response.find(',');
        if (pos1 == std::string::npos) {
            m_logger.error("Invalid order response format: " + response);
            return;
        }

        std::string type = response.substr(0, pos1);
        if (type != "ORDER_STATUS") {
            m_logger.warning("Unknown response type: " + type);
            return;
        }

        size_t pos2 = response.find(',', pos1 + 1);
        if (pos2 == std::string::npos) {
            m_logger.error("Invalid order response format: " + response);
            return;
        }

        std::string orderId = response.substr(pos1 + 1, pos2 - pos1 - 1);
        std::string statusStr = response.substr(pos2 + 1);

        try {
            int statusInt = std::stoi(statusStr);
            OrderStatus status = static_cast<OrderStatus>(statusInt);
            updateOrderStatus(orderId, status);
        } catch (const std::exception& e) {
            m_logger.error("Failed to parse order status: " + statusStr + ", error: " + e.what());
        }
    });
}

void OrderRouting::updateOrderStatus(const std::string& orderId, OrderStatus status) {
    std::lock_guard<std::mutex> orderLock(m_orderMutex);

    // 查找订单并更新状态
    for (auto& order : m_orders) {
        if (order.orderId == orderId) {
            OrderStatus oldStatus = order.status;
            order.status = status;

            m_logger.info("Order status updated: " + orderId + " from " + std::to_string(static_cast<int>(oldStatus)) + " to " + std::to_string(static_cast<int>(status)));

            // 通知回调
            std::lock_guard<std::mutex> callbackLock(m_callbackMutex);
            for (auto& callback : m_callbacks) {
                try {
                    callback(order);
                } catch (const std::exception& e) {
                    m_logger.error("Exception in order status callback: " + std::string(e.what()));
                }
            }

            return;
        }
    }

    m_logger.warning("Order not found when updating status: " + orderId);
}

} // namespace network
} // namespace hft