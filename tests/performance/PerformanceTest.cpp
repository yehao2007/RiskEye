#include <gtest/gtest.h>
#include <benchmark/benchmark.h>
#include "../utils/PerformanceMonitor.h"
#include "../market/MarketDataAggregator.h"
#include "../execution/OrderExecutor.h"

// 市场数据处理性能测试
static void BM_MarketDataProcessing(benchmark::State& state) {
    hft::market::MarketDataAggregator aggregator;
    hft::market::MarketData data;
    
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            aggregator.addData(data);
        }
        aggregator.processBatch(state.range(0));
    }
}
BENCHMARK(BM_MarketDataProcessing)->Range(8, 8<<10);

// 订单执行性能测试
static void BM_OrderExecution(benchmark::State& state) {
    hft::execution::OrderExecutor executor;
    hft::execution::Order order;
    
    for (auto _ : state) {
        executor.processOrder(order);
    }
}
BENCHMARK(BM_OrderExecution)->Iterations(1000000);

// 系统延迟测试
static void BM_SystemLatency(benchmark::State& state) {
    hft::utils::PerformanceMonitor monitor;
    
    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        // 模拟系统操作
        benchmark::DoNotOptimize(monitor.recordLatency(
            std::chrono::high_resolution_clock::now() - start
        ));
    }
}
BENCHMARK(BM_SystemLatency)->Iterations(1000000);

BENCHMARK_MAIN();
