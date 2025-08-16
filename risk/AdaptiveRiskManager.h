#pragma once
#include "../core/Types.h"
#include "../market/MarketData.h"
#include "../ai/MLModels.h"
#include <vector>
#include <memory>

namespace hft {
namespace risk {

// 自适应风险管理系统
class AdaptiveRiskManager {
public:
    struct RiskState {
        double currentRisk;         // 当前风险水平
        double riskCapacity;        // 风险承受能力
        double utilizationRate;     // 风险利用率
        std::vector<double> riskFactors;  // 风险因子
    };
    
    struct RiskLimits {
        double maxPositionSize;     // 最大持仓规模
        double maxDrawdown;         // 最大回撤限制
        double valueAtRisk;         // 风险价值限制
        double leverageRatio;       // 杠杆率限制
    };
    
    // 初始化和配置
    void initialize();
    void setInitialLimits(const RiskLimits& limits);
    
    // 风险评估和调整
    RiskState assessRiskState(const market::MarketData& data);
    RiskLimits adjustRiskLimits(const RiskState& state);
    
    // 风险预测
    struct RiskPrediction {
        double expectedRisk;        // 预期风险
        double worstCaseRisk;      // 最差情况风险
        double confidenceLevel;     // 预测置信度
    };
    
    RiskPrediction predictRisk(const market::MarketData& data);
    
    // 动态风险管理
    bool validateOrder(const Order& order);
    void updateRiskExposure(const Trade& trade);
    void monitorRiskLimits();
    
private:
    // 风险模型组件
    class RiskModel {
    public:
        virtual ~RiskModel() = default;
        virtual double calculateRisk(const market::MarketData& data) = 0;
        virtual void update(const market::MarketData& data) = 0;
    };
    
    // 具体风险模型实现
    class VolatilityModel : public RiskModel {
    public:
        double calculateRisk(const market::MarketData& data) override;
        void update(const market::MarketData& data) override;
        
    private:
        double estimateVolatility(const std::vector<double>& returns);
    };
    
    class LiquidityModel : public RiskModel {
    public:
        double calculateRisk(const market::MarketData& data) override;
        void update(const market::MarketData& data) override;
        
    private:
        double estimateLiquidityCost(const Order& order);
    };
    
    class CorrelationModel : public RiskModel {
    public:
        double calculateRisk(const market::MarketData& data) override;
        void update(const market::MarketData& data) override;
        
    private:
        std::vector<std::vector<double>> correlationMatrix_;
    };
    
    // 机器学习增强的风险预测
    class MLRiskPredictor {
    public:
        void initialize();
        void train(const std::vector<market::MarketData>& data,
                  const std::vector<double>& riskLabels);
        double predictRisk(const market::MarketData& data);
        
    private:
        std::unique_ptr<ai::MLModel> model_;
    };
    
    // 风险限额优化器
    class RiskLimitOptimizer {
    public:
        void initialize();
        RiskLimits optimizeLimits(const RiskState& state);
        void updateModel(const RiskState& state, const RiskLimits& limits);
        
    private:
        std::unique_ptr<ai::MLModel> optimizationModel_;
    };
    
    // 成员变量
    RiskLimits currentLimits_;
    RiskState currentState_;
    std::vector<std::unique_ptr<RiskModel>> riskModels_;
    std::unique_ptr<MLRiskPredictor> riskPredictor_;
    std::unique_ptr<RiskLimitOptimizer> limitOptimizer_;
};

// 自适应风险控制器
class AdaptiveRiskController {
public:
    void initialize();
    void processMarketData(const market::MarketData& data);
    bool validateTrade(const Trade& trade);
    void updateRiskModels();
    
private:
    std::unique_ptr<AdaptiveRiskManager> riskManager_;
    std::vector<market::MarketData> historicalData_;
};

} // namespace risk
} // namespace hft
