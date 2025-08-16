#include "StrategyPortfolioOptimizer.h"
#include "../core/Logger.h"
#include "../risk/RiskManager.h"
#include "../ai/MLModels.h"
#include <Eigen/Dense>
#include <algorithm>

namespace hft {
namespace strategy {

void StrategyPortfolioOptimizer::initialize() {
    // 初始化机器学习模型
    ml_model_ = std::make_unique<ai::MLModel>();
    
    // 设置默认参数
    params_.max_allocation_per_strategy = 0.3;  // 单个策略最大配置30%
    params_.min_sharpe_ratio = 1.5;            // 最小夏普比率
    params_.max_drawdown = 0.1;                // 最大回撤10%
    params_.risk_free_rate = 0.02;            // 无风险利率2%
    params_.rebalance_threshold = 0.05;        // 再平衡阈值5%
}

PortfolioAllocation StrategyPortfolioOptimizer::optimizeAllocation(
    const std::vector<StrategyMetrics>& strategies,
    const MarketCondition& market_condition) {
    
    PortfolioAllocation allocation;
    
    try {
        // 1. 计算策略相关性矩阵
        Eigen::MatrixXd correlation_matrix = calculateCorrelationMatrix(strategies);
        
        // 2. 计算风险调整后收益
        std::vector<double> risk_adjusted_returns = 
            calculateRiskAdjustedReturns(strategies);
        
        // 3. 考虑市场条件调整
        adjustForMarketCondition(risk_adjusted_returns, market_condition);
        
        // 4. 执行投资组合优化
        allocation = optimizePortfolio(
            strategies, correlation_matrix, risk_adjusted_returns);
        
        // 5. 应用约束条件
        applyConstraints(allocation);
        
        // 6. 计算组合风险指标
        calculatePortfolioMetrics(allocation);
        
    } catch (const std::exception& e) {
        Logger::error("Portfolio optimization failed: {}", e.what());
    }
    
    return allocation;
}

void StrategyPortfolioOptimizer::updateAllocation(
    PortfolioAllocation& allocation,
    const std::vector<StrategyMetrics>& current_metrics) {
    
    try {
        // 1. 检查是否需要再平衡
        if (!needRebalancing(allocation, current_metrics)) {
            return;
        }
        
        // 2. 计算目标配置
        auto target_allocation = calculateTargetAllocation(
            allocation, current_metrics);
        
        // 3. 生成再平衡计划
        auto rebalance_plan = generateRebalancePlan(
            allocation, target_allocation);
        
        // 4. 执行再平衡
        executeRebalance(allocation, rebalance_plan);
        
    } catch (const std::exception& e) {
        Logger::error("Portfolio rebalancing failed: {}", e.what());
    }
}

Eigen::MatrixXd StrategyPortfolioOptimizer::calculateCorrelationMatrix(
    const std::vector<StrategyMetrics>& strategies) {
    
    size_t n = strategies.size();
    Eigen::MatrixXd correlation = Eigen::MatrixXd::Identity(n, n);
    
    // 计算策略间的相关性
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            double corr = calculateStrategyCorrelation(
                strategies[i], strategies[j]);
            correlation(i, j) = corr;
            correlation(j, i) = corr;
        }
    }
    
    return correlation;
}

std::vector<double> StrategyPortfolioOptimizer::calculateRiskAdjustedReturns(
    const std::vector<StrategyMetrics>& strategies) {
    
    std::vector<double> returns;
    returns.reserve(strategies.size());
    
    for (const auto& strategy : strategies) {
        // 计算夏普比率
        double sharpe = (strategy.annual_return - params_.risk_free_rate) / 
                       strategy.annual_volatility;
                       
        // 计算索提诺比率
        double sortino = (strategy.annual_return - params_.risk_free_rate) / 
                        strategy.downside_deviation;
                        
        // 计算calmar比率
        double calmar = strategy.annual_return / strategy.max_drawdown;
        
        // 综合评分
        double score = (sharpe * 0.4 + sortino * 0.3 + calmar * 0.3);
        
        returns.push_back(score);
    }
    
    return returns;
}

