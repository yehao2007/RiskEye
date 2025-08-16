#pragma once
#include <string>
#include <vector>
#include <memory>

namespace hft {
namespace hardware {

class FpgaAccelerator {
public:
    FpgaAccelerator(const std::string& devicePath);
    ~FpgaAccelerator();

    // 初始化FPGA
    bool initialize();
    // 加载位流文件
    bool loadBitstream(const std::string& bitstreamPath);
    // 重置FPGA
    void reset();
    // 检查FPGA状态
    bool isReady() const { return m_ready; }

    // 数据处理接口 - 低延迟路径
    void processData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);
    // 批处理接口
    void batchProcess(const std::vector<std::vector<uint8_t>>& inputs,
                      std::vector<std::vector<uint8_t>>& outputs);

    // 配置FPGA寄存器
    bool configureRegister(uint32_t address, uint32_t value);
    // 读取FPGA寄存器
    uint32_t readRegister(uint32_t address);

private:
    std::string m_devicePath;
    int m_deviceHandle;
    bool m_ready;

    // 内部方法
    bool openDevice();
    void closeDevice();
};

} // namespace hardware
} // namespace hft