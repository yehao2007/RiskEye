#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include "market/MarketData.h"
#include "utils/LockFreeQueue.h"

namespace hft {
namespace market {

// 市场数据分发器
class MarketDataDistributor {
public:
    using MarketDataCallback = std::function<void(const std::shared_ptr<MarketData>&)>;

    MarketDataDistributor();
    ~MarketDataDistributor();

    // 初始化分发器
    bool initialize();
    // 注册数据回调
    bool registerCallback(const std::string& symbol, MarketDataCallback callback);
    // 移除数据回调
    bool unregisterCallback(const std::string& symbol);
    // 添加数据队列
    bool addDataQueue(const std::string& symbol, utils::LockFreeQueue<std::shared_ptr<MarketData>>* queue);
    // 移除数据队列
    bool removeDataQueue(const std::string& symbol);
    // 启动分发
    void start();
    // 停止分发
    void stop();

private:
    std::atomic<bool> m_running;
    std::mutex m_mutex;
    std::vector<std::thread> m_worker_threads;

    // 回调映射
    std::unordered_map<std::string, std::vector<MarketDataCallback>> m_callbacks;
    // 数据队列映射
    std::unordered_map<std::string, utils::LockFreeQueue<std::shared_ptr<MarketData>>*> m_data_queues;

    // 工作线程函数
    void workerLoop(const std::string& symbol);
    // 获取所有符号
    std::vector<std::string> getAllSymbols();
};

} // namespace market
} // namespace hft