#include "PhotonicEngine.h"
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>

PhotonicEngine::PhotonicEngine() : fpgaHandle(nullptr), initialized(false), pulseCount(0), lastSendTimestamp(0) {
    logger = std::make_unique<LowLatencyLogger>("photonic_engine.log", LogLevel::INFO);
}

PhotonicEngine::~PhotonicEngine() {
    if (initialized) {
        shutdown();
    }
}

bool PhotonicEngine::initialize(const std::string& devicePath) {
    // 在实际实现中，这里会打开FPGA设备并初始化通信
    logger->log(LogLevel::INFO, "Initializing photonic engine...");

    // 模拟FPGA初始化
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    fpgaHandle = reinterpret_cast<void*>(0x12345678); // 模拟设备句柄
    initialized = true;

    logger->log(LogLevel::INFO, "Photonic engine initialized successfully");
    return true;
}

bool PhotonicEngine::checkFPGAConnection() const {
    // 检查FPGA连接状态
    return (fpgaHandle != nullptr) && initialized;
}

void PhotonicEngine::encodeOrderToPulse(const Order& order, uint8_t* pulseBuffer, size_t& bufferSize) {
    // 将订单编码为光子脉冲序列
    // 实际实现中会根据FPGA的通信协议进行编码
    bufferSize = sizeof(Order);
    memcpy(pulseBuffer, &order, bufferSize);
}

bool PhotonicEngine::sendPulse(const uint8_t* pulseBuffer, size_t bufferSize) {
    // 向FPGA发送光子脉冲
    if (!checkFPGAConnection()) {
        logger->log(LogLevel::ERROR, "FPGA not connected");
        return false;
    }

    // 模拟脉冲发送
    std::this_thread::sleep_for(std::chrono::nanoseconds(50)); // 模拟50ns延迟
    return true;
}

bool PhotonicEngine::sendOrder(const Order& order) {
    if (!checkFPGAConnection()) {
        logger->log(LogLevel::ERROR, "Cannot send order: FPGA not connected");
        return false;
    }

    // 记录发送开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    // 编码订单
    uint8_t pulseBuffer[128]; // 假设最大脉冲缓冲区为128字节
    size_t bufferSize;
    encodeOrderToPulse(order, pulseBuffer, bufferSize);

    // 发送脉冲
    bool success = sendPulse(pulseBuffer, bufferSize);

    // 记录发送结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    lastSendTimestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

    if (success) {
        pulseCount++;
        logger->log(LogLevel::DEBUG, "Order sent successfully, latency: " + std::to_string(lastSendTimestamp) + " ns");
    } else {
        logger->log(LogLevel::ERROR, "Failed to send order");
    }

    return success;
}

uint64_t PhotonicEngine::getLastSendLatency() const {
    return lastSendTimestamp;
}

uint64_t PhotonicEngine::getTotalSendCount() const {
    return pulseCount;
}

void PhotonicEngine::shutdown() {
    if (initialized) {
        logger->log(LogLevel::INFO, "Shutting down photonic engine...");
        // 实际实现中会关闭FPGA设备
        fpgaHandle = nullptr;
        initialized = false;
        logger->log(LogLevel::INFO, "Photonic engine shutdown completed");
    }
}