#include "StatisticalArbitrageModel.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"
#include "../ai/TensorFlowModel.h"
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

namespace hft {
namespace strategy {

StatisticalArbitrageModel::StatisticalArbitrageModel() {
    // 初始化深度学习模型
    initializeNeuralNetwork();
    
    // 设置默认参数
    params_.lookback_period = 100;          // 回看周期
    params_.confidence_level = 0.95;        // 置信水平
    params_.min_correlation = 0.8;          // 最小相关系数
    params_.zscore_threshold = 2.0;         // z分数阈值
    params_.min_profit_threshold = 0.001;   // 最小利润率0.1%
}

void StatisticalArbitrageModel::train(
    const std::vector<market::MarketData>& data) {
    
    // 1. 协整性分析
    findCointegrationPairs(data);
    
    // 2. 训练深度学习模型
    trainNeuralNetwork(data);
    
    // 3. 更新模型参数
    updateModelParameters(data);
}

std::vector<ArbitrageOpportunity> StatisticalArbitrageModel::findOpportunities(
    const std::vector<market::MarketData>& data) {
    
    std::vector<ArbitrageOpportunity> opportunities;
    
    // 对每个配对进行分析
    for (const auto& pair : trading_pairs_) {
        auto spread = calculateSpread(pair, data);
        auto zscore = calculateZScore(spread);
        
        // 检查是否存在套利机会
        if (std::abs(zscore) > params_.zscore_threshold) {
            auto opp = createArbitrageOpportunity(pair, zscore, data);
            if (isValidOpportunity(opp)) {
                opportunities.push_back(std::move(opp));
            }
        }
    }
    
    return opportunities;
}

void StatisticalArbitrageModel::findCointegrationPairs(
    const std::vector<market::MarketData>& data) {
    
    std::vector<std::string> instruments;
    for (const auto& md : data) {
        instruments.push_back(md.instrument_id);
    }
    
    // 对所有可能的配对进行协整性测试
    for (size_t i = 0; i < instruments.size(); ++i) {
        for (size_t j = i + 1; j < instruments.size(); ++j) {
            if (testCointegration(instruments[i], instruments[j], data)) {
                trading_pairs_.push_back({instruments[i], instruments[j]});
            }
        }
    }
}

bool StatisticalArbitrageModel::testCointegration(
    const std::string& inst1,
    const std::string& inst2,
    const std::vector<market::MarketData>& data) {
    
    // 1. 提取价格序列
    std::vector<double> prices1 = extractPrices(inst1, data);
    std::vector<double> prices2 = extractPrices(inst2, data);
    
    // 2. 计算相关系数
    double correlation = calculateCorrelation(prices1, prices2);
    if (correlation < params_.min_correlation) {
        return false;
    }
    
    // 3. 进行ADF测试
    double adf_stat = calculateADFStatistic(prices1, prices2);
    if (!isStationarySpread(adf_stat)) {
        return false;
    }
    
    // 4. 计算半衰期
    double half_life = calculateHalfLife(prices1, prices2);
    if (!isValidHalfLife(half_life)) {
        return false;
    }
    
    return true;
}

ArbitrageOpportunity StatisticalArbitrageModel::createArbitrageOpportunity(
    const TradingPair& pair,
    double zscore,
    const std::vector<market::MarketData>& data) {
    
    ArbitrageOpportunity opp;
    opp.type = ArbitrageType::STATISTICAL;
    opp.instruments = {pair.first, pair.second};
    
    // 计算预期收益
    double mean_reversion_target = calculateMeanReversionTarget(pair, data);
    opp.expected_profit = std::abs(zscore) * calculateVolatility(pair, data);
    
    // 评估执行概率
    opp.execution_probability = calculateExecutionProbability(zscore);
    
    // 计算风险分数
    opp.risk_score = calculateRiskScore(pair, zscore, data);
    
    // 估计执行时间窗口
    opp.time_window = estimateExecutionWindow(pair, zscore);
    
    return opp;
}

double StatisticalArbitrageModel::calculateSpread(
    const TradingPair& pair,
    const std::vector<market::MarketData>& data) {
    
    // 获取两个资产的最新价格
    double price1 = getLatestPrice(pair.first, data);
    double price2 = getLatestPrice(pair.second, data);
    
    // 使用协整系数计算价差
    double beta = pair_coefficients_[pair].beta;
    return price1 - beta * price2;
}

double StatisticalArbitrageModel::calculateZScore(double spread) {
    return (spread - spread_stats_.mean) / spread_stats_.std_dev;
}

double StatisticalArbitrageModel::calculateExecutionProbability(double zscore) {
    // 基于历史均值回归概率计算执行概率
    double mean_reversion_prob = calculateMeanReversionProbability(zscore);
    
    // 考虑市场流动性
    double liquidity_factor = calculateLiquidityFactor();
    
    // 考虑市场波动性
    double volatility_factor = calculateVolatilityFactor();
    
    return mean_reversion_prob * liquidity_factor * volatility_factor;
}

double StatisticalArbitrageModel::calculateRiskScore(
    const TradingPair& pair,
    double zscore,
    const std::vector<market::MarketData>& data) {
    
    // 1. 计算市场风险
    double market_risk = calculateMarketRisk(pair, data);
    
    // 2. 计算流动性风险
    double liquidity_risk = calculateLiquidityRisk(pair, data);
    
    // 3. 计算模型风险
    double model_risk = calculateModelRisk(zscore);
    
    // 4. 计算交易对手风险
    double counterparty_risk = calculateCounterpartyRisk(pair);
    
    // 综合风险评分
    return (market_risk * 0.3 + 
            liquidity_risk * 0.3 + 
            model_risk * 0.2 + 
            counterparty_risk * 0.2);
}

std::chrono::microseconds StatisticalArbitrageModel::estimateExecutionWindow(
    const TradingPair& pair,
    double zscore) {
    
    // 1. 基于均值回归速度估计
    auto mean_reversion_time = estimateMeanReversionTime(zscore);
    
    // 2. 考虑市场流动性条件
    auto liquidity_window = estimateLiquidityWindow(pair);
    
    // 3. 考虑市场波动性
    auto volatility_window = estimateVolatilityWindow(pair);
    
    // 取最小值作为安全执行窗口
    return std::min({mean_reversion_time, liquidity_window, volatility_window});
}

void StatisticalArbitrageModel::updateModelParameters(
    const std::vector<market::MarketData>& data) {
    
    // 1. 更新协整系数
    updateCointegrationCoefficients(data);
    
    // 2. 更新价差统计
    updateSpreadStatistics(data);
    
    // 3. 更新风险参数
    updateRiskParameters(data);
    
    // 4. 更新神经网络模型
    updateNeuralNetwork(data);
}

} // namespace strategy
} // namespace hft
