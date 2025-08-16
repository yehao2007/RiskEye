#pragma once
#include "../core/Types.h"
#include "../market/MarketData.h"
#include "../execution/OrderExecution.h"
#include "../risk/RiskAnalysis.h"
#include <vector>
#include <memory>
#include <map>

namespace hft {
namespace strategy {

// 高级策略基类
class AdvancedStrategy {
public:
    virtual ~AdvancedStrategy() = default;
    
    // 策略生命周期
    virtual void initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;
    
    // 市场数据处理
    virtual void onMarketData(const market::L2Data& data) = 0;
    virtual void onTrade(const market::Trade& trade) = 0;
    virtual void onOrderUpdate(const execution::Order& order) = 0;
    
    // 风险控制
    virtual void onRiskAlert(const risk::RiskAlert& alert) = 0;
    
    // 性能指标
    virtual double getSharpeRatio() const = 0;
    virtual double getMaxDrawdown() const = 0;
    virtual double getWinRate() const = 0;
    
protected:
    bool isRunning_{false};
    std::map<std::string, double> parameters_;
    std::unique_ptr<execution::OrderManager> orderManager_;
    std::unique_ptr<risk::RiskAnalysisEngine> riskEngine_;
};

// 机器学习增强型做市策略
class MLEnhancedMarketMaking : public AdvancedStrategy {
public:
    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;
    
    void onMarketData(const market::L2Data& data) override;
    void onTrade(const market::Trade& trade) override;
    void onOrderUpdate(const execution::Order& order) override;
    void onRiskAlert(const risk::RiskAlert& alert) override;
    
    double getSharpeRatio() const override;
    double getMaxDrawdown() const override;
    double getWinRate() const override;

private:
    // ML模型组件
    struct ModelFeatures {
        std::vector<double> priceHistory;
        std::vector<double> volumeHistory;
        std::vector<double> spreadHistory;
        std::vector<double> volatilityHistory;
        std::vector<double> orderImbalance;
    };
    
    void updateFeatures(const market::L2Data& data);
    double predictSpread(const ModelFeatures& features);
    double predictVolatility(const ModelFeatures& features);
    void adjustQuotes(double predictedSpread, double predictedVol);
    void retrainModel();
    
    ModelFeatures features_;
    std::unique_ptr<void> mlModel_;  // 具体ML模型的指针
};

// 多因子套利策略
class MultiFactorArbitrage : public AdvancedStrategy {
public:
    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;
    
    void onMarketData(const market::L2Data& data) override;
    void onTrade(const market::Trade& trade) override;
    void onOrderUpdate(const execution::Order& order) override;
    void onRiskAlert(const risk::RiskAlert& alert) override;
    
    double getSharpeRatio() const override;
    double getMaxDrawdown() const override;
    double getWinRate() const override;

private:
    struct Factor {
        std::string name;
        double weight;
        double value;
        double zscore;
    };
    
    void updateFactors();
    double calculateAlpha();
    void optimizePortfolio();
    void rebalancePositions();
    
    std::vector<Factor> factors_;
    std::map<std::string, double> positions_;
};

// 高频动态对冲策略
class DynamicHedging : public AdvancedStrategy {
public:
    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;
    
    void onMarketData(const market::L2Data& data) override;
    void onTrade(const market::Trade& trade) override;
    void onOrderUpdate(const execution::Order& order) override;
    void onRiskAlert(const risk::RiskAlert& alert) override;
    
    double getSharpeRatio() const override;
    double getMaxDrawdown() const override;
    double getWinRate() const override;

private:
    struct HedgePosition {
        std::string instrument;
        double delta;
        double gamma;
        double vega;
        double theta;
        double rho;
    };
    
    void calculateGreeks();
    void optimizeHedgeRatio();
    void executeHedge();
    void monitorHedgeEffectiveness();
    
    std::vector<HedgePosition> hedgePositions_;
    std::map<std::string, double> riskExposures_;
};

// 自适应套利策略
class AdaptiveArbitrage : public AdvancedStrategy {
public:
    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;
    
    void onMarketData(const market::L2Data& data) override;
    void onTrade(const market::Trade& trade) override;
    void onOrderUpdate(const execution::Order& order) override;
    void onRiskAlert(const risk::RiskAlert& alert) override;
    
    double getSharpeRatio() const override;
    double getMaxDrawdown() const override;
    double getWinRate() const override;

private:
    struct OpportunityMetrics {
        double profitPotential;
        double executionCost;
        double riskScore;
        double confidence;
    };
    
    void detectOpportunities();
    void evaluateOpportunity(const OpportunityMetrics& metrics);
    void optimizeExecution();
    void adaptParameters();
    
    std::vector<OpportunityMetrics> opportunities_;
    std::map<std::string, double> strategyParameters_;
};

} // namespace strategy
} // namespace hft
