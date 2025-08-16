#include "LatencyArbitrageModel.h"
#include "../core/Logger.h"
#include "../core/TimeManager.h"
#include "../market/MarketData.h"
#include "../network/NetworkLatencyMonitor.h"
#include <algorithm>

namespace hft {
namespace strategy {

void LatencyArbitrageModel::initialize(
    const std::vector<std::string>& venues) {
    
    // 初始化延迟监控
    network::NetworkLatencyMonitor::getInstance().initialize(venues);
    
    // 设置默认参数
    params_.min_latency_diff_us = 100;      // 最小延迟差异(微秒)
    params_.max_latency_us = 1000;          // 最大可接受延迟(微秒)
    params_.min_price_diff = 0.0001;        // 最小价差
    params_.min_profit_threshold = 0.0002;   // 最小利润率
    
    // 初始化各交易场所延迟统计
    for (const auto& venue : venues) {
        venue_latencies_[venue] = {
            .mean_latency_us = measureVenueLatency(venue),
            .jitter_us = calculateLatencyJitter(venue),
            .is_reliable = checkVenueReliability(venue)
        };
    }
}

std::vector<ArbitrageOpportunity> LatencyArbitrageModel::findOpportunities(
    const std::vector<market::MarketData>& data) {
    
    std::vector<ArbitrageOpportunity> opportunities;
    
    // 按交易场所分组
    auto venue_data = groupByVenue(data);
    
    // 检查各交易场所间的延迟套利机会
    for (const auto& [venue1, data1] : venue_data) {
        for (const auto& [venue2, data2] : venue_data) {
            if (venue1 >= venue2) continue;
            
            checkLatencyArbitrage(venue1, data1, venue2, data2, opportunities);
        }
    }
    
    return opportunities;
}

void LatencyArbitrageModel::checkLatencyArbitrage(
    const std::string& venue1,
    const std::vector<market::MarketData>& data1,
    const std::string& venue2,
    const std::vector<market::MarketData>& data2,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    // 计算延迟差异
    double latency_diff = calculateLatencyDifference(venue1, venue2);
    if (latency_diff < params_.min_latency_diff_us) {
        return;
    }
    
    // 对每个共同交易的品种检查套利机会
    auto common_instruments = findCommonInstruments(data1, data2);
    for (const auto& inst : common_instruments) {
        auto price1 = getPrice(inst, data1);
        auto price2 = getPrice(inst, data2);
        
        // 计算价差
        double price_diff = std::abs(price1 - price2);
        if (price_diff < params_.min_price_diff) {
            continue;
        }
        
        // 创建套利机会
        auto opp = createArbitrageOpportunity(
            inst, venue1, venue2, price1, price2, latency_diff);
            
        if (isValidOpportunity(opp)) {
            opportunities.push_back(std::move(opp));
        }
    }
}

ArbitrageOpportunity LatencyArbitrageModel::createArbitrageOpportunity(
    const std::string& instrument,
    const std::string& venue1,
    const std::string& venue2,
    double price1,
    double price2,
    double latency_diff) {
    
    ArbitrageOpportunity opp;
    opp.type = ArbitrageType::LATENCY;
    opp.instruments = {instrument + "@" + venue1, instrument + "@" + venue2};
    
    // 计算预期收益
    opp.expected_profit = std::abs(price1 - price2);
    
    // 评估执行概率
    opp.execution_probability = calculateExecutionProbability(
        venue1, venue2, latency_diff);
    
    // 计算风险分数
    opp.risk_score = calculateRiskScore(
        venue1, venue2, latency_diff, price1, price2);
    
    // 估计执行时间窗口
    opp.time_window = estimateExecutionWindow(venue1, venue2, latency_diff);
    
    return opp;
}

double LatencyArbitrageModel::calculateLatencyDifference(
    const std::string& venue1,
    const std::string& venue2) {
    
    const auto& latency1 = venue_latencies_[venue1];
    const auto& latency2 = venue_latencies_[venue2];
    
    // 考虑平均延迟和抖动
    double effective_latency1 = latency1.mean_latency_us + latency1.jitter_us;
    double effective_latency2 = latency2.mean_latency_us + latency2.jitter_us;
    
    return std::abs(effective_latency1 - effective_latency2);
}

double LatencyArbitrageModel::calculateExecutionProbability(
    const std::string& venue1,
    const std::string& venue2,
    double latency_diff) {
    
    // 基础概率(基于延迟差异)
    double base_prob = calculateBaseLatencyProbability(latency_diff);
    
    // 考虑网络可靠性
    double reliability_factor = calculateReliabilityFactor(venue1, venue2);
    
    // 考虑市场流动性
    double liquidity_factor = calculateLiquidityFactor(venue1, venue2);
    
    // 考虑历史成功率
    double historical_factor = calculateHistoricalSuccessRate(venue1, venue2);
    
    return base_prob * reliability_factor * liquidity_factor * historical_factor;
}

double LatencyArbitrageModel::calculateRiskScore(
    const std::string& venue1,
    const std::string& venue2,
    double latency_diff,
    double price1,
    double price2) {
    
    // 1. 延迟风险
    double latency_risk = calculateLatencyRisk(latency_diff);
    
    // 2. 价格风险
    double price_risk = calculatePriceRisk(price1, price2);
    
    // 3. 执行风险
    double execution_risk = calculateExecutionRisk(venue1, venue2);
    
    // 4. 技术风险
    double technical_risk = calculateTechnicalRisk(venue1, venue2);
    
    // 综合风险评分
    return (latency_risk * 0.4 + 
            price_risk * 0.3 + 
            execution_risk * 0.2 + 
            technical_risk * 0.1);
}

std::chrono::microseconds LatencyArbitrageModel::estimateExecutionWindow(
    const std::string& venue1,
    const std::string& venue2,
    double latency_diff) {
    
    // 1. 基于延迟差异估计
    auto latency_window = std::chrono::microseconds(
        static_cast<int64_t>(latency_diff * 0.8));  // 80%的延迟差异时间
    
    // 2. 考虑市场更新频率
    auto market_update_window = getMarketUpdateInterval(venue1, venue2);
    
    // 3. 考虑网络抖动
    auto jitter_window = estimateJitterWindow(venue1, venue2);
    
    // 取最小值作为安全执行窗口
    return std::min({latency_window, market_update_window, jitter_window});
}

bool LatencyArbitrageModel::isValidOpportunity(
    const ArbitrageOpportunity& opp) {
    
    // 1. 检查预期收益是否达到最小阈值
    if (opp.expected_profit < params_.min_profit_threshold) {
        return false;
    }
    
    // 2. 检查执行概率是否足够高
    if (opp.execution_probability < 0.6) {  // 60%的最小执行概率
        return false;
    }
    
    // 3. 检查风险分数是否在可接受范围内
    if (opp.risk_score > 0.7) {  // 70%的最大风险容忍度
        return false;
    }
    
    // 4. 检查执行时间窗口是否足够
    if (opp.time_window < std::chrono::microseconds(100)) {  // 最小100微秒
        return false;
    }
    
    return true;
}

} // namespace strategy
} // namespace hft
