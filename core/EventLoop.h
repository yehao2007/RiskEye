#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <memory>

namespace hft {
namespace core {

class EventLoop {
public:
    using EventCallback = std::function<void()>;
    using TimerCallback = std::function<void()>;
    using TimerId = uint64_t;

    EventLoop();
    ~EventLoop();

    // 启动事件循环
    void start();
    // 停止事件循环
    void stop();
    // 运行在事件循环线程
    bool isInLoopThread() const;

    // 向事件循环中添加任务
    void postTask(EventCallback callback);
    // 添加定时任务 (一次性)
    TimerId addTimer(TimerCallback callback, std::chrono::milliseconds delay);
    // 添加周期性任务
    TimerId addPeriodicTimer(TimerCallback callback, std::chrono::milliseconds interval);
    // 取消定时任务
    bool cancelTimer(TimerId timerId);

private:
    struct Timer {
        TimerId id;
        TimerCallback callback;
        std::chrono::steady_clock::time_point expiration;
        std::chrono::milliseconds interval;  // 0 表示一次性任务

        Timer(TimerId id, TimerCallback cb, std::chrono::milliseconds delay, std::chrono::milliseconds interval)
            : id(id), callback(std::move(cb)),
              expiration(std::chrono::steady_clock::now() + delay),
              interval(interval) {}

        // 用于优先队列排序
        bool operator<(const Timer& other) const {
            return expiration > other.expiration;  // 小顶堆
        }
    };

    void loop();
    void processTasks();
    void processTimers();

    std::atomic<bool> m_running;
    std::thread m_loopThread;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<EventCallback> m_taskQueue;
    std::priority_queue<Timer> m_timerQueue;
    std::unordered_map<TimerId, bool> m_activeTimers;
    TimerId m_nextTimerId;
    std::thread::id m_loopThreadId;
};

} // namespace core
} // namespace hft