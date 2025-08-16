#include "FuturesSpotArbitrageModel.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"
#include "../pricing/FuturesPricing.h"
#include <algorithm>
#include <cmath>

namespace hft {
namespace strategy {

void FuturesSpotArbitrageModel::initialize(
    const std::vector<std::string>& futures) {
    
    // 初始化期货定价引擎
    pricing_engine_ = std::make_unique<pricing::FuturesPricingEngine>();
    
    // 加载期货合约信息
    for (const auto& future : futures) {
        loadFuturesInfo(future);
    }
    
    // 设置默认参数
    params_.min_basis = 0.0001;          // 最小基差
    params_.min_profit_threshold = 0.001; // 最小利润率0.1%
    params_.max_position_size = 1000;    // 最大持仓数量
    params_.max_holding_cost = 0.005;    // 最大持有成本0.5%
}

std::vector<ArbitrageOpportunity> FuturesSpotArbitrageModel::findOpportunities(
    const std::vector<market::MarketData>& data) {
    
    std::vector<ArbitrageOpportunity> opportunities;
    
    // 遍历所有期货合约
    for (const auto& [future_code, info] : futures_info_) {
        // 获取现货价格
        double spot_price = getSpotPrice(info.underlying, data);
        
        // 获取期货价格
        double futures_price = getFuturesPrice(future_code, data);
        
        // 计算理论基差
        double theo_basis = calculateTheoreticalBasis(
            spot_price, info.expiry, info.cost_of_carry);
        
        // 计算实际基差
        double market_basis = futures_price - spot_price;
        
        // 检查套利机会
        if (std::abs(market_basis - theo_basis) > params_.min_basis) {
            auto opp = createArbitrageOpportunity(
                future_code, info.underlying,
                futures_price, spot_price,
                market_basis, theo_basis);
                
            if (isValidOpportunity(opp)) {
                opportunities.push_back(std::move(opp));
            }
        }
    }
    
    return opportunities;
}

void FuturesSpotArbitrageModel::loadFuturesInfo(
    const std::string& future_code) {
    
    try {
        // 从数据源加载期货合约信息
        FuturesInfo info;
        auto contract_info = fetchFuturesContract(future_code);
        
        info.underlying = contract_info.underlying;
        info.expiry = contract_info.expiry;
        info.multiplier = contract_info.multiplier;
        info.margin_ratio = contract_info.margin_ratio;
        
        // 计算持有成本
        info.cost_of_carry = calculateCostOfCarry(
            info.underlying, info.expiry);
        
        futures_info_.emplace(future_code, std::move(info));
        
    } catch (const std::exception& e) {
        Logger::error("Failed to load futures info for {}: {}", 
                     future_code, e.what());
    }
}

double FuturesSpotArbitrageModel::calculateTheoreticalBasis(
    double spot_price,
    const std::chrono::system_clock::time_point& expiry,
    double cost_of_carry) {
    
    // 计算到期时间
    auto now = std::chrono::system_clock::now();
    double time_to_expiry = std::chrono::duration<double>(
        expiry - now).count() / 365.0;  // 年化
    
    // 使用持有成本模型计算理论基差
    return spot_price * (std::exp(cost_of_carry * time_to_expiry) - 1.0);
}

ArbitrageOpportunity FuturesSpotArbitrageModel::createArbitrageOpportunity(
    const std::string& future_code,
    const std::string& underlying,
    double futures_price,
    double spot_price,
    double market_basis,
    double theo_basis) {
    
    ArbitrageOpportunity opp;
    opp.type = ArbitrageType::FUTURES_SPOT;
    opp.instruments = {future_code, underlying};
    
    // 计算理论价差
    double basis_diff = market_basis - theo_basis;
    
    // 确定交易方向
    if (basis_diff > 0) {
        // 卖期货买现货
        opp.expected_profit = calculateLongBasisProfit(
            basis_diff, futures_price, spot_price);
    } else {
        // 买期货卖现货
        opp.expected_profit = calculateShortBasisProfit(
            basis_diff, futures_price, spot_price);
    }
    
    // 评估执行概率
    opp.execution_probability = calculateExecutionProbability(
        basis_diff, futures_price, spot_price);
    
    // 计算风险分数
    opp.risk_score = calculateRiskScore(
        basis_diff, futures_price, spot_price);
    
    // 估计执行时间窗口
    opp.time_window = estimateExecutionWindow(future_code, underlying);
    
    return opp;
}

double FuturesSpotArbitrageModel::calculateExecutionProbability(
    double basis_diff,
    double futures_price,
    double spot_price) {
    
    // 基础概率(基于基差偏离度)
    double basis_prob = calculateBasisProbability(basis_diff);
    
    // 考虑市场流动性
    double liquidity_factor = calculateLiquidityFactor(
        futures_price, spot_price);
    
    // 考虑持有成本
    double cost_factor = calculateCostFactor(basis_diff);
    
    // 考虑历史收敛概率
    double convergence_prob = calculateConvergenceProbability(basis_diff);
    
    return basis_prob * liquidity_factor * cost_factor * convergence_prob;
}

double FuturesSpotArbitrageModel::calculateRiskScore(
    double basis_diff,
    double futures_price,
    double spot_price) {
    
    // 1. 基差风险
    double basis_risk = calculateBasisRisk(basis_diff);
    
    // 2. 流动性风险
    double liquidity_risk = calculateLiquidityRisk(
        futures_price, spot_price);
    
    // 3. 持有成本风险
    double carrying_risk = calculateCarryingRisk(basis_diff);
    
    // 4. 交割风险
    double delivery_risk = calculateDeliveryRisk();
    
    // 综合风险评分
    return (basis_risk * 0.3 + 
            liquidity_risk * 0.3 + 
            carrying_risk * 0.2 + 
            delivery_risk * 0.2);
}

std::chrono::microseconds FuturesSpotArbitrageModel::estimateExecutionWindow(
    const std::string& future_code,
    const std::string& underlying) {
    
    // 1. 基于品种特性估计
    auto base_window = estimateBaseWindow(future_code, underlying);
    
    // 2. 考虑市场流动性
    auto liquidity_window = estimateLiquidityWindow(future_code, underlying);
    
    // 3. 考虑价格更新频率
    auto price_update_window = estimatePriceUpdateWindow(future_code, underlying);
    
    // 取最大值作为安全执行窗口
    return std::max({base_window, liquidity_window, price_update_window});
}

bool FuturesSpotArbitrageModel::isValidOpportunity(
    const ArbitrageOpportunity& opp) {
    
    // 1. 检查预期收益
    if (opp.expected_profit < params_.min_profit_threshold) {
        return false;
    }
    
    // 2. 检查持有成本
    if (!checkCarryingCost(opp)) {
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
