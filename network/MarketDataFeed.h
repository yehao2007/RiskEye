#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <functional>
#include "LowLatencyNetwork.h"
#include "../core/Configuration.h"
#include "../core/EventLoop.h"
#include "../core/Logger.h"

namespace hft {
namespace network {

// 市场数据类型
enum class MarketDataType {
    ORDER_BOOK,
    TRADE,
    QUOTE,
    OHLCV,
    NEWS
};

// 市场数据feed
class MarketDataFeed {
public:
    MarketDataFeed(const std::string& name, const core::Configuration& config,
                   std::shared_ptr<LowLatencyNetwork> network,
                   std::shared_ptr<core::EventLoop> eventLoop);
    ~MarketDataFeed();

    // 获取feed名称
    std::string getName() const { return m_name; }

    // 初始化feed
    bool initialize();
    // 启动feed
    void start();
    // 停止feed
    void stop();
    // feed状态
    bool isRunning() const { return m_running; }

    // 订阅市场数据
    bool subscribe(const std::string& symbol, MarketDataType dataType);
    // 取消订阅
    bool unsubscribe(const std::string& symbol, MarketDataType dataType);

    // 注册市场数据回调
    using MarketDataCallback = std::function<void(const std::string& symbol, MarketDataType dataType, const void* data, size_t size)>;
    void registerCallback(MarketDataType dataType, MarketDataCallback callback);

private:
    std::string m_name;
    core::Configuration m_config;
    std::shared_ptr<LowLatencyNetwork> m_network;
    std::shared_ptr<core::EventLoop> m_eventLoop;
    core::Logger m_logger;
    std::atomic<bool> m_running;

    // 订阅信息
    struct Subscription {
        std::string symbol;
        MarketDataType dataType;
        std::string exchange;

        Subscription(const std::string& s, MarketDataType t, const std::string& e)
            : symbol(s), dataType(t), exchange(e) {}
    };

    std::vector<Subscription> m_subscriptions;
    std::mutex m_subscriptionMutex;

    // 回调映射
    std::unordered_map<MarketDataType, std::vector<MarketDataCallback>> m_callbacks;
    std::mutex m_callbackMutex;

    // 连接信息
    std::string m_host;
    uint16_t m_port;

    // 处理接收到的数据
    void handleData(const std::string& host, uint16_t port, const void* data, size_t size);
    // 解析市场数据
    void parseMarketData(const void* data, size_t size);
};

} // namespace network
} // namespace hft