#pragma once

#include <memory>
#include <vector>
#include <string>
#include "../core/Types.h"
#include "../core/Logger.h"

namespace hft {
namespace core {

class ConfigOptimizer {
public:
    // 参数类型
    enum class ParamType {
        INTEGER,
        FLOAT,
        CATEGORICAL,
        BOOLEAN
    };

    // 参数定义
    struct Parameter {
        std::string name;
        ParamType type;
        double min_value;
        double max_value;
        std::vector<std::string> categories;
        double current_value;
        double step_size;
        bool is_tunable;
    };

    // 优化目标
    struct OptimizationObjective {
        std::string metric_name;
        double target_value;
        double weight;
        bool maximize;
    };

    // 初始化优化器
    bool initialize() {
        try {
            // 初始化搜索空间
            initializeSearchSpace();
            
            // 启动优化循环
            startOptimizationLoop();
            
            // 初始化机器学习模型
            initializeMLModel();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Config optimizer initialization failed: {}", e.what());
            return false;
        }
    }

    // 添加参数
    void addParameter(const Parameter& param) {
        parameters_.push_back(param);
    }

    // 设置优化目标
    void setObjective(const OptimizationObjective& objective) {
        objectives_.push_back(objective);
    }

    // 更新指标
    void updateMetric(const std::string& metric_name, double value) {
        metric_history_[metric_name].push_back(value);
        triggerOptimization();
    }

    // 获取优化建议
    struct OptimizationSuggestion {
        std::unordered_map<std::string, double> param_values;
        double expected_improvement;
        double confidence;
        std::vector<std::string> reasoning;
    };
    OptimizationSuggestion getSuggestion();

    // 应用配置
    bool applyConfiguration(const std::unordered_map<std::string, double>& config);

    // 获取优化状态
    struct OptimizationStats {
        int iterations;
        double best_score;
        std::unordered_map<std::string, double> best_config;
        double convergence_rate;
        std::vector<double> improvement_history;
    };
    OptimizationStats getStats() const;

private:
    // 贝叶斯优化器
    class BayesianOptimizer {
    public:
        void initialize(const std::vector<Parameter>& params);
        OptimizationSuggestion suggest();
        void update(const std::unordered_map<std::string, double>& config,
                   double score);
    
    private:
        // 高斯过程模型
        struct GPModel {
            torch::Tensor train_X;
            torch::Tensor train_y;
            torch::Tensor kernel_params;
        };
        GPModel gp_model_;
    };

    // 进化策略优化器
    class EvolutionaryOptimizer {
    public:
        void initialize(const std::vector<Parameter>& params);
        std::vector<std::unordered_map<std::string, double>> evolve();
        void updatePopulation(const std::vector<double>& fitness);
    
    private:
        std::vector<std::unordered_map<std::string, double>> population_;
        static constexpr int POPULATION_SIZE = 100;
    };

    // 强化学习优化器
    class RLOptimizer {
    public:
        void initialize(const std::vector<Parameter>& params);
        std::unordered_map<std::string, double> getAction(
            const std::unordered_map<std::string, double>& state);
        void update(double reward);
    
    private:
        torch::nn::Sequential policy_network;
        torch::nn::Sequential value_network;
    };

private:
    // 核心优化方法
    void initializeSearchSpace();
    void startOptimizationLoop();
    void triggerOptimization();
    double evaluateConfiguration(
        const std::unordered_map<std::string, double>& config);
    
    // 智能搜索策略
    void performBayesianOptimization();
    void performEvolutionarySearch();
    void performReinforcementLearning();
    
    // 约束处理
    bool validateConfiguration(
        const std::unordered_map<std::string, double>& config);
    void enforceConstraints(
        std::unordered_map<std::string, double>& config);

private:
    std::vector<Parameter> parameters_;
    std::vector<OptimizationObjective> objectives_;
    std::unique_ptr<BayesianOptimizer> bayes_opt_;
    std::unique_ptr<EvolutionaryOptimizer> evo_opt_;
    std::unique_ptr<RLOptimizer> rl_opt_;
    
    // 优化历史
    std::unordered_map<std::string, std::vector<double>> metric_history_;
    std::vector<std::unordered_map<std::string, double>> config_history_;
    std::vector<double> score_history_;
    
    // 优化参数
    struct OptimizationParams {
        int max_iterations{1000};
        double convergence_threshold{1e-6};
        int patience{20};
        bool use_parallel_search{true};
    } opt_params_;
};

} // namespace core
} // namespace hft
