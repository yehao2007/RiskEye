#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "core/System.h"
#include "market/MarketData.h"
#include "execution/Order.h"
#include "utils/PerformanceMetrics.h"

namespace hft {
namespace tests {

// 系统集成测试
TEST(SystemTest, BasicFunctionality) {
    // 创建系统
    core::System system;

    // 初始化系统
    bool initialized = system.initialize();
    EXPECT_TRUE(initialized);

    // 运行系统
    system.run();

    // 等待系统启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 创建市场数据
    market::MarketData market_data;
    market_data.timestamp = system.getTimeManager()->getCurrentTimestamp();
    market_data.symbol = "AAPL";
    market_data.last_price = 150.0;
    market_data.volume = 1000;
    market_data.volatility = 0.02;

    // 推送市场数据到系统
    system.pushMarketData(market_data);

    // 等待策略处理
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 获取生成的订单
    std::vector<execution::Order> orders = system.getGeneratedOrders();
    std::cout << "Generated " << orders.size() << " orders" << std::endl;

    // 停止系统
    system.stop();
}

// 系统性能测试
TEST(SystemTest, Performance) {
    // 创建性能指标收集器
    utils::PerformanceMetrics metrics;

    // 创建系统
    core::System system;

    // 初始化系统
    system.initialize();

    // 运行系统
    system.run();

    // 等待系统启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 测试系统处理市场数据的性能
    const int num_iterations = 1000;
    for (int i = 0; i < num_iterations; ++i) {
        // 创建市场数据
        market::MarketData market_data;
        market_data.timestamp = system.getTimeManager()->getCurrentTimestamp();
        market_data.symbol = "AAPL";
        market_data.last_price = 150.0 + (i % 100) * 0.1;
        market_data.volume = 1000 + i * 10;
        market_data.volatility = 0.02 + (i % 5) * 0.001;

        // 计时并推送市场数据
        metrics.startTimer("market_data_processing");
        system.pushMarketData(market_data);
        metrics.endTimer("market_data_processing");
    }

    // 等待所有数据处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 打印性能指标
    std::cout << "System performance metrics:" << std::endl;
    std::cout << "Average market data processing time: " << metrics.getAverage("market_data_processing") << " us" << std::endl;
    std::cout << "Min time: " << metrics.getMin("market_data_processing") << " us" << std::endl;
    std::cout << "Max time: " << metrics.getMax("market_data_processing") << " us" << std::endl;

    // 停止系统
    system.stop();
}

} // namespace tests
} // namespace hft

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}