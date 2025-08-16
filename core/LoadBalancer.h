#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <future>
#include "../core/Logger.h"
#include "../core/Types.h"

namespace hft {
namespace core {

class LoadBalancer {
public:
    // 任务优先级
    enum class Priority {
        REALTIME,     // 实时任务
        HIGH,         // 高优先级
        NORMAL,       // 普通优先级
        LOW,         // 低优先级
        BACKGROUND   // 后台任务
    };

    // 任务描述
    struct Task {
        std::function<void()> func;
        Priority priority;
        uint64_t deadline_ns;
        std::string name;
        
        // 任务统计
        struct Stats {
            uint64_t enqueue_time;
            uint64_t start_time;
            uint64_t finish_time;
            int32_t assigned_core;
        } stats;
    };

    // 初始化负载均衡器
    bool initialize(size_t thread_count = std::thread::hardware_concurrency()) {
        try {
            // 初始化线程池
            worker_threads_.resize(thread_count);
            core_loads_.resize(thread_count);
            
            // 设置线程亲和性
            for (size_t i = 0; i < thread_count; ++i) {
                worker_threads_[i] = std::thread(
                    &LoadBalancer::workerFunction, this, i);
                setThreadAffinity(worker_threads_[i], i);
            }
            
            // 启动负载监控
            startLoadMonitoring();
            
            running_ = true;
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Load balancer initialization failed: {}", e.what());
            return false;
        }
    }

    // 提交任务
    template<typename F>
    auto submitTask(F&& func, Priority priority = Priority::NORMAL, 
                   uint64_t deadline_ns = 0) 
        -> std::future<decltype(func())> {
        
        using ReturnType = decltype(func());
        auto promise = std::make_shared<std::promise<ReturnType>>();
        auto future = promise->get_future();

        auto wrapped_task = [func = std::forward<F>(func), 
                           promise = std::move(promise)]() mutable {
            try {
                if constexpr (std::is_void_v<ReturnType>) {
                    func();
                    promise->set_value();
                } else {
                    promise->set_value(func());
                }
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };

        Task task;
        task.func = std::move(wrapped_task);
        task.priority = priority;
        task.deadline_ns = deadline_ns;
        task.stats.enqueue_time = getCurrentTimestamp();

        scheduleTask(std::move(task));
        return future;
    }

    // 获取负载统计
    struct LoadStats {
        std::vector<double> core_utilization;  // 每个核心的使用率
        std::vector<uint64_t> tasks_completed; // 每个核心完成的任务数
        std::vector<double> avg_latency_ns;    // 每个核心的平均延迟
        double total_utilization;              // 总体使用率
    };
    LoadStats getLoadStats() const;

    // 动态调整策略
    void adjustStrategy(double target_utilization = 0.8);

private:
    // 工作线程函数
    void workerFunction(size_t thread_id);
    
    // 调度任务
    void scheduleTask(Task&& task);
    
    // 选择最优执行核心
    int32_t selectOptimalCore(const Task& task);
    
    // 负载预测
    double predictCoreLoad(int32_t core_id, const Task& task);
    
    // 设置线程亲和性
    void setThreadAffinity(std::thread& thread, size_t core_id);
    
    // 监控负载
    void monitorLoad();
    
    // 任务优先级队列比较器
    struct TaskComparator {
        bool operator()(const Task& a, const Task& b) const {
            if (a.priority != b.priority)
                return a.priority > b.priority;
            return a.deadline_ns > b.deadline_ns;
        }
    };

private:
    std::atomic<bool> running_{false};
    std::vector<std::thread> worker_threads_;
    std::vector<std::atomic<double>> core_loads_;
    
    // 任务队列
    using TaskQueue = std::priority_queue<Task, std::vector<Task>, TaskComparator>;
    std::vector<TaskQueue> core_queues_;
    std::mutex queues_mutex_;
    
    // 负载均衡参数
    static constexpr double LOAD_THRESHOLD = 0.8;    // 负载阈值
    static constexpr double IMBALANCE_THRESHOLD = 0.2; // 不平衡阈值
    static constexpr auto MONITOR_INTERVAL = std::chrono::milliseconds(100);
    
    // 任务统计
    struct CoreStats {
        std::atomic<uint64_t> tasks_completed{0};
        std::atomic<uint64_t> total_latency_ns{0};
    };
    std::vector<CoreStats> core_stats_;
};

} // namespace core
} // namespace hft
