#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include "../core/Types.h"

namespace hft {
namespace network {

class KernelBypass {
public:
    // 初始化DPDK环境
    bool initializeDPDK(int argc, char** argv);
    
    // 配置网卡
    bool configureNIC(const char* device);
    
    // 启动数据接收
    void startReceiving();
    
    // 注册数据处理回调
    void registerCallback(std::function<void(const void*, size_t)> callback);
    
    // 零拷贝发送
    bool sendZeroCopy(const void* data, size_t len);
    
    // 获取统计信息
    struct Stats {
        uint64_t packets_received;
        uint64_t packets_sent;
        uint64_t bytes_received;
        uint64_t bytes_sent;
        double avg_latency_ns;
    };
    Stats getStats() const;

private:
    // 分配内存池
    void setupMemoryPool();
    
    // 配置接收队列
    void configureRxQueues();
    
    // 配置发送队列
    void configureTxQueues();
    
    // 轮询网卡
    void pollNIC();

private:
    void* dpdk_port_{nullptr};          // DPDK端口
    std::atomic<bool> running_{false};   // 运行标志
    std::vector<void*> rx_mbufs_;       // 接收缓冲区
    std::vector<void*> tx_mbufs_;       // 发送缓冲区
    
    static constexpr int NUM_RX_RINGS = 8;   // 接收环数量
    static constexpr int NUM_TX_RINGS = 8;   // 发送环数量
    static constexpr int MBUF_CACHE_SIZE = 512;  // 缓冲区缓存大小
};

} // namespace network
} // namespace hft
