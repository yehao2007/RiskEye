#pragma once
#include <cstddef>
#include <mutex>
#include <vector>
#include <memory>

namespace hft {
namespace utils {

class MemoryPool {
public:
    // 创建内存池，block_size: 每个块的大小，num_blocks: 初始块数量
    MemoryPool(size_t block_size, size_t num_blocks);
    ~MemoryPool();

    // 分配内存块
    void* allocate();
    // 释放内存块
    void deallocate(void* ptr);
    // 获取内存池状态信息
    void getStats(size_t& total_blocks, size_t& free_blocks, size_t& used_blocks) const;

private:
    // 内存块结构
    struct Block {
        Block* next;  // 指向下一个空闲块
    };

    size_t m_block_size;     // 块大小
    size_t m_total_blocks;   // 总块数
    Block* m_free_list;      // 空闲块链表
    std::vector<char*> m_memory_chunks; // 分配的内存块
    mutable std::mutex m_mutex;         // 互斥锁
};

// 内存池智能指针
template<typename T>
class PoolAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = PoolAllocator<U>;
    };

    PoolAllocator(MemoryPool& pool) : m_pool(pool) {}

    template<typename U>
    PoolAllocator(const PoolAllocator<U>& other) : m_pool(other.m_pool) {}

    T* allocate(size_t n) {
        if (n != 1) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(m_pool.allocate());
    }

    void deallocate(T* ptr, size_t n) {
        if (n != 1) {
            return;
        }
        m_pool.deallocate(ptr);
    }

    template<typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        new(ptr) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }

private:
    MemoryPool& m_pool;
};

} // namespace utils
} // namespace hft