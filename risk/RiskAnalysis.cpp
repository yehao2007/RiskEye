#include "RiskAnalysis.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace hft {
namespace risk {

ExtendedRiskMetrics RiskAnalyzer::calculateRiskMetrics(
    const std::vector<double>& returns,
    const std::vector<double>& prices,
    const std::vector<double>& volumes) {
    
    ExtendedRiskMetrics metrics;
    
    // 计算VaR
    metrics.valueAtRisk = calculateValueAtRisk(returns, 0.99);  // 99% 置信度
    
    // 计算期望损失
    metrics.expectedShortfall = calculateExpectedShortfall(returns, 0.99);
    
    // 计算波动率相关指标
    metrics.impliedVolatility = calculateImpliedVolatility(prices);
    metrics.realizedVolatility = calculateRealizedVolatility(returns);
    metrics.volatilitySkew = calculateVolatilitySkew(prices);
    
    // 计算相关性和Beta
    metrics.beta = calculateBeta(returns, marketReturns_);
    metrics.correlationMatrix = calculateCorrelationMatrix(returns);
    
    // 计算流动性指标
    metrics.bidAskSpread = calculateBidAskSpread(prices);
    metrics.marketDepth = calculateMarketDepth(volumes);
    metrics.turnoverRatio = calculateTurnoverRatio(volumes, prices);
    
    // 计算尾部风险指标
    metrics.tailRisk = calculateTailRisk(returns);
    metrics.extremeValueMetric = calculateExtremeValueMetric(returns);
    
    // 压力测试
    metrics.stressTestLoss = runStressTest(returns);
    metrics.scenarioImpact = calculateScenarioImpact(returns);
    
    return metrics;
}

double RiskAnalyzer::calculateValueAtRisk(
    const std::vector<double>& returns,
    double confidence) {
    
    std::vector<double> sortedReturns = returns;
    std::sort(sortedReturns.begin(), sortedReturns.end());
    
    int index = static_cast<int>((1 - confidence) * returns.size());
    return -sortedReturns[index];
}

double RiskAnalyzer::calculateExpectedShortfall(
    const std::vector<double>& returns,
    double confidence) {
    
    double var = calculateValueAtRisk(returns, confidence);
    double sum = 0;
    int count = 0;
    
    for (double ret : returns) {
        if (ret < -var) {
            sum += ret;
            count++;
        }
    }
    
    return -sum / count;
}

double RiskAnalyzer::calculateImpliedVolatility(
    const std::vector<double>& prices) {
    
    std::vector<double> returns(prices.size() - 1);
    for (size_t i = 1; i < prices.size(); ++i) {
        returns[i-1] = std::log(prices[i] / prices[i-1]);
    }
    
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0;
    
    for (double ret : returns) {
        variance += (ret - mean) * (ret - mean);
    }
    
    return std::sqrt(variance / (returns.size() - 1)) * std::sqrt(252);  // 年化
}

double RiskAnalyzer::calculateVolatilitySkew(
    const std::vector<double>& prices) {
    
    std::vector<double> returns(prices.size() - 1);
    for (size_t i = 1; i < prices.size(); ++i) {
        returns[i-1] = std::log(prices[i] / prices[i-1]);
    }
    
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0;
    double skewness = 0;
    
    for (double ret : returns) {
        double diff = ret - mean;
        variance += diff * diff;
        skewness += diff * diff * diff;
    }
    
    variance /= (returns.size() - 1);
    skewness /= (returns.size() - 1) * std::pow(variance, 1.5);
    
    return skewness;
}

double RiskAnalyzer::calculateTailRisk(
    const std::vector<double>& returns) {
    
    // 使用Hill估计器计算尾部风险
    std::vector<double> absReturns;
    for (double ret : returns) {
        absReturns.push_back(std::abs(ret));
    }
    
    std::sort(absReturns.begin(), absReturns.end(), std::greater<double>());
    
    int k = static_cast<int>(returns.size() * 0.1);  // 使用10%的极端值
    double sum = 0;
    
    for (int i = 0; i < k; ++i) {
        sum += std::log(absReturns[i] / absReturns[k-1]);
    }
    
    return 1.0 / (sum / k);
}

std::vector<double> RiskAnalyzer::runStressTest(
    const std::vector<double>& positions,
    const std::vector<std::string>& scenarios) {
    
    std::vector<double> results;
    
    for (const auto& scenario : scenarios) {
        if (scenario == "MarketCrash") {
            results.push_back(simulateMarketCrash(positions));
        } else if (scenario == "LiquidityCrisis") {
            results.push_back(simulateLiquidityCrisis(positions));
        } else if (scenario == "VolatilitySpike") {
            results.push_back(simulateVolatilitySpike(positions));
        }
    }
    
    return results;
}

double RiskAnalyzer::simulateMarketCrash(
    const std::vector<double>& positions) {
    
    double totalLoss = 0;
    for (size_t i = 0; i < positions.size(); ++i) {
        double shock = -0.1 - 0.05 * normalDist_(generator_);  // 10-15% 下跌
        totalLoss += positions[i] * shock;
    }
    
    return totalLoss;
}

} // namespace risk
} // namespace hft
