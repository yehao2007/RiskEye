#pragma once

#include "SystemDiagnostics.h"
#include <deque>
#include <mutex>

namespace hft {
namespace diagnostics {

// 性能指标数据源
class PerformanceDataSource : public IDiagnosticDataSource {
public:
    PerformanceDataSource(size_t history_size = 1000)
        : history_size_(history_size) {}
        
    std::string getName() const override {
        return "Performance";
    }
    
    json getData() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        json data;
        data["latency"] = latency_history_;
        data["throughput"] = throughput_history_;
        data["cpu_usage"] = cpu_usage_history_;
        data["memory_usage"] = memory_usage_history_;
        data["timestamps"] = timestamps_;
        
        return data;
    }
    
    std::vector<std::string> getMetrics() const override {
        return {"latency", "throughput", "cpu_usage", "memory_usage"};
    }
    
    // 添加性能数据
    void addPerformanceData(double latency,
                           double throughput,
                           double cpu_usage,
                           double memory_usage) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        
        // 添加新数据
        latency_history_.push_back(latency);
        throughput_history_.push_back(throughput);
        cpu_usage_history_.push_back(cpu_usage);
        memory_usage_history_.push_back(memory_usage);
        timestamps_.push_back(
            std::chrono::system_clock::to_time_t(now));
            
        // 限制历史数据大小
        while (latency_history_.size() > history_size_) {
            latency_history_.pop_front();
            throughput_history_.pop_front();
            cpu_usage_history_.pop_front();
            memory_usage_history_.pop_front();
            timestamps_.pop_front();
        }
    }
    
    // 清除历史数据
    void clearHistory() {
        std::lock_guard<std::mutex> lock(mutex_);
        latency_history_.clear();
        throughput_history_.clear();
        cpu_usage_history_.clear();
        memory_usage_history_.clear();
        timestamps_.clear();
    }
    
private:
    size_t history_size_;
    mutable std::mutex mutex_;
    std::deque<double> latency_history_;
    std::deque<double> throughput_history_;
    std::deque<double> cpu_usage_history_;
    std::deque<double> memory_usage_history_;
    std::deque<time_t> timestamps_;
};

// 网络指标数据源
class NetworkDataSource : public IDiagnosticDataSource {
public:
    NetworkDataSource(size_t history_size = 1000)
        : history_size_(history_size) {}
        
    std::string getName() const override {
        return "Network";
    }
    
    json getData() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        json data;
        data["bandwidth"] = bandwidth_history_;
        data["packet_loss"] = packet_loss_history_;
        data["connection_count"] = connection_count_history_;
        data["error_rate"] = error_rate_history_;
        data["timestamps"] = timestamps_;
        
        return data;
    }
    
    std::vector<std::string> getMetrics() const override {
        return {"bandwidth", "packet_loss", "connection_count", "error_rate"};
    }
    
    // 添加网络数据
    void addNetworkData(double bandwidth,
                       double packet_loss,
                       int connection_count,
                       double error_rate) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        
        // 添加新数据
        bandwidth_history_.push_back(bandwidth);
        packet_loss_history_.push_back(packet_loss);
        connection_count_history_.push_back(connection_count);
        error_rate_history_.push_back(error_rate);
        timestamps_.push_back(
            std::chrono::system_clock::to_time_t(now));
            
        // 限制历史数据大小
        while (bandwidth_history_.size() > history_size_) {
            bandwidth_history_.pop_front();
            packet_loss_history_.pop_front();
            connection_count_history_.pop_front();
            error_rate_history_.pop_front();
            timestamps_.pop_front();
        }
    }
    
    // 清除历史数据
    void clearHistory() {
        std::lock_guard<std::mutex> lock(mutex_);
        bandwidth_history_.clear();
        packet_loss_history_.clear();
        connection_count_history_.clear();
        error_rate_history_.clear();
        timestamps_.clear();
    }
    
private:
    size_t history_size_;
    mutable std::mutex mutex_;
    std::deque<double> bandwidth_history_;
    std::deque<double> packet_loss_history_;
    std::deque<int> connection_count_history_;
    std::deque<double> error_rate_history_;
    std::deque<time_t> timestamps_;
};

} // namespace diagnostics
} // namespace hft
