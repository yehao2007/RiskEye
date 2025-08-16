#pragma once
#include "../core/Types.h"
#include <vector>
#include <map>
#include <memory>

namespace hft {
namespace market {

// 市场微观结构分析器
class MicrostructureAnalyzer {
public:
    struct MicrostructureMetrics {
        // 订单流相关指标
        double orderImbalance;      // 买卖订单不平衡度
        double orderFlowToxicity;   // 订单流毒性
        double marketImpact;        // 市场冲击
        
        // 价格发现指标
        double priceEfficiency;     // 价格效率性
        double informationRatio;    // 信息比率
        double priceReversion;      // 价格回复率
        
        // 流动性指标
        double effectiveSpread;     // 有效价差
        double realizedSpread;      // 实现价差
        double depthImbalance;      // 深度不平衡
        double resiliency;          // 市场弹性
        
        // 高频特征
        double microPriceVol;       // 微观价格波动
        double tickFrequency;       // 价格跳动频率
        double quoteIntensity;      // 报价强度
    };
    
    struct MarketQuality {
        double liquidityScore;      // 流动性评分
        double marketEfficiency;    // 市场效率
        double tradingCost;         // 交易成本
        double marketStability;     // 市场稳定性
    };
    
    // 初始化和配置
    void initialize();
    void setParameters(const std::map<std::string, double>& params);
    
    // 实时分析方法
    MicrostructureMetrics analyzeMicrostructure(const market::L2Data& data);
    MarketQuality assessMarketQuality(const MicrostructureMetrics& metrics);
    
    // 订单流分析
    struct OrderFlowMetrics {
        double buyPressure;         // 买入压力
        double sellPressure;        // 卖出压力
        double orderFlowImbalance;  // 订单流不平衡
        double toxicityScore;       // 毒性得分
    };
    
    OrderFlowMetrics analyzeOrderFlow(const std::vector<Order>& orders);
    
    // 价格发现分析
    struct PriceDiscoveryMetrics {
        double informationShare;    // 信息份额
        double priceLeadership;     // 价格领先度
        double priceEfficiency;     // 价格效率性
    };
    
    PriceDiscoveryMetrics analyzePriceDiscovery(const std::vector<Trade>& trades);
    
    // 流动性分析
    struct LiquidityMetrics {
        double bidAskSpread;        // 买卖价差
        double marketDepth;         // 市场深度
        double immediacy;           // 及时性
        double resilience;          // 恢复力
    };
    
    LiquidityMetrics analyzeLiquidity(const market::L2Data& data);
    
    // 高频特征分析
    struct HighFrequencyMetrics {
        double microstructureNoise; // 微观结构噪声
        double jumpIntensity;       // 跳跃强度
        double volatilitySignature; // 波动率特征
    };
    
    HighFrequencyMetrics analyzeHighFrequency(const std::vector<Trade>& trades);
    
private:
    // 内部分析方法
    double calculateOrderImbalance(const std::vector<Order>& orders);
    double calculatePriceEfficiency(const std::vector<Trade>& trades);
    double calculateMarketImpact(const Order& order, const market::L2Data& data);
    double calculateEffectiveSpread(const Trade& trade, const market::L2Data& data);
    
    // 高频分析辅助方法
    double estimateMicrostructureNoise(const std::vector<Trade>& trades);
    double detectJumps(const std::vector<Trade>& trades);
    double calculateVolatilitySignature(const std::vector<Trade>& trades);
    
    // 市场质量评估
    double calculateLiquidityScore(const LiquidityMetrics& metrics);
    double calculateMarketEfficiency(const PriceDiscoveryMetrics& metrics);
    double calculateTradingCost(const MicrostructureMetrics& metrics);
    
    // 成员变量
    std::map<std::string, double> params_;
    std::unique_ptr<TimeSeriesAnalyzer> timeSeriesAnalyzer_;
    std::unique_ptr<StatisticalAnalyzer> statAnalyzer_;
};

// 订单流分析器
class OrderFlowAnalyzer {
public:
    void initialize();
    void processOrder(const Order& order);
    void analyzeToxicity();
    double calculateVPIN(); // 成交量同步知情交易概率
    
private:
    struct OrderFlowState {
        std::vector<Order> buyOrders;
        std::vector<Order> sellOrders;
        double toxicityLevel;
    };
    
    OrderFlowState state_;
};

// 价格发现分析器
class PriceDiscoveryAnalyzer {
public:
    void initialize();
    void processTrade(const Trade& trade);
    double calculateInformationShare();
    double estimatePriceImpact();
    
private:
    struct PriceDiscoveryState {
        std::vector<Trade> trades;
        double cumulativeImpact;
    };
    
    PriceDiscoveryState state_;
};

} // namespace market
} // namespace hft
