#include "EventLoop.h"
#include <iostream>

namespace hft {
namespace core {

EventLoop::EventLoop()
    : m_running(false),
      m_nextTimerId(1) {
}

EventLoop::~EventLoop() {
    if (m_running) {
        stop();
    }
}

void EventLoop::start() {
    if (m_running) {
        return;
    }

    m_running = true;
    m_loopThread = std::thread([this]() { loop(); });
    m_loopThreadId = m_loopThread.get_id();
}

void EventLoop::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;
    // 唤醒事件循环线程
    m_condition.notify_one();
    if (m_loopThread.joinable()) {
        m_loopThread.join();
    }
}

bool EventLoop::isInLoopThread() const {
    return std::this_thread::get_id() == m_loopThreadId;
}

void EventLoop::postTask(EventCallback callback) {
    {{
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskQueue.push(std::move(callback));
    }}
    m_condition.notify_one();
}

EventLoop::TimerId EventLoop::addTimer(TimerCallback callback, std::chrono::milliseconds delay) {
    return addPeriodicTimer(std::move(callback), delay, std::chrono::milliseconds(0));
}

EventLoop::TimerId EventLoop::addPeriodicTimer(TimerCallback callback, std::chrono::milliseconds interval) {
    return addPeriodicTimer(std::move(callback), std::chrono::milliseconds(0), interval);
}

EventLoop::TimerId EventLoop::addPeriodicTimer(TimerCallback callback, std::chrono::milliseconds delay, std::chrono::milliseconds interval) {
    TimerId id = m_nextTimerId++;

    {{
        std::lock_guard<std::mutex> lock(m_mutex);
        m_timerQueue.emplace(id, std::move(callback), delay, interval);
        m_activeTimers[id] = true;
    }}
    m_condition.notify_one();

    return id;
}

bool EventLoop::cancelTimer(TimerId timerId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_activeTimers.find(timerId);
    if (it != m_activeTimers.end()) {
        it->second = false;
        return true;
    }
    return false;
}

void EventLoop::loop() {
    while (m_running) {
        processTasks();
        processTimers();

        // 等待新任务或定时器触发
        std::unique_lock<std::mutex> lock(m_mutex);
        auto now = std::chrono::steady_clock::now();
        std::chrono::milliseconds waitTime = std::chrono::milliseconds::max();

        if (!m_timerQueue.empty()) {
            auto nextExpiration = m_timerQueue.top().expiration;
            if (nextExpiration > now) {
                waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(nextExpiration - now);
            } else {
                waitTime = std::chrono::milliseconds(0);
            }
        }

        m_condition.wait_for(lock, waitTime, [this]() {
            return !m_running || !m_taskQueue.empty() ||
                   (!m_timerQueue.empty() && m_timerQueue.top().expiration <= std::chrono::steady_clock::now());
        });
    }
}

void EventLoop::processTasks() {
    while (true) {
        EventCallback task;
        {{
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_taskQueue.empty()) {
                break;
            }
            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }}

        try {
            task();
        } catch (const std::exception& e) {
            std::cerr << "Exception in task: " << e.what() << std::endl;
        }
    }
}

void EventLoop::processTimers() {
    auto now = std::chrono::steady_clock::now();

    while (true) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_timerQueue.empty() || m_timerQueue.top().expiration > now) {
            break;
        }

        Timer timer = m_timerQueue.top();
        m_timerQueue.pop();

        // 检查定时器是否被取消
        auto it = m_activeTimers.find(timer.id);
        if (it != m_activeTimers.end() && !it->second) {
            m_activeTimers.erase(it);
            continue;
        }

        // 移除一次性定时器
        if (timer.interval == std::chrono::milliseconds(0)) {
            m_activeTimers.erase(timer.id);
        } else {
            // 更新周期性定时器
            timer.expiration = now + timer.interval;
            m_timerQueue.push(timer);
        }

        lock.unlock();

        try {
            timer.callback();
        } catch (const std::exception& e) {
            std::cerr << "Exception in timer callback: " << e.what() << std::endl;
        }
    }
}

} // namespace core
} // namespace hft