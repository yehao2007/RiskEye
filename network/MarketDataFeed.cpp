#include "MarketDataFeed.h"
#include <iostream>
#include <chrono>

namespace hft {
namespace network {

MarketDataFeed::MarketDataFeed(const std::string& name, const core::Configuration& config,
                               std::shared_ptr<LowLatencyNetwork> network,
                               std::shared_ptr<core::EventLoop> eventLoop)
    : m_name(name), m_config(config), m_network(network), m_eventLoop(eventLoop),
      m_logger("MarketDataFeed[" + name + "]"), m_running(false) {
}

MarketDataFeed::~MarketDataFeed() {
    stop();
}

bool MarketDataFeed::initialize() {
    m_logger.info("Initializing market data feed...");

    // 从配置中获取连接信息
    m_host = m_config.getString("market_data_feed." + m_name + ".host", "127.0.0.1");
    m_port = static_cast<uint16_t>(m_config.getInt("market_data_feed." + m_name + ".port", 5555));

    // 从配置中获取默认订阅
    std::vector<std::string> symbols = m_config.getStringList("market_data_feed." + m_name + ".symbols");
    if (!symbols.empty()) {
        std::string exchange = m_config.getString("market_data_feed." + m_name + ".exchange", "default");
        for (const auto& symbol : symbols) {
            subscribe(symbol, MarketDataType::ORDER_BOOK);
            subscribe(symbol, MarketDataType::TRADE);
        }
    }

    m_logger.info("Market data feed initialized successfully");
    return true;
}

void MarketDataFeed::start() {
    if (m_running) {
        m_logger.warning("Market data feed is already running");
        return;
    }

    m_logger.info("Starting market data feed...");
    m_running = true;

    // 连接到市场数据服务器
    m_network->connect(m_host, m_port);

    // 注册数据回调
    m_network->registerDataCallback(m_host, m_port, [this](const std::string& host, uint16_t port, const void* data, size_t size) {
        handleData(host, port, data, size);
    });

    // 发送订阅请求
    std::lock_guard<std::mutex> lock(m_subscriptionMutex);
    for (const auto& sub : m_subscriptions) {
        // 实际实现中，这里应该发送订阅请求到服务器
        m_logger.info("Subscribed to " + sub.symbol + " " + std::to_string(static_cast<int>(sub.dataType)));
    }

    m_logger.info("Market data feed started successfully");
}

void MarketDataFeed::stop() {
    if (!m_running) {
        m_logger.warning("Market data feed is not running");
        return;
    }

    m_logger.info("Stopping market data feed...");
    m_running = false;

    // 发送取消订阅请求
    std::lock_guard<std::mutex> lock(m_subscriptionMutex);
    for (const auto& sub : m_subscriptions) {
        // 实际实现中，这里应该发送取消订阅请求到服务器
        m_logger.info("Unsubscribed from " + sub.symbol + " " + std::to_string(static_cast<int>(sub.dataType)));
    }

    // 断开连接
    m_network->disconnect(m_host, m_port);

    m_logger.info("Market data feed stopped successfully");
}

bool MarketDataFeed::subscribe(const std::string& symbol, MarketDataType dataType) {
    std::lock_guard<std::mutex> lock(m_subscriptionMutex);

    // 检查是否已经订阅
    for (const auto& sub : m_subscriptions) {
        if (sub.symbol == symbol && sub.dataType == dataType) {
            m_logger.warning("Already subscribed to " + symbol + " " + std::to_string(static_cast<int>(dataType)));
            return false;
        }
    }

    // 添加到订阅列表
    std::string exchange = m_config.getString("market_data_feed." + m_name + ".exchange", "default");
    m_subscriptions.emplace_back(symbol, dataType, exchange);

    // 如果feed正在运行，立即发送订阅请求
    if (m_running) {
        // 实际实现中，这里应该发送订阅请求到服务器
        m_logger.info("Subscribed to " + symbol + " " + std::to_string(static_cast<int>(dataType)));
    }

    return true;
}

bool MarketDataFeed::unsubscribe(const std::string& symbol, MarketDataType dataType) {
    std::lock_guard<std::mutex> lock(m_subscriptionMutex);

    // 查找并删除订阅
    for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
        if (it->symbol == symbol && it->dataType == dataType) {
            // 如果feed正在运行，立即发送取消订阅请求
            if (m_running) {
                // 实际实现中，这里应该发送取消订阅请求到服务器
                m_logger.info("Unsubscribed from " + symbol + " " + std::to_string(static_cast<int>(dataType)));
            }

            m_subscriptions.erase(it);
            return true;
        }
    }

    m_logger.warning("No subscription found for " + symbol + " " + std::to_string(static_cast<int>(dataType)));
    return false;
}

void MarketDataFeed::registerCallback(MarketDataType dataType, MarketDataCallback callback) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_callbacks[dataType].push_back(callback);
    m_logger.info("Registered callback for data type: " + std::to_string(static_cast<int>(dataType)));
}

void MarketDataFeed::handleData(const std::string& host, uint16_t port, const void* data, size_t size) {
    // 在事件循环中处理数据
    m_eventLoop->postTask([this, data, size]() {
        parseMarketData(data, size);
    });
}

void MarketDataFeed::parseMarketData(const void* data, size_t size) {
    // 解析市场数据
    // 注意：这只是一个框架，实际实现需要根据具体的市场数据协议进行解析

    // 假设我们解析出了symbol和dataType
    std::string symbol = "AAPL";
    MarketDataType dataType = MarketDataType::ORDER_BOOK;

    // 调用相应的回调
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    auto it = m_callbacks.find(dataType);
    if (it != m_callbacks.end()) {
        for (auto& callback : it->second) {
            try {
                callback(symbol, dataType, data, size);
            } catch (const std::exception& e) {
                m_logger.error("Exception in market data callback: " + std::string(e.what()));
            }
        }
    }
}

} // namespace network
} // namespace hft