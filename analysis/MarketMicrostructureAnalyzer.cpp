#include "MarketMicrostructureAnalyzer.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"
#include "../ai/MLModels.h"
#include <algorithm>
#include <cmath>

namespace hft {
namespace analysis {

void MarketMicrostructureAnalyzer::initialize() {
    // 初始化机器学习模型
    ml_model_ = std::make_unique<ai::MLModel>();
    
    // 设置默认参数
    params_.tick_size = 0.01;              // 最小价格变动单位
    params_.lot_size = 100;                // 最小交易单位
    params_.analysis_window = 1000;        // 分析窗口大小
    params_.significance_level = 0.05;     // 显著性水平
}

MarketMicrostructure MarketMicrostructureAnalyzer::analyzeMarket(
    const std::vector<market::MarketData>& data) {
    
    MarketMicrostructure structure;
    
    try {
        // 1. 分析价格发现过程
        analyzePriceDiscovery(data, structure);
        
        // 2. 分析市场流动性
        analyzeLiquidity(data, structure);
        
        // 3. 分析交易成本
        analyzeTransactionCosts(data, structure);
        
        // 4. 分析市场效率
        analyzeMarketEfficiency(data, structure);
        
        // 5. 分析订单流特征
        analyzeOrderFlow(data, structure);
        
    } catch (const std::exception& e) {
        Logger::error("Market microstructure analysis failed: {}", e.what());
    }
    
    return structure;
}

void MarketMicrostructureAnalyzer::analyzePriceDiscovery(
    const std::vector<market::MarketData>& data,
    MarketMicrostructure& structure) {
    
    // 1. 计算价格效率
    structure.price_efficiency = calculatePriceEfficiency(data);
    
    // 2. 分析价格冲击
    structure.price_impact = analyzePriceImpact(data);
    
    // 3. 计算信息比率
    structure.information_ratio = calculateInformationRatio(data);
    
    // 4. 分析价格发现贡献
    structure.price_discovery = analyzePriceDiscoveryContribution(data);
}

void MarketMicrostructureAnalyzer::analyzeLiquidity(
    const std::vector<market::MarketData>& data,
    MarketMicrostructure& structure) {
    
    // 1. 计算买卖价差
    structure.liquidity.spread = calculateBidAskSpread(data);
    
    // 2. 计算市场深度
    structure.liquidity.depth = calculateMarketDepth(data);
    
    // 3. 分析弹性
    structure.liquidity.resilience = calculateMarketResilience(data);
    
    // 4. 计算成交量分布
    structure.liquidity.volume_profile = analyzeVolumeProfile(data);
}

void MarketMicrostructureAnalyzer::analyzeTransactionCosts(
    const std::vector<market::MarketData>& data,
    MarketMicrostructure& structure) {
    
    // 1. 计算显式成本
    structure.transaction_costs.explicit_costs = 
        calculateExplicitCosts(data);
    
    // 2. 估计隐含成本
    structure.transaction_costs.implicit_costs = 
        estimateImplicitCosts(data);
    
    // 3. 分析交易时机成本
    structure.transaction_costs.timing_costs = 
        analyzeTimingCosts(data);
    
    // 4. 评估流动性成本
    structure.transaction_costs.liquidity_costs = 
        assessLiquidityCosts(data);
}

void MarketMicrostructureAnalyzer::analyzeMarketEfficiency(
    const std::vector<market::MarketData>& data,
    MarketMicrostructure& structure) {
    
    // 1. 计算市场效率比率
    structure.efficiency.market_efficiency_ratio = 
        calculateMarketEfficiencyRatio(data);
    
    // 2. 分析价格序列
    structure.efficiency.price_series_analysis = 
        analyzePriceSeries(data);
    
    // 3. 检测市场异常
    structure.efficiency.market_anomalies = 
        detectMarketAnomalies(data);
    
    // 4. 评估信息效率
    structure.efficiency.information_efficiency = 
        assessInformationEfficiency(data);
}

void MarketMicrostructureAnalyzer::analyzeOrderFlow(
    const std::vector<market::MarketData>& data,
    MarketMicrostructure& structure) {
    
    // 1. 分析订单流毒性
    structure.order_flow.toxicity = calculateOrderFlowToxicity(data);
    
    // 2. 检测订单流失衡
    structure.order_flow.imbalance = detectOrderImbalance(data);
    
    // 3. 分析订单流模式
    structure.order_flow.patterns = analyzeOrderPatterns(data);
    
    // 4. 评估订单流预测能力
    structure.order_flow.predictability = 
        assessOrderFlowPredictability(data);
}

double MarketMicrostructureAnalyzer::calculatePriceEfficiency(
    const std::vector<market::MarketData>& data) {
    
    // 1. 计算随机游走偏差
    double random_walk_deviation = calculateRandomWalkDeviation(data);
    
    // 2. 计算方差比
    double variance_ratio = calculateVarianceRatio(data);
    
    // 3. 计算自相关系数
    double autocorrelation = calculateAutocorrelation(data);
    
    // 综合评分
    return combineEfficiencyMetrics(
        random_walk_deviation, variance_ratio, autocorrelation);
}

PriceImpact MarketMicrostructureAnalyzer::analyzePriceImpact(
    const std::vector<market::MarketData>& data) {
    
    PriceImpact impact;
    
    // 1. 计算临时性冲击
    impact.temporary = calculateTemporaryImpact(data);
    
    // 2. 计算永久性冲击
    impact.permanent = calculatePermanentImpact(data);
    
    // 3. 估计价格弹性
    impact.elasticity = estimatePriceElasticity(data);
    
    // 4. 计算冲击衰减
    impact.decay = calculateImpactDecay(data);
    
    return impact;
}

double MarketMicrostructureAnalyzer::calculateBidAskSpread(
    const std::vector<market::MarketData>& data) {
    
    // 1. 计算实际价差
    double quoted_spread = calculateQuotedSpread(data);
    
    // 2. 计算有效价差
    double effective_spread = calculateEffectiveSpread(data);
    
    // 3. 计算实现价差
    double realized_spread = calculateRealizedSpread(data);
    
    // 4. 计算加权平均价差
    return calculateWeightedSpread(
        quoted_spread, effective_spread, realized_spread);
}

OrderFlowToxicity MarketMicrostructureAnalyzer::calculateOrderFlowToxicity(
    const std::vector<market::MarketData>& data) {
    
    OrderFlowToxicity toxicity;
    
    // 1. 计算VPIN指标
    toxicity.vpin = calculateVPIN(data);
    
    // 2. 计算订单流失衡
    toxicity.order_imbalance = calculateOrderImbalance(data);
    
    // 3. 检测毒性聚集
    toxicity.toxicity_clusters = detectToxicityClusters(data);
    
    // 4. 评估信息泄漏
    toxicity.information_leakage = assessInformationLeakage(data);
    
    return toxicity;
}

} // namespace analysis
} // namespace hft
