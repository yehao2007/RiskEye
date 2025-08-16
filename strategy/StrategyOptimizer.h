#pragma once
#include <vector>
#include <map>
#include <memory>
#include "../backtest/BacktestEngine.h"

namespace hft {
namespace strategy {

struct OptimizationParameter {
    std::string name;
    double minValue;
    double maxValue;
    double step;
    std::vector<double> customValues;  // 自定义参数值
};

struct OptimizationResult {
    std::map<std::string, double> bestParameters;
    double objectiveValue;  // 优化目标值（如夏普比率）
    backtest::BacktestReport report;
};

class StrategyOptimizer {
public:
    // 配置优化器
    void initialize(const std::vector<OptimizationParameter>& params);
    void setObjectiveFunction(std::function<double(const backtest::BacktestReport&)> func);
    
    // 优化方法
    OptimizationResult gridSearch();  // 网格搜索
    OptimizationResult geneticAlgorithm();  // 遗传算法
    OptimizationResult bayesianOptimization();  // 贝叶斯优化
    
    // 交叉验证
    std::vector<OptimizationResult> crossValidate(int folds);
    
    // 稳健性测试
    void walkForwardAnalysis();
    void monteCarloSimulation();
    
private:
    struct Individual {
        std::map<std::string, double> parameters;
        double fitness;
    };
    
    // 遗传算法辅助函数
    std::vector<Individual> initializePopulation();
    Individual crossover(const Individual& parent1, const Individual& parent2);
    void mutate(Individual& individual);
    Individual selectParent(const std::vector<Individual>& population);
    
    // 贝叶斯优化辅助函数
    double acquisitionFunction(const std::map<std::string, double>& params);
    void updateGaussianProcess(const std::map<std::string, double>& params, double result);
    
    std::vector<OptimizationParameter> parameters_;
    std::function<double(const backtest::BacktestReport&)> objectiveFunction_;
    std::unique_ptr<backtest::BacktestEngine> backtester_;
};

// 参数优化配置
struct OptimizationConfig {
    enum class Method {
        GRID_SEARCH,
        GENETIC_ALGORITHM,
        BAYESIAN_OPTIMIZATION
    };
    
    Method method;
    int populationSize;  // 用于遗传算法
    int generations;     // 用于遗传算法
    double mutationRate; // 用于遗传算法
    int numIterations;   // 用于贝叶斯优化
    bool useParallel;    // 是否使用并行计算
};

// 优化器工厂
class StrategyOptimizerFactory {
public:
    static std::unique_ptr<StrategyOptimizer> create(
        const OptimizationConfig& config,
        std::shared_ptr<AdvancedStrategy> strategy
    );
};

} // namespace strategy
} // namespace hft
