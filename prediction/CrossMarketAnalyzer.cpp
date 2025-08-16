#include "EnhancedPredictor.h"
#include "../core/Logger.h"
#include <torch/torch.h>
#include <algorithm>

namespace hft {
namespace prediction {

void CrossMarketAnalyzer::initialize() {
    // 初始化注意力模型
    crossAttention_ = std::make_unique<AttentionTransformer>();
    crossAttention_->initialize(inputDim_, numHeads_);
    
    // 初始化市场状态缓存
    marketStates_.clear();
    
    // 初始化相关性计算器
    correlationCalculator_ = std::make_unique<MarketCorrelationCalculator>();
    
    Logger::info("Cross market analyzer initialized");
}

MarketCorrelation CrossMarketAnalyzer::analyzeCorrelation(
    const std::string& market1,
    const std::string& market2) {
    
    MarketCorrelation correlation;
    
    // 获取市场数据
    auto& state1 = marketStates_[market1];
    auto& state2 = marketStates_[market2];
    
    // 计算价格相关性
    correlation.correlation = calculatePriceCorrelation(
        state1.prices, state2.prices);
    
    // 计算领先-滞后关系
    correlation.leadLag = calculateLeadLag(
        state1.prices, state2.prices);
    
    // 计算信息份额
    correlation.informationShare = calculateInformationShare(
        state1, state2);
    
    // 计算溢出效应
    correlation.spilloverEffect = calculateSpilloverEffect(
        state1, state2);
    
    return correlation;
}

std::vector<std::string> CrossMarketAnalyzer::findLeadingMarkets() {
    std::vector<std::string> leadingMarkets;
    std::map<std::string, double> leadershipScores;
    
    // 计算每个市场的领导力得分
    for (const auto& [market, state] : marketStates_) {
        double score = calculateLeadershipScore(market);
        leadershipScores[market] = score;
    }
    
    // 选择得分最高的市场
    const int topN = 5;
    std::partial_sort_copy(
        leadershipScores.begin(),
        leadershipScores.end(),
        leadingMarkets.begin(),
        leadingMarkets.begin() + topN,
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        }
    );
    
    return leadingMarkets;
}

double CrossMarketAnalyzer::calculateLeadershipScore(
    const std::string& market) {
    
    double score = 0.0;
    
    // 信息领先性
    score += calculateInformationLeadership(market);
    
    // 价格发现贡献
    score += calculatePriceDiscoveryContribution(market);
    
    // 流动性影响
    score += calculateLiquidityImpact(market);
    
    // 交易量权重
    score *= calculateVolumeWeight(market);
    
    return score;
}

double CrossMarketAnalyzer::calculateSpilloverEffect() {
    // 使用GARCH模型计算波动率溢出
    std::vector<double> spillovers;
    
    for (const auto& [market1, state1] : marketStates_) {
        for (const auto& [market2, state2] : marketStates_) {
            if (market1 != market2) {
                double spillover = calculateVolatilitySpillover(
                    state1, state2);
                spillovers.push_back(spillover);
            }
        }
    }
    
    return std::accumulate(spillovers.begin(), spillovers.end(), 0.0) /
           spillovers.size();
}

torch::Tensor CrossMarketAnalyzer::calculateAttentionWeights(
    const std::vector<MarketState>& states) {
    
    // 准备输入数据
    std::vector<torch::Tensor> inputs;
    for (const auto& state : states) {
        auto features = extractFeatures(state);
        inputs.push_back(features);
    }
    
    // 计算注意力权重
    auto inputTensor = torch::stack(inputs);
    return crossAttention_->forward(inputTensor);
}

std::vector<double> CrossMarketAnalyzer::extractFeatures(
    const MarketState& state) {
    
    std::vector<double> features;
    
    // 价格特征
    features.push_back(calculatePriceFeatures(state));
    
    // 量特征
    features.push_back(calculateVolumeFeatures(state));
    
    // 波动率特征
    features.push_back(calculateVolatilityFeatures(state));
    
    // 订单簿特征
    auto bookFeatures = calculateOrderBookFeatures(state);
    features.insert(features.end(), 
                   bookFeatures.begin(), 
                   bookFeatures.end());
    
    return features;
}

double CrossMarketAnalyzer::calculateInformationShare(
    const MarketState& state1,
    const MarketState& state2) {
    
    // 计算协整关系
    auto cointegration = calculateCointegration(
        state1.prices, state2.prices);
    
    // 计算格兰杰因果关系
    auto grangerCausality = calculateGrangerCausality(
        state1.prices, state2.prices);
    
    // 计算信息份额
    return (cointegration + grangerCausality) / 2.0;
}

double CrossMarketAnalyzer::calculateVolatilitySpillover(
    const MarketState& state1,
    const MarketState& state2) {
    
    // 计算条件波动率
    auto volatility1 = calculateConditionalVolatility(state1);
    auto volatility2 = calculateConditionalVolatility(state2);
    
    // 计算溢出效应
    return calculateSpilloverCoefficient(volatility1, volatility2);
}

} // namespace prediction
} // namespace hft
