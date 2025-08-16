#include <gtest/gtest.h>
#include "../strategy/StrategyOptimizer.h"
#include "../market/MarketImpactModel.h"
#include "../utils/PerformanceMonitor.h"
#include <memory>

namespace hft {
namespace tests {

class StrategyOptimizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        optimizer_ = std::make_unique<strategy::StrategyOptimizer>();
        std::vector<strategy::OptimizationParameter> params = {
            {"param1", 0.0, 1.0, 0.1},
            {"param2", -1.0, 1.0, 0.2}
        };
        optimizer_->initialize(params);
    }
    
    std::unique_ptr<strategy::StrategyOptimizer> optimizer_;
};

TEST_F(StrategyOptimizerTest, GridSearchTest) {
    auto result = optimizer_->gridSearch();
    EXPECT_GT(result.objectiveValue, -std::numeric_limits<double>::infinity());
    EXPECT_FALSE(result.bestParameters.empty());
}

TEST_F(StrategyOptimizerTest, GeneticAlgorithmTest) {
    auto result = optimizer_->geneticAlgorithm();
    EXPECT_GT(result.objectiveValue, -std::numeric_limits<double>::infinity());
    EXPECT_FALSE(result.bestParameters.empty());
}

TEST_F(StrategyOptimizerTest, CrossValidationTest) {
    auto results = optimizer_->crossValidate(5);
    EXPECT_EQ(results.size(), 5);
    for (const auto& result : results) {
        EXPECT_GT(result.objectiveValue, -std::numeric_limits<double>::infinity());
    }
}

class MarketImpactModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        model_ = std::make_unique<market::MarketImpactModel>();
        market::MarketImpactModel::ImpactParameters params{
            .temporaryImpactFactor = 0.1,
            .permanentImpactFactor = 0.05,
            .decayFactor = 0.2,
            .volumeThreshold = 1000.0
        };
        model_->initialize(params);
    }
    
    std::unique_ptr<market::MarketImpactModel> model_;
};

TEST_F(MarketImpactModelTest, ImpactCostCalculation) {
    market::Order order{.volume = 1000, .price = 100.0};
    market::L2Data orderBook;  // 填充测试数据
    
    double cost = model_->calculateImpactCost(order, orderBook);
    EXPECT_GT(cost, 0.0);
}

TEST_F(MarketImpactModelTest, ExecutionOptimization) {
    market::Order order{.volume = 5000, .price = 100.0};
    market::MarketCondition condition;  // 填充测试数据
    
    auto plan = model_->optimizeExecution(order, condition);
    EXPECT_GT(plan.volumes.size(), 0);
    EXPECT_GT(plan.timings.size(), 0);
}

class PerformanceMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        monitor_ = std::make_unique<utils::PerformanceMonitor>();
        monitor_->initialize();
    }
    
    std::unique_ptr<utils::PerformanceMonitor> monitor_;
};

TEST_F(PerformanceMonitorTest, LatencyRecording) {
    monitor_->recordLatency(std::chrono::microseconds(100));
    auto metrics = monitor_->getCurrentMetrics();
    EXPECT_GT(metrics.latency, 0.0);
}

TEST_F(PerformanceMonitorTest, SystemMetrics) {
    monitor_->recordSystemMetrics();
    auto metrics = monitor_->getCurrentMetrics();
    EXPECT_GE(metrics.cpuUsage, 0.0);
    EXPECT_LE(metrics.cpuUsage, 100.0);
    EXPECT_GE(metrics.memoryUsage, 0.0);
    EXPECT_LE(metrics.memoryUsage, 100.0);
}

TEST_F(PerformanceMonitorTest, AlertSystem) {
    std::map<std::string, double> thresholds{
        {"latency", 1000.0},
        {"cpuUsage", 90.0}
    };
    monitor_->setAlertThresholds(thresholds);
    
    monitor_->recordLatency(std::chrono::microseconds(2000));
    auto metrics = monitor_->getCurrentMetrics();
    EXPECT_TRUE(monitor_->checkAlerts(metrics));
}

} // namespace tests
} // namespace hft

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
