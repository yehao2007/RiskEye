#include "MemoryPool.h"
#include <cstring>
#include <stdexcept>
#include <iostream>

namespace hft {
namespace utils {

MemoryPool::MemoryPool(size_t block_size, size_t num_blocks)
    : m_block_size(std::max(block_size, sizeof(Block))),
      m_total_blocks(num_blocks),
      m_free_list(nullptr) {

    if (num_blocks == 0) {
        throw std::invalid_argument("Number of blocks must be greater than 0");
    }

    // 分配内存块
    char* chunk = new char[m_block_size * num_blocks];
    m_memory_chunks.push_back(chunk);

    // 初始化空闲链表
    for (size_t i = 0; i < num_blocks - 1; ++i) {
        Block* block = reinterpret_cast<Block*>(chunk + i * m_block_size);
        block->next = reinterpret_cast<Block*>(chunk + (i + 1) * m_block_size);
    }

    // 最后一个块
    Block* last_block = reinterpret_cast<Block*>(chunk + (num_blocks - 1) * m_block_size);
    last_block->next = nullptr;

    m_free_list = reinterpret_cast<Block*>(chunk);
}

MemoryPool::~MemoryPool() {
    // 释放所有内存块
    for (char* chunk : m_memory_chunks) {
        delete[] chunk;
    }
}

void* MemoryPool::allocate() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_free_list == nullptr) {
        // 内存池已满，扩展容量
        size_t new_blocks = m_total_blocks / 2;
        if (new_blocks == 0) new_blocks = 1;

        char* chunk = new char[m_block_size * new_blocks];
        m_memory_chunks.push_back(chunk);

        // 初始化新块
        for (size_t i = 0; i < new_blocks - 1; ++i) {
            Block* block = reinterpret_cast<Block*>(chunk + i * m_block_size);
            block->next = reinterpret_cast<Block*>(chunk + (i + 1) * m_block_size);
        }

        // 最后一个新块
        Block* last_block = reinterpret_cast<Block*>(chunk + (new_blocks - 1) * m_block_size);
        last_block->next = m_free_list;

        m_free_list = reinterpret_cast<Block*>(chunk);
        m_total_blocks += new_blocks;
    }

    // 从空闲链表获取一个块
    Block* block = m_free_list;
    m_free_list = block->next;

    // 清零内存
    memset(block, 0, m_block_size);

    return block;
}

void MemoryPool::deallocate(void* ptr) {
    if (ptr == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // 将块放回空闲链表
    Block* block = static_cast<Block*>(ptr);
    block->next = m_free_list;
    m_free_list = block;
}

void MemoryPool::getStats(size_t& total_blocks, size_t& free_blocks, size_t& used_blocks) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    total_blocks = m_total_blocks;

    // 计算空闲块数量
    free_blocks = 0;
    Block* current = m_free_list;
    while (current != nullptr) {
        free_blocks++;
        current = current->next;
    }

    used_blocks = total_blocks - free_blocks;
}

} // namespace utils
} // namespace hft