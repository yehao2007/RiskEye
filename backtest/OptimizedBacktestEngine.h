#pragma once
#include <vector>
#include <queue>
#include <memory>
#include "../core/Types.h"
#include "../strategy/AdvancedStrategy.h"
#include "../utils/PerformanceOptimization.h"

namespace hft {
namespace backtest {

class OptimizedBacktestEngine {
public:
    struct BacktestConfig {
        std::string dataPath;
        double initialCapital;
        std::string startDate;
        std::string endDate;
        bool useParallelProcessing;
        size_t batchSize;
        std::string outputPath;
    };
    
    void initialize(const BacktestConfig& config);
    void runBacktest(std::shared_ptr<strategy::AdvancedStrategy> strategy);
    
private:
    // 优化的数据结构
    struct OrderBook {
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
        
        void update(const market::L2Data& data) {
            // 使用内存池优化的更新逻辑
            auto& pool = utils::PerformanceManager::getInstance()
                            .getMemoryPool<PriceLevel>();
            // ...更新逻辑
        }
    };
    
    struct PositionManager {
        std::unordered_map<std::string, Position> positions;
        utils::LRUCache<std::string, double> priceCache;
        
        void updatePosition(const std::string& symbol,
                          double quantity, double price) {
            // 使用缓存优化的更新逻辑
            priceCache.put(symbol, price);
            // ...更新逻辑
        }
    };
    
    // 并行处理方法
    void processMarketDataBatch(const std::vector<market::MarketData>& batch);
    void processOrdersBatch(const std::vector<Order>& batch);
    
    // 优化的计算方法
    void updateMetrics();
    void checkRiskLimits();
    void generateReports();
    
    // 成员变量
    BacktestConfig config_;
    std::shared_ptr<strategy::AdvancedStrategy> strategy_;
    std::unique_ptr<OrderBook> orderBook_;
    std::unique_ptr<PositionManager> positionManager_;
    utils::ThreadPool& threadPool_;
    
    // 性能监控
    struct PerformanceMetrics {
        double processTime;
        double memoryUsage;
        int eventCount;
    };
    std::vector<PerformanceMetrics> metrics_;
};

// 优化的数据加载器
class OptimizedDataLoader {
public:
    void initialize(const std::string& dataPath);
    
    template<typename T>
    std::vector<T> loadBatch(size_t batchSize) {
        // 使用内存映射文件优化的批量加载
        return std::vector<T>();
    }
    
private:
    // 内存映射文件处理
    struct MemoryMappedFile {
        void* data;
        size_t size;
    };
    
    std::vector<MemoryMappedFile> mappedFiles_;
};

// 优化的结果分析器
class OptimizedResultAnalyzer {
public:
    void initialize();
    void processResults(const std::vector<Trade>& trades,
                       const std::vector<Position>& positions);
    void generateReport();
    
private:
    // 使用线程池的并行分析
    void analyzeTradesParallel(const std::vector<Trade>& trades);
    void analyzePositionsParallel(const std::vector<Position>& positions);
    
    utils::ThreadPool& threadPool_;
    std::vector<std::future<void>> analysisTasks_;
};

} // namespace backtest
} // namespace hft
