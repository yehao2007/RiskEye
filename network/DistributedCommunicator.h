#pragma once

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include "../core/Types.h"

namespace hft {
namespace network {

class DistributedCommunicator {
public:
    // 初始化RDMA
    bool initializeRDMA(const char* device_name);
    
    // 配置共享内存
    bool setupSharedMemory(size_t size);
    
    // 建立RDMA连接
    bool connectPeer(const std::string& remote_address);
    
    // 零延迟数据传输
    bool sendDataRDMA(const void* data, size_t size, uint32_t remote_key);
    
    // 异步接收数据
    void registerReceiveCallback(std::function<void(const void*, size_t)> callback);
    
    // 获取网络状态
    struct NetworkStats {
        uint64_t bytes_sent;
        uint64_t bytes_received;
        double avg_latency_ns;
        double bandwidth_gbps;
        int active_connections;
    };
    NetworkStats getNetworkStats() const;

private:
    // 内存注册与映射
    void registerMemoryRegion();
    
    // QP(队列对)配置
    void configureQueuePair();
    
    // 完成队列管理
    void handleCompletionQueue();
    
    // 网络监控
    void monitorNetwork();

private:
    void* rdma_context_{nullptr};     // RDMA上下文
    void* shared_memory_{nullptr};     // 共享内存区域
    std::atomic<bool> running_{false}; // 运行标志
    
    // RDMA参数
    static constexpr int MAX_WR = 4096;          // 最大工作请求数
    static constexpr int MAX_SGE = 16;           // 最大分散/聚集元素
    static constexpr int CQ_SIZE = 4096;         // 完成队列大小
    static constexpr int MAX_INLINE_DATA = 256;  // 最大内联数据大小
    
    // 性能参数
    static constexpr double TARGET_LATENCY_NS = 300.0;  // 目标延迟
    static constexpr double MIN_BANDWIDTH_GBPS = 100.0; // 最小带宽要求
};

} // namespace network
} // namespace hft
