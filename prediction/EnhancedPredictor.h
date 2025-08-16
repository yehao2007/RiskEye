#pragma once
#include "../core/Types.h"
#include <torch/torch.h>
#include <vector>
#include <memory>

namespace hft {
namespace prediction {

// 高精度市场预测系统
class EnhancedMarketPredictor {
public:
    struct MarketState {
        std::vector<double> prices;
        std::vector<double> volumes;
        std::vector<double> spreads;
        std::vector<std::vector<double>> orderBook;
        std::vector<std::string> newsFeatures;
        std::vector<double> sentimentScores;
    };
    
    struct PredictionResult {
        double expectedPrice;
        double volatility;
        double tradingVolume;
        std::vector<double> priceDistribution;
        double confidence;
        std::map<std::string, double> factorContributions;
    };
    
    // 注意力机制的Transformer模型
    class AttentionTransformer {
    public:
        void initialize(int inputDim, int headNum);
        torch::Tensor forward(torch::Tensor input);
        
    private:
        struct TransformerConfig {
            int inputDim;
            int hiddenDim;
            int numHeads;
            int numLayers;
            double dropoutRate;
        };
        
        torch::nn::MultiheadAttention attention_{nullptr};
        torch::nn::TransformerEncoder encoder_{nullptr};
        TransformerConfig config_;
    };
    
    // 混合深度学习模型
    class HybridDeepLearner {
    public:
        void initialize();
        PredictionResult predict(const MarketState& state);
        void train(const std::vector<MarketState>& states,
                  const std::vector<PredictionResult>& results);
                  
    private:
        std::unique_ptr<AttentionTransformer> transformer_;
        torch::nn::LSTM lstm_{nullptr};
        torch::nn::Conv2d cnn_{nullptr};
        torch::nn::Linear mlp_{nullptr};
    };
    
    // 自适应采样策略
    class AdaptiveSampler {
    public:
        void initialize();
        std::vector<MarketState> sampleData(const std::vector<MarketState>& data);
        void updateSamplingStrategy(double predictionError);
        
    private:
        double sampleRate_;
        double errorThreshold_;
        std::vector<double> importanceWeights_;
    };
    
    // 集成预测器
    void initialize();
    PredictionResult predict(const MarketState& state);
    void updateModels(const MarketState& state, const PredictionResult& actual);
    
private:
    std::unique_ptr<HybridDeepLearner> deepLearner_;
    std::unique_ptr<AdaptiveSampler> sampler_;
    std::vector<std::unique_ptr<torch::nn::Module>> ensembleModels_;
};

// 高性能计算优化器
class ComputeOptimizer {
public:
    struct ComputeMetrics {
        double gpuUtilization;
        double memoryUsage;
        double powerEfficiency;
        double throughput;
    };
    
    void initialize();
    void optimizeComputation(const std::string& modelType);
    ComputeMetrics monitorPerformance();
    
private:
    void distributeWorkload();
    void adjustPrecision();
    void cacheResults();
    
    torch::Device device_{torch::kCUDA};
    bool useHalfPrecision_{false};
    std::map<std::string, torch::Tensor> computeCache_;
};

// 隐私保护数据处理器
class PrivacyPreserver {
public:
    struct PrivacyConfig {
        double epsilonDelta;
        int noiseScale;
        bool useHomomorphicEncryption;
    };
    
    void initialize(const PrivacyConfig& config);
    torch::Tensor encryptData(const torch::Tensor& data);
    torch::Tensor decryptData(const torch::Tensor& encrypted);
    
    // 差分隐私
    torch::Tensor addNoise(const torch::Tensor& data);
    double calculatePrivacyBudget();
    
private:
    PrivacyConfig config_;
    std::unique_ptr<torch::nn::Module> encryptionModel_;
};

// 跨市场分析器
class CrossMarketAnalyzer {
public:
    struct MarketCorrelation {
        double correlation;
        double leadLag;
        double informationShare;
        double spilloverEffect;
    };
    
    void initialize();
    MarketCorrelation analyzeCorrelation(
        const std::string& market1,
        const std::string& market2);
        
    std::vector<std::string> findLeadingMarkets();
    double calculateSpilloverEffect();
    
private:
    std::map<std::string, MarketState> marketStates_;
    std::unique_ptr<AttentionTransformer> crossAttention_;
};

// 自适应GPU优化器
class GPUOptimizer {
public:
    void initialize();
    void optimizeKernels();
    void balanceWorkload();
    void monitorThermals();
    
private:
    void adjustClockSpeed();
    void managePowerStates();
    void scheduleKernels();
    
    struct ThermalMetrics {
        double temperature;
        double powerDraw;
        double fanSpeed;
    };
    
    ThermalMetrics thermals_;
    std::vector<cudaStream_t> streams_;
};

} // namespace prediction
} // namespace hft
