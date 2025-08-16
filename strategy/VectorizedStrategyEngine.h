#pragma once
#include <immintrin.h>
#include <vector>
#include <cmath>
#include "hardware/PhotonicEngine.h"
#include "LowLatencyLogger.h"

class VectorizedStrategyEngine {
private:
    // AVX-512向量化处理
    static constexpr int VECTOR_WIDTH = 16; // 512位/32位浮点数 = 16个元素
    PhotonicEngine& photonicEngine;
    LowLatencyLogger& logger;
    float riskThreshold;

    // 延迟统计
    std::atomic<uint32_t> minLatency{UINT32_MAX};
    std::atomic<uint32_t> maxLatency{0};
    std::atomic<uint64_t> totalLatency{0};
    std::atomic<uint64_t> processedCount{0};

public:
    VectorizedStrategyEngine(PhotonicEngine& engine, LowLatencyLogger& log, float threshold = 1.5f)
        : photonicEngine(engine), logger(log), riskThreshold(threshold) {}

    // 向量化订单簿处理
    void processOrderBook(const std::vector<float>& bids, const std::vector<float>& asks) {
        auto startTime = __rdtsc(); // 开始计时

        // 确保输入数据足够长
        if (bids.size() < VECTOR_WIDTH || asks.size() < VECTOR_WIDTH) {
            logger.logWarning("Insufficient data for vectorization");
            return;
        }

        // 加载数据到向量寄存器
        __m512 bidVector = _mm512_loadu_ps(bids.data());
        __m512 askVector = _mm512_loadu_ps(asks.data());

        // 向量化计算买卖压力: log(bid/ask)
        __m512 pressure = _mm512_log_ps(_mm512_div_ps(bidVector, askVector));

        // 提取第一个lane的值进行决策
        float* pressureValues = (float*)&pressure;
        Order order;
        if (pressureValues[0] > riskThreshold) {
            // 触发光子引擎发送买入订单
            order.type = OrderType::BUY;
            // 设置其他订单字段...
            photonicEngine.sendOrder(order);
        } else if (pressureValues[0] < -riskThreshold) {
            // 触发光子引擎发送卖出订单
            order.type = OrderType::SELL;
            // 设置其他订单字段...
            photonicEngine.sendOrder(order);
        }

        // 计算并记录延迟
        auto endTime = __rdtsc();
        uint32_t latency = static_cast<uint32_t>((endTime - startTime) / 3.5); // 假设3.5GHz CPU
        logger.logLatency("Strategy processing", latency);

        // 更新统计信息
        if (latency < minLatency) minLatency = latency;
        if (latency > maxLatency) maxLatency = latency;
        totalLatency += latency;
        processedCount++;
    }

    // 获取性能统计
    void getPerformanceStats(uint32_t& min, uint32_t& max, double& avg) {
        min = minLatency;
        max = maxLatency;
        avg = processedCount ? static_cast<double>(totalLatency) / processedCount : 0;
    }
};