#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <torch/torch.h>
#include "AttentionTransformer.h"
#include "MarketState.h"

namespace hft {
namespace prediction {

struct MarketCorrelation {
    double correlation;    // 相关性系数
    double leadLag;       // 领先-滞后关系
    double informationShare; // 信息份额
    double spilloverEffect; // 溢出效应
};

class CrossMarketAnalyzer {
public:
    CrossMarketAnalyzer() = default;
    ~CrossMarketAnalyzer() = default;

    // 初始化分析器
    void initialize();

    // 分析两个市场之间的相关性
    MarketCorrelation analyzeCorrelation(
        const std::string& market1,
        const std::string& market2);

    // 找出领先市场
    std::vector<std::string> findLeadingMarkets();

    // 计算市场领导力得分
    double calculateLeadershipScore(const std::string& market);

    // 计算整体溢出效应
    double calculateSpilloverEffect();

private:
    // 计算注意力权重
    torch::Tensor calculateAttentionWeights(
        const std::vector<MarketState>& states);

    // 提取特征
    std::vector<double> extractFeatures(const MarketState& state);

    // 计算信息份额
    double calculateInformationShare(
        const MarketState& state1,
        const MarketState& state2);

    // 计算波动率溢出
    double calculateVolatilitySpillover(
        const MarketState& state1,
        const MarketState& state2);

private:
    std::unique_ptr<AttentionTransformer> crossAttention_;
    std::unique_ptr<MarketCorrelationCalculator> correlationCalculator_;
    std::map<std::string, MarketState> marketStates_;
    
    const int inputDim_ = 256;  // 输入维度
    const int numHeads_ = 8;    // 注意力头数
};

} // namespace prediction
} // namespace hft
