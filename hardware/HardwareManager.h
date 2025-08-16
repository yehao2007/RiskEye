#pragma once
#include <string>
#include <memory>
#include "FpgaAccelerator.h"
#include "AsicAccelerator.h"

namespace hft {
namespace hardware {

class HardwareManager {
public:
    HardwareManager();
    ~HardwareManager();

    // 初始化硬件
    bool initialize(const std::string& configPath);
    // 关闭硬件
    void shutdown();

    // 获取FPGA加速器
    std::shared_ptr<FpgaAccelerator> getFpgaAccelerator() const {
        return m_fpgaAccelerator;
    }

    // 获取ASIC加速器
    std::shared_ptr<AsicAccelerator> getAsicAccelerator() const {
        return m_asicAccelerator;
    }

    // 根据任务类型选择合适的加速器
    std::shared_ptr<FpgaAccelerator> selectAcceleratorForTask(const std::string& taskType);

private:
    std::shared_ptr<FpgaAccelerator> m_fpgaAccelerator;
    std::shared_ptr<AsicAccelerator> m_asicAccelerator;
    bool m_initialized;
};

} // namespace hardware
} // namespace hft