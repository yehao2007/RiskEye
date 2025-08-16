#include "FeatureExtractor.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace hft {
namespace market {

FeatureExtractor::FeatureExtractor() {
}

void FeatureExtractor::initialize(const std::vector<FeatureConfig>& configs) {
    m_configs = configs;
    m_feature_names.clear();

    // 生成特征名称
    for (const auto& config : m_configs) {
        std::string name;
        switch (config.type) {
            case FeatureType::FEATURE_TYPE_PRICE:
                name = "price";
                break;
            case FeatureType::FEATURE_TYPE_VOLUME:
                name = "volume";
                break;
            case FeatureType::FEATURE_TYPE_SPREAD:
                name = "spread";
                break;
            case FeatureType::FEATURE_TYPE_DEPTH:
                name = "depth";
                break;
            case FeatureType::FEATURE_TYPE_VOLATILITY:
                name = "volatility";
                break;
            case FeatureType::FEATURE_TYPE_TREND:
                name = "trend";
                break;
            default:
                name = "unknown";
                break;
        }
        name += "_" + std::to_string(config.window_size);
        m_feature_names.push_back(name);
    }
}

std::vector<double> FeatureExtractor::extractFeatures(const OrderBook& order_book) {
    std::vector<double> features;

    for (const auto& config : m_configs) {
        switch (config.type) {
            case FeatureType::FEATURE_TYPE_PRICE:
                features.push_back(calculatePriceFeature(order_book, config));
                break;
            case FeatureType::FEATURE_TYPE_SPREAD:
                features.push_back(calculateSpreadFeature(order_book));
                break;
            case FeatureType::FEATURE_TYPE_DEPTH:
                features.push_back(calculateDepthFeature(order_book, config.window_size));
                break;
            default:
                features.push_back(0.0);
                break;
        }
    }

    return features;
}

std::vector<double> FeatureExtractor::extractFeatures(const MarketData& market_data) {
    std::vector<double> features;

    for (const auto& config : m_configs) {
        switch (config.type) {
            case FeatureType::FEATURE_TYPE_VOLUME:
                features.push_back(calculateVolumeFeature(market_data, config));
                break;
            default:
                features.push_back(0.0);
                break;
        }
    }

    return features;
}

std::vector<std::string> FeatureExtractor::getFeatureNames() const {
    return m_feature_names;
}

double FeatureExtractor::calculatePriceFeature(const OrderBook& order_book, const FeatureConfig& config) {
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }

    // 计算中间价
    double mid_price = (order_book.getAsks()[0].price + order_book.getBids()[0].price) / 2.0;
    return mid_price;
}

double FeatureExtractor::calculateVolumeFeature(const MarketData& market_data, const FeatureConfig& config) {
    // 计算指定窗口内的平均成交量
    uint32_t window_size = std::min(config.window_size, static_cast<uint32_t>(market_data.trades.size()));
    if (window_size == 0) {
        return 0.0;
    }

    double total_volume = 0.0;
    for (uint32_t i = 0; i < window_size; ++i) {
        total_volume += market_data.trades[market_data.trades.size() - 1 - i].size;
    }

    return total_volume / window_size;
}

double FeatureExtractor::calculateSpreadFeature(const OrderBook& order_book) {
    // 修改前
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        return 0.0;
    }
    
    // 修改后
    if (order_book.getAsks().empty() || order_book.getBids().empty()) {
        throw std::invalid_argument("Order book is empty");
        // 或使用 return std::nullopt; 并修改函数返回类型为 std::optional<double>
    }
    return order_book.getAsks()[0].price - order_book.getBids()[0].price;
}

double FeatureExtractor::calculateDepthFeature(const OrderBook& order_book, uint32_t levels) {
    double bid_depth = 0.0, ask_depth = 0.0;
    int bid_count = 0, ask_count = 0;

    for (const auto& bid : order_book.getBids()) {
        if (bid_count++ < levels) bid_depth += bid.size;
        else break;
    }
    for (const auto& ask : order_book.getAsks()) {
        if (ask_count++ < levels) ask_depth += ask.size;
        else break;
    }

    return std::min(bid_depth, ask_depth);
}

// 在类中添加成员变量
private:
    // 添加缓存
    mutable std::unordered_map<std::string, double> m_feature_cache;
    mutable uint64_t m_last_cache_timestamp = 0;

// 修改 calculateVolatilityFeature 方法
double FeatureExtractor::calculateVolatilityFeature(const std::vector<double>& prices, uint32_t window_size) {
    // 检查缓存
    std::string cache_key = "vol_" + std::to_string(window_size) + "_" + std::to_string(prices.size());
    if (m_feature_cache.find(cache_key) != m_feature_cache.end()) {
        return m_feature_cache[cache_key];
    }

    if (prices.size() < window_size + 1) {
        return 0.0;
    }

    std::vector<double> returns;
    for (size_t i = 1; i < window_size + 1; ++i) {
        double ret = log(prices[prices.size() - i] / prices[prices.size() - i - 1]);
        returns.push_back(ret);
    }

    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    for (double ret : returns) {
        variance += (ret - mean) * (ret - mean);
    }
    variance /= returns.size();

    return sqrt(variance);

    // 存入缓存
    m_feature_cache[cache_key] = result;
    return result;
}

double FeatureExtractor::calculateTrendFeature(const std::vector<double>& prices, uint32_t window_size) {
    // 使用加权线性回归，近期数据权重更高
    if (prices.size() < window_size) {
        return 0.0;
    }

    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0, sum_w = 0.0;
    for (uint32_t i = 0; i < window_size; ++i) {
        double x = i;
        double y = prices[prices.size() - window_size + i];
        double weight = (i + 1.0) / window_size;  // 线性加权

        sum_x += x * weight;
        sum_y += y * weight;
        sum_xy += x * y * weight;
        sum_x2 += x * x * weight;
        sum_w += weight;
    }

    double denominator = sum_w * sum_x2 - sum_x * sum_x;
    if (denominator == 0.0) {
        return 0.0;
    }

    double slope = (sum_w * sum_xy - sum_x * sum_y) / denominator;
    return slope;
}

} // namespace market
} // namespace hft