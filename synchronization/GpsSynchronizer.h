#pragma once
#include <string>
#include <atomic>
#include <thread>
#include "TimeProvider.h"

namespace hft {
namespace synchronization {

class GpsSynchronizer {
public:
    GpsSynchronizer(const std::string& devicePath);
    ~GpsSynchronizer();

    // 初始化GPS
    bool initialize();
    // 启动同步
    void start();
    // 停止同步
    void stop();
    // 检查同步状态
    bool isSynchronized() const { return m_synchronized; }

    // 获取当前时间（纳秒级）
    uint64_t getCurrentNanos() const;
    // 获取当前时间（UTC）
    std::string getCurrentUtcTime() const;

private:
    std::string m_devicePath;
    int m_deviceHandle;
    std::atomic<bool> m_running;
    std::atomic<bool> m_synchronized;
    std::thread m_syncThread;
    TimeProvider m_timeProvider;

    // 同步线程函数
    void syncThreadFunc();
    // 解析GPS数据
    bool parseGpsData(const std::string& data);
    // 打开GPS设备
    bool openDevice();
    // 关闭GPS设备
    void closeDevice();
};

} // namespace synchronization
} // namespace hft