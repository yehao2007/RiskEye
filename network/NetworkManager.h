#pragma once
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include "LowLatencyNetwork.h"
#include "MarketDataFeed.h"
#include "OrderRouting.h"
#include "../core/Configuration.h"
#include "../core/Logger.h"
#include "../core/EventLoop.h"

namespace hft {
namespace network {

class NetworkManager {
public:
    NetworkManager(const core::Configuration& config);
    ~NetworkManager();

    // 初始化网络模块
    bool initialize();
    // 启动网络服务
    void start();
    // 停止网络服务
    void stop();
    // 网络状态
    bool isRunning() const { return m_running; }

    // 获取市场数据feed
    std::shared_ptr<MarketDataFeed> getMarketDataFeed(const std::string& feedName);
    // 获取订单路由
    std::shared_ptr<OrderRouting> getOrderRouting(const std::string& routingName);

private:
    core::Configuration m_config;
    core::Logger m_logger;
    std::atomic<bool> m_running;
    std::shared_ptr<core::EventLoop> m_eventLoop;
    std::unique_ptr<LowLatencyNetwork> m_lowLatencyNetwork;

    // 市场数据feed集合
    std::vector<std::shared_ptr<MarketDataFeed>> m_marketDataFeeds;
    // 订单路由集合
    std::vector<std::shared_ptr<OrderRouting>> m_orderRoutings;

    std::mutex m_mutex;

    // 初始化低延迟网络
    bool initializeLowLatencyNetwork();
    // 初始化市场数据feed
    bool initializeMarketDataFeeds();
    // 初始化订单路由
    bool initializeOrderRoutings();
};

} // namespace network
} // namespace hft