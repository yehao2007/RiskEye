#include "PhotonicProcessor.h"
#include "../core/Logger.h"
#include <cmath>

namespace hft {
namespace hardware {

bool PhotonicProcessor::initialize(const char* device_path) {
    try {
        // 初始化光子设备
        photonic_device_ = openPhotonicDevice(device_path);
        if (!photonic_device_) {
            Logger::error("Failed to open photonic device");
            return false;
        }

        // 配置光学调制器
        configureModulator();
        
        // 校准相位
        calibratePhase();
        
        // 初始化温度控制
        controlTemperature();
        
        is_initialized_ = true;
        Logger::info("Photonic processor initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Photonic processor initialization failed: {}", e.what());
        return false;
    }
}

void PhotonicProcessor::computeOptionPricing(const OptionData& data) {
    if (!is_initialized_) {
        Logger::error("Photonic processor not initialized");
        return;
    }

    try {
        // 准备量子态
        std::vector<std::complex<double>> quantum_state(1 << QUBIT_COUNT);
        encodeOptionParameters(data, quantum_state);
        
        // 编码量子态
        encodeQuantumState(quantum_state);
        
        // 应用量子傅里叶变换
        std::vector<std::complex<double>> fft_result(quantum_state.size());
        opticalFFT(quantum_state.data(), fft_result.data(), quantum_state.size());
        
        // 测量结果并解码期权价格
        decodeOptionPrice(fft_result);
        
    } catch (const std::exception& e) {
        Logger::error("Option pricing computation failed: {}", e.what());
    }
}

void PhotonicProcessor::encodeQuantumState(
    const std::vector<std::complex<double>>& state) {
    
    // 检查量子态维度
    if (state.size() != (1u << QUBIT_COUNT)) {
        throw std::invalid_argument("Invalid quantum state dimension");
    }

    // 配置光学调制器
    configureLightModulator(state);
    
    // 监控光强
    monitorOpticalPower();
    
    // 应用相位调制
    applyPhaseModulation(state);
}

void PhotonicProcessor::opticalFFT(
    const std::complex<double>* input,
    std::complex<double>* output,
    size_t size) {
    
    // 检查输入大小是2的幂
    if ((size & (size - 1)) != 0) {
        throw std::invalid_argument("FFT size must be power of 2");
    }

    // 配置光学FFT网络
    configureFFTNetwork(size);
    
    // 加载输入数据
    loadInputData(input, size);
    
    // 执行光学FFT
    performOpticalFFT();
    
    // 读取输出结果
    readOutputData(output, size);
}

PhotonicProcessor::PerformanceMetrics 
PhotonicProcessor::getPerformanceMetrics() const {
    PerformanceMetrics metrics;
    
    // 测量功耗
    metrics.power_consumption_w = measurePowerConsumption();
    
    // 测量计算时间
    metrics.computation_time_ns = measureComputationTime();
    
    // 计算能效比
    metrics.energy_efficiency = 
        1e9 / (metrics.power_consumption_w * metrics.computation_time_ns);
    
    // 测量量子保真度
    metrics.quantum_fidelity = measureQuantumFidelity();
    
    return metrics;
}

void PhotonicProcessor::controlTemperature() {
    // 读取当前温度
    double current_temp = readTemperature();
    
    // 如果温度偏离目标值太多，调整控制器
    if (std::abs(current_temp - operating_temp_) > TEMP_TOLERANCE) {
        adjustTemperatureController(operating_temp_ - current_temp);
    }
    
    // 记录温度数据
    Logger::debug("Current temperature: {:.2f}°C", current_temp);
}

} // namespace hardware
} // namespace hft
