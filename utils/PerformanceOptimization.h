#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include "../core/Logger.h"

namespace hft {
namespace utils {

// 线程安全队列
template<typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cv_.notify_one();
    }
    
    bool tryPop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cv_;
};

// 线程池实现
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this] { 
                            return !tasks_.empty() || stop_; 
                        });
                        
                        if (stop_ && tasks_.empty()) return;
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
        Logger::info("ThreadPool initialized with {} threads", numThreads);
    }
    
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using ReturnType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<ReturnType> result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return result;
    }
    
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& worker : workers_) {
            worker.join();
        }
    }
    
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_{false};
};

// 内存池实现
template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    MemoryPool() : currentBlock_(nullptr), currentSlot_(nullptr), lastSlot_(nullptr) {
        allocateBlock();
    }
    
    T* allocate() {
        if (currentSlot_ >= lastSlot_) {
            allocateBlock();
        }
        return reinterpret_cast<T*>(currentSlot_++);
    }
    
    void deallocate(T* p) {
        if (p != nullptr) {
            p->~T();
            freeSlots_.push(reinterpret_cast<char*>(p));
        }
    }
    
    template<typename... Args>
    T* construct(Args&&... args) {
        T* p = allocate();
        new(p) T(std::forward<Args>(args)...);
        return p;
    }
    
private:
    struct Block {
        Block* next;
        char data[BlockSize - sizeof(Block*)];
    };
    
    void allocateBlock() {
        Block* newBlock = reinterpret_cast<Block*>(new char[BlockSize]);
        newBlock->next = currentBlock_;
        currentBlock_ = newBlock;
        currentSlot_ = currentBlock_->data;
        lastSlot_ = currentBlock_->data + BlockSize - sizeof(Block*);
    }
    
    Block* currentBlock_;
    char* currentSlot_;
    char* lastSlot_;
    ThreadSafeQueue<char*> freeSlots_;
};

// 高性能缓存实现
template<typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}
    
    Value get(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(key);
        if (it == cache_.end()) return Value();
        
        // 将访问的元素移到链表头部
        lruList_.splice(lruList_.begin(), lruList_, it->second.second);
        return it->second.first;
    }
    
    void put(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(key);
        
        if (it != cache_.end()) {
            // 更新现有值
            it->second.first = value;
            lruList_.splice(lruList_.begin(), lruList_, it->second.second);
        } else {
            // 添加新值
            if (cache_.size() >= capacity_) {
                // 移除最久未使用的元素
                const Key& oldKey = lruList_.back();
                cache_.erase(oldKey);
                lruList_.pop_back();
            }
            lruList_.push_front(key);
            cache_[key] = {value, lruList_.begin()};
        }
    }
    
private:
    size_t capacity_;
    std::list<Key> lruList_;
    std::unordered_map<Key, std::pair<Value, typename std::list<Key>::iterator>> cache_;
    std::mutex mutex_;
};

// 性能优化管理器
class PerformanceManager {
public:
    static PerformanceManager& getInstance() {
        static PerformanceManager instance;
        return instance;
    }
    
    ThreadPool& getThreadPool() { return threadPool_; }
    
    template<typename T>
    MemoryPool<T>& getMemoryPool() {
        static MemoryPool<T> pool;
        return pool;
    }
    
    template<typename Key, typename Value>
    LRUCache<Key, Value>& getCache(size_t capacity = 1000) {
        static LRUCache<Key, Value> cache(capacity);
        return cache;
    }
    
private:
    PerformanceManager() : threadPool_(std::thread::hardware_concurrency()) {}
    ThreadPool threadPool_;
};

} // namespace utils
} // namespace hft
