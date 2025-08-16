#include "OrderExecution.h"
#include <random>
#include <chrono>
#include <sstream>

namespace hft {

OrderExecution::OrderExecution(NetworkManager* networkManager, EventLoop* eventLoop)
    : m_networkManager(networkManager), m_eventLoop(eventLoop), m_lastOrderId(0) {
    // 注册网络响应处理回调
    m_networkManager->registerResponseHandler([this](const std::string& response) {
        m_eventLoop->post([this, response]() {
            handleOrderResponse(response);
        });
    });
}

OrderExecution::~OrderExecution() {
}

uint64_t OrderExecution::sendOrder(const std::string& symbol, OrderType type, OrderSide side,
                                  uint64_t quantity, double price, double stopPrice,
                                  uint64_t displayQuantity) {
    OrderPtr order = std::make_shared<Order>();
    order->orderId = generateOrderId();
    order->symbol = symbol;
    order->type = type;
    order->side = side;
    order->quantity = quantity;
    order->price = price;
    order->stopPrice = stopPrice;
    order->displayQuantity = displayQuantity;
    order->status = OrderStatus::PENDING_NEW;
    order->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // 将订单存储在映射中
    {
        std::lock_guard<std::mutex> lock(m_ordersMutex);
        m_orders[order->orderId] = order;
    }

    // 构建订单消息并发送
    std::stringstream ss;
    ss << "NEW_ORDER," << order->orderId << "," << symbol << ",";
    ss << static_cast<int>(type) << "," << static_cast<int>(side) << "," << quantity << "," << price;
    if (type == OrderType::STOP || type == OrderType::STOP_LIMIT) {
        ss << "," << stopPrice;
    }
    if (type == OrderType::ICEBERG) {
        ss << "," << displayQuantity;
    }

    m_networkManager->sendMessage(ss.str());

    return order->orderId;
}

bool OrderExecution::cancelOrder(uint64_t orderId) {
    std::lock_guard<std::mutex> lock(m_ordersMutex);
    auto it = m_orders.find(orderId);
    if (it == m_orders.end()) {
        return false;
    }

    OrderPtr order = it->second;
    if (order->status != OrderStatus::NEW && order->status != OrderStatus::PARTIALLY_FILLED) {
        return false;
    }

    // 发送取消订单消息
    std::stringstream ss;
    ss << "CANCEL_ORDER," << orderId;
    m_networkManager->sendMessage(ss.str());

    return true;
}

bool OrderExecution::modifyOrder(uint64_t orderId, uint64_t newQuantity, double newPrice) {
    std::lock_guard<std::mutex> lock(m_ordersMutex);
    auto it = m_orders.find(orderId);
    if (it == m_orders.end()) {
        return false;
    }

    OrderPtr order = it->second;
    if (order->status != OrderStatus::NEW && order->status != OrderStatus::PARTIALLY_FILLED) {
        return false;
    }

    // 发送修改订单消息
    std::stringstream ss;
    ss << "MODIFY_ORDER," << orderId << "," << newQuantity << "," << newPrice;
    m_networkManager->sendMessage(ss.str());

    return true;
}

OrderStatus OrderExecution::getOrderStatus(uint64_t orderId) const {
    std::lock_guard<std::mutex> lock(m_ordersMutex);
    auto it = m_orders.find(orderId);
    if (it == m_orders.end()) {
        return OrderStatus::REJECTED;
    }
    return it->second->status;
}

OrderPtr OrderExecution::getOrder(uint64_t orderId) const {
    std::lock_guard<std::mutex> lock(m_ordersMutex);
    auto it = m_orders.find(orderId);
    if (it == m_orders.end()) {
        return nullptr;
    }
    return it->second;
}

void OrderExecution::registerOrderUpdateCallback(const OrderUpdateCallback& callback) {
    m_orderUpdateCallback = callback;
}

void OrderExecution::handleOrderResponse(const std::string& response) {
    // 解析响应
    std::stringstream ss(response);
    std::string token;
    std::vector<std::string> parts;

    while (std::getline(ss, token, ',')) {
        parts.push_back(token);
    }

    if (parts.empty()) {
        return;
    }

    if (parts[0] == "ORDER_ACK") {
        if (parts.size() < 2) return;
        uint64_t orderId = std::stoull(parts[1]);

        std::lock_guard<std::mutex> lock(m_ordersMutex);
        auto it = m_orders.find(orderId);
        if (it != m_orders.end()) {
            it->second->status = OrderStatus::NEW;
            if (m_orderUpdateCallback) {
                m_orderUpdateCallback(it->second);
            }
        }
    }
    else if (parts[0] == "ORDER_FILL") {
        if (parts.size() < 5) return;
        uint64_t orderId = std::stoull(parts[1]);
        uint64_t quantity = std::stoull(parts[2]);
        double price = std::stod(parts[3]);
        uint64_t remaining = std::stoull(parts[4]);

        std::lock_guard<std::mutex> lock(m_ordersMutex);
        auto it = m_orders.find(orderId);
        if (it != m_orders.end()) {
            OrderPtr order = it->second;
            order->filledQuantity += quantity;
            // 更新平均成交价
            order->avgFillPrice = (order->avgFillPrice * (order->filledQuantity - quantity) + price * quantity) / order->filledQuantity;

            if (remaining == 0) {
                order->status = OrderStatus::FILLED;
            } else {
                order->status = OrderStatus::PARTIALLY_FILLED;
            }

            if (m_orderUpdateCallback) {
                m_orderUpdateCallback(order);
            }
        }
    }
    else if (parts[0] == "ORDER_CANCELLED") {
        if (parts.size() < 2) return;
        uint64_t orderId = std::stoull(parts[1]);

        std::lock_guard<std::mutex> lock(m_ordersMutex);
        auto it = m_orders.find(orderId);
        if (it != m_orders.end()) {
            it->second->status = OrderStatus::CANCELLED;
            if (m_orderUpdateCallback) {
                m_orderUpdateCallback(it->second);
            }
        }
    }
    else if (parts[0] == "ORDER_REJECTED") {
        if (parts.size() < 3) return;
        uint64_t orderId = std::stoull(parts[1]);
        std::string reason = parts[2];

        std::lock_guard<std::mutex> lock(m_ordersMutex);
        auto it = m_orders.find(orderId);
        if (it != m_orders.end()) {
            it->second->status = OrderStatus::REJECTED;
            if (m_orderUpdateCallback) {
                m_orderUpdateCallback(it->second);
            }
        }
    }
}

uint64_t OrderExecution::generateOrderId() {
    // 使用时间戳和随机数生成唯一订单ID
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> distrib(0, 999999);

    uint64_t randomPart = distrib(gen);
    m_lastOrderId++;

    // 组合时间戳、随机数和序列号
    return (timestamp << 24) | (randomPart << 16) | m_lastOrderId;
}

} // namespace hft