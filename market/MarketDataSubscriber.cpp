#include "MarketDataSubscriber.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include "market/MarketDataParser.h"

namespace hft {
namespace market {

MarketDataSubscriber::MarketDataSubscriber(network::LowLatencyNetwork* network)
    : m_network(network),
      m_running(false) {
}

MarketDataSubscriber::~MarketDataSubscriber() {
    stop();
}

bool MarketDataSubscriber::initialize() {
    if (!m_network || !m_network->isInitialized()) {
        std::cerr << "LowLatencyNetwork not initialized" << std::endl;
        return false;
    }

    std::cout << "MarketDataSubscriber initialized" << std::endl;
    return true;
}

bool MarketDataSubscriber::subscribe(const SubscriptionConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 检查是否已经订阅
    auto it = m_subscriptions.find(config.symbol);
    if (it != m_subscriptions.end()) {
        for (const auto& sub : it->second) {
            if (sub.type == config.type) {
                std::cout << "Already subscribed to " << config.symbol << " " << static_cast<int>(config.type) << std::endl;
                return true;
            }
        }
        it->second.push_back(config);
    } else {
        m_subscriptions[config.symbol] = {config};
        // 创建数据队列
        createDataQueue(config.symbol);
    }

    // 发送订阅请求到网络
    std::string request = "SUBSCRIBE " + config.symbol + " " + std::to_string(static_cast<int>(config.type)) + " " +
                          std::to_string(config.update_frequency_ms) + " " + (config.snapshot_required ? "1" : "0");
    m_network->send(request);

    std::cout << "Subscribed to " << config.symbol << " " << static_cast<int>(config.type) << std::endl;
    return true;
}

bool MarketDataSubscriber::unsubscribe(const std::string& symbol, SubscriptionType type) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_subscriptions.find(symbol);
    if (it == m_subscriptions.end()) {
        return false;
    }

    // 查找并删除订阅
    auto& subs = it->second;
    auto sub_it = std::remove_if(subs.begin(), subs.end(),
        [type](const SubscriptionConfig& sub) { return sub.type == type; });

    if (sub_it == subs.end()) {
        return false;
    }

    subs.erase(sub_it, subs.end());

    // 如果没有更多订阅，删除数据队列
    if (subs.empty()) {
        m_subscriptions.erase(it);
        m_data_queues.erase(symbol);
    }

    // 发送取消订阅请求到网络
    std::string request = "UNSUBSCRIBE " + symbol + " " + std::to_string(static_cast<int>(type));
    m_network->send(request);

    std::cout << "Unsubscribed from " << symbol << " " << static_cast<int>(type) << std::endl;
    return true;
}

utils::LockFreeQueue<std::shared_ptr<MarketData>>* MarketDataSubscriber::getMarketDataQueue(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_data_queues.find(symbol);
    if (it == m_data_queues.end()) {
        return nullptr;
    }
    return it->second.get();
}

void MarketDataSubscriber::start() {
    if (m_running) {
        return;
    }

    m_running = true;
    m_receive_thread = std::thread(&MarketDataSubscriber::receiveLoop, this);
    std::cout << "MarketDataSubscriber started" << std::endl;
}

void MarketDataSubscriber::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;
    m_cv.notify_all();
    if (m_receive_thread.joinable()) {
        m_receive_thread.join();
    }
    std::cout << "MarketDataSubscriber stopped" << std::endl;
}

void MarketDataSubscriber::receiveLoop() {
    while (m_running) {
        // 等待数据
        std::vector<char> data;
        if (m_network->receive(data, 100)) { // 100ms超时
            processData(data);
        } else {
            // 短暂休眠以避免CPU占用过高
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

void MarketDataSubscriber::processData(const std::vector<char>& data) {
    // 解析数据
    MarketDataParser parser;
    auto market_data = parser.parse(data);
    if (!market_data) {
        std::cerr << "Failed to parse market data" << std::endl;
        return;
    }

    // 将数据放入对应的数据队列
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_data_queues.find(market_data->symbol);
    if (it != m_data_queues.end()) {
        it->second->push(market_data);
    }
}

void MarketDataSubscriber::createDataQueue(const std::string& symbol) {
    if (m_data_queues.find(symbol) == m_data_queues.end()) {
        m_data_queues[symbol] = std::make_unique<utils::LockFreeQueue<std::shared_ptr<MarketData>>>();
    }
}

} // namespace market
} // namespace hft