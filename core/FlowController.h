#pragma once

#include <atomic>
#include <chrono>
#include <queue>
#include "../core/Types.h"
#include "../core/Logger.h"

namespace hft {
namespace core {

class FlowController {
public:
    // 流量控制模式
    enum class ControlMode {
        RATE_LIMIT,     // 速率限制
        ADAPTIVE,       // 自适应控制
        PREDICTIVE,     // 预测性控制
        CONGESTION      // 拥塞控制
    };

    // 流量控制策略
    struct ControlPolicy {
        uint64_t max_messages_per_second;    // 最大消息速率
        uint64_t max_bytes_per_second;       // 最大字节速率
        uint64_t burst_size;                 // 突发大小
        double target_latency_ms;            // 目标延迟
        double congestion_threshold;         // 拥塞阈值
    };

    // 初始化流量控制器
    bool initialize(const ControlPolicy& policy = defaultPolicy()) {
        try {
            policy_ = policy;
            token_bucket_.store(policy.burst_size);
            last_refill_time_ = std::chrono::steady_clock::now();
            
            // 启动监控线程
            startMonitoring();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Flow controller initialization failed: {}", e.what());
            return false;
        }
    }

    // 检查是否允许发送消息
    bool canSend(size_t message_size) {
        refillTokens();
        
        // 获取当前可用令牌
        auto available_tokens = token_bucket_.load();
        
        // 检查是否有足够的令牌
        if (available_tokens >= message_size) {
            // 尝试原子减少令牌
            while (available_tokens >= message_size) {
                if (token_bucket_.compare_exchange_weak(
                    available_tokens, 
                    available_tokens - message_size)) {
                    return true;
                }
            }
        }
        
        return false;
    }

    // 设置控制模式
    void setControlMode(ControlMode mode) {
        control_mode_ = mode;
        adaptControlStrategy();
    }

    // 更新流量控制策略
    void updatePolicy(const ControlPolicy& new_policy) {
        policy_ = new_policy;
        adaptControlStrategy();
    }

    // 获取流量统计
    struct FlowStats {
        uint64_t total_messages;
        uint64_t total_bytes;
        double current_message_rate;
        double current_byte_rate;
        double avg_latency_ms;
        double congestion_level;
    };
    FlowStats getStats() const;

    // 注册拥塞回调
    using CongestionCallback = std::function<void(double level)>;
    void registerCongestionCallback(CongestionCallback callback) {
        congestion_callback_ = std::move(callback);
    }

private:
    // 默认策略
    static ControlPolicy defaultPolicy() {
        return ControlPolicy{
            .max_messages_per_second = 1000000,  // 1M消息/秒
            .max_bytes_per_second = 1024*1024*1024, // 1GB/秒
            .burst_size = 10000,
            .target_latency_ms = 0.1,           // 100微秒
            .congestion_threshold = 0.8
        };
    }

    // 补充令牌
    void refillTokens() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now - last_refill_time_).count();
            
        if (elapsed > 0) {
            // 计算需要补充的令牌
            auto tokens_to_add = (elapsed * policy_.max_messages_per_second) / 1e9;
            
            // 更新令牌桶
            auto current = token_bucket_.load();
            auto new_value = std::min(
                current + tokens_to_add,
                policy_.burst_size
            );
            
            token_bucket_.store(new_value);
            last_refill_time_ = now;
        }
    }

    // 自适应控制策略
    void adaptControlStrategy() {
        switch (control_mode_) {
            case ControlMode::ADAPTIVE:
                adaptToLatency();
                break;
            case ControlMode::PREDICTIVE:
                predictCongestion();
                break;
            case ControlMode::CONGESTION:
                handleCongestion();
                break;
            default:
                break;
        }
    }

    // 基于延迟的自适应控制
    void adaptToLatency() {
        auto stats = getStats();
        if (stats.avg_latency_ms > policy_.target_latency_ms) {
            // 降低速率
            policy_.max_messages_per_second *= 0.9;
        } else if (stats.avg_latency_ms < policy_.target_latency_ms * 0.8) {
            // 增加速率
            policy_.max_messages_per_second *= 1.1;
        }
    }

    // 预测性拥塞控制
    void predictCongestion() {
        auto stats = getStats();
        if (stats.congestion_level > policy_.congestion_threshold * 0.8) {
            // 提前降低速率
            policy_.max_messages_per_second *= 0.95;
        }
    }

    // 拥塞处理
    void handleCongestion() {
        auto stats = getStats();
        if (stats.congestion_level > policy_.congestion_threshold) {
            // 触发拥塞回调
            if (congestion_callback_) {
                congestion_callback_(stats.congestion_level);
            }
            // 显著降低速率
            policy_.max_messages_per_second *= 0.5;
        }
    }

private:
    ControlPolicy policy_;                      // 控制策略
    ControlMode control_mode_{ControlMode::RATE_LIMIT}; // 控制模式
    std::atomic<uint64_t> token_bucket_{0};     // 令牌桶
    std::chrono::steady_clock::time_point last_refill_time_; // 上次补充时间
    CongestionCallback congestion_callback_;     // 拥塞回调
    
    // 统计数据
    std::atomic<uint64_t> total_messages_{0};
    std::atomic<uint64_t> total_bytes_{0};
    std::atomic<double> current_message_rate_{0.0};
    std::atomic<double> current_byte_rate_{0.0};
    std::atomic<double> avg_latency_ms_{0.0};
    std::atomic<double> congestion_level_{0.0};
};

} // namespace core
} // namespace hft