void StrategyPortfolioOptimizer::adjustForMarketCondition(
    std::vector<double>& returns,
    const MarketCondition& market_condition) {
    
    // 使用机器学习模型预测市场环境对各策略的影响
    auto market_impact = ml_model_->predictMarketImpact(
        market_condition, returns);
    
    // 调整风险调整后收益
    for (size_t i = 0; i < returns.size(); ++i) {
        returns[i] *= market_impact[i];
    }
}

PortfolioAllocation StrategyPortfolioOptimizer::optimizePortfolio(
    const std::vector<StrategyMetrics>& strategies,
    const Eigen::MatrixXd& correlation,
    const std::vector<double>& returns) {
    
    PortfolioAllocation allocation;
    size_t n = strategies.size();
    
    // 设置优化问题
    Eigen::VectorXd w(n);  // 权重向量
    Eigen::VectorXd r = Eigen::Map<const Eigen::VectorXd>(
        returns.data(), returns.size());  // 收益向量
    
    // 最大化风险调整后收益
    // 使用二次规划求解
    optimization::QuadraticProgramming qp;
    qp.setObjective(correlation, r);
    qp.addConstraints(createConstraints(n));
    
    // 求解优化问题
    auto result = qp.solve();
    
    // 设置分配结果
    for (size_t i = 0; i < n; ++i) {
        allocation.weights[strategies[i].id] = result[i];
    }
    
    return allocation;
}

void StrategyPortfolioOptimizer::applyConstraints(
    PortfolioAllocation& allocation) {
    
    // 1. 检查单个策略限制
    for (auto& [id, weight] : allocation.weights) {
        if (weight > params_.max_allocation_per_strategy) {
            weight = params_.max_allocation_per_strategy;
        }
    }
    
    // 2. 确保权重和为1
    normalizeWeights(allocation);
    
    // 3. 检查风险限制
    checkRiskLimits(allocation);
}

void StrategyPortfolioOptimizer::calculatePortfolioMetrics(
    PortfolioAllocation& allocation) {
    
    // 1. 计算组合预期收益
    allocation.metrics.expected_return = calculatePortfolioReturn(allocation);
    
    // 2. 计算组合风险
    allocation.metrics.volatility = calculatePortfolioRisk(allocation);
    
    // 3. 计算风险调整后收益指标
    allocation.metrics.sharpe_ratio = calculatePortfolioSharpe(allocation);
    
    // 4. 计算VaR和ES
    calculatePortfolioRiskMetrics(allocation);
}

bool StrategyPortfolioOptimizer::needRebalancing(
    const PortfolioAllocation& current,
    const std::vector<StrategyMetrics>& metrics) {
    
    // 1. 检查偏离度
    if (calculateDeviationScore(current, metrics) > 
        params_.rebalance_threshold) {
        return true;
    }
    
    // 2. 检查风险指标变化
    if (hasSignificantRiskChange(current, metrics)) {
        return true;
    }
    
    // 3. 检查相关性变化
    if (hasSignificantCorrelationChange(metrics)) {
        return true;
    }
    
    return false;
}

void StrategyPortfolioOptimizer::executeRebalance(
    PortfolioAllocation& allocation,
    const RebalancePlan& plan) {
    
    // 1. 验证再平衡计划
    if (!validateRebalancePlan(plan)) {
        throw std::runtime_error("Invalid rebalance plan");
    }
    
    // 2. 执行调整
    for (const auto& adjustment : plan.adjustments) {
        allocation.weights[adjustment.strategy_id] = adjustment.target_weight;
    }
    
    // 3. 更新组合指标
    calculatePortfolioMetrics(allocation);
    
    // 4. 记录再平衡操作
    logRebalanceAction(allocation, plan);
}

} // namespace strategy
} // namespace hft
