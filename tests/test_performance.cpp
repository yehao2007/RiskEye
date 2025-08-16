#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include "core/System.h"
#include "execution/OrderExecution.h"
#include "market/FeatureExtractor.h"
#include "hardware/AsicAccelerator.h"

using namespace hft;
using namespace std::chrono;

// 性能测试工具类
class PerformanceTester {
public:
    PerformanceTester(const std::string& testName) : m_testName(testName) {}

    void start() {
        m_startTime = high_resolution_clock::now();
    }

    void stop() {
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(endTime - m_startTime).count();
        m_totalDuration = duration;
        std::cout << m_testName << " took " << duration << " microseconds" << std::endl;
    }

    double getDurationMs() const {
        return m_totalDuration / 1000.0;
    }

private:
    std::string m_testName;
    high_resolution_clock::time_point m_startTime;
    uint64_t m_totalDuration = 0;
};

// 测试订单执行性能
void testOrderExecutionPerformance() {
    std::cout << "\n=== Testing Order Execution Performance ===" << std::endl;

    // 创建系统和订单执行器
    auto system = std::make_shared<core::System>();
    system->initialize();
    auto orderExecution = system->getOrderExecution();

    // 准备测试订单
    const int numOrders = 10000;
    std::vector<execution::Order> orders;
    orders.reserve(numOrders);

    // 生成随机订单
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> priceDist(100.0, 200.0);
    std::uniform_int_distribution<> sizeDist(1, 100);

    for (int i = 0; i < numOrders; ++i) {
        execution::Order order;
        order.symbol = "AAPL";
        order.price = priceDist(gen);
        order.size = sizeDist(gen);
        order.side = (i % 2 == 0) ? execution::OrderSide::BUY : execution::OrderSide::SELL;
        order.type = execution::OrderType::MARKET;
        orders.push_back(order);
    }

    // 测试订单执行
    PerformanceTester tester("Order Execution");
    tester.start();

    for (const auto& order : orders) {
        orderExecution->executeOrder(order);
    }

    tester.stop();
    std::cout << "Throughput: " << (numOrders / tester.getDurationMs() * 1000) << " orders/second" << std::endl;
}

// 测试特征提取性能
void testFeatureExtractionPerformance() {
    std::cout << "\n=== Testing Feature Extraction Performance ===" << std::endl;

    // 创建特征提取器
    market::FeatureExtractor featureExtractor;

    // 准备测试数据
    const int numSamples = 10000;
    std::vector<market::MarketData> marketDataSamples;
    marketDataSamples.reserve(numSamples);

    // 生成随机市场数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> priceDist(100.0, 200.0);
    std::uniform_real_distribution<> sizeDist(1.0, 1000.0);

    for (int i = 0; i < numSamples; ++i) {
        market::MarketData data;
        data.symbol = "AAPL";
        data.timestamp = i;
        data.lastPrice = priceDist(gen);
        data.bidPrice = data.lastPrice * 0.999;
        data.askPrice = data.lastPrice * 1.001;
        data.bidSize = sizeDist(gen);
        data.askSize = sizeDist(gen);

        // 添加一些深度数据
        for (int j = 0; j < 5; ++j) {
            data.bids.push_back({data.bidPrice - j * 0.01, sizeDist(gen)});
            data.asks.push_back({data.askPrice + j * 0.01, sizeDist(gen)});
        }

        marketDataSamples.push_back(data);
    }

    // 测试特征提取
    PerformanceTester tester("Feature Extraction");
    tester.start();

    for (const auto& data : marketDataSamples) {
        featureExtractor.extractFeatures(data);
    }

    tester.stop();
    std::cout << "Throughput: " << (numSamples / tester.getDurationMs() * 1000) << " samples/second" << std::endl;
}

// 测试ASIC加速性能
void testAsicAccelerationPerformance() {
    std::cout << "\n=== Testing ASIC Acceleration Performance ===" << std::endl;

    // 创建ASIC加速器
    hardware::AsicAccelerator asicAccelerator;
    if (!asicAccelerator.initialize()) {
        std::cerr << "Failed to initialize ASIC accelerator, skipping test" << std::endl;
        return;
    }

    // 准备测试数据
    const int dataSize = 1000000;
    std::vector<float> inputData(dataSize);
    std::vector<float> outputData(dataSize);

    // 生成随机输入数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(-1.0, 1.0);

    for (int i = 0; i < dataSize; ++i) {
        inputData[i] = static_cast<float>(dist(gen));
    }

    // 测试ASIC加速计算
    PerformanceTester tester("ASIC Acceleration");
    tester.start();

    asicAccelerator.executeTask(inputData.data(), outputData.data(), dataSize);

    tester.stop();
    std::cout << "Performance: " << (dataSize / 1000000.0) / (tester.getDurationMs() / 1000.0) << " million operations/second" << std::endl;
}

int main() {
    std::cout << "Starting performance tests..." << std::endl;

    // 运行所有性能测试
    testOrderExecutionPerformance();
    testFeatureExtractionPerformance();
    testAsicAccelerationPerformance();

    std::cout << "All performance tests completed." << std::endl;
    return 0;
}