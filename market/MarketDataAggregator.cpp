#include "MarketDataAggregator.h"
#include <iostream>
#include <algorithm>
#include <numeric>

namespace hft {
namespace market {

MarketDataAggregator::MarketDataAggregator(core::TimeManager* time_manager)
    : m_time_manager(time_manager),
      m_aggregation_period_ms(100) { // 默认100毫秒聚合一次
}

bool MarketDataAggregator::initialize() {
    std::cout << "MarketDataAggregator initialized with aggregation period: " << m_aggregation_period_ms << "ms" << std::endl;
    return true;
}

void MarketDataAggregator::addMarketData(const std::shared_ptr<MarketData>& data) {
    if (!data) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    const std::string& symbol = data->symbol;

    // 添加到缓冲区
    m_data_buffers[symbol].push_back(data);

    // 检查是否需要聚合
    if (needAggregation(symbol)) {
        aggregateData(symbol);
    }
}

std::shared_ptr<MarketData> MarketDataAggregator::getAggregatedData(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_aggregated_data.find(symbol);
    if (it == m_aggregated_data.end()) {
        return nullptr;
    }

    return it->second;
}

std::vector<std::shared_ptr<MarketData>> MarketDataAggregator::getAggregatedData(const std::vector<std::string>& symbols) {
    std::vector<std::shared_ptr<MarketData>> result;
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& symbol : symbols) {
        auto it = m_aggregated_data.find(symbol);
        if (it != m_aggregated_data.end()) {
            result.push_back(it->second);
        }
    }

    return result;
}

std::vector<std::shared_ptr<MarketData>> MarketDataAggregator::getAllAggregatedData() {
    std::vector<std::shared_ptr<MarketData>> result;
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& [_, data] : m_aggregated_data) {
        result.push_back(data);
    }

    return result;
}

void MarketDataAggregator::setAggregationPeriod(uint32_t period_ms) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_aggregation_period_ms = period_ms;
    std::cout << "Aggregation period set to: " << m_aggregation_period_ms << "ms" << std::endl;
}

void MarketDataAggregator::clearAggregatedData() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data_buffers.clear();
    m_aggregated_data.clear();
    std::cout << "Aggregated data cleared" << std::endl;
}

void MarketDataAggregator::aggregateData(const std::string& symbol) {
    auto it = m_data_buffers.find(symbol);
    if (it == m_data_buffers.end() || it->second.empty()) {
        return;
    }

    const auto& data_buffer = it->second;

    // 创建聚合数据
    auto aggregated = std::make_shared<MarketData>();
    aggregated->symbol = symbol;
    aggregated->timestamp = m_time_manager->getCurrentTimestamp();

    // 计算平均值
    double avg_price = 0.0;
    double avg_volume = 0.0;
    double avg_best_bid = 0.0;
    double avg_best_ask = 0.0;
    double sum_volume = 0.0;

    for (const auto& data : data_buffer) {
        avg_price += data->last_price * data->volume;
        avg_volume += data->volume;
        avg_best_bid += data->best_bid;
        avg_best_ask += data->best_ask;
        sum_volume += data->volume;

        // 保留最高买价和最低卖价
        if (data->best_bid > aggregated->best_bid) {
            aggregated->best_bid = data->best_bid;
        }
        if (data->best_ask < aggregated->best_ask || aggregated->best_ask == 0) {
            aggregated->best_ask = data->best_ask;
        }

        // 保留最高和最低价格
        if (data->last_price > aggregated->high || aggregated->high == 0) {
            aggregated->high = data->last_price;
        }
        if (data->last_price < aggregated->low || aggregated->low == 0) {
            aggregated->low = data->last_price;
        }
    }

    // 计算成交量加权平均价格
    aggregated->last_price = avg_price / sum_volume;
    aggregated->volume = avg_volume;
    aggregated->best_bid = avg_best_bid / data_buffer.size();
    aggregated->best_ask = avg_best_ask / data_buffer.size();

    // 开盘价和收盘价
    aggregated->open = data_buffer.front()->last_price;
    aggregated->close = data_buffer.back()->last_price;

    // 更新聚合数据
    m_aggregated_data[symbol] = aggregated;

    // 清空缓冲区
    m_data_buffers[symbol].clear();

    std::cout << "Aggregated data for " << symbol << ", price: " << aggregated->last_price << ", volume: " << aggregated->volume << std::endl;
}

bool MarketDataAggregator::needAggregation(const std::string& symbol) {
    auto it = m_data_buffers.find(symbol);
    if (it == m_data_buffers.end() || it->second.empty()) {
        return false;
    }

    uint64_t current_time = m_time_manager->getCurrentTimestamp();
    uint64_t first_data_time = it->second.front()->timestamp;

    return (current_time - first_data_time) >= m_aggregation_period_ms * 1000; // 转换为微秒
}

} // namespace market
} // namespace hft