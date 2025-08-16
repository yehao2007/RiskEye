#pragma once

#include <memory>
#include <complex>
#include <vector>
#include "../core/Types.h"

namespace hft {
namespace hardware {

class PhotonicProcessor {
public:
    // 初始化光子芯片
    bool initialize(const char* device_path);
    
    // 光电混合计算
    void computeOptionPricing(const OptionData& data);
    
    // 量子态编码
    void encodeQuantumState(const std::vector<std::complex<double>>& state);
    
    // 光学FFT计算
    void opticalFFT(const std::complex<double>* input, 
                   std::complex<double>* output, 
                   size_t size);
    
    // 获取计算性能指标
    struct PerformanceMetrics {
        double power_consumption_w;    // 功耗(瓦特)
        double computation_time_ns;    // 计算时间(纳秒)
        double energy_efficiency;      // 能效比
        double quantum_fidelity;       // 量子保真度
    };
    PerformanceMetrics getPerformanceMetrics() const;

private:
    // 光学调制器配置
    void configureModulator();
    
    // 相位校准
    void calibratePhase();
    
    // 光强监控
    void monitorOpticalPower();
    
    // 温度控制
    void controlTemperature();

private:
    void* photonic_device_{nullptr};  // 光子设备句柄
    bool is_initialized_{false};       // 初始化标志
    double operating_temp_{20.0};      // 工作温度(摄氏度)
    
    // 光学参数
    static constexpr double WAVELENGTH_NM = 1550.0;  // 工作波长(纳米)
    static constexpr double MAX_POWER_MW = 100.0;    // 最大光功率(毫瓦)
    static constexpr double TEMP_TOLERANCE = 0.1;    // 温度容差(摄氏度)
    
    // 量子计算参数
    static constexpr int QUBIT_COUNT = 8;           // 量子比特数
    static constexpr double COHERENCE_TIME_US = 100.0; // 相干时间(微秒)
};

} // namespace hardware
} // namespace hft
