#include "StrategyOptimizer.h"
#include "../core/Logger.h"
#include <algorithm>
#include <random>
#include <thread>
#include <future>

namespace hft {
namespace strategy {

void StrategyOptimizer::initialize(const std::vector<OptimizationParameter>& params) {
    parameters_ = params;
    backtester_ = std::make_unique<backtest::BacktestEngine>();
    Logger::info("Strategy optimizer initialized with {} parameters", params.size());
}

OptimizationResult StrategyOptimizer::gridSearch() {
    OptimizationResult bestResult;
    bestResult.objectiveValue = std::numeric_limits<double>::lowest();
    
    // 创建参数网格
    std::vector<std::vector<double>> parameterGrid;
    for (const auto& param : parameters_) {
        std::vector<double> values;
        for (double value = param.minValue; value <= param.maxValue; value += param.step) {
            values.push_back(value);
        }
        parameterGrid.push_back(values);
    }
    
    // 并行网格搜索
    std::vector<std::future<OptimizationResult>> futures;
    for (size_t i = 0; i < parameterGrid[0].size(); ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, &parameterGrid]() {
            std::map<std::string, double> params;
            for (size_t j = 0; j < parameters_.size(); ++j) {
                params[parameters_[j].name] = parameterGrid[j][i];
            }
            
            auto report = backtester_->runBacktest(params);
            double objective = objectiveFunction_(report);
            
            return OptimizationResult{params, objective, report};
        }));
    }
    
    // 收集结果
    for (auto& future : futures) {
        auto result = future.get();
        if (result.objectiveValue > bestResult.objectiveValue) {
            bestResult = result;
        }
    }
    
    Logger::info("Grid search completed. Best objective value: {}", bestResult.objectiveValue);
    return bestResult;
}

OptimizationResult StrategyOptimizer::geneticAlgorithm() {
    const int populationSize = 100;
    const int generations = 50;
    const double mutationRate = 0.1;
    
    // 初始化种群
    std::vector<Individual> population = initializePopulation();
    
    for (int gen = 0; gen < generations; ++gen) {
        // 评估适应度
        for (auto& individual : population) {
            auto report = backtester_->runBacktest(individual.parameters);
            individual.fitness = objectiveFunction_(report);
        }
        
        // 选择和繁殖
        std::vector<Individual> newPopulation;
        while (newPopulation.size() < populationSize) {
            auto parent1 = selectParent(population);
            auto parent2 = selectParent(population);
            auto child = crossover(parent1, parent2);
            
            // 变异
            if (std::rand() / (double)RAND_MAX < mutationRate) {
                mutate(child);
            }
            
            newPopulation.push_back(child);
        }
        
        population = std::move(newPopulation);
        Logger::debug("Generation {} completed", gen + 1);
    }
    
    // 找出最佳个体
    auto best = *std::max_element(population.begin(), population.end(),
        [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
        });
    
    OptimizationResult result;
    result.bestParameters = best.parameters;
    result.objectiveValue = best.fitness;
    return result;
}

OptimizationResult StrategyOptimizer::bayesianOptimization() {
    const int numIterations = 100;
    OptimizationResult bestResult;
    bestResult.objectiveValue = std::numeric_limits<double>::lowest();
    
    for (int i = 0; i < numIterations; ++i) {
        // 使用高斯过程预测下一个采样点
        auto nextParams = predictNextParameters();
        
        // 评估该点
        auto report = backtester_->runBacktest(nextParams);
        double objective = objectiveFunction_(report);
        
        // 更新高斯过程
        updateGaussianProcess(nextParams, objective);
        
        // 更新最佳结果
        if (objective > bestResult.objectiveValue) {
            bestResult.bestParameters = nextParams;
            bestResult.objectiveValue = objective;
        }
        
        Logger::debug("Bayesian optimization iteration {} completed", i + 1);
    }
    
    return bestResult;
}

std::vector<OptimizationResult> StrategyOptimizer::crossValidate(int folds) {
    std::vector<OptimizationResult> results;
    auto data = backtester_->getHistoricalData();
    
    // 分割数据
    size_t foldSize = data.size() / folds;
    for (int i = 0; i < folds; ++i) {
        size_t testStart = i * foldSize;
        size_t testEnd = (i + 1) * foldSize;
        
        // 设置训练和测试数据
        backtester_->setTrainingData(data.begin(), data.begin() + testStart);
        backtester_->setTestingData(data.begin() + testStart, data.begin() + testEnd);
        
        // 在训练集上优化
        auto result = gridSearch();
        
        // 在测试集上验证
        auto testReport = backtester_->runBacktest(result.bestParameters);
        result.report = testReport;
        
        results.push_back(result);
    }
    
    return results;
}

Individual StrategyOptimizer::selectParent(const std::vector<Individual>& population) {
    // 锦标赛选择
    const int tournamentSize = 5;
    std::vector<Individual> tournament;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < tournamentSize; ++i) {
        int idx = std::uniform_int_distribution<>(0, population.size() - 1)(gen);
        tournament.push_back(population[idx]);
    }
    
    return *std::max_element(tournament.begin(), tournament.end(),
        [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
        });
}

void StrategyOptimizer::mutate(Individual& individual) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 0.1);
    
    for (auto& [name, value] : individual.parameters) {
        if (std::rand() / (double)RAND_MAX < 0.1) {
            auto it = std::find_if(parameters_.begin(), parameters_.end(),
                [&name](const OptimizationParameter& p) { return p.name == name; });
                
            if (it != parameters_.end()) {
                value = std::clamp(value + d(gen),
                                 it->minValue,
                                 it->maxValue);
            }
        }
    }
}

} // namespace strategy
} // namespace hft
