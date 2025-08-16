#include "LowLatencyOrderEngine.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std::chrono;

namespace hft {
namespace execution {

LowLatencyOrderEngine::LowLatencyOrderEngine()
    : m_nextOrderId(1),
      m_usePhotonSimulation(false),
      m_photonLatencyNs(100) // 模拟100ns光子延迟
{
}

LowLatencyOrderEngine::~LowLatencyOrderEngine()
{
}

bool LowLatencyOrderEngine::initialize(const std::string& configPath)
{
    try {
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            std::cerr << "Failed to open config file: " << configPath << std::endl;
            return false;
        }

        json config;
        configFile >> config;

        m_networkInterface = config.value("network_interface", "eth0");
        m_priorityLevel = config.value("priority_level", 0);
        m_usePhotonSimulation = config.value("photon_simulation", false);
        m_photonLatencyNs = config.value("photon_latency_ns", 100.0);

        // 初始化网络接口 (实际环境中会调用特定API)
        std::cout << "Order engine initialized with interface: " << m_networkInterface << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing order engine: " << e.what() << std::endl;
        return false;
    }
}

bool LowLatencyOrderEngine::sendOrder(const Order& order)
{
    // 记录开始时间
    auto startTime = high_resolution_clock::now();
    uint64_t startTimeNs = duration_cast<nanoseconds>(startTime.time_since_epoch()).count();

    // 复制订单并分配ID
    Order newOrder = order;
    newOrder.orderId = m_nextOrderId++;
    newOrder.timestamp = startTimeNs;

    // 处理订单
    processOrder(newOrder);

    // 发送到网络
    uint64_t sequenceId;
    bool success = sendToNetwork(newOrder, sequenceId);

    // 测量延迟
    if (success) {
        measureLatency(startTimeNs, newOrder);
    }

    return success;
}

void LowLatencyOrderEngine::processOrder(const Order& order)
{
    // 在实际系统中，这里会进行订单验证、路由决策等
    // 为了低延迟，通常会使用无锁数据结构和内存池

    // 添加到未成交订单
    {
        std::lock_guard<std::mutex> lock(m_ordersMutex);
        m_openOrders[order.orderId] = order;
    }
}

bool LowLatencyOrderEngine::sendToNetwork(const Order& order, uint64_t& sequenceId)
{
    // 模拟网络发送
    sequenceId = order.orderId;

    // 模拟光子传输延迟
    if (m_usePhotonSimulation) {
        std::this_thread::sleep_for(nanoseconds(static_cast<uint64_t>(m_photonLatencyNs)));
    } else {
        // 实际网络发送代码
        // 这里会调用网络API发送订单
    }

    return true;
}

void LowLatencyOrderEngine::onMarketDataUpdate(const market_data::MarketData& data)
{
    // 更新订单簿
    updateOrderBook(data);

    // 在实际系统中，这里会触发基于市场数据的订单决策
}

void LowLatencyOrderEngine::updateOrderBook(const market_data::MarketData& data)
{
    // 维护订单簿的逻辑
    // 实际实现中会使用高效的数据结构
}

void LowLatencyOrderEngine::measureLatency(uint64_t startTimeNs, const Order& order)
{
    auto endTime = high_resolution_clock::now();
    uint64_t endTimeNs = duration_cast<nanoseconds>(endTime.time_since_epoch()).count();
    uint64_t latencyNs = endTimeNs - startTimeNs;

    {
        std::lock_guard<std::mutex> lock(m_latencyMutex);
        m_latencyHistory.push_back(latencyNs);
        // 保持历史记录在合理大小
        if (m_latencyHistory.size() > 10000) {
            m_latencyHistory.erase(m_latencyHistory.begin());
        }
    }
}

std::vector<Order> LowLatencyOrderEngine::getOpenOrders() const
{
    std::vector<Order> orders;
    {
        std::lock_guard<std::mutex> lock(m_ordersMutex);
        orders.reserve(m_openOrders.size());
        for (const auto& [id, order] : m_openOrders) {
            orders.push_back(order);
        }
    }
    return orders;
}

std::pair<double, double> LowLatencyOrderEngine::getOrderLatencyStats() const
{
    if (m_latencyHistory.empty()) {
        return {0.0, 0.0};
    }

    double sum = 0.0;
    double max = 0.0;
    {
        std::lock_guard<std::mutex> lock(m_latencyMutex);
        for (uint64_t latency : m_latencyHistory) {
            sum += latency;
            if (latency > max) {
                max = latency;
            }
        }
    }

    return {sum / m_latencyHistory.size(), max};
}

} // namespace execution
} // namespace hft