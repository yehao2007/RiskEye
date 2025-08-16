#include "MarketDataDistributor.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include "core/TimeManager.h"

namespace hft {
namespace market {

MarketDataDistributor::MarketDataDistributor()
    : m_running(false) {
}

MarketDataDistributor::~MarketDataDistributor() {
    stop();
}

bool MarketDataDistributor::initialize() {
    std::cout << "MarketDataDistributor initialized" << std::endl;
    return true;
}

bool MarketDataDistributor::registerCallback(const std::string& symbol, MarketDataCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks[symbol].push_back(callback);
    std::cout << "Callback registered for symbol: " << symbol << std::endl;
    return true;
}

bool MarketDataDistributor::unregisterCallback(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_callbacks.find(symbol);
    if (it == m_callbacks.end()) {
        return false;
    }

    m_callbacks.erase(it);
    std::cout << "Callbacks unregistered for symbol: " << symbol << std::endl;
    return true;
}

bool MarketDataDistributor::addDataQueue(const std::string& symbol, utils::LockFreeQueue<std::shared_ptr<MarketData>>* queue) {
    if (!queue) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_data_queues[symbol] = queue;

    // 如果分发器已经在运行，启动新的工作线程
    if (m_running) {
        m_worker_threads.emplace_back(&MarketDataDistributor::workerLoop, this, symbol);
    }

    std::cout << "Data queue added for symbol: " << symbol << std::endl;
    return true;
}

bool MarketDataDistributor::removeDataQueue(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_data_queues.find(symbol);
    if (it == m_data_queues.end()) {
        return false;
    }

    m_data_queues.erase(it);
    std::cout << "Data queue removed for symbol: " << symbol << std::endl;
    return true;
}

void MarketDataDistributor::start() {
    if (m_running) {
        return;
    }

    m_running = true;

    // 为每个符号启动工作线程
    std::vector<std::string> symbols = getAllSymbols();
    for (const auto& symbol : symbols) {
        m_worker_threads.emplace_back(&MarketDataDistributor::workerLoop, this, symbol);
    }

    std::cout << "MarketDataDistributor started with " << m_worker_threads.size() << " worker threads" << std::endl;
}

void MarketDataDistributor::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;

    // 等待所有工作线程结束
    for (auto& thread : m_worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    m_worker_threads.clear();
    std::cout << "MarketDataDistributor stopped" << std::endl;
}

void MarketDataDistributor::workerLoop(const std::string& symbol) {
    while (m_running) {
        utils::LockFreeQueue<std::shared_ptr<MarketData>>* queue = nullptr;
        std::vector<MarketDataCallback> callbacks;

        // 获取队列和回调
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it_queue = m_data_queues.find(symbol);
            auto it_callback = m_callbacks.find(symbol);

            if (it_queue == m_data_queues.end() || it_callback == m_callbacks.end()) {
                // 没有队列或回调，短暂休眠
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            queue = it_queue->second;
            callbacks = it_callback->second;
        }

        // 处理队列中的数据
        std::shared_ptr<MarketData> data;
        while (queue->pop(data)) {
            // 调用所有回调
            for (const auto& callback : callbacks) {
                try {
                    callback(data);
                } catch (const std::exception& e) {
                    std::cerr << "Exception in market data callback: " << e.what() << std::endl;
                }
            }
        }

        // 短暂休眠以避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

std::vector<std::string> MarketDataDistributor::getAllSymbols() {
    std::vector<std::string> symbols;
    for (const auto& [symbol, _] : m_data_queues) {
        symbols.push_back(symbol);
    }
    return symbols;
}

} // namespace market
} // namespace hft