#include "DistributedCommunicator.h"
#include "../core/Logger.h"
#include <infiniband/verbs.h>
#include <sys/mman.h>
#include <atomic>

namespace hft {
namespace network {

bool DistributedCommunicator::initializeRDMA(const char* device_name) {
    try {
        // 打开RDMA设备
        ibv_context* context = ibv_open_device(findDevice(device_name));
        if (!context) {
            Logger::error("Failed to open RDMA device");
            return false;
        }
        rdma_context_ = context;

        // 分配保护域
        ibv_pd* pd = ibv_alloc_pd(context);
        if (!pd) {
            Logger::error("Failed to allocate protection domain");
            return false;
        }

        // 创建完成队列
        ibv_cq* cq = ibv_create_cq(context, CQ_SIZE, nullptr, nullptr, 0);
        if (!cq) {
            Logger::error("Failed to create completion queue");
            return false;
        }

        // 配置QP属性
        configureQueuePair();

        running_ = true;
        Logger::info("RDMA communication initialized");
        return true;

    } catch (const std::exception& e) {
        Logger::error("RDMA initialization failed: {}", e.what());
        return false;
    }
}

bool DistributedCommunicator::setupSharedMemory(size_t size) {
    // 分配共享内存
    void* shm = mmap(nullptr, size, 
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS,
                     -1, 0);
                     
    if (shm == MAP_FAILED) {
        Logger::error("Failed to allocate shared memory");
        return false;
    }

    shared_memory_ = shm;
    
    // 注册内存区域
    registerMemoryRegion();
    
    return true;
}

bool DistributedCommunicator::connectPeer(const std::string& remote_address) {
    try {
        // 解析远程地址
        auto addr_info = parseAddress(remote_address);
        
        // 建立RDMA连接
        if (!establishConnection(addr_info)) {
            return false;
        }
        
        // 交换QP信息
        exchangeQPInfo();
        
        // 将QP转换为RTS状态
        if (!transitionToRTS()) {
            return false;
        }
        
        Logger::info("Connected to peer: {}", remote_address);
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Peer connection failed: {}", e.what());
        return false;
    }
}

bool DistributedCommunicator::sendDataRDMA(
    const void* data, size_t size, uint32_t remote_key) {
    
    // 准备发送请求
    ibv_send_wr wr{};
    ibv_sge sge{};
    
    sge.addr = reinterpret_cast<uint64_t>(data);
    sge.length = size;
    sge.lkey = getLKey();
    
    wr.wr_id = generateWrId();
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.opcode = IBV_WR_RDMA_WRITE;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr.rdma.remote_addr = getRemoteAddr();
    wr.wr.rdma.rkey = remote_key;
    
    // 提交发送请求
    ibv_send_wr* bad_wr = nullptr;
    if (ibv_post_send(getQP(), &wr, &bad_wr)) {
        Logger::error("Failed to post send request");
        return false;
    }
    
    // 等待完成
    return waitForCompletion();
}

void DistributedCommunicator::handleCompletionQueue() {
    while (running_) {
        ibv_wc wc{};
        int n = ibv_poll_cq(getCQ(), 1, &wc);
        
        if (n < 0) {
            Logger::error("Failed to poll completion queue");
            continue;
        }
        
        if (n > 0) {
            if (wc.status != IBV_WC_SUCCESS) {
                Logger::error("Completion with error: {}", wc.status);
                continue;
            }
            
            processCompletion(wc);
        }
    }
}

DistributedCommunicator::NetworkStats 
DistributedCommunicator::getNetworkStats() const {
    NetworkStats stats;
    
    // 获取基本统计信息
    ibv_query_qp(getQP(), &qp_attr_, &qp_init_attr_, 0);
    
    stats.bytes_sent = qp_attr_.bytes_sent;
    stats.bytes_received = qp_attr_.bytes_received;
    
    // 计算平均延迟
    stats.avg_latency_ns = calculateAverageLatency();
    
    // 计算带宽
    stats.bandwidth_gbps = calculateBandwidth();
    
    // 获取活动连接数
    stats.active_connections = getActiveConnectionCount();
    
    return stats;
}

void DistributedCommunicator::monitorNetwork() {
    while (running_) {
        // 检查网络健康状况
        auto stats = getNetworkStats();
        
        // 检查延迟
        if (stats.avg_latency_ns > TARGET_LATENCY_NS) {
            Logger::warn("High network latency: {} ns", stats.avg_latency_ns);
        }
        
        // 检查带宽
        if (stats.bandwidth_gbps < MIN_BANDWIDTH_GBPS) {
            Logger::warn("Low network bandwidth: {} Gbps", stats.bandwidth_gbps);
        }
        
        // 休眠一段时间
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace network
} // namespace hft
