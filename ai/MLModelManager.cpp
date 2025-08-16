#include "MLModelManager.h"
#include "../core/Logger.h"
#include <torch/torch.h>
#include <dlib/dnn.h>
#include <algorithm>
#include <filesystem>

namespace hft {
namespace ai {

void MLModelManager::initialize() {
    // 初始化模型仓库
    initializeModelRegistry();
    
    // 设置默认参数
    params_.model_update_interval = std::chrono::minutes(30);
    params_.performance_threshold = 0.75;
    params_.max_ensemble_size = 5;
    params_.min_training_samples = 1000;
}

void MLModelManager::registerModel(
    const std::string& model_id,
    std::unique_ptr<BaseModel> model,
    const ModelConfig& config) {
    
    try {
        // 验证模型配置
        if (!validateModelConfig(config)) {
            throw std::runtime_error("Invalid model configuration");
        }
        
        // 注册模型
        ModelEntry entry{
            .model = std::move(model),
            .config = config,
            .status = ModelStatus::INITIALIZED,
            .last_update = std::chrono::system_clock::now(),
            .performance_metrics = {}
        };
        
        models_[model_id] = std::move(entry);
        Logger::info("Model {} registered successfully", model_id);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to register model {}: {}", model_id, e.what());
        throw;
    }
}

void MLModelManager::trainModel(
    const std::string& model_id,
    const TrainingData& data) {
    
    auto& model_entry = models_.at(model_id);
    
    try {
        // 1. 数据预处理
        auto processed_data = preprocessTrainingData(data);
        
        // 2. 特征工程
        auto features = extractFeatures(processed_data);
        
        // 3. 模型训练
        model_entry.model->train(features);
        
        // 4. 性能评估
        auto metrics = evaluateModel(model_entry.model.get(), processed_data);
        
        // 5. 更新模型状态
        updateModelStatus(model_id, metrics);
        
    } catch (const std::exception& e) {
        Logger::error("Model {} training failed: {}", model_id, e.what());
        model_entry.status = ModelStatus::ERROR;
        throw;
    }
}

Prediction MLModelManager::predict(
    const std::string& model_id,
    const PredictionInput& input) {
    
    const auto& model_entry = models_.at(model_id);
    
    // 检查模型状态
    if (model_entry.status != ModelStatus::READY) {
        throw std::runtime_error("Model is not ready for prediction");
    }
    
    try {
        // 1. 输入预处理
        auto processed_input = preprocessPredictionInput(input);
        
        // 2. 特征提取
        auto features = extractPredictionFeatures(processed_input);
        
        // 3. 模型预测
        auto prediction = model_entry.model->predict(features);
        
        // 4. 后处理
        return postprocessPrediction(prediction);
        
    } catch (const std::exception& e) {
        Logger::error("Model {} prediction failed: {}", model_id, e.what());
        throw;
    }
}

void MLModelManager::updateModel(
    const std::string& model_id,
    const TrainingData& new_data) {
    
    auto& model_entry = models_.at(model_id);
    
    try {
        // 1. 检查是否需要更新
        if (!needsUpdate(model_entry)) {
            return;
        }
        
        // 2. 增量训练
        auto processed_data = preprocessTrainingData(new_data);
        model_entry.model->update(processed_data);
        
        // 3. 评估新性能
        auto metrics = evaluateModel(model_entry.model.get(), processed_data);
        
        // 4. 判断是否接受更新
        if (acceptModelUpdate(model_entry, metrics)) {
            updateModelStatus(model_id, metrics);
        } else {
            rollbackUpdate(model_entry);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Model {} update failed: {}", model_id, e.what());
        rollbackUpdate(model_entry);
        throw;
    }
}

void MLModelManager::createEnsemble(
    const std::string& ensemble_id,
    const std::vector<std::string>& model_ids,
    const EnsembleConfig& config) {
    
    try {
        // 1. 验证成员模型
        validateEnsembleMembers(model_ids);
        
        // 2. 创建集成模型
        auto ensemble = createEnsembleModel(model_ids, config);
        
        // 3. 注册集成模型
        ModelConfig ensemble_config{
            .type = ModelType::ENSEMBLE,
            .parameters = config.parameters
        };
        registerModel(ensemble_id, std::move(ensemble), ensemble_config);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to create ensemble {}: {}", ensemble_id, e.what());
        throw;
    }
}

void MLModelManager::optimizeHyperparameters(
    const std::string& model_id,
    const TrainingData& data) {
    
    auto& model_entry = models_.at(model_id);
    
    try {
        // 1. 定义参数搜索空间
        auto param_space = defineParameterSpace(model_entry.config);
        
        // 2. 执行贝叶斯优化
        auto optimal_params = performBayesianOptimization(
            model_entry.model.get(), data, param_space);
        
        // 3. 更新模型参数
        model_entry.model->setParameters(optimal_params);
        
        // 4. 重新训练和评估
        trainModel(model_id, data);
        
    } catch (const std::exception& e) {
        Logger::error("Model {} hyperparameter optimization failed: {}", 
                     model_id, e.what());
        throw;
    }
}

void MLModelManager::saveModel(
    const std::string& model_id,
    const std::string& path) {
    
    const auto& model_entry = models_.at(model_id);
    
    try {
        // 1. 序列化模型
        auto serialized = serializeModel(model_entry);
        
        // 2. 保存到文件
        std::filesystem::path model_path(path);
        saveModelToFile(serialized, model_path);
        
        // 3. 保存元数据
        saveModelMetadata(model_entry, model_path);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to save model {}: {}", model_id, e.what());
        throw;
    }
}

void MLModelManager::loadModel(
    const std::string& model_id,
    const std::string& path) {
    
    try {
        // 1. 加载元数据
        auto metadata = loadModelMetadata(path);
        
        // 2. 创建模型实例
        auto model = createModelFromMetadata(metadata);
        
        // 3. 加载模型参数
        loadModelParameters(model.get(), path);
        
        // 4. 注册模型
        registerModel(model_id, std::move(model), metadata.config);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to load model {}: {}", model_id, e.what());
        throw;
    }
}

PerformanceMetrics MLModelManager::getModelMetrics(
    const std::string& model_id) const {
    
    const auto& model_entry = models_.at(model_id);
    return model_entry.performance_metrics;
}

ModelStatus MLModelManager::getModelStatus(
    const std::string& model_id) const {
    
    const auto& model_entry = models_.at(model_id);
    return model_entry.status;
}

} // namespace ai
} // namespace hft
