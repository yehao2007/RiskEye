#pragma once

#include <chrono>
#include <atomic>
#include <mutex>

class TimeManager {
public:
    // Returns the current system time in nanoseconds
    static int64_t nowNano() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count();
    }

    // Returns the current system time in milliseconds
    static int64_t nowMilli() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count();
    }

    // Singleton instance accessor
    static TimeManager& instance() {
        static TimeManager tm;
        return tm;
    }

    // Set a custom time offset (for simulation/testing)
    void setOffset(int64_t offsetNano) {
        std::lock_guard<std::mutex> lock(mutex_);
        offsetNano_ = offsetNano;
    }

    // Get current time with offset applied
    int64_t nowNanoWithOffset() {
        std::lock_guard<std::mutex> lock(mutex_);
        return nowNano() + offsetNano_;
    }

private:
    TimeManager() : offsetNano_(0) {}
    std::mutex mutex_;
    int64_t offsetNano_;
};