#pragma once
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <execution/Order.h>
#include <market_data/MarketData.h>

namespace hft {
namespace execution {

class LowLatencyOrderEngine {
public:
    LowLatencyOrderEngine();
    ~LowLatencyOrderEngine();

    // 初始化订单引擎
    bool initialize(const std::string& configPath);

    // 发送订单 (线程安全)
    bool sendOrder(const Order& order);

    // 取消订单
    bool cancelOrder(uint64_t orderId);

    // 市场数据更新回调
    void onMarketDataUpdate(const market_data::MarketData& data);

    // 获取当前未成交订单
    std::vector<Order> getOpenOrders() const;

    // 获取订单延迟统计 (单位: 纳秒)
    std::pair<double, double> getOrderLatencyStats() const; // <avg, max>

private:
    // 内部订单处理函数
    void processOrder(const Order& order);

    // 网络发送函数 (模拟光子传输接口)
    bool sendToNetwork(const Order& order, uint64_t& sequenceId);

    // 订单簿维护
    void updateOrderBook(const market_data::MarketData& data);

    // 延迟测量
    void measureLatency(uint64_t startTimeNs, const Order& order);

    // 订单存储
    std::atomic<uint64_t> m_nextOrderId;
    std::unordered_map<uint64_t, Order> m_openOrders;
    mutable std::mutex m_ordersMutex;

    // 延迟统计
    std::vector<uint64_t> m_latencyHistory;
    mutable std::mutex m_latencyMutex;

    // 网络配置
    std::string m_networkInterface;
    int m_priorityLevel;

    // 模拟光子传输参数
    bool m_usePhotonSimulation;
    double m_photonLatencyNs;
};

} // namespace execution
} // namespace hft