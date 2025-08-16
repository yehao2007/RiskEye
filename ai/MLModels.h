#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "../core/Types.h"
#include "../market/MarketData.h"

namespace hft {
namespace ai {

// 特征工程
class FeatureExtractor {
public:
    struct MarketFeatures {
        std::vector<double> priceFeatures;
        std::vector<double> volumeFeatures;
        std::vector<double> orderBookFeatures;
        std::vector<double> technicalIndicators;
        std::vector<double> microstructureFeatures;
    };
    
    void initialize(const std::map<std::string, bool>& featureFlags);
    MarketFeatures extractFeatures(const market::MarketData& data);
    
private:
    std::vector<double> calculatePriceFeatures(const market::MarketData& data);
    std::vector<double> calculateVolumeFeatures(const market::MarketData& data);
    std::vector<double> calculateOrderBookFeatures(const market::MarketData& data);
    std::vector<double> calculateMicrostructureFeatures(const market::MarketData& data);
};

// 模型接口
class MLModel {
public:
    virtual ~MLModel() = default;
    virtual void train(const std::vector<std::vector<double>>& features,
                      const std::vector<double>& labels) = 0;
    virtual std::vector<double> predict(const std::vector<std::vector<double>>& features) = 0;
    virtual void save(const std::string& path) = 0;
    virtual void load(const std::string& path) = 0;
};

// LSTM模型
class LSTMModel : public MLModel {
public:
    struct LSTMConfig {
        int inputSize;
        int hiddenSize;
        int numLayers;
        double learningRate;
        int batchSize;
        int epochs;
    };
    
    void initialize(const LSTMConfig& config);
    void train(const std::vector<std::vector<double>>& features,
               const std::vector<double>& labels) override;
    std::vector<double> predict(const std::vector<std::vector<double>>& features) override;
    void save(const std::string& path) override;
    void load(const std::string& path) override;
    
private:
    LSTMConfig config_;
    // LSTM specific members
};

// XGBoost模型
class XGBoostModel : public MLModel {
public:
    struct XGBoostConfig {
        int maxDepth;
        double learningRate;
        int numRounds;
        double gamma;
        double minChildWeight;
    };
    
    void initialize(const XGBoostConfig& config);
    void train(const std::vector<std::vector<double>>& features,
               const std::vector<double>& labels) override;
    std::vector<double> predict(const std::vector<std::vector<double>>& features) override;
    void save(const std::string& path) override;
    void load(const std::string& path) override;
    
private:
    XGBoostConfig config_;
    // XGBoost specific members
};

// 集成学习模型
class EnsembleModel : public MLModel {
public:
    void addModel(std::shared_ptr<MLModel> model, double weight);
    void train(const std::vector<std::vector<double>>& features,
               const std::vector<double>& labels) override;
    std::vector<double> predict(const std::vector<std::vector<double>>& features) override;
    void save(const std::string& path) override;
    void load(const std::string& path) override;
    
private:
    std::vector<std::pair<std::shared_ptr<MLModel>, double>> models_;
};

// 在线学习系统
class OnlineLearningSystem {
public:
    void initialize();
    void updateModel(const market::MarketData& data, double reward);
    double predictAction(const market::MarketData& data);
    void adaptToMarketConditions(const market::MarketCondition& condition);
    
private:
    std::unique_ptr<FeatureExtractor> featureExtractor_;
    std::unique_ptr<MLModel> model_;
    std::queue<std::pair<std::vector<double>, double>> replayBuffer_;
};

// 强化学习交易代理
class RLTradingAgent {
public:
    struct State {
        std::vector<double> marketFeatures;
        std::vector<double> positionFeatures;
        std::vector<double> riskFeatures;
    };
    
    struct Action {
        double position;
        double price;
        std::string orderType;
    };
    
    void initialize();
    Action getAction(const State& state);
    void update(const State& state, const Action& action,
                double reward, const State& nextState);
    void train();
    
private:
    std::unique_ptr<MLModel> policyNetwork_;
    std::unique_ptr<MLModel> valueNetwork_;
    std::vector<std::tuple<State, Action, double, State>> experience_;
};

} // namespace ai
} // namespace hft
