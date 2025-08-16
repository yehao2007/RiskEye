#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "market/MarketData.h"
#include "core/TimeManager.h"

namespace hft {
namespace market {

// 市场数据聚合器
class MarketDataAggregator {
public:
    MarketDataAggregator(core::TimeManager* time_manager);
    ~MarketDataAggregator() = default;

    // 初始化聚合器
    bool initialize();
    // 添加市场数据
    void addMarketData(const std::shared_ptr<MarketData>& data);
    // 获取聚合的市场数据
    std::shared_ptr<MarketData> getAggregatedData(const std::string& symbol);
    // 获取多个符号的聚合数据
    std::vector<std::shared_ptr<MarketData>> getAggregatedData(const std::vector<std::string>& symbols);
    // 获取所有符号的聚合数据
    std::vector<std::shared_ptr<MarketData>> getAllAggregatedData();
    // 设置聚合周期(毫秒)
    void setAggregationPeriod(uint32_t period_ms);
    // 清除聚合数据
    void clearAggregatedData();

private:
    core::TimeManager* m_time_manager;
    std::mutex m_mutex;
    uint32_t m_aggregation_period_ms;
    std::unordered_map<std::string, std::vector<std::shared_ptr<MarketData>>> m_data_buffers;
    std::unordered_map<std::string, std::shared_ptr<MarketData>> m_aggregated_data;

    // 聚合数据
    void aggregateData(const std::string& symbol);
    // 检查是否需要聚合
    bool needAggregation(const std::string& symbol);
};

} // namespace market
} // namespace hft
// market/MarketDataAggregator.h 改进
class MarketDataAggregator {
public:
    // 使用无锁队列提升性能
    void addData(const MarketData& data);
    
    // 批量处理以减少延迟
    void processBatch(size_t batchSize = 100);
    
    // 添加数据过滤
    void setFilter(std::function<bool(const MarketData&)> filter);
    
private:
    LockFreeQueue<MarketData> dataQueue_;
    // ... 其他成员
};