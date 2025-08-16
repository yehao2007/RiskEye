#pragma once
#include "../core/Types.h"
#include "../market/MarketData.h"
#include <torch/torch.h>
#include <memory>
#include <vector>

namespace hft {
namespace ai {

// 联邦学习管理器
class FederatedLearningManager {
public:
    struct ModelUpdate {
        torch::Tensor weights;
        torch::Tensor gradients;
        double localLoss;
        int localSamples;
    };
    
    void initialize();
    void aggregateModels(const std::vector<ModelUpdate>& updates);
    void distributeModel();
    
private:
    torch::Tensor globalModel_;
    std::vector<torch::Tensor> clientModels_;
};

// 可持续性优化器
class SustainabilityOptimizer {
public:
    struct EnergyMetrics {
        double gpuUsage;
        double powerConsumption;
        double carbonFootprint;
    };
    
    void initialize();
    void optimizeComputation(const std::string& strategy);
    EnergyMetrics monitorEnergy();
    
private:
    void switchToGPU();
    void optimizeBatchSize();
    void reducePrecision();
};

// 深度强化学习交易代理
class DRLTradingAgent {
public:
    struct State {
        torch::Tensor marketFeatures;
        torch::Tensor technicalIndicators;
        torch::Tensor orderBookState;
        torch::Tensor positions;
    };
    
    struct Action {
        double positionSize;
        double orderPrice;
        std::string orderType;
        double stopLoss;
        double takeProfit;
    };
    
    void initialize(const std::map<std::string, double>& params);
    Action getAction(const State& state);
    void train(const std::vector<State>& states,
              const std::vector<Action>& actions,
              const std::vector<double>& rewards);
              
private:
    // PPO网络
    struct PPONetworks {
        torch::nn::Sequential actor;
        torch::nn::Sequential critic;
    };
    
    PPONetworks networks_;
    torch::optim::Adam optimizer_;
    
    double calculateAdvantage(double value, double reward, double nextValue);
    void updateNetworks(const std::vector<torch::Tensor>& trajectories);
};

// 多模态深度学习预测器
class MultiModalPredictor {
public:
    struct MarketPrediction {
        double priceDirection;
        double volatility;
        double volume;
        double spread;
        std::map<std::string, double> confidenceScores;
    };
    
    void initialize();
    MarketPrediction predict(const market::MarketData& data);
    void train(const std::vector<market::MarketData>& data,
              const std::vector<MarketPrediction>& labels);
              
private:
    struct MultiModalNetwork {
        torch::nn::LSTM priceEncoder;
        torch::nn::Conv2d orderBookEncoder;
        torch::nn::Linear newsEncoder;
        torch::nn::Linear decoder;
    };
    
    MultiModalNetwork network_;
    SustainabilityOptimizer sustainOptimizer_;
};

// 跨市场套利策略
class CrossMarketArbitrage {
public:
    struct ArbitrageOpportunity {
        std::string market1;
        std::string market2;
        double priceDiff;
        double profitPotential;
        double executionRisk;
    };
    
    void initialize();
    std::vector<ArbitrageOpportunity> findOpportunities(
        const std::map<std::string, market::MarketData>& marketDataMap);
    void executeArbitrage(const ArbitrageOpportunity& opportunity);
    
private:
    std::unique_ptr<MultiModalPredictor> predictor_;
    FederatedLearningManager fedLearning_;
};

// DeFi集成模块
class DeFiIntegrator {
public:
    struct DeFiMetrics {
        double liquidityDepth;
        double gasPrice;
        double slippage;
        double impermanentLoss;
    };
    
    void initialize();
    void monitorDeFiMarkets();
    void executeDeFiStrategy(const std::string& protocol,
                           const std::string& pool);
                           
private:
    std::unique_ptr<torch::jit::script::Module> smartContractModule_;
    std::map<std::string, DeFiMetrics> protocolMetrics_;
};

// 隐私保护交易系统
class PrivacyPreservingTrading {
public:
    void initialize();
    void encryptTradeData();
    void executePrivateTransaction();
    void verifyTransaction();
    
private:
    std::unique_ptr<FederatedLearningManager> fedLearning_;
    std::vector<torch::Tensor> encryptedModels_;
};

} // namespace ai
} // namespace hft
