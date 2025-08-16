#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <memory>
#include "market/MarketData.h"
#include "network/LowLatencyNetwork.h"
#include "utils/LockFreeQueue.h"

namespace hft {
namespace market {

// 数据订阅类型
enum class SubscriptionType {
    LEVEL1,         // 一级行情
    LEVEL2,         // 二级行情
    ORDER_BOOK,     // 订单簿
    TRADE_HISTORY,  // 交易历史
    MARKET_DEPTH    // 市场深度
};

// 订阅配置
struct SubscriptionConfig {
    std::string symbol;
    SubscriptionType type;
    uint32_t update_frequency_ms; // 更新频率(毫秒)
    bool snapshot_required;       // 是否需要快照
};

// 市场数据订阅器
class MarketDataSubscriber {
public:
    MarketDataSubscriber(network::LowLatencyNetwork* network);
    ~MarketDataSubscriber();

    // 初始化订阅器
    bool initialize();
    // 订阅市场数据
    bool subscribe(const SubscriptionConfig& config);
    // 取消订阅
    bool unsubscribe(const std::string& symbol, SubscriptionType type);
    // 获取订阅的数据队列
    utils::LockFreeQueue<std::shared_ptr<MarketData>>* getMarketDataQueue(const std::string& symbol);
    // 启动订阅
    void start();
    // 停止订阅
    void stop();

private:
    network::LowLatencyNetwork* m_network;
    std::atomic<bool> m_running;
    std::thread m_receive_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    // 订阅配置映射
    std::unordered_map<std::string, std::vector<SubscriptionConfig>> m_subscriptions;
    // 数据队列映射
    std::unordered_map<std::string, std::unique_ptr<utils::LockFreeQueue<std::shared_ptr<MarketData>>>> m_data_queues;

    // 接收线程函数
    void receiveLoop();
    // 处理接收到的数据
    void processData(const std::vector<char>& data);
    // 创建数据队列
    void createDataQueue(const std::string& symbol);
};

} // namespace market
} // namespace hft