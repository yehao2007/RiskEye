#pragma once

/**
 * 🏭 全球顶级风险预测工厂 - RiskPredictionFactory
 * 
 * 功能：
 * - 🎯 智能模型选择
 * - 🔄 动态模型切换
 * - 📊 性能基准测试
 * - 🛠️ 模型组装与配置
 * - 🚀 快速原型开发
 * - 📈 A/B测试框架
 */

#include "GlobalRiskPredictionModel.h"
#include "QuantumInterface.h"
#include "../core/Logger.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <future>

namespace hft {
namespace ai {

/**
 * 🎯 模型性能指标
 */
struct ModelPerformanceProfile {
    std::string model_id;
    ModelArchitecture architecture;
    double accuracy;
    double precision;
    double recall;
    double f1_score;
    double sharpe_ratio;
    uint64_t avg_prediction_time_ns;
    double memory_usage_mb;
    double gpu_utilization;
    
    // 特定风险类型性能
    std::unordered_map<RiskType, double> risk_specific_accuracy;
    
    // 时间范围性能
    std::unordered_map<PredictionHorizon, double> horizon_specific_accuracy;
    
    // 最近更新时间
    std::chrono::system_clock::time_point last_updated;
};

/**
 * 🧠 智能模型选择器
 */
class IntelligentModelSelector {
public:
    IntelligentModelSelector();
    
    // 根据条件选择最佳模型
    ModelArchitecture selectBestModel(
        RiskType risk_type,
        PredictionHorizon horizon,
        const MultiModalMarketData& current_market_state,
        double required_accuracy = 0.9,
        uint64_t max_latency_ns = 1000000  // 1ms
    );
    
    // 更新模型性能
    void updateModelPerformance(
        ModelArchitecture architecture,
        const ModelPerformanceProfile& performance
    );
    
    // 获取推荐模型组合
    std::vector<ModelArchitecture> getEnsembleRecommendation(
        RiskType risk_type,
        int max_models = 3
    );
    
    // 基于历史表现的自动模型排序
    std::vector<ModelArchitecture> rankModels(RiskType risk_type);
    
private:
    std::unordered_map<ModelArchitecture, ModelPerformanceProfile> model_profiles_;
    
    // 机器学习模型选择
    double calculateModelScore(
        const ModelPerformanceProfile& profile,
        RiskType risk_type,
        PredictionHorizon horizon,
        double required_accuracy,
        uint64_t max_latency_ns
    );
};

/**
 * 🔧 模型配置管理器
 */
class ModelConfigurationManager {
public:
    struct ModelConfig {
        ModelArchitecture architecture;
        json hyperparameters;
        std::string model_path;
        bool use_quantization;
        bool enable_distributed_inference;
        int gpu_devices;
        double memory_limit_gb;
        
        // 训练配置
        struct TrainingConfig {
            double learning_rate;
            int batch_size;
            int num_epochs;
            double dropout_rate;
            std::string optimizer;
            std::string scheduler;
            bool use_mixed_precision;
        } training_config;
        
        // 推理配置
        struct InferenceConfig {
            bool use_tensorrt;
            bool use_quantization;
            int max_batch_size;
            bool enable_cuda_graphs;
            bool use_dynamic_batching;
        } inference_config;
    };
    
    ModelConfigurationManager();
    
    // 配置管理
    void registerModelConfig(const std::string& config_name, const ModelConfig& config);
    ModelConfig getModelConfig(const std::string& config_name);
    std::vector<std::string> getAvailableConfigs();
    
    // 自动配置生成
    ModelConfig generateOptimalConfig(
        ModelArchitecture architecture,
        RiskType primary_risk_type,
        double target_accuracy,
        uint64_t target_latency_ns
    );
    
    // 配置优化
    ModelConfig optimizeConfig(
        const ModelConfig& base_config,
        const std::vector<ModelPerformanceProfile>& performance_history
    );
    
private:
    std::unordered_map<std::string, ModelConfig> configs_;
    
    // 超参数优化
    json optimizeHyperparameters(
        ModelArchitecture architecture,
        const json& search_space,
        int num_trials = 100
    );
};

/**
 * 🏭 风险预测工厂
 */
class RiskPredictionFactory {
public:
    RiskPredictionFactory();
    ~RiskPredictionFactory();
    
    // 工厂初始化
    bool initialize(const json& factory_config);
    void shutdown();
    
    // 模型创建
    std::unique_ptr<GlobalRiskPredictionModel> createModel(
        const std::string& model_name,
        const ModelConfigurationManager::ModelConfig& config
    );
    
    std::unique_ptr<GlobalRiskPredictionModel> createOptimalModel(
        RiskType primary_risk_type,
        PredictionHorizon primary_horizon,
        double target_accuracy = 0.95,
        uint64_t max_latency_ns = 500000  // 0.5ms
    );
    
    // 模型集成创建
    std::unique_ptr<GlobalRiskPredictionModel> createEnsembleModel(
        const std::vector<ModelArchitecture>& architectures,
        const std::vector<double>& weights = {}
    );
    
    // 快速原型
    std::unique_ptr<GlobalRiskPredictionModel> createPrototype(
        const std::string& experiment_name,
        const json& experiment_config
    );
    
    // 模型管理
    void registerModel(
        const std::string& model_id,
        std::shared_ptr<GlobalRiskPredictionModel> model
    );
    
    std::shared_ptr<GlobalRiskPredictionModel> getModel(const std::string& model_id);
    void removeModel(const std::string& model_id);
    
    // 模型评估与基准测试
    json benchmarkModel(
        const std::string& model_id,
        const std::vector<MultiModalMarketData>& test_data,
        const std::vector<RiskPrediction>& ground_truth
    );
    
