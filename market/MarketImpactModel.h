#pragma once
#include "../market/MarketData.h"
#include "../core/Types.h"
#include <vector>
#include <map>

namespace hft {
namespace market {

// 市场冲击成本模型
class MarketImpactModel {
public:
    struct ImpactParameters {
        double temporaryImpactFactor;  // 临时冲击因子
        double permanentImpactFactor;  // 永久冲击因子
        double decayFactor;           // 衰减因子
        double volumeThreshold;       // 量比阈值
    };
    
    void initialize(const ImpactParameters& params);
    
    // 计算市场冲击成本
    double calculateImpactCost(const Order& order, const L2Data& orderBook);
    double calculateTemporaryImpact(double orderVolume, double marketVolume);
    double calculatePermanentImpact(double orderVolume, double marketVolume);
    
    // 优化执行策略
    struct ExecutionPlan {
        std::vector<double> volumes;      // 分割后的交易量
        std::vector<double> timings;      // 执行时间点
        double expectedImpactCost;        // 预期冲击成本
    };
    
    ExecutionPlan optimizeExecution(const Order& order, 
                                  const MarketCondition& condition);
    
    // 流动性分析
    double calculateMarketResiliency(const std::vector<L2Data>& orderBookHistory);
    double estimateMaxTradeSize(const L2Data& orderBook, double maxImpact);
    
    // 模型校准
    void calibrateModel(const std::vector<Trade>& historicalTrades,
                       const std::vector<L2Data>& orderBookHistory);
    void updateParameters(const ImpactParameters& newParams);
    
private:
    // 内部计算方法
    double calculateVolumeProfile(const std::vector<Trade>& trades);
    double estimateMarketDepth(const L2Data& orderBook);
    double calculatePriceImpact(double volume, double depth);
    
    // 模型参数
    ImpactParameters params_;
    std::map<std::string, double> marketDepths_;
    std::map<std::string, double> volumeProfiles_;
};

// 自适应市场冲击模型
class AdaptiveImpactModel : public MarketImpactModel {
public:
    void initialize() override;
    
    // 实时适应
    void adaptToMarketConditions(const MarketCondition& condition);
    void updateModelParameters(const MarketData& data);
    
    // 机器学习增强
    void trainModel(const std::vector<Trade>& historicalTrades,
                   const std::vector<L2Data>& orderBookHistory);
    double predictImpact(const Order& order);
    
private:
    // ML模型组件
    struct ModelFeatures {
        double orderSize;
        double marketVolume;
        double volatility;
        double spread;
        double depth;
    };
    
    ModelFeatures extractFeatures(const Order& order, 
                                const L2Data& orderBook);
    void updateFeatures(const ModelFeatures& features);
    
    // 模型状态
    std::vector<ModelFeatures> featureHistory_;
    std::vector<double> impactHistory_;
};

// 市场冲击监控器
class ImpactMonitor {
public:
    void initialize();
    void start();
    void stop();
    
    // 监控功能
    void recordImpact(const Trade& trade, double estimatedImpact);
    void analyzeImpactPattern();
    void generateImpactReport();
    
    // 警报系统
    void setAlertThresholds(const std::map<std::string, double>& thresholds);
    bool checkAlerts(double impact);
    
private:
    struct ImpactMetrics {
        double averageImpact;
        double maxImpact;
        double cumulativeImpact;
        int numExceedances;
    };
    
    std::map<std::string, ImpactMetrics> metrics_;
    std::map<std::string, double> alertThresholds_;
    bool isRunning_{false};
};

} // namespace market
} // namespace hft
