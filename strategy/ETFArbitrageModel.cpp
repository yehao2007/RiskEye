#include "ETFArbitrageModel.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"
#include <algorithm>
#include <numeric>

namespace hft {
namespace strategy {

void ETFArbitrageModel::initialize(const std::vector<std::string>& etfs) {
    for (const auto& etf : etfs) {
        loadETFComponents(etf);
        calculateWeights(etf);
        updateCreationUnitSize(etf);
    }
}

std::vector<ArbitrageOpportunity> ETFArbitrageModel::findOpportunities(
    const std::vector<market::MarketData>& data) {
    
    std::vector<ArbitrageOpportunity> opportunities;
    
    for (const auto& [etf_code, etf_info] : etf_data_) {
        // 获取ETF当前价格
        double etf_price = getETFPrice(etf_code, data);
        
        // 计算成分股投资组合价值
        double nav = calculateNAV(etf_code, data);
        
        // 检查套利机会
        checkCreationArbitrage(etf_code, etf_price, nav, opportunities);
        checkRedemptionArbitrage(etf_code, etf_price, nav, opportunities);
    }
    
    return opportunities;
}

void ETFArbitrageModel::loadETFComponents(const std::string& etf_code) {
    try {
        // 从数据源加载ETF成分股信息
        ETFData etf_data;
        auto components = fetchETFComponents(etf_code);
        
        for (const auto& comp : components) {
            etf_data.components.push_back(comp.code);
            etf_data.weights.push_back(comp.weight);
        }
        
        etf_data_.emplace(etf_code, std::move(etf_data));
        
    } catch (const std::exception& e) {
        Logger::error("Failed to load ETF components for {}: {}", 
                     etf_code, e.what());
    }
}

double ETFArbitrageModel::calculateNAV(
    const std::string& etf_code,
    const std::vector<market::MarketData>& data) {
    
    const auto& etf_info = etf_data_[etf_code];
    double nav = 0.0;
    
    for (size_t i = 0; i < etf_info.components.size(); ++i) {
        const auto& comp_code = etf_info.components[i];
        double comp_price = getComponentPrice(comp_code, data);
        nav += comp_price * etf_info.weights[i];
    }
    
    return nav;
}

void ETFArbitrageModel::checkCreationArbitrage(
    const std::string& etf_code,
    double etf_price,
    double nav,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    const double CREATION_THRESHOLD = 0.002; // 0.2%的套利空间
    
    // 如果ETF价格显著高于NAV，可以通过申购ETF获利
    if ((etf_price - nav) / nav > CREATION_THRESHOLD) {
        ArbitrageOpportunity opp;
        opp.type = ArbitrageType::ETF;
        opp.instruments = {etf_code};
        opp.instruments.insert(opp.instruments.end(),
                             etf_data_[etf_code].components.begin(),
                             etf_data_[etf_code].components.end());
        
        // 计算预期收益
        opp.expected_profit = (etf_price - nav) * etf_data_[etf_code].creation_unit_size;
        
        // 评估执行概率
        opp.execution_probability = calculateExecutionProbability(opp);
        
        // 计算风险分数
        opp.risk_score = calculateRiskScore(opp);
        
        // 估计执行时间窗口
        opp.time_window = estimateExecutionWindow(opp);
        
        opportunities.push_back(std::move(opp));
    }
}

void ETFArbitrageModel::checkRedemptionArbitrage(
    const std::string& etf_code,
    double etf_price,
    double nav,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    const double REDEMPTION_THRESHOLD = 0.002; // 0.2%的套利空间
    
    // 如果ETF价格显著低于NAV，可以通过赎回ETF获利
    if ((nav - etf_price) / nav > REDEMPTION_THRESHOLD) {
        ArbitrageOpportunity opp;
        opp.type = ArbitrageType::ETF;
        opp.instruments = {etf_code};
        
        // 计算预期收益
        opp.expected_profit = (nav - etf_price) * etf_data_[etf_code].creation_unit_size;
        
        // 评估执行概率
        opp.execution_probability = calculateExecutionProbability(opp);
        
        // 计算风险分数
        opp.risk_score = calculateRiskScore(opp);
        
        // 估计执行时间窗口
        opp.time_window = estimateExecutionWindow(opp);
        
        opportunities.push_back(std::move(opp));
    }
}

double ETFArbitrageModel::calculateExecutionProbability(
    const ArbitrageOpportunity& opp) {
    
    // 考虑以下因素：
    // 1. 市场流动性
    double liquidity_score = assessLiquidity(opp);
    
    // 2. 价格稳定性
    double price_stability = assessPriceStability(opp);
    
    // 3. 历史成功率
    double historical_success = getHistoricalSuccessRate(opp);
    
    // 综合评分
    return (liquidity_score * 0.4 + 
            price_stability * 0.3 + 
            historical_success * 0.3);
}

double ETFArbitrageModel::calculateRiskScore(
    const ArbitrageOpportunity& opp) {
    
    // 考虑以下风险因素：
    // 1. 市场风险
    double market_risk = assessMarketRisk(opp);
    
    // 2. 流动性风险
    double liquidity_risk = assessLiquidityRisk(opp);
    
    // 3. 交易对手风险
    double counterparty_risk = assessCounterpartyRisk(opp);
    
    // 4. 操作风险
    double operational_risk = assessOperationalRisk(opp);
    
    // 综合风险评分
    return (market_risk * 0.3 + 
            liquidity_risk * 0.3 + 
            counterparty_risk * 0.2 + 
            operational_risk * 0.2);
}

std::chrono::microseconds ETFArbitrageModel::estimateExecutionWindow(
    const ArbitrageOpportunity& opp) {
    
    // 考虑以下因素：
    // 1. 市场波动性
    auto volatility_window = estimateVolatilityWindow(opp);
    
    // 2. 流动性条件
    auto liquidity_window = estimateLiquidityWindow(opp);
    
    // 3. 历史执行时间
    auto historical_window = getHistoricalExecutionWindow(opp);
    
    // 取最小值作为安全窗口
    return std::min({volatility_window, liquidity_window, historical_window});
}

} // namespace strategy
} // namespace hft
