#include "HardwareManager.h"
#include <iostream>
#include <stdexcept>

namespace hft {
namespace hardware {

HardwareManager::HardwareManager() : m_initialized(false) {
}

HardwareManager::~HardwareManager() {
    if (m_initialized) {
        shutdown();
    }
}

bool HardwareManager::initialize(const std::string& configPath) {
    // 在实际应用中，这里会从配置文件读取硬件信息
    // 此示例直接使用默认配置

    try {
        // 初始化FPGA加速器
        m_fpgaAccelerator = std::make_shared<FpgaAccelerator>("\\.\\FPGA0");
        if (!m_fpgaAccelerator->initialize()) {
            std::cerr << "Failed to initialize FPGA accelerator" << std::endl;
            return false;
        }

        // 加载FPGA位流
        if (!m_fpgaAccelerator->loadBitstream("config\\fpga_bitstream.bin")) {
            std::cerr << "Failed to load FPGA bitstream" << std::endl;
            return false;
        }

        // 初始化ASIC加速器
        m_asicAccelerator = std::make_shared<AsicAccelerator>("\\.\\ASIC0");
        if (!m_asicAccelerator->initialize()) {
            std::cerr << "Failed to initialize ASIC accelerator" << std::endl;
            return false;
        }

        // 加载ASIC固件
        if (!m_asicAccelerator->loadFirmware("config\\asic_firmware.bin")) {
            std::cerr << "Failed to load ASIC firmware" << std::endl;
            return false;
        }

        m_initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Hardware initialization error: " << e.what() << std::endl;
        return false;
    }
}

void HardwareManager::shutdown() {
    if (m_initialized) {
        m_fpgaAccelerator.reset();
        m_asicAccelerator.reset();
        m_initialized = false;
    }
}

std::shared_ptr<FpgaAccelerator> HardwareManager::selectAcceleratorForTask(const std::string& taskType) {
    if (!m_initialized) {
        throw std::runtime_error("Hardware not initialized");
    }

    // 根据任务类型选择加速器
    // 实际应用中会有更复杂的选择逻辑
    if (taskType == "market_data_feed" || taskType == "order_execution") {
        // 低延迟任务优先使用ASIC
        if (m_asicAccelerator && m_asicAccelerator->isReady()) {
            return m_fpgaAccelerator; // 这里返回FPGA是示例，实际应返回ASIC
        }
    }

    // 默认返回FPGA
    return m_fpgaAccelerator;
}

} // namespace hardware
} // namespace hft