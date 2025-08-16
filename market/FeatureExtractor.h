#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include "OrderBook.h"
#include "MarketData.h"

namespace hft {
namespace market {

// 特征类型
// 在 FeatureType 枚举中添加
enum class FeatureType {
    // 现有特征...
    FEATURE_TYPE_MACD,
    FEATURE_TYPE_RSI,
    FEATURE_TYPE_BOLLINGER_BANDS
};

// 添加相应的计算方法声明
double calculateMACD(const std::vector<double>& prices, int fast_period, int slow_period, int signal_period);
double calculateRSI(const std::vector<double>& prices, int period);
std::vector<double> calculateBollingerBands(const std::vector<double>& prices, int period, double std_dev);

// 特征配置
struct FeatureConfig {
    FeatureType type;
    uint32_t window_size;
    double threshold;
};

// 特征提取器
class FeatureExtractor {
public:
    FeatureExtractor();
    ~FeatureExtractor() override = default;

    // 初始化特征提取器
    void initialize(const std::vector<FeatureConfig>& configs);

    // 从订单簿提取特征
    std::vector<double> extractFeatures(const OrderBook& order_book);

    // 从市场数据提取特征
    std::vector<double> extractFeatures(const MarketData& market_data);

    // 获取特征名称
    std::vector<std::string> getFeatureNames() const;

private:
    std::vector<FeatureConfig> m_configs;
    std::vector<std::string> m_feature_names;

    // 计算价格特征
    double calculatePriceFeature(const OrderBook& order_book, const FeatureConfig& config);

    // 计算成交量特征
    double calculateVolumeFeature(const MarketData& market_data, const FeatureConfig& config);

    // 计算买卖价差特征
    double calculateSpreadFeature(const OrderBook& order_book);

    // 计算市场深度特征
    double calculateDepthFeature(const OrderBook& order_book, uint32_t levels);

    // 计算波动率特征
    double calculateVolatilityFeature(const std::vector<double>& prices, uint32_t window_size);

    // 计算趋势特征
    double calculateTrendFeature(const std::vector<double>& prices, uint32_t window_size);

    // 添加用于测试的友元类或方法
    #ifdef TESTING
    public:
        // 暴露内部方法用于测试
        friend class FeatureExtractorTest;
    #endif
};

using FeatureExtractorPtr = std::shared_ptr<FeatureExtractor>;

} // namespace market
} // namespace hft