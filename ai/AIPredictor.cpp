#include "AIPredictor.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"
#include <torch/torch.h>
#include <algorithm>

namespace hft {
namespace ai {

void AIPredictor::initialize() {
    // 初始化AI组件
    initializeComponents();
    
    // 加载模型
    loadModels();
    
    // 配置GPU加速
    setupGPUAcceleration();
    
    // 初始化预测引擎
    initializePredictionEngine();
}

void AIPredictor::initializeComponents() {
    // 深度学习模型
    deep_learning_model_ = std::make_unique<DeepLearningModel>();
    deep_learning_model_->initialize();
    
    // 强化学习模型
    reinforcement_model_ = std::make_unique<ReinforcementModel>();
    reinforcement_model_->initialize();
    
    // 统计学习模型
    statistical_model_ = std::make_unique<StatisticalModel>();
    statistical_model_->initialize();
    
    // 集成学习模型
    ensemble_model_ = std::make_unique<EnsembleModel>();
    ensemble_model_->initialize();
}

void AIPredictor::loadModels() {
    try {
        // 加载价格预测模型
        loadPricePredictionModel();
        
        // 加载波动率预测模型
        loadVolatilityPredictionModel();
        
        // 加载流动性预测模型
        loadLiquidityPredictionModel();
        
        // 加载市场微结构模型
        loadMicrostructureModel();
        
    } catch (const std::exception& e) {
        Logger::error("Model loading failed: {}", e.what());
        throw;
    }
}

void AIPredictor::setupGPUAcceleration() {
    if (torch::cuda::is_available()) {
        // 配置CUDA设备
        cuda_device_ = torch::Device(torch::kCUDA);
        
        // 移动模型到GPU
        moveModelsToGPU();
        
        // 配置CUDA流
        setupCUDAStreams();
        
        // 优化GPU内存
        optimizeGPUMemory();
    }
}

PredictionResult AIPredictor::predict(
    const MarketData& data,
    PredictionType type) {
    
    PredictionResult result;
    
    try {
        // 1. 数据预处理
        auto processed_data = preprocessData(data);
        
        // 2. 特征提取
        auto features = extractFeatures(processed_data);
        
        // 3. 选择预测模型
        auto model = selectPredictionModel(type);
        
        // 4. 执行预测
        result = executePrediction(model, features);
        
        // 5. 后处理
        postProcessPrediction(result);
        
    } catch (const std::exception& e) {
        Logger::error("Prediction failed: {}", e.what());
        result.status = PredictionStatus::ERROR;
    }
    
    return result;
}

torch::Tensor AIPredictor::preprocessData(
    const MarketData& data) {
    
    // 1. 数据清洗
    auto cleaned_data = cleanData(data);
    
    // 2. 数据标准化
    auto normalized_data = normalizeData(cleaned_data);
    
    // 3. 时间序列处理
    auto processed_time_series = processTimeSeries(normalized_data);
    
    // 4. 转换为张量
    return convertToTensor(processed_time_series);
}

torch::Tensor AIPredictor::extractFeatures(
    const torch::Tensor& data) {
    
    // 1. 技术指标特征
    auto technical_features = extractTechnicalFeatures(data);
    
    // 2. 统计特征
    auto statistical_features = extractStatisticalFeatures(data);
    
    // 3. 微观结构特征
    auto microstructure_features = extractMicrostructureFeatures(data);
    
    // 4. 组合特征
    return combineFeatures({
        technical_features,
        statistical_features,
        microstructure_features
    });
}

std::shared_ptr<BaseModel> AIPredictor::selectPredictionModel(
    PredictionType type) {
    
    switch (type) {
        case PredictionType::PRICE:
            return price_prediction_model_;
            
        case PredictionType::VOLATILITY:
            return volatility_prediction_model_;
            
        case PredictionType::LIQUIDITY:
            return liquidity_prediction_model_;
            
        case PredictionType::MICROSTRUCTURE:
            return microstructure_model_;
            
        default:
            throw std::runtime_error("Unknown prediction type");
    }
}

PredictionResult AIPredictor::executePrediction(
    std::shared_ptr<BaseModel> model,
    const torch::Tensor& features) {
    
    // 1. 准备预测
    preparePrediction(model, features);
    
    // 2. 执行预测
    auto prediction = model->predict(features);
    
    // 3. 计算不确定性
    auto uncertainty = calculateUncertainty(model, features);
    
    // 4. 生成结果
    return createPredictionResult(prediction, uncertainty);
}

void AIPredictor::postProcessPrediction(
    PredictionResult& result) {
    
    // 1. 结果验证
    validatePrediction(result);
    
    // 2. 不确定性调整
    adjustUncertainty(result);
    
    // 3. 添加置信区间
    addConfidenceIntervals(result);
    
    // 4. 更新统计信息
    updatePredictionStats(result);
}

void AIPredictor::updateModel(
    const MarketData& new_data,
    const PredictionResult& actual_result) {
    
    try {
        // 1. 评估预测性能
        evaluatePredictionPerformance(actual_result);
        
        // 2. 更新模型权重
        updateModelWeights(new_data, actual_result);
        
        // 3. 优化模型参数
        optimizeModelParameters();
        
        // 4. 保存更新后的模型
        saveModelCheckpoint();
        
    } catch (const std::exception& e) {
        Logger::error("Model update failed: {}", e.what());
        handleModelUpdateFailure();
    }
}

PredictionMetrics AIPredictor::getMetrics() const {
    PredictionMetrics metrics;
    
    // 收集预测准确度指标
    metrics.accuracy = calculateAccuracyMetrics();
    
    // 收集不确定性指标
    metrics.uncertainty = calculateUncertaintyMetrics();
    
    // 收集性能指标
    metrics.performance = calculatePerformanceMetrics();
    
    // 收集模型健康度指标
    metrics.model_health = calculateModelHealthMetrics();
    
    return metrics;
}

} // namespace ai
} // namespace hft
