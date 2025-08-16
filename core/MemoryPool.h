#pragma once

#include <cstddef>
#include <memory>
#include <array>
#include <mutex>
#include <atomic>
#include "../core/Logger.h"

namespace hft {
namespace core {

template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    // 初始化内存池
    bool initialize(size_t initial_size = 1024 * 1024) {
        try {
            // 分配大页内存
            void* huge_page = allocateHugePages(initial_size);
            if (!huge_page) {
                Logger::error("Failed to allocate huge pages");
                return false;
            }
            
            base_address_ = reinterpret_cast<char*>(huge_page);
            pool_size_ = initial_size;
            
            // 初始化空闲块链表
            initializeFreeList();
            
            // 锁定内存防止页面交换
            if (!lockMemory()) {
                Logger::warn("Failed to lock memory pages");
            }
            
            is_initialized_ = true;
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Memory pool initialization failed: {}", e.what());
            return false;
        }
    }

    // 分配对象
    T* allocate() {
        if (!is_initialized_) {
            throw std::runtime_error("Memory pool not initialized");
        }

        // 从空闲链表获取块
        Block* block = getFreeBlock();
        if (!block) {
            // 需要扩展内存池
            if (!expandPool()) {
                throw std::bad_alloc();
            }
            block = getFreeBlock();
        }

        // 更新统计信息
        ++allocated_blocks_;
        
        return reinterpret_cast<T*>(block->data);
    }

    // 释放对象
    void deallocate(T* ptr) {
        if (!ptr) return;

        // 获取块头
        Block* block = reinterpret_cast<Block*>(
            reinterpret_cast<char*>(ptr) - offsetof(Block, data)
        );

        // 将块放回空闲链表
        returnBlockToFreeList(block);
        
        // 更新统计信息
        --allocated_blocks_;
    }

    // 获取统计信息
    struct Stats {
        size_t total_size;
        size_t allocated_blocks;
        size_t free_blocks;
        double fragmentation_ratio;
    };
    
    Stats getStats() const {
        Stats stats;
        stats.total_size = pool_size_;
        stats.allocated_blocks = allocated_blocks_.load();
        stats.free_blocks = free_blocks_.load();
        stats.fragmentation_ratio = calculateFragmentation();
        return stats;
    }

private:
    // 内存块结构
    struct alignas(64) Block {  // 缓存行对齐
        Block* next;
        char data[BlockSize];
    };

    // 分配大页内存
    void* allocateHugePages(size_t size) {
        // 使用2MB大页
        constexpr size_t HUGE_PAGE_SIZE = 2 * 1024 * 1024;
        size = (size + HUGE_PAGE_SIZE - 1) & ~(HUGE_PAGE_SIZE - 1);
        
        #ifdef _WIN32
            return VirtualAlloc(nullptr, size, 
                MEM_LARGE_PAGES | MEM_COMMIT | MEM_RESERVE,
                PAGE_READWRITE);
        #else
            return mmap(nullptr, size, 
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                -1, 0);
        #endif
    }

    // 锁定内存
    bool lockMemory() {
        #ifdef _WIN32
            return VirtualLock(base_address_, pool_size_) != 0;
        #else
            return mlock(base_address_, pool_size_) == 0;
        #endif
    }

    // 初始化空闲块链表
    void initializeFreeList() {
        size_t num_blocks = pool_size_ / sizeof(Block);
        Block* current = reinterpret_cast<Block*>(base_address_);
        
        for (size_t i = 0; i < num_blocks - 1; ++i) {
            current->next = reinterpret_cast<Block*>(
                reinterpret_cast<char*>(current) + sizeof(Block)
            );
            current = current->next;
        }
        
        current->next = nullptr;
        free_list_ = reinterpret_cast<Block*>(base_address_);
        free_blocks_ = num_blocks;
    }

    // 获取空闲块
    Block* getFreeBlock() {
        Block* block = nullptr;
        
        // 无锁快速路径
        block = free_list_.load(std::memory_order_relaxed);
        if (!block) return nullptr;
        
        // CAS循环
        while (block && !free_list_.compare_exchange_weak(
            block, block->next,
            std::memory_order_release,
            std::memory_order_relaxed)) {
            // 重试
        }
        
        if (block) {
            --free_blocks_;
        }
        
        return block;
    }

    // 扩展内存池
    bool expandPool() {
        std::lock_guard<std::mutex> lock(expand_mutex_);
        
        // 双倍扩展
        size_t new_size = pool_size_ * 2;
        void* new_memory = allocateHugePages(new_size);
        
        if (!new_memory) {
            return false;
        }
        
        // 更新基地址和大小
        char* old_base = base_address_;
        base_address_ = reinterpret_cast<char*>(new_memory);
        pool_size_ = new_size;
        
        // 重新初始化空闲链表
        initializeFreeList();
        
        // 复制旧数据
        std::memcpy(base_address_, old_base, pool_size_ / 2);
        
        // 释放旧内存
        #ifdef _WIN32
            VirtualFree(old_base, 0, MEM_RELEASE);
        #else
            munmap(old_base, pool_size_ / 2);
        #endif
        
        return true;
    }

    // 计算内存碎片率
    double calculateFragmentation() const {
        if (allocated_blocks_ == 0) return 0.0;
        
        size_t total_blocks = pool_size_ / sizeof(Block);
        size_t used_blocks = allocated_blocks_;
        
        return 1.0 - (static_cast<double>(used_blocks) / total_blocks);
    }

private:
    char* base_address_{nullptr};              // 内存池基地址
    size_t pool_size_{0};                      // 内存池大小
    std::atomic<Block*> free_list_{nullptr};   // 空闲块链表
    std::atomic<size_t> allocated_blocks_{0};  // 已分配块数
    std::atomic<size_t> free_blocks_{0};       // 空闲块数
    std::mutex expand_mutex_;                  // 扩展锁
    bool is_initialized_{false};               // 初始化标志
};

} // namespace core
} // namespace hft
