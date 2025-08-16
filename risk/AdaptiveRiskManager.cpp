#include "AdaptiveRiskManager.h"
#include "../core/Logger.h"
#include <torch/torch.h>

namespace hft {
namespace risk {

void AdaptiveRiskManager::MLRiskPredictor::initialize() {
    // 创建深度神经网络模型
    model_ = std::make_unique<torch::nn::Sequential>(
        torch::nn::Linear(64, 128),
        torch::nn::ReLU(),
        torch::nn::Dropout(0.3),
        torch::nn::Linear(128, 64),
        torch::nn::ReLU(),
        torch::nn::Linear(64, 32),
        torch::nn::ReLU(),
        torch::nn::Linear(32, 1)
    );
    
    optimizer_ = std::make_unique<torch::optim::Adam>(
        model_->parameters(), 
        torch::optim::AdamOptions(0.001)
    );
}

void AdaptiveRiskManager::MLRiskPredictor::train(
    const std::vector<market::MarketData>& data,
    const std::vector<double>& riskLabels) {
    
    // 准备训练数据
    std::vector<torch::Tensor> features;
    std::vector<torch::Tensor> labels;
    
    for (size_t i = 0; i < data.size(); ++i) {
        auto marketFeatures = extractFeatures(data[i]);
        features.push_back(marketFeatures);
        labels.push_back(torch::tensor(riskLabels[i]));
    }
    
    // 训练循环
    const int epochs = 100;
    const int batchSize = 32;
    
    for (int epoch = 0; epoch < epochs; ++epoch) {
        model_->train();
        double epochLoss = 0.0;
        
        for (size_t i = 0; i < features.size(); i += batchSize) {
            optimizer_->zero_grad();
            
            // 准备批次数据
            auto batchEnd = std::min(i + batchSize, features.size());
            std::vector<torch::Tensor> batchFeatures(
                features.begin() + i,
                features.begin() + batchEnd
            );
            std::vector<torch::Tensor> batchLabels(
                labels.begin() + i,
                labels.begin() + batchEnd
            );
            
            auto featureBatch = torch::stack(batchFeatures);
            auto labelBatch = torch::stack(batchLabels);
            
            // 前向传播
            auto prediction = model_->forward(featureBatch);
            auto loss = torch::mse_loss(prediction, labelBatch);
            
            // 反向传播
            loss.backward();
            optimizer_->step();
            
            epochLoss += loss.item<double>();
        }
        
        Logger::debug("Epoch {}: Loss = {:.6f}", epoch + 1, epochLoss / features.size());
    }
}

double AdaptiveRiskManager::MLRiskPredictor::predictRisk(
    const market::MarketData& data) {
    
    model_->eval();
    torch::NoGradGuard no_grad;
    
    auto features = extractFeatures(data);
    auto prediction = model_->forward(features);
    
    return prediction.item<double>();
}

torch::Tensor AdaptiveRiskManager::MLRiskPredictor::extractFeatures(
    const market::MarketData& data) {
    
    std::vector<double> features;
    
    // 价格特征
    features.push_back(data.price);
    features.push_back(data.volume);
    
    // 波动率特征
    auto volFeatures = calculateVolatilityFeatures(data);
    features.insert(features.end(), volFeatures.begin(), volFeatures.end());
    
    // 流动性特征
    auto liqFeatures = calculateLiquidityFeatures(data);
    features.insert(features.end(), liqFeatures.begin(), liqFeatures.end());
    
    // 订单簿特征
    auto bookFeatures = calculateOrderBookFeatures(data);
    features.insert(features.end(), bookFeatures.begin(), bookFeatures.end());
    
    return torch::tensor(features);
}

void AdaptiveRiskManager::RiskLimitOptimizer::optimize(
    const RiskState& state) {
    
    // 更新风险限额
    RiskLimits newLimits;
    
    // 根据当前风险状态调整限额
    double riskFactor = state.currentRisk / state.riskCapacity;
    
    newLimits.maxPositionSize = currentLimits_.maxPositionSize * (1.0 / riskFactor);
    newLimits.maxDrawdown = currentLimits_.maxDrawdown * (1.0 / riskFactor);
    newLimits.valueAtRisk = currentLimits_.valueAtRisk * (1.0 / riskFactor);
    
    // 应用机器学习模型优化限额
    auto optimizedLimits = optimizationModel_->forward(
        torch::tensor({riskFactor, state.utilizationRate})
    );
    
    // 应用优化结果
    newLimits.maxPositionSize *= optimizedLimits[0].item<double>();
    newLimits.maxDrawdown *= optimizedLimits[1].item<double>();
    newLimits.valueAtRisk *= optimizedLimits[2].item<double>();
    
    // 更新当前限额
    currentLimits_ = newLimits;
}

void AdaptiveRiskManager::updateRiskModels(
    const market::MarketData& data) {
    
    // 更新风险模型
    for (auto& model : riskModels_) {
        model->update(data);
    }
    
    // 更新风险预测器
    riskPredictor_->update(data);
    
    // 更新限额优化器
    RiskState currentState = assessRiskState(data);
    limitOptimizer_->optimize(currentState);
}

} // namespace risk
} // namespace hft
