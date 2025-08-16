#include "IndexArbitrageModel.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"
#include "../execution/OrderExecution.h"
#include <algorithm>
#include <numeric>

namespace hft {
namespace strategy {

void IndexArbitrageModel::initialize(const std::vector<std::string>& indices) {
    // 初始化指数成分股信息
    for (const auto& index : indices) {
        loadIndexComponents(index);
        calculateWeights(index);
        updateIndexDivisor(index);
    }

    // 设置默认参数
    params_.min_tracking_error = 0.0001;    // 最小跟踪误差
    params_.min_spread = 0.0002;            // 最小价差
    params_.max_position_size = 1000000;    // 最大持仓规模
    params_.max_basket_deviation = 0.001;    // 最大篮子偏离度
}

std::vector<ArbitrageOpportunity> IndexArbitrageModel::findOpportunities(
    const std::vector<market::MarketData>& data) {
    
    std::vector<ArbitrageOpportunity> opportunities;
    
    // 对每个跟踪的指数进行分析
    for (const auto& [index_code, index_info] : index_data_) {
        // 计算指数理论价值
        double theo_value = calculateTheoreticalValue(index_code, data);
        
        // 获取指数实际价格
        double market_price = getIndexPrice(index_code, data);
        
        // 检查套利机会
        if (std::abs(market_price - theo_value) > params_.min_spread) {
            auto opp = createArbitrageOpportunity(
                index_code, market_price, theo_value, data);
            
            if (isValidOpportunity(opp)) {
                opportunities.push_back(std::move(opp));
            }
        }
    }
    
    return opportunities;
}

void IndexArbitrageModel::loadIndexComponents(const std::string& index_code) {
    try {
        // 从数据源加载指数成分股信息
        IndexData index_data;
        auto components = fetchIndexComponents(index_code);
        
        for (const auto& comp : components) {
            index_data.components.push_back(comp.code);
            index_data.weights.push_back(comp.weight);
        }
        
        // 计算指数因子
        index_data.divisor = calculateIndexDivisor(index_code, components);
        
        index_data_.emplace(index_code, std::move(index_data));
        
    } catch (const std::exception& e) {
        Logger::error("Failed to load index components for {}: {}", 
                     index_code, e.what());
    }
}

double IndexArbitrageModel::calculateTheoreticalValue(
    const std::string& index_code,
    const std::vector<market::MarketData>& data) {
    
    const auto& index_info = index_data_[index_code];
    double total_value = 0.0;
    
    // 计算成分股总市值
    for (size_t i = 0; i < index_info.components.size(); ++i) {
        const auto& comp_code = index_info.components[i];
        double comp_price = getComponentPrice(comp_code, data);
        double comp_shares = getComponentShares(comp_code);
        
        total_value += comp_price * comp_shares * index_info.weights[i];
    }
    
    // 使用指数因子计算理论价值
    return total_value / index_info.divisor;
}

ArbitrageOpportunity IndexArbitrageModel::createArbitrageOpportunity(
    const std::string& index_code,
    double market_price,
    double theo_value,
    const std::vector<market::MarketData>& data) {
    
    ArbitrageOpportunity opp;
    opp.type = ArbitrageType::INDEX;
    
    // 添加交易品种(指数和所有成分股)
    opp.instruments = {index_code};
    const auto& components = index_data_[index_code].components;
    opp.instruments.insert(
        opp.instruments.end(), components.begin(), components.end());
    
    // 计算预期收益
    opp.expected_profit = std::abs(market_price - theo_value) * 
                         calculateTradeSize(index_code);
    
    // 评估执行概率
    opp.execution_probability = calculateExecutionProbability(
        index_code, market_price, theo_value);
    
    // 计算风险分数
    opp.risk_score = calculateRiskScore(index_code, market_price, theo_value);
    
    // 估计执行时间窗口
    opp.time_window = estimateExecutionWindow(index_code);
    
    return opp;
}

double IndexArbitrageModel::calculateExecutionProbability(
    const std::string& index_code,
    double market_price,
    double theo_value) {
    
    // 基础概率(基于价差)
    double spread_prob = calculateSpreadProbability(market_price, theo_value);
    
    // 考虑成分股流动性
    double liquidity_factor = calculateBasketLiquidity(index_code);
    
    // 考虑跟踪误差
    double tracking_error = calculateTrackingError(index_code);
    
    // 考虑市场波动性
    double volatility_factor = calculateMarketVolatility(index_code);
    
    return spread_prob * liquidity_factor * (1 - tracking_error) * volatility_factor;
}

double IndexArbitrageModel::calculateRiskScore(
    const std::string& index_code,
    double market_price,
    double theo_value) {
    
    // 1. 基础风险(价差风险)
    double spread_risk = calculateSpreadRisk(market_price, theo_value);
    
    // 2. 流动性风险
    double liquidity_risk = calculateLiquidityRisk(index_code);
    
    // 3. 跟踪误差风险
    double tracking_risk = calculateTrackingRisk(index_code);
    
    // 4. 执行风险
    double execution_risk = calculateExecutionRisk(index_code);
    
    // 综合风险评分
    return (spread_risk * 0.3 + 
            liquidity_risk * 0.3 + 
            tracking_risk * 0.2 + 
            execution_risk * 0.2);
}

std::chrono::microseconds IndexArbitrageModel::estimateExecutionWindow(
    const std::string& index_code) {
    
    // 1. 基于成分股数量估计
    auto basket_window = estimateBasketExecutionTime(index_code);
    
    // 2. 考虑市场流动性
    auto liquidity_window = estimateLiquidityWindow(index_code);
    
    // 3. 考虑价格更新频率
    auto price_update_window = estimatePriceUpdateWindow(index_code);
    
    // 取最大值作为安全执行窗口
    return std::max({basket_window, liquidity_window, price_update_window});
}

bool IndexArbitrageModel::isValidOpportunity(
    const ArbitrageOpportunity& opp) {
    
    // 1. 检查预期收益
    if (opp.expected_profit < params_.min_profit_threshold) {
        return false;
    }
    
    // 2. 检查跟踪误差
    if (calculateTrackingError(opp) > params_.min_tracking_error) {
        return false;
    }
    
    // 3. 检查流动性条件
    if (!checkLiquidityConditions(opp)) {
        return false;
    }
    
    // 4. 检查风险限额
    if (!checkRiskLimits(opp)) {
        return false;
    }
    
    return true;
}

} // namespace strategy
} // namespace hft
