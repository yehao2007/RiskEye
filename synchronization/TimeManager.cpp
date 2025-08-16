#include "TimeManager.h"
#include <chrono>
#include <mutex>

class TimeManager {
public:
    static TimeManager& instance() {
        static TimeManager tm;
        return tm;
    }

    // Returns current time in nanoseconds since epoch
    int64_t now() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto tp = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
    }

private:
    TimeManager() = default;
    ~TimeManager() = default;
    TimeManager(const TimeManager&) = delete;
    TimeManager& operator=(const TimeManager&) = delete;

    std::mutex mutex_;
};