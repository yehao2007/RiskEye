#include "evolution/EvolutionEngine.h"
#include "core/Logger.h"
#include "core/Configuration.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <nlohmann/json.hpp>

namespace hft {
namespace evolution {

EvolutionEngine::EvolutionEngine(core::System* system,
                                ai::IntelligentDecisionSystem* decision_system,
                                risk::AdvancedRiskManager* risk_manager)
    : system_(system),
      decision_system_(decision_system),
      risk_manager_(risk_manager),
      is_learning_(false),
      learning_rate_(0.01f) {
    // 初始化进化引擎
    if (system_ == nullptr || decision_system_ == nullptr || risk_manager_ == nullptr) {
        throw std::invalid_argument("Invalid system components");
    }
}

bool EvolutionEngine::initialize(const std::string& config_path) {
    try {
        // 加载配置文件
        std::ifstream config_file(config_path);
        if (!config_file.is_open()) {
            HFT_LOG_ERROR("Failed to open configuration file: {}", config_path);
            return false;
        }

        nlohmann::json config;
        config_file >> config;

        // 读取学习速率
        if (config.contains("learning_rate")) {
            learning_rate_ = config["learning_rate"];
        }

        HFT_LOG_INFO("Evolution engine initialized with config: {}", config_path);
        return true;
    } catch (const std::exception& e) {
        HFT_LOG_ERROR("Failed to initialize evolution engine: {}", e.what());
        return false;
    }
}

void EvolutionEngine::startLearning() {
    if (is_learning_) {
        HFT_LOG_WARNING("Learning is already running");
        return;
    }

    is_learning_ = true;
    learning_thread_ = std::thread(&EvolutionEngine::learningLoop, this);
    HFT_LOG_INFO("Adaptive learning started");
}

void EvolutionEngine::stopLearning() {
    if (!is_learning_) {
        HFT_LOG_WARNING("Learning is not running");
        return;
    }

    is_learning_ = false;
    if (learning_thread_.joinable()) {
        learning_thread_.join();
    }
    HFT_LOG_INFO("Adaptive learning stopped");
}

bool EvolutionEngine::isLearning() const {
    return is_learning_;
}

void EvolutionEngine::setLearningRate(float rate) {
    if (rate <= 0.0f) {
        HFT_LOG_ERROR("Invalid learning rate: {}", rate);
        return;
    }

    learning_rate_ = rate;
    HFT_LOG_INFO("Learning rate set to: {}", rate);
}

void EvolutionEngine::learningLoop() {
    while (is_learning_) {
        // 评估当前系统性能
        float performance = evaluatePerformance();

        // 收集性能指标
        std::vector<float> performance_metrics = {performance};

        // 优化系统参数
        optimizeParameters(performance_metrics);

        // 保存优化后的参数
        saveOptimizedParameters();

        // 等待一段时间再进行下一次迭代
        std::this_thread::sleep_for(std::chrono::seconds(30)); // 每30秒执行一次
    }
}

float EvolutionEngine::evaluatePerformance() {
    // 评估系统性能的综合分数
    float performance_score = 0.0f;

    // 1. 获取决策延迟指标
    uint64_t decision_latency = decision_system_->getLastDecisionLatency();
    float latency_score = 100.0f - static_cast<float>(decision_latency) / 100.0f;
    latency_score = std::max(0.0f, std::min(100.0f, latency_score));

    // 2. 获取风险评分
    double risk_score = risk_manager_->getRiskScore();
    float risk_metric = 100.0f - static_cast<float>(std::min(100.0, risk_score));

    // 3. 假设我们还有其他性能指标
    float market_accuracy = 85.0f; // 示例值

    // 加权计算综合分数
    performance_score = 0.4f * latency_score + 0.3f * risk_metric + 0.3f * market_accuracy;

    HFT_LOG_INFO("Performance evaluation: {:.2f} (latency: {:.2f}, risk: {:.2f}, accuracy: {:.2f})",
                 performance_score, latency_score, risk_metric, market_accuracy);
    return performance_score;
}

void EvolutionEngine::optimizeParameters(const std::vector<float>& performance_metrics) {
    // 基于性能指标优化参数
    float current_performance = performance_metrics.back();

    // 自适应调整学习速率
    if (current_performance > 90.0f) {
        // 性能很好，降低学习速率以保持稳定
        setLearningRate(learning_rate_ * 0.9f);
    } else if (current_performance < 70.0f) {
        // 性能不佳，增加学习速率以加快改进
        setLearningRate(learning_rate_ * 1.1f);
    }

    // 示例：根据性能调整风险参数
    if (current_performance < 60.0f) {
        risk_manager_->setRiskLimits(50000, 0.03, 1.5); // 降低风险容忍度
    } else if (current_performance > 85.0f) {
        risk_manager_->setRiskLimits(150000, 0.07, 2.5); // 提高风险容忍度
    }

    HFT_LOG_INFO("Parameters optimized based on performance: {:.2f}", current_performance);
}

void EvolutionEngine::saveOptimizedParameters() {
    try {
        // 保存优化后的参数
        std::ofstream params_file("config/optimized_params.json");
        if (!params_file.is_open()) {
            HFT_LOG_ERROR("Failed to open parameters file for writing");
            return;
        }

        nlohmann::json params;
        params["learning_rate"] = learning_rate_;

        // 保存其他参数
        auto risk_limits = nlohmann::json::object();
        risk_limits["max_position_size"] = 100000; // 示例值
        risk_limits["max_daily_loss"] = 0.05; // 示例值
        risk_limits["var_threshold"] = 2.0; // 示例值
        params["risk_limits"] = risk_limits;

        params_file << params.dump(4);
        HFT_LOG_INFO("Optimized parameters saved to config/optimized_params.json");
    } catch (const std::exception& e) {
        HFT_LOG_ERROR("Failed to save optimized parameters: {}", e.what());
    }
}

} // namespace evolution
} // namespace hft