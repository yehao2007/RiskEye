#include "NetworkManager.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace hft {
namespace network {

NetworkManager::NetworkManager(const core::Configuration& config)
    : m_config(config),
      m_logger("NetworkManager"),
      m_running(false),
      m_lowLatencyNetwork(std::make_unique<LowLatencyNetwork>()) {
}

NetworkManager::~NetworkManager() {
    stop();
}

bool NetworkManager::initialize() {
    m_logger.info("Initializing NetworkManager");

    // 初始化事件循环
    m_eventLoop = std::make_shared<core::EventLoop>();
    if (!m_eventLoop->initialize()) {
        m_logger.error("Failed to initialize EventLoop");
        return false;
    }

    // 初始化低延迟网络
    if (!initializeLowLatencyNetwork()) {
        m_logger.error("Failed to initialize LowLatencyNetwork");
        return false;
    }

    // 初始化市场数据feed
    if (!initializeMarketDataFeeds()) {
        m_logger.error("Failed to initialize MarketDataFeeds");
        return false;
    }

    // 初始化订单路由
    if (!initializeOrderRoutings()) {
        m_logger.error("Failed to initialize OrderRoutings");
        return false;
    }

    return true;
}

void NetworkManager::start() {
    if (m_running) {
        return;
    }

    m_logger.info("Starting NetworkManager");
    m_running = true;

    // 启动事件循环
    m_eventLoop->start();

    // 启动网络服务
    m_lowLatencyNetwork->start();

    // 启动市场数据feed
    for (auto& feed : m_marketDataFeeds) {
        feed->start();
    }

    // 启动订单路由
    for (auto& routing : m_orderRoutings) {
        routing->start();
    }
}

void NetworkManager::stop() {
    if (!m_running) {
        return;
    }

    m_logger.info("Stopping NetworkManager");
    m_running = false;

    // 停止订单路由
    for (auto& routing : m_orderRoutings) {
        routing->stop();
    }

    // 停止市场数据feed
    for (auto& feed : m_marketDataFeeds) {
        feed->stop();
    }

    // 停止网络服务
    m_lowLatencyNetwork->stop();

    // 停止事件循环
    m_eventLoop->stop();
}

std::shared_ptr<MarketDataFeed> NetworkManager::getMarketDataFeed(const std::string& feedName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& feed : m_marketDataFeeds) {
        if (feed->getName() == feedName) {
            return feed;
        }
    }
    return nullptr;
}

std::shared_ptr<OrderRouting> NetworkManager::getOrderRouting(const std::string& routingName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& routing : m_orderRoutings) {
        if (routing->getName() == routingName) {
            return routing;
        }
    }
    return nullptr;
}

bool NetworkManager::initializeLowLatencyNetwork() {
    // 配置低延迟网络
    LowLatencyNetworkConfig config;
    config.use_tcp = m_config.get<bool>("network.use_tcp", true);
    config.use_udp = m_config.get<bool>("network.use_udp", false);
    config.tcp_buffer_size = m_config.get<int>("network.tcp_buffer_size", 1024 * 1024);
    config.udp_buffer_size = m_config.get<int>("network.udp_buffer_size", 1024 * 1024);
    config.receive_timeout_ms = m_config.get<int>("network.receive_timeout_ms", 100);
    config.send_timeout_ms = m_config.get<int>("network.send_timeout_ms", 100);
    config.max_retries = m_config.get<int>("network.max_retries", 3);
    config.retry_delay_ms = m_config.get<int>("network.retry_delay_ms", 10);

    // 初始化低延迟网络
    if (!m_lowLatencyNetwork->initialize(config)) {
        m_logger.error("Failed to initialize LowLatencyNetwork");
        return false;
    }

    // 注册网络事件回调
    m_lowLatencyNetwork->setDataReceivedCallback([this](const std::string& endpoint, const void* data, size_t size) {
        // 处理接收到的数据
        // 根据端点分发到对应的市场数据feed或订单路由
    });

    return true;
}

bool NetworkManager::initializeMarketDataFeeds() {
    // 从配置中获取市场数据feed列表
    auto feeds_config = m_config.getJson("network.market_data_feeds");
    if (!feeds_config.is_array()) {
        m_logger.warning("No market data feeds configured");
        return true;
    }

    // 创建并初始化市场数据feed
    for (const auto& feed_config : feeds_config) {
        auto feed_name = feed_config["name"].get<std::string>();
        auto feed_type = feed_config["type"].get<std::string>();
        auto endpoint = feed_config["endpoint"].get<std::string>();

        std::shared_ptr<MarketDataFeed> feed;
        if (feed_type == "tcp") {
            feed = std::make_shared<TcpMarketDataFeed>(feed_name, endpoint);
        } else if (