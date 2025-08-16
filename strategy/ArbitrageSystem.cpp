#include "ArbitrageSystem.h"
#include "../core/Logger.h"
#include "../execution/OrderExecution.h"
#include "../market/MarketData.h"
#include "../risk/RiskManager.h"
#include <algorithm>
#include <cmath>

namespace hft {
namespace strategy {

bool ArbitrageSystem::initialize() {
    // 初始化各个套利模型
    stat_arb_model_ = std::make_unique<StatisticalArbitrageModel>();
    latency_arb_model_ = std::make_unique<LatencyArbitrageModel>();
    etf_arb_model_ = std::make_unique<ETFArbitrageModel>();

    // 设置默认配置
    config_.min_profit_threshold = 0.0001;  // 最小利润率 0.01%
    config_.max_risk_threshold = 0.05;      // 最大风险度 5%
    config_.max_concurrent_trades = 10;      // 最大并发交易数
    config_.max_execution_time = std::chrono::microseconds(1000); // 1ms执行时间限制

    return true;
}

void ArbitrageSystem::detectStatisticalArbitrage(
    const std::vector<market::MarketData>& data,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    auto stat_opportunities = stat_arb_model_->findOpportunities(data);
    
    // 过滤掉不满足最小利润要求的机会
    auto it = std::remove_if(stat_opportunities.begin(), stat_opportunities.end(),
        [this](const ArbitrageOpportunity& opp) {
            return opp.expected_profit < config_.min_profit_threshold;
        });
    stat_opportunities.erase(it, stat_opportunities.end());
    
    opportunities.insert(opportunities.end(), 
                        stat_opportunities.begin(), 
                        stat_opportunities.end());
}

void ArbitrageSystem::detectLatencyArbitrage(
    const std::vector<market::MarketData>& data,
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    auto latency_opportunities = latency_arb_model_->findOpportunities(data);
    
    // 过滤掉执行时间窗口过短的机会
    auto it = std::remove_if(latency_opportunities.begin(), latency_opportunities.end(),
        [this](const ArbitrageOpportunity& opp) {
            return opp.time_window < config_.max_execution_time;
        });
    latency_opportunities.erase(it, latency_opportunities.end());
    
    opportunities.insert(opportunities.end(), 
                        latency_opportunities.begin(), 
                        latency_opportunities.end());
}

void ArbitrageSystem::filterOpportunities(
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    // 移除风险过高的机会
    auto it = std::remove_if(opportunities.begin(), opportunities.end(),
        [this](const ArbitrageOpportunity& opp) {
            return calculateExecutionRisk(opp) > config_.max_risk_threshold;
        });
    opportunities.erase(it, opportunities.end());
}

void ArbitrageSystem::rankOpportunities(
    std::vector<ArbitrageOpportunity>& opportunities) {
    
    // 按照预期收益率和执行概率排序
    std::sort(opportunities.begin(), opportunities.end(),
        [this](const ArbitrageOpportunity& a, const ArbitrageOpportunity& b) {
            double score_a = a.expected_profit * a.execution_probability;
            double score_b = b.expected_profit * b.execution_probability;
            return score_a > score_b;
        });
}

ArbitrageSystem::ArbitrageResult 
ArbitrageSystem::executeArbitrage(const ArbitrageOpportunity& opportunity) {
    ArbitrageResult result;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 检查是否超过并发交易限制
    if (state_.active_trades >= config_.max_concurrent_trades) {
        result.success = false;
        return result;
    }
    
    // 增加活跃交易计数
    state_.active_trades++;
    
    try {
        // 执行套利交易
        for (const auto& instrument : opportunity.instruments) {
            // 创建并发送订单
            execution::Order order;
            order.instrument = instrument;
            order.type = execution::OrderType::IOC; // 立即成交否则撤销
            
            auto exec_result = execution::OrderExecution::getInstance()
                                 .executeOrder(order);
            
            if (exec_result.status == execution::OrderStatus::FILLED) {
                result.executed_orders.push_back(instrument);
            }
        }
        
        // 计算实际利润
        auto end_time = std::chrono::high_resolution_clock::now();
        result.execution_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
            
        // 更新统计信息
        if (!result.executed_orders.empty()) {
            state_.successful_trades++;
            state_.total_profit += result.realized_profit;
        }
        
        result.success = true;
        
    } catch (const std::exception& e) {
        Logger::error("Arbitrage execution failed: {}", e.what());
        result.success = false;
    }
    
    // 减少活跃交易计数
    state_.active_trades--;
    
    return result;
}

ArbitrageSystem::RiskMetrics 
ArbitrageSystem::calculateRisk(const ArbitrageOpportunity& opportunity) {
    RiskMetrics metrics;
    
    // 计算95% VaR
    metrics.var_95 = calculateVar95(opportunity);
    
    // 计算最大回撤
    metrics.max_drawdown = calculateMaxDrawdown(opportunity);
    
    // 计算夏普比率
    metrics.sharpe_ratio = calculateSharpeRatio(opportunity);
    
    // 计算相关性风险
    metrics.correlation_risk = calculateCorrelationRisk(opportunity);
    
    return metrics;
}

double ArbitrageSystem::calculateProfitability(
    const ArbitrageOpportunity& opportunity) {
    
    // 考虑交易成本
    double total_cost = calculateTradingCost(opportunity);
    
    // 计算净收益
    double net_profit = opportunity.expected_profit - total_cost;
    
    // 计算收益率
    return net_profit / calculateRequiredCapital(opportunity);
}

double ArbitrageSystem::calculateExecutionRisk(
    const ArbitrageOpportunity& opportunity) {
    
    // 计算市场冲击成本
    double market_impact = calculateMarketImpact(opportunity);
    
    // 计算流动性风险
    double liquidity_risk = calculateLiquidityRisk(opportunity);
    
    // 计算执行延迟风险
    double latency_risk = calculateLatencyRisk(opportunity);
    
    // 综合风险评分
    return market_impact + liquidity_risk + latency_risk;
}

// StatisticalArbitrageModel实现
void ArbitrageSystem::StatisticalArbitrageModel::train(
    const std::vector<market::MarketData>& data) {
    
    // 使用协整性分析找出相关性强的资产对
    findCointegrationPairs(data);
    
    // 训练神经网络模型
    trainNeuralNetwork(data);
}

// LatencyArbitrageModel实现
void ArbitrageSystem::LatencyArbitrageModel::initialize(
    const std::vector<std::string>& venues) {
    
    // 初始化各交易场所的延迟统计信息
    for (const auto& venue : venues) {
        venue_latencies_[venue] = {
            .mean_latency_us = measureVenueLatency(venue),
            .jitter_us = calculateLatencyJitter(venue),
            .is_reliable = checkVenueReliability(venue)
        };
    }
}

} // namespace strategy
} // namespace hft
