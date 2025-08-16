#pragma once
#include <string>
#include <vector>
#include <memory>

namespace hft {
namespace hardware {

class AsicAccelerator {
public:
    AsicAccelerator(const std::string& devicePath);
    ~AsicAccelerator();

    // 初始化ASIC
    bool initialize();
    // 加载固件
    bool loadFirmware(const std::string& firmwarePath);
    // 重置ASIC
    void reset();
    // 检查ASIC状态
    bool isReady() const { return m_ready; }

    // 低延迟数据处理
    void processData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);
    // 批量处理多个数据流
    void batchProcess(const std::vector<std::vector<uint8_t>>& inputs,
                      std::vector<std::vector<uint8_t>>& outputs);

    // 配置ASIC参数
    bool configureParameter(const std::string& paramName, const std::string& paramValue);
    // 获取ASIC参数
    std::string getParameter(const std::string& paramName);

    // 获取ASIC温度
    float getTemperature() const;
    // 获取ASIC功耗
    float getPowerConsumption() const;

private:
    std::string m_devicePath;
    int m_deviceHandle;
    bool m_ready;
    float m_temperature;
    float m_powerConsumption;

    // 内部方法
    bool openDevice();
    void closeDevice();
};

} // namespace hardware
} // namespace hft