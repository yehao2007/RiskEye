#include <iostream>
#include <vector>
#include <memory>
#include "gtest/gtest.h"
#include "strategy/Strategy.h"
#include "strategy/AdaptiveStrategy.h"
#include "strategy/MarketMaking.h"
#include "strategy/StatisticalArbitrage.h"
#include "market/MarketData.h"
#include "market/OrderBook.h"
#include "core/TimeManager.h"
#include "utils/PerformanceMetrics.h"

namespace hft {
namespace tests {

// 测试策略基类
TEST(StrategyTest, BasicFunctionality) {
    // 创建时间管理器
    core::TimeManager time_manager;
    time_manager.initialize();
    time_manager.synchronizeWithGps("localhost:8080");

    // 创建策略
    strategy::Strategy strategy;
    strategy.initialize();

    // 创建市场数据
    market::MarketData market_data;
    market_data.timestamp = time_manager.getCurrentTimestamp();
    market_data.symbol = "AAPL";
    market_data.last_price = 150.0;
    market_data.volume = 1000;

    // 执行策略
    strategy.execute(market_data);

    // 检查策略状态
    EXPECT_TRUE(strategy.isInitialized());

    // 更新参数
    strategy::Parameters params;
    params.set("order_size", 10);
    strategy.updateParameters(params);

    // 关闭策略
    strategy.shutdown();
}

// 测试自适应策略
TEST(AdaptiveStrategyTest, MarketStateIdentification) {
    // 创建自适应策略
    strategy::AdaptiveStrategy adaptive_strategy;
    adaptive_strategy.initialize();

    // 创建策略池
    std::vector<std::unique_ptr<strategy::Strategy>> strategies;
    strategies.push_back(std::make_unique<strategy::MarketMaking>());
    strategies.push_back(std::make_unique<strategy::StatisticalArbitrage>());
    adaptive_strategy.setStrategyPool(strategies);

    // 创建市场数据（低波动率）
    market::MarketData low_vol_data;
    low_vol_data.timestamp = 1620000000000;
    low_vol_data.symbol = "AAPL";
    low_vol_data.last_price = 150.0;
    low_vol_data.volume = 1000;
    low_vol_data.volatility = 0.01;

    // 识别市场状态
    auto state = adaptive_strategy.identifyMarketState(low_vol_data);
    EXPECT_EQ(state, strategy::MarketState::LOW_VOLATILITY);

    // 创建市场数据（高波动率）
    market::MarketData high_vol_data;
    high_vol_data.timestamp = 1620000000000;
    high_vol_data.symbol = "AAPL";
    high_vol_data.last_price = 150.0;
    high_vol_data.volume = 5000;
    high_vol_data.volatility = 0.05;

    // 识别市场状态
    state = adaptive_strategy.identifyMarketState(high_vol_data);
    EXPECT_EQ(state, strategy::MarketState::HIGH_VOLATILITY);

    // 关闭策略
    adaptive_strategy.shutdown();
}

// 测试策略性能
TEST(StrategyPerformanceTest, ExecutionTime) {
    // 创建性能指标收集器
    utils::PerformanceMetrics metrics;

    // 创建自适应策略
    strategy::AdaptiveStrategy adaptive_strategy;
    adaptive_strategy.initialize();

    // 创建策略池
    std::vector<std::unique_ptr<strategy::Strategy>> strategies;
    strategies.push_back(std::make_unique<strategy::MarketMaking>());
    strategies.push_back(std::make_unique<strategy::StatisticalArbitrage>());
    adaptive_strategy.setStrategyPool(strategies);

    // 创建市场数据
    market::MarketData market_data;
    market_data.timestamp = 1620000000000;
    market_data.symbol = "AAPL";
    market_data.last_price = 150.0;
    market_data.volume = 1000;
    market_data.volatility = 0.02;

    // 测试多次执行时间
    const int num_iterations = 1000;
    for (int i = 0; i < num_iterations; ++i) {
        metrics.startTimer("strategy_execution");
        adaptive_strategy.execute(market_data);
        metrics.endTimer("strategy_execution");
    }

    // 打印性能指标
    std::cout << "Strategy execution performance metrics:" << std::endl;
    std::cout << "Average time: " << metrics.getAverage("strategy_execution") << " us" << std::endl;
    std::cout << "Min time: " << metrics.getMin("strategy_execution") << " us" << std::endl;
    std::cout << "Max time: " << metrics.getMax("strategy_execution") << " us" << std::endl;

    // 检查执行时间是否在合理范围内
    EXPECT_LT(metrics.getAverage("strategy_execution"), 100.0); // 平均执行时间应小于100微秒

    // 关闭策略
    adaptive_strategy.shutdown();
}

} // namespace tests
} // namespace hft

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}