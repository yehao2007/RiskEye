#include "OptimizedBacktestEngine.h"
#include "../utils/PerformanceOptimization.h"
#include "../core/Logger.h"
#include <tbb/parallel_for.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>

namespace hft {
namespace backtest {

void OptimizedBacktestEngine::initialize(const BacktestConfig& config) {
    config_ = config;
    orderBook_ = std::make_unique<OrderBook>();
    positionManager_ = std::make_unique<PositionManager>();
    dataLoader_ = std::make_unique<OptimizedDataLoader>();
    
    // 初始化并行处理组件
    eventQueue_ = std::make_unique<tbb::concurrent_queue<MarketEvent>>();
    processingThreads_ = std::vector<std::thread>(config.numThreads);
    
    // 启动处理线程
    for (auto& thread : processingThreads_) {
        thread = std::thread(&OptimizedBacktestEngine::processEvents, this);
    }
    
    Logger::info("Initialized backtest engine with {} processing threads", config.numThreads);
}

void OptimizedBacktestEngine::runBacktest(std::shared_ptr<strategy::AdvancedStrategy> strategy) {
    strategy_ = strategy;
    isRunning_ = true;
    
    // 并行加载和处理数据
    auto& threadPool = utils::PerformanceManager::getInstance().getThreadPool();
    
    // 划分数据批次
    std::vector<DataBatch> batches = dataLoader_->loadDataInBatches(config_.batchSize);
    
    // 并行处理每个批次
    std::vector<std::future<void>> futures;
    for (const auto& batch : batches) {
        futures.push_back(threadPool.submit([this, &batch]() {
            processDataBatch(batch);
        }));
    }
    
    // 等待所有批次处理完成
    for (auto& future : futures) {
        future.wait();
    }
    
    // 生成结果
    generateResults();
}

void OptimizedBacktestEngine::processDataBatch(const DataBatch& batch) {
    for (const auto& data : batch.marketData) {
        // 使用无锁队列分发事件
        eventQueue_->push(MarketEvent{data});
        
        // 并行更新订单簿
        updateOrderBookParallel(data.orderBookUpdates);
        
        // 并行处理订单
        processOrdersParallel(batch.orders);
    }
}

void OptimizedBacktestEngine::updateOrderBookParallel(
    const std::vector<OrderBookUpdate>& updates) {
    
    // 使用TBB并行处理订单簿更新
    tbb::parallel_for(tbb::blocked_range<size_t>(0, updates.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                const auto& update = updates[i];
                orderBook_->updateLevel(update);
            }
        });
}

void OptimizedBacktestEngine::processOrdersParallel(
    const std::vector<Order>& orders) {
    
    // 使用TBB并行处理订单
    tbb::parallel_for(tbb::blocked_range<size_t>(0, orders.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                processOrder(orders[i]);
            }
        });
}

void OptimizedBacktestEngine::processOrder(const Order& order) {
    // 使用内存池分配订单对象
    auto& orderPool = utils::PerformanceManager::getInstance()
                         .getMemoryPool<Order>();
    
    auto* newOrder = orderPool.construct(order);
    
    // 使用LRU缓存检查订单状态
    auto& orderCache = utils::PerformanceManager::getInstance()
                          .getCache<std::string, OrderStatus>();
    
    auto status = orderCache.get(order.id);
    if (status == OrderStatus::NONE) {
        // 新订单处理
        if (validateOrder(order)) {
            executeOrder(order);
            orderCache.put(order.id, OrderStatus::EXECUTED);
        }
    }
}

void OptimizedBacktestEngine::processEvents() {
    MarketEvent event;
    while (isRunning_ || !eventQueue_->empty()) {
        if (eventQueue_->try_pop(event)) {
            // 处理市场事件
            strategy_->onMarketData(event.data);
            
            // 更新性能指标
            updatePerformanceMetrics(event);
        }
    }
}

void OptimizedBacktestEngine::updatePerformanceMetrics(const MarketEvent& event) {
    auto now = std::chrono::high_resolution_clock::now();
    
    PerformanceMetrics metrics;
    metrics.processTime = std::chrono::duration_cast<std::chrono::microseconds>(
        now - event.timestamp).count();
    metrics.eventCount++;
    
    // 使用无锁容器存储性能指标
    metrics_.push_back(metrics);
}

void OptimizedBacktestEngine::generateResults() {
    BacktestResults results;
    
    // 并行计算统计指标
    tbb::parallel_invoke(
        [&]() { results.pnl = calculatePnL(); },
        [&]() { results.sharpeRatio = calculateSharpeRatio(); },
        [&]() { results.maxDrawdown = calculateMaxDrawdown(); }
    );
    
    // 生成报告
    reportGenerator_->generateReport(results);
}

} // namespace backtest
} // namespace hft
