#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <mutex>

namespace hft {
namespace hardware {

// ASIC设备类型
enum class AsicDeviceType {
    DEVICE_TYPE_FPGA,
    DEVICE_TYPE_GPU,
    DEVICE_TYPE_ASIC
};

// ASIC配置参数
struct AsicConfig {
    AsicDeviceType device_type;
    std::string device_path;
    uint32_t clock_freq_mhz;
    uint32_t memory_size_mb;
    uint32_t num_cores;
};

// ASIC驱动程序
class AsicDriver {
public:
    AsicDriver();
    ~AsicDriver();

    // 初始化ASIC设备
    bool initialize(const AsicConfig& config);

    // 关闭ASIC设备
    void shutdown();

    // 检查设备是否已初始化
    bool isInitialized() const;

    // 发送数据到ASIC
    bool sendData(const std::vector<uint8_t>& data);

    // 从ASIC接收数据
    bool receiveData(std::vector<uint8_t>& data, uint32_t timeout_ms);

    // 执行ASIC上的计算任务
    bool executeTask(uint32_t task_id, const std::vector<uint8_t>& input,
                    std::vector<uint8_t>& output, uint32_t timeout_ms);

    // 获取设备状态
    std::string getDeviceStatus() const;

private:
    AsicConfig m_config;
    bool m_initialized;
    mutable std::mutex m_mutex;
    void* m_device_handle; // 实际实现中会替换为具体的设备句柄类型

    // 打开设备
    bool openDevice();

    // 关闭设备
    void closeDevice();

    // 配置设备
    bool configureDevice();
};

using AsicDriverPtr = std::shared_ptr<AsicDriver>;

} // namespace hardware
} // namespace hft