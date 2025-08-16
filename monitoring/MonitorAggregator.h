#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include "../core/Types.h"

namespace hft {
namespace monitoring {

class MonitorAggregator {
public:
    // 节点信息
    struct NodeInfo {
        std::string id;
        std::string address;
        std::string role;
        bool is_leader;
        uint64_t last_heartbeat;
    };

    // 监控指标
    struct MetricData {
        std::string name;
        double value;
        std::string unit;
        uint64_t timestamp;
        std::unordered_map<std::string, std::string> labels;
    };

    // 聚合配置
    struct AggregationConfig {
        std::chrono::milliseconds collection_interval{100};
        std::chrono::seconds retention_period{3600};
        bool enable_compression{true};
        size_t max_metrics_per_node{10000};
    };

    // 初始化聚合器
    bool initialize(const AggregationConfig& config = AggregationConfig{}) {
        try {
            config_ = config;
            
            // 初始化存储系统
            initializeStorage();
            
            // 启动集群发现
            startClusterDiscovery();
            
            // 初始化聚合引擎
            initializeAggregationEngine();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Monitor aggregator initialization failed: {}", e.what());
            return false;
        }
    }

    // 注册节点
    bool registerNode(const NodeInfo& node);
    
    // 提交指标数据
    void submitMetrics(const std::string& node_id, 
                      const std::vector<MetricData>& metrics);
    
    // 查询聚合指标
    std::vector<MetricData> queryMetrics(
        const std::string& metric_name,
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end);
    
    // 获取集群状态
    struct ClusterState {
        std::vector<NodeInfo> nodes;
        std::unordered_map<std::string, double> aggregated_metrics;
        uint64_t total_metrics_count;
        double cluster_health_score;
    };
    ClusterState getClusterState() const;

private:
    // 时间序列数据库
    class TimeSeriesDB {
    public:
        void store(const std::string& metric, 
                  const MetricData& data);
        std::vector<MetricData> query(
            const std::string& metric,
            uint64_t start_time,
            uint64_t end_time);
        void compress();
        
    private:
        struct TimeBlock {
            uint64_t timestamp;
            std::vector<double> values;
            std::vector<uint64_t> timestamps;
        };
        std::unordered_map<std::string, std::vector<TimeBlock>> data_;
    };

    // 聚合引擎
    class AggregationEngine {
    public:
        void aggregate(const std::vector<MetricData>& metrics);
        std::vector<MetricData> getAggregates();
        
    private:
        struct AggregateStats {
            double sum;
            double min;
            double max;
            double avg;
            size_t count;
        };
        std::unordered_map<std::string, AggregateStats> stats_;
    };

    // 集群管理器
    class ClusterManager {
    public:
        void addNode(const NodeInfo& node);
        void removeNode(const std::string& node_id);
        void updateHeartbeat(const std::string& node_id);
        std::vector<NodeInfo> getActiveNodes();
        
    private:
        std::unordered_map<std::string, NodeInfo> nodes_;
        std::mutex nodes_mutex_;
    };

    // 压缩引擎
    class CompressionEngine {
    public:
        std::vector<uint8_t> compress(const MetricData& data);
        MetricData decompress(const std::vector<uint8_t>& compressed);
        
    private:
        static constexpr size_t COMPRESSION_BUFFER_SIZE = 4096;
    };

private:
    AggregationConfig config_;
    std::unique_ptr<TimeSeriesDB> tsdb_;
    std::unique_ptr<AggregationEngine> aggregator_;
    std::unique_ptr<ClusterManager> cluster_manager_;
    std::unique_ptr<CompressionEngine> compressor_;
    
    // 聚合统计
    std::atomic<uint64_t> total_metrics_{0};
    std::atomic<uint64_t> compressed_size_{0};
    std::atomic<double> compression_ratio_{0.0};
};

} // namespace monitoring
} // namespace hft
