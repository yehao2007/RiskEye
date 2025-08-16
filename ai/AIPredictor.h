#pragma once

#include <vector>
#include <memory>
#include <torch/torch.h>
#include "../core/Types.h"
#include "../market/MarketData.h"

namespace hft {
namespace prediction {

class AIPredictor {
public:
    // 预测模型类型
    enum class ModelType {
        LSTM,           // 长短期记忆网络
        TRANSFORMER,    // Transformer模型
        ATTENTION_RNN,  // 注意力RNN
        HYBRID_CNN,    // 混合CNN
        GRU_GATE       // GRU门控网络
    };

    // 模型配置
    struct ModelConfig {
        ModelType type;
        int input_dim;
        int hidden_dim;
        int num_layers;
        double learning_rate;
        bool use_gpu;
        std::string checkpoint_path;
    };

    // 预测结果
    struct PredictionResult {
        double predicted_value;
        double confidence;
        std::vector<double> probability_distribution;
        std::vector<std::string> contributing_factors;
        uint64_t computation_time_ns;
    };

    // 初始化预测器
    bool initialize(const ModelConfig& config = defaultConfig()) {
        try {
            config_ = config;
            
            // 初始化CUDA设备
            if (config_.use_gpu && torch::cuda::is_available()) {
                device_ = torch::kCUDA;
                Logger::info("Using GPU for AI predictions");
            }
            
            // 创建模型
            createModel();
            
            // 加载预训练权重
            if (!config_.checkpoint_path.empty()) {
                loadCheckpoint(config_.checkpoint_path);
            }
            
            // 初始化优化器
            initializeOptimizer();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("AI predictor initialization failed: {}", e.what());
            return false;
        }
    }

    // 进行预测
    PredictionResult predict(const market::MarketData& data) {
        PredictionResult result;
        auto start_time = std::chrono::steady_clock::now();
        
        try {
            // 准备输入数据
            auto input_tensor = preprocessData(data);
            
            // 执行预测
            torch::NoGradGuard no_grad;
            auto output = model_->forward(input_tensor);
            
            // 后处理结果
            result = postprocessOutput(output);
            
        } catch (const std::exception& e) {
            Logger::error("Prediction failed: {}", e.what());
        }
        
        result.computation_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start_time).count();
        
        return result;
    }

    // 在线学习
    void onlineLearn(const market::MarketData& data, const double& actual_value) {
        try {
            // 准备训练数据
            auto input_tensor = preprocessData(data);
            auto target_tensor = torch::tensor({actual_value});
            
            // 执行训练步骤
            optimizer_->zero_grad();
            auto output = model_->forward(input_tensor);
            auto loss = criterion_(output, target_tensor);
            loss.backward();
            optimizer_->step();
            
            // 更新统计信息
            updateTrainingStats(loss.item<double>());
            
        } catch (const std::exception& e) {
            Logger::error("Online learning failed: {}", e.what());
        }
    }

    // 获取模型状态
    struct ModelStats {
        double training_loss;
        double validation_loss;
        int training_iterations;
        double learning_rate;
        std::vector<double> loss_history;
    };
    ModelStats getModelStats() const;

    // 保存模型
    void saveModel(const std::string& path);
    
    // 加载模型
    void loadModel(const std::string& path);

private:
    // 默认配置
    static ModelConfig defaultConfig() {
        return ModelConfig{
            .type = ModelType::TRANSFORMER,
            .input_dim = 256,
            .hidden_dim = 512,
            .num_layers = 4,
            .learning_rate = 0.001,
            .use_gpu = true
        };
    }

    // 创建模型
    void createModel() {
        switch (config_.type) {
            case ModelType::TRANSFORMER:
                model_ = std::make_shared<TransformerModel>(
                    config_.input_dim,
                    config_.hidden_dim,
                    config_.num_layers
                );
                break;
            case ModelType::LSTM:
                model_ = std::make_shared<LSTMModel>(
                    config_.input_dim,
                    config_.hidden_dim,
                    config_.num_layers
                );
                break;
            // ... 其他模型类型
        }
        
        if (device_ == torch::kCUDA) {
            model_->to(device_);
        }
    }

    // 预处理数据
    torch::Tensor preprocessData(const market::MarketData& data) {
        // 特征提取和标准化
        std::vector<float> features = extractFeatures(data);
        auto tensor = torch::from_blob(features.data(), {1, static_cast<long>(features.size())});
        return tensor.to(device_);
    }

    // 后处理输出
    PredictionResult postprocessOutput(const torch::Tensor& output) {
        PredictionResult result;
        auto probabilities = output.softmax(0);
        result.predicted_value = output.item<double>();
        result.confidence = calculateConfidence(probabilities);
        result.probability_distribution = tensorToVector(probabilities);
        result.contributing_factors = analyzeContributingFactors(output);
        return result;
    }

private:
    ModelConfig config_;
    std::shared_ptr<torch::nn::Module> model_;
    std::unique_ptr<torch::optim::Optimizer> optimizer_;
    torch::Device device_{torch::kCPU};
    torch::nn::MSELoss criterion_;
    
    // 训练统计
    struct TrainingStats {
        std::deque<double> loss_history;
        std::atomic<int> iterations{0};
        std::atomic<double> current_loss{0.0};
    } training_stats_;
    
    static constexpr size_t MAX_HISTORY = 1000;
    static constexpr int VALIDATION_INTERVAL = 100;
};

} // namespace prediction
} // namespace hft