    json compareModels(
        const std::vector<std::string>& model_ids,
        const std::vector<MultiModalMarketData>& test_data
    );
    
    // A/B测试
    void startABTest(
        const std::string& test_name,
        const std::vector<std::string>& model_ids,
        double traffic_split = 0.5,
        int duration_minutes = 60
    );
    
    json getABTestResults(const std::string& test_name);
    void stopABTest(const std::string& test_name);
    
    // 模型监控
    void enableModelMonitoring(const std::string& model_id);
    json getModelHealthMetrics(const std::string& model_id);
    void setAlertThresholds(const std::string& model_id, const json& thresholds);
    
    // 自动模型更新
    void enableAutoModelUpdate(
        const std::string& model_id,
        double performance_threshold = 0.05,
        int evaluation_window_hours = 24
    );
    
    // 模型版本管理
    void saveModelVersion(
        const std::string& model_id,
        const std::string& version_tag
    );
    
    bool rollbackModel(
        const std::string& model_id,
        const std::string& version_tag
    );
    
    std::vector<std::string> getModelVersions(const std::string& model_id);
    
    // 分布式推理
    void enableDistributedInference(
        const std::string& model_id,
        const std::vector<std::string>& worker_nodes
    );
    
    // 资源管理
    json getResourceUsage();
    void optimizeResourceAllocation();
    void setResourceLimits(const json& limits);
    
private:
    // 核心组件
    std::unique_ptr<IntelligentModelSelector> model_selector_;
    std::unique_ptr<ModelConfigurationManager> config_manager_;
    std::shared_ptr<quantum::QuantumProcessor> quantum_processor_;
    
    // 模型注册表
    std::unordered_map<std::string, std::shared_ptr<GlobalRiskPredictionModel>> models_;
    
    // 性能监控
    std::unordered_map<std::string, ModelPerformanceProfile> model_performance_;
    
    // A/B测试管理
    struct ABTest {
        std::string name;
        std::vector<std::string> model_ids;
        double traffic_split;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point end_time;
        json results;
        bool active;
    };
    std::unordered_map<std::string, ABTest> ab_tests_;
    
    // 版本管理
    std::unordered_map<std::string, std::vector<std::string>> model_versions_;
    
    // 配置
    json factory_config_;
    bool is_initialized_;
    
    // 辅助函数
    void initializeComponents();
    void setupQuantumProcessor();
    void validateModelConfig(const ModelConfigurationManager::ModelConfig& config);
    ModelPerformanceProfile evaluateModelPerformance(
        const std::string& model_id,
        const std::vector<MultiModalMarketData>& test_data
    );
    
    // 资源监控
    void monitorResourceUsage();
    std::thread resource_monitor_thread_;
    std::atomic<bool> monitoring_active_;
};

/**
 * 🚀 快速启动助手
 */
class RiskPredictionQuickStart {
public:
    // 预设配置
    static std::unique_ptr<GlobalRiskPredictionModel> createHighFrequencyModel() {
        RiskPredictionFactory factory;
        json config = {
            {"target_latency_ns", 100000},  // 100μs
            {"accuracy_threshold", 0.85},
            {"primary_risk_type", static_cast<int>(RiskType::MARKET_RISK)},
            {"enable_quantum", false}  // 禁用量子计算以降低延迟
        };
        factory.initialize(config);
        
        return factory.createOptimalModel(
            RiskType::MARKET_RISK,
            PredictionHorizon::MICROSECONDS_100,
            0.85,
            100000
        );
    }
    
    static std::unique_ptr<GlobalRiskPredictionModel> createHighAccuracyModel() {
        RiskPredictionFactory factory;
        json config = {
            {"target_accuracy", 0.99},
            {"enable_quantum", true},
            {"use_ensemble", true},
            {"max_latency_ms", 10}
        };
        factory.initialize(config);
        
        return factory.createOptimalModel(
            RiskType::SYSTEMIC_RISK,
            PredictionHorizon::MINUTES_1,
            0.99,
            10000000  // 10ms
        );
    }
    
    static std::unique_ptr<GlobalRiskPredictionModel> createBalancedModel() {
        RiskPredictionFactory factory;
        json config = {
            {"balance_accuracy_speed", true},
            {"target_accuracy", 0.92},
            {"target_latency_ns", 1000000},  // 1ms
            {"enable_adaptive_selection", true}
        };
        factory.initialize(config);
        
        return factory.createOptimalModel(
            RiskType::VOLATILITY_RISK,
            PredictionHorizon::MILLISECONDS_10,
            0.92,
            1000000
        );
    }
    
    // 一键部署
    static bool deployProductionSystem(
        const std::string& deployment_config_path,
        const std::vector<std::string>& target_hosts
    ) {
        try {
            RiskPredictionFactory factory;
            
            // 读取部署配置
            std::ifstream config_file(deployment_config_path);
            json deploy_config;
            config_file >> deploy_config;
            
            // 初始化工厂
            factory.initialize(deploy_config);
            
            // 创建生产模型
            auto prod_model = factory.createOptimalModel(
                RiskType::MARKET_RISK,
                PredictionHorizon::MILLISECONDS_1,
                0.95,
                500000  // 0.5ms
            );
            
            // 启用分布式推理
            factory.enableDistributedInference("production_model", target_hosts);
            
            // 启用监控
            factory.enableModelMonitoring("production_model");
            
            // 设置自动更新
            factory.enableAutoModelUpdate("production_model");
            
            Logger::info("Production risk prediction system deployed successfully");
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Deployment failed: {}", e.what());
            return false;
        }
    }
};

} // namespace ai
} // namespace hft
