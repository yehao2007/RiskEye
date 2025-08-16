#include "MicrostructureAnalyzer.h"
#include "../core/Logger.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace hft {
namespace market {

MicrostructureMetrics MicrostructureAnalyzer::analyzeMicrostructure(
    const market::L2Data& data) {
    
    MicrostructureMetrics metrics;
    
    // 计算订单流不平衡度
    metrics.orderImbalance = calculateOrderImbalance(data);
    
    // 计算订单流毒性
    metrics.orderFlowToxicity = calculateOrderFlowToxicity(data);
    
    // 计算有效价差
    metrics.effectiveSpread = calculateEffectiveSpread(data);
    
    // 计算市场深度不平衡
    metrics.depthImbalance = calculateDepthImbalance(data);
    
    // 计算市场弹性
    metrics.resiliency = calculateResiliency(data);
    
    return metrics;
}

double MicrostructureAnalyzer::calculateOrderImbalance(const market::L2Data& data) {
    double totalBuyVolume = 0.0;
    double totalSellVolume = 0.0;
    
    for (const auto& level : data.bids) {
        totalBuyVolume += level.volume;
    }
    
    for (const auto& level : data.asks) {
        totalSellVolume += level.volume;
    }
    
    return (totalBuyVolume - totalSellVolume) / (totalBuyVolume + totalSellVolume);
}

double MicrostructureAnalyzer::calculateOrderFlowToxicity(const market::L2Data& data) {
    // 使用VPIN (Volume-synchronized Probability of Informed Trading) 方法
    double volumeBucket = 0.0;
    double buyVolume = 0.0;
    double sellVolume = 0.0;
    
    for (const auto& trade : recentTrades_) {
        volumeBucket += trade.volume;
        if (trade.side == OrderSide::BUY) {
            buyVolume += trade.volume;
        } else {
            sellVolume += trade.volume;
        }
    }
    
    double vpin = std::abs(buyVolume - sellVolume) / volumeBucket;
    return vpin;
}

double MicrostructureAnalyzer::calculateEffectiveSpread(const market::L2Data& data) {
    double midPrice = (data.bids[0].price + data.asks[0].price) / 2.0;
    double weightedSpread = 0.0;
    double totalVolume = 0.0;
    
    // 计算加权价差
    for (size_t i = 0; i < std::min(data.bids.size(), data.asks.size()); ++i) {
        double bidVolume = data.bids[i].volume;
        double askVolume = data.asks[i].volume;
        double spread = data.asks[i].price - data.bids[i].price;
        
        weightedSpread += spread * (bidVolume + askVolume);
        totalVolume += (bidVolume + askVolume);
    }
    
    return weightedSpread / totalVolume;
}

double MicrostructureAnalyzer::calculateDepthImbalance(const market::L2Data& data) {
    std::vector<double> bidDepths;
    std::vector<double> askDepths;
    
    // 计算各个价格层次的深度
    for (const auto& level : data.bids) {
        bidDepths.push_back(level.price * level.volume);
    }
    
    for (const auto& level : data.asks) {
        askDepths.push_back(level.price * level.volume);
    }
    
    // 计算深度差异
    double totalBidDepth = std::accumulate(bidDepths.begin(), bidDepths.end(), 0.0);
    double totalAskDepth = std::accumulate(askDepths.begin(), askDepths.end(), 0.0);
    
    return (totalBidDepth - totalAskDepth) / (totalBidDepth + totalAskDepth);
}

double MicrostructureAnalyzer::calculateResiliency(const market::L2Data& data) {
    // 计算价格冲击后的恢复速度
    double priceImpact = calculatePriceImpact(data);
    double recoverySpeed = calculateRecoverySpeed(data);
    
    return recoverySpeed / priceImpact;
}

double MicrostructureAnalyzer::calculatePriceImpact(const market::L2Data& data) {
    double midPrice = (data.bids[0].price + data.asks[0].price) / 2.0;
    double impact = 0.0;
    
    // 使用Kyle's lambda模型
    for (const auto& trade : recentTrades_) {
        double priceChange = std::abs(trade.price - midPrice);
        impact += priceChange / trade.volume;
    }
    
    return impact / recentTrades_.size();
}

double MicrostructureAnalyzer::calculateRecoverySpeed(const market::L2Data& data) {
    std::vector<double> priceChanges;
    std::vector<double> timeDiffs;
    
    // 计算价格变化和时间差
    for (size_t i = 1; i < recentTrades_.size(); ++i) {
        double priceChange = std::abs(recentTrades_[i].price - recentTrades_[i-1].price);
        double timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
            recentTrades_[i].timestamp - recentTrades_[i-1].timestamp).count();
        
        priceChanges.push_back(priceChange);
        timeDiffs.push_back(timeDiff);
    }
    
    // 计算恢复速度
    double avgPriceChange = std::accumulate(priceChanges.begin(), priceChanges.end(), 0.0) 
                           / priceChanges.size();
    double avgTimeDiff = std::accumulate(timeDiffs.begin(), timeDiffs.end(), 0.0) 
                        / timeDiffs.size();
    
    return avgPriceChange / avgTimeDiff;
}

MarketQuality MicrostructureAnalyzer::assessMarketQuality(
    const MicrostructureMetrics& metrics) {
    
    MarketQuality quality;
    
    // 计算流动性评分
    quality.liquidityScore = calculateLiquidityScore(metrics);
    
    // 计算市场效率
    quality.marketEfficiency = calculateMarketEfficiency(metrics);
    
    // 计算交易成本
    quality.tradingCost = calculateTradingCost(metrics);
    
    // 计算市场稳定性
    quality.marketStability = calculateMarketStability(metrics);
    
    return quality;
}

} // namespace market
} // namespace hft
