#include "VectorizedStrategyEngine.h"
#include <immintrin.h>
#include <iostream>

VectorizedStrategyEngine::VectorizedStrategyEngine(PhotonicEngine& engine, float threshold) 
    : photonicEngine(engine), riskThreshold(threshold), logger("strategy_engine.log", LogLevel::INFO) {
    logger.log(LogLevel::INFO, "Vectorized strategy engine initialized");
}

void VectorizedStrategyEngine::processMarketData(const MarketData& data) {
    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    // 确保数据足够进行向量化处理
    if (data.bids.size() < VECTOR_WIDTH || data.asks.size() < VECTOR_WIDTH) {
        logger.log(LogLevel::WARNING, "Insufficient market data for vector processing");
        return;
    }

    // 向量化处理买卖盘数据
    processOrderBook(data.bids, data.asks);

    // 计算延迟
    auto endTime = std::chrono::high_resolution_clock::now();
    uint64_t latency = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

    // 记录延迟
    logger.log(LogLevel::DEBUG, "Market data processing latency: " + std::to_string(latency) + " ns");

    // 跟踪延迟统计
    updateLatencyStats(latency);
}

void VectorizedStrategyEngine::processOrderBook(const std::vector<float>& bids, const std::vector<float>& asks) {
    // 加载数据到向量寄存器
    __m512 bidVector = _mm512_loadu_ps(bids.data());
    __m512 askVector = _mm512_loadu_ps(asks.data());

    // 向量化计算买卖压力: log(bid/ask)
    __m512 pressure = _mm512_log_ps(_mm512_div_ps(bidVector, askVector));

    // 提取压力值进行决策
    float pressureValues[VECTOR_WIDTH];
    _mm512_storeu_ps(pressureValues, pressure);

    // 基于压力值生成交易信号
    generateTradingSignals(pressureValues);
}

void VectorizedStrategyEngine::generateTradingSignals(const float* pressureValues) {
    // 检查是否有买入信号
    if (pressureValues[0] > riskThreshold) {
        // 创建买入订单
        Order buyOrder = createOrder(OrderType::BUY, "AAPL", 100.0, 10);
        if (photonicEngine.sendOrder(buyOrder)) {
            logger.log(LogLevel::INFO, "Buy order sent");
        }
    }
    // 检查是否有卖出信号
    else if (pressureValues[0] < -riskThreshold) {
        // 创建卖出订单
        Order sellOrder = createOrder(OrderType::SELL, "AAPL", 100.0, 10);
        if (photonicEngine.sendOrder(sellOrder)) {
            logger.log(LogLevel::INFO, "Sell order sent");
        }
    }
}

Order VectorizedStrategyEngine::createOrder(OrderType type, const std::string& symbol, double price, uint32_t quantity) {
    Order order;
    order.orderId = generateOrderId();
    order.symbol = symbol;
    order.type = type;
    order.price = price;
    order.quantity = quantity;
    order.timestamp = getCurrentNanoTimestamp();
    return order;
}

uint64_t VectorizedStrategyEngine::generateOrderId() {
    static uint64_t nextOrderId = 1;
    return nextOrderId++;
}

uint64_t VectorizedStrategyEngine::getCurrentNanoTimestamp() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

void VectorizedStrategyEngine::updateLatencyStats(uint64_t latency) {
    // 更新延迟统计信息
    static uint64_t minLatency = UINT64_MAX;
    static uint64_t maxLatency = 0;
    static uint64_t totalLatency = 0;
    static uint64_t sampleCount = 0;

    minLatency = std::min(minLatency, latency);
    maxLatency = std::max(maxLatency, latency);
    totalLatency += latency;
    sampleCount++;

    // 每1000个样本输出一次统计信息
    if (sampleCount % 1000 == 0) {
        uint64_t avgLatency = totalLatency / sampleCount;
        logger.log(LogLevel::INFO, "Latency stats - Min: " + std::to_string(minLatency) + " ns, Avg: " + 
                   std::to_string(avgLatency) + " ns, Max: " + std::to_string(maxLatency) + " ns");
    }
}