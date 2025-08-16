#include "MemoryManager.h"
#include "../core/Logger.h"
#include <algorithm>
#include <memory>
#include <numa.h>

namespace hft {
namespace core {

void MemoryManager::initialize() {
    // 初始化内存池
    initializeMemoryPools();
    
    // 设置NUMA策略
    setupNUMAPolicy();
    
    // 初始化默认参数
    params_.pool_size = 1024 * 1024 * 1024;  // 1GB每个池
    params_.block_size = 4096;               // 4KB块大小
    params_.alignment = 64;                  // 缓存行对齐
    params_.numa_nodes = numa_num_configured_nodes();
}

void* MemoryManager::allocate(size_t size, MemoryType type) {
    try {
        // 选择合适的内存池
        auto& pool = selectMemoryPool(size, type);
        
        // 分配内存
        void* ptr = pool.allocate(size);
        
        // 记录分配信息
        trackAllocation(ptr, size, type);
        
        return ptr;
        
    } catch (const std::exception& e) {
        Logger::error("Memory allocation failed: {}", e.what());
        throw;
    }
}

void MemoryManager::deallocate(void* ptr) {
    try {
        // 获取分配信息
        auto info = getAllocationInfo(ptr);
        
        // 选择对应的内存池
        auto& pool = selectMemoryPool(info.size, info.type);
        
        // 释放内存
        pool.deallocate(ptr);
        
        // 移除跟踪信息
        untrackAllocation(ptr);
        
    } catch (const std::exception& e) {
        Logger::error("Memory deallocation failed: {}", e.what());
        throw;
    }
}

void MemoryManager::initializeMemoryPools() {
    // 为每个NUMA节点创建内存池
    for (int node = 0; node < params_.numa_nodes; ++node) {
        // 普通内存池
        memory_pools_[node][MemoryType::GENERAL] = 
            createMemoryPool(params_.pool_size, node);
            
        // 低延迟内存池
        memory_pools_[node][MemoryType::LOW_LATENCY] = 
            createLowLatencyPool(params_.pool_size, node);
            
        // 大块内存池
        memory_pools_[node][MemoryType::LARGE_BLOCK] = 
            createLargeBlockPool(params_.pool_size * 2, node);
    }
}

MemoryPool& MemoryManager::selectMemoryPool(
    size_t size, 
    MemoryType type) {
    
    // 获取当前线程所在的NUMA节点
    int current_node = numa_node_of_cpu(sched_getcpu());
    
    // 根据大小选择合适的池类型
    if (size > params_.pool_size / 100) {  // 大于1%池大小使用大块池
        type = MemoryType::LARGE_BLOCK;
    }
    
    return memory_pools_[current_node][type];
}

void MemoryManager::setupNUMAPolicy() {
    // 设置NUMA内存分配策略
    numa_set_preferred(numa_node_of_cpu(sched_getcpu()));
    
    // 绑定内存到本地节点
    numa_set_localalloc();
    
    // 设置内存互联策略
    numa_set_interleave_mask(numa_all_nodes_ptr);
}

void MemoryManager::trackAllocation(
    void* ptr, 
    size_t size, 
    MemoryType type) {
    
    std::lock_guard<std::mutex> lock(tracking_mutex_);
    
    AllocationInfo info{
        .size = size,
        .type = type,
        .allocation_time = std::chrono::steady_clock::now(),
        .thread_id = std::this_thread::get_id(),
        .numa_node = numa_node_of_cpu(sched_getcpu())
    };
    
    allocations_[ptr] = info;
}

void MemoryManager::untrackAllocation(void* ptr) {
    std::lock_guard<std::mutex> lock(tracking_mutex_);
    allocations_.erase(ptr);
}

AllocationInfo MemoryManager::getAllocationInfo(void* ptr) {
    std::lock_guard<std::mutex> lock(tracking_mutex_);
    return allocations_.at(ptr);
}

MemoryStats MemoryManager::getStats() const {
    MemoryStats stats;
    
    std::lock_guard<std::mutex> lock(tracking_mutex_);
    
    // 计算总分配量
    for (const auto& [ptr, info] : allocations_) {
        stats.total_allocated += info.size;
        stats.allocation_count++;
        
        // 按类型统计
        stats.type_allocated[info.type] += info.size;
        
        // 按NUMA节点统计
        stats.node_allocated[info.numa_node] += info.size;
    }
    
    // 计算碎片化率
    stats.fragmentation = calculateFragmentation();
    
    // 计算利用率
    stats.utilization = calculateUtilization();
    
    return stats;
}

void MemoryManager::defragment() {
    for (int node = 0; node < params_.numa_nodes; ++node) {
        for (auto& [type, pool] : memory_pools_[node]) {
            // 检查碎片化程度
            if (pool.getFragmentation() > 0.3) {  // 30%碎片化阈值
                // 执行碎片整理
                pool.defragment();
            }
        }
    }
}

void MemoryManager::prefetch(void* ptr, size_t size) {
    // 硬件预取
    __builtin_prefetch(ptr, 1, 3);  // rw=1, locality=3
    
    // 软件预取
    for (size_t offset = 0; offset < size; offset += 64) {  // 每个缓存行
        volatile char* p = static_cast<char*>(ptr) + offset;
        *p;  // 触发预取
    }
}

bool MemoryManager::isLowMemory() const {
    MemoryStats stats = getStats();
    
    // 检查总体使用率
    if (stats.utilization > 0.9) {  // 90%使用率阈值
        return true;
    }
    
    // 检查特定类型的使用率
    for (const auto& [type, allocated] : stats.type_allocated) {
        size_t capacity = getTypeCapacity(type);
        if (allocated > capacity * 0.9) {
            return true;
        }
    }
    
    return false;
}

void MemoryManager::registerLowMemoryHandler(
    LowMemoryHandler handler) {
    
    low_memory_handlers_.push_back(std::move(handler));
}

void MemoryManager::handleLowMemory() {
    // 通知所有处理器
    for (const auto& handler : low_memory_handlers_) {
        handler();
    }
    
    // 尝试释放未使用的内存
    releaseUnusedMemory();
    
    // 触发碎片整理
    defragment();
}

} // namespace core
} // namespace hft
