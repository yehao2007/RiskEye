#include "OptionArbitrageModel.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"
#include "../pricing/OptionPricing.h"
#include <algorithm>

namespace hft {
namespace strategy {

void OptionArbitrageModel::initialize(
    const std::vector<std::string>& underlyings) {
    
    // 初始化期权定价引擎
    pricing_engine_ = std::make_unique<pricing::OptionPricingEngine>();
    
    // 加载期权链
    for (const auto& underlying : underlyings) {
        loadOptionChain(underlying);
    }
    
    // 设置默认参数
    params_.min_price_diff = 0.0001;      // 最小价差
    params_.max_position_size = 100;       // 最大持仓数量
    params_.max_gamma = 100.0;            // 最大Gamma限制
    params_.max_vega = 1000.0;            // 最大Vega限制
}

std::vector<ArbitrageOpportunity> OptionArbitrageModel::findOpportunities(
    const std::vector<market::MarketData>& data) {
    
    std::vector<ArbitrageOpportunity> opportunities;
    
    // 1. 检查跨式套利机会(Put-Call Parity)
    findPutCallParityOpportunities(data, opportunities);
    
    // 2. 检查垂直套利机会(Vertical Spread)
    findVerticalSpreadOpportunities(data, opportunities);
    
    // 3. 检查跨期套利机会(Calendar Spread)
    findCalendarSpreadOpportunities(data, opportunities);
    
    // 4. 检查蝶式套利机会(Butterfly)
    findButterflyOpportunities(data, opportunities);
    
    return opportunities;
}

void OptionArbitrageModel::findPutCallParityOpportunities(
    const std::vector<market::MarketData>& data,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    for (const auto& [underlying, chain] : option_chains_) {
        double spot_price = getSpotPrice(underlying, data);
        double risk_free_rate = getRiskFreeRate();
        
        for (const auto& strike : chain.strikes) {
            for (const auto& expiry : chain.expiries) {
                // 获取看涨和看跌期权的市场价格
                double call_price = getOptionPrice(
                    underlying, strike, expiry, OptionType::CALL, data);
                double put_price = getOptionPrice(
                    underlying, strike, expiry, OptionType::PUT, data);
                
                // 计算理论价差
                double theo_diff = calculatePutCallParity(
                    spot_price, strike, risk_free_rate, expiry);
                
                // 检查实际价差是否存在套利机会
                double market_diff = call_price - put_price;
                if (std::abs(market_diff - theo_diff) > params_.min_price_diff) {
                    auto opp = createPutCallParityOpportunity(
                        underlying, strike, expiry,
                        call_price, put_price, theo_diff);
                    
                    if (isValidOpportunity(opp)) {
                        opportunities.push_back(std::move(opp));
                    }
                }
            }
        }
    }
}

void OptionArbitrageModel::findVerticalSpreadOpportunities(
    const std::vector<market::MarketData>& data,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    for (const auto& [underlying, chain] : option_chains_) {
        for (const auto& expiry : chain.expiries) {
            // 对相邻行权价进行分析
            for (size_t i = 0; i < chain.strikes.size() - 1; ++i) {
                double strike1 = chain.strikes[i];
                double strike2 = chain.strikes[i + 1];
                
                // 检查看涨垂直套利
                checkCallVerticalSpread(
                    underlying, expiry, strike1, strike2, data, opportunities);
                
                // 检查看跌垂直套利
                checkPutVerticalSpread(
                    underlying, expiry, strike1, strike2, data, opportunities);
            }
        }
    }
}

void OptionArbitrageModel::findCalendarSpreadOpportunities(
    const std::vector<market::MarketData>& data,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    for (const auto& [underlying, chain] : option_chains_) {
        for (const auto& strike : chain.strikes) {
            // 对相邻到期日进行分析
            for (size_t i = 0; i < chain.expiries.size() - 1; ++i) {
                auto expiry1 = chain.expiries[i];
                auto expiry2 = chain.expiries[i + 1];
                
                // 检查看涨跨期套利
                checkCallCalendarSpread(
                    underlying, strike, expiry1, expiry2, data, opportunities);
                
                // 检查看跌跨期套利
                checkPutCalendarSpread(
                    underlying, strike, expiry1, expiry2, data, opportunities);
            }
        }
    }
}

ArbitrageOpportunity OptionArbitrageModel::createArbitrageOpportunity(
    const std::string& underlying,
    const std::vector<OptionPosition>& positions,
    double expected_profit) {
    
    ArbitrageOpportunity opp;
    opp.type = ArbitrageType::OPTION;
    
    // 添加交易品种
    opp.instruments.push_back(underlying);
    for (const auto& pos : positions) {
        opp.instruments.push_back(generateOptionCode(pos));
    }
    
    // 设置预期收益
    opp.expected_profit = expected_profit;
    
    // 评估执行概率
    opp.execution_probability = calculateExecutionProbability(positions);
    
    // 计算风险分数
    opp.risk_score = calculateRiskScore(positions);
    
    // 估计执行时间窗口
    opp.time_window = estimateExecutionWindow(positions);
    
    return opp;
}

double OptionArbitrageModel::calculateRiskScore(
    const std::vector<OptionPosition>& positions) {
    
    // 1. Delta风险
    double delta_risk = calculateDeltaRisk(positions);
    
    // 2. Gamma风险
    double gamma_risk = calculateGammaRisk(positions);
    
    // 3. Vega风险
    double vega_risk = calculateVegaRisk(positions);
    
    // 4. Theta风险
    double theta_risk = calculateThetaRisk(positions);
    
    // 5. 流动性风险
    double liquidity_risk = calculateLiquidityRisk(positions);
    
    // 综合风险评分
    return (delta_risk * 0.25 + 
            gamma_risk * 0.25 + 
            vega_risk * 0.2 + 
            theta_risk * 0.15 + 
            liquidity_risk * 0.15);
}

std::chrono::microseconds OptionArbitrageModel::estimateExecutionWindow(
    const std::vector<OptionPosition>& positions) {
    
    // 1. 基于期权数量估计
    auto base_window = std::chrono::microseconds(
        positions.size() * 100);  // 每个期权100微秒
    
    // 2. 考虑市场流动性
    auto liquidity_window = estimateLiquidityWindow(positions);
    
    // 3. 考虑价格更新频率
    auto price_update_window = estimatePriceUpdateWindow(positions);
    
    // 取最大值作为安全执行窗口
    return std::max({base_window, liquidity_window, price_update_window});
}

bool OptionArbitrageModel::isValidOpportunity(
    const ArbitrageOpportunity& opp) {
    
    // 1. 检查预期收益
    if (opp.expected_profit < params_.min_profit_threshold) {
        return false;
    }
    
    // 2. 检查Greeks限制
    if (!checkGreeksLimits(opp)) {
        return false;
    }
    
    // 3. 检查流动性条件
    if (!checkLiquidityConditions(opp)) {
        return false;
    }
    
    // 4. 检查保证金要求
    if (!checkMarginRequirements(opp)) {
        return false;
    }
    
    return true;
}

} // namespace strategy
} // namespace hft
