#pragma once

#include <atomic>
#include <chrono>
#include <unordered_map>
#include <vector>
#include "../core/Logger.h"

namespace hft {
namespace core {

class SystemMonitor {
public:
    // 监控指标类型
    enum class MetricType {
        CPU_USAGE,          // CPU使用率
        MEMORY_USAGE,       // 内存使用
        NETWORK_LATENCY,    // 网络延迟
        DISK_IO,           // 磁盘IO
        THREAD_STATS,      // 线程统计
        QUEUE_DEPTH,       // 队列深度
        ORDER_RATE,        // 订单速率
        MARKET_DATA_RATE   // 行情速率
    };

    // 系统状态
    struct SystemState {
        // CPU状态
        struct CPUState {
            double total_usage;
            std::vector<double> core_usage;
            double temperature;
            uint64_t context_switches;
        } cpu;

        // 内存状态
        struct MemoryState {
            size_t total_bytes;
            size_t used_bytes;
            size_t cached_bytes;
            size_t page_faults;
        } memory;

        // 网络状态
        struct NetworkState {
            double ingress_mbps;
            double egress_mbps;
            double latency_us;
            int connection_count;
        } network;

        // 应用状态
        struct ApplicationState {
            uint64_t orders_per_second;
            uint64_t market_data_per_second;
            double order_latency_us;
            int active_strategies;
        } application;
    };

    // 初始化监控器
    bool initialize();
    
    // 启动监控
    void startMonitoring();
    
    // 停止监控
    void stopMonitoring();
    
    // 获取当前系统状态
    SystemState getSystemState() const;
    
    // 获取特定指标
    double getMetric(MetricType type) const;
    
    // 设置告警阈值
    void setAlertThreshold(MetricType type, double threshold);
    
    // 注册状态变更回调
    using StateCallback = std::function<void(const SystemState&)>;
    void registerStateCallback(StateCallback callback);
    
    // 导出监控数据
    void exportMetrics(const std::string& filename);

private:
    // 监控线程函数
    void monitoringThread();
    
    // 收集系统指标
    void collectMetrics();
    
    // 更新系统状态
    void updateSystemState();
    
    // 检查告警条件
    void checkAlerts();
    
    // 分析性能趋势
    void analyzeTrends();
    
    // 预测系统行为
    void predictBehavior();
    
    // 收集详细指标
    void collectCPUMetrics();
    void collectMemoryMetrics();
    void collectNetworkMetrics();
    void collectApplicationMetrics();
    
    // 监控数据存储
    class MetricStore {
    public:
        void addMetric(MetricType type, double value);
        double getAverage(MetricType type) const;
        double getPercentile(MetricType type, double percentile) const;
        
    private:
        struct MetricData {
            std::vector<double> values;
            uint64_t timestamp;
        };
        std::unordered_map<MetricType, MetricData> metrics_;
        static constexpr size_t MAX_HISTORY = 1000;
    };

private:
    std::atomic<bool> running_{false};
    std::thread monitor_thread_;
    SystemState current_state_;
    MetricStore metric_store_;
    StateCallback state_callback_;
    
    // 告警阈值
    std::unordered_map<MetricType, double> alert_thresholds_;
    
    // 监控配置
    struct MonitorConfig {
        std::chrono::milliseconds sampling_interval{100};
        std::chrono::seconds export_interval{60};
        size_t metric_history_size{1000};
        bool detailed_logging{true};
    } config_;
    
    // 性能预测
    class PerformancePredictor {
    public:
        void addDataPoint(const SystemState& state);
        SystemState predictNextState();
        std::vector<std::string> getAnomalies();
        
    private:
        std::vector<SystemState> state_history_;
        static constexpr size_t PREDICTION_WINDOW = 100;
    };
    std::unique_ptr<PerformancePredictor> predictor_;
};

} // namespace core
} // namespace hft
