#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include "utils/LowLatencyLogger.h"

// 订单类型枚举
enum class OrderType {
    BUY,
    SELL,
    MODIFY,
    CANCEL
};

// 订单结构体
struct Order {
    uint64_t orderId;
    std::string symbol;
    OrderType type;
    double price;
    uint32_t quantity;
    uint64_t timestamp; // 纳秒级时间戳
};

// 光子引擎类
class PhotonicEngine {
private:
    // FPGA设备句柄
    void* fpgaHandle;
    // 低延迟日志器
    std::unique_ptr<LowLatencyLogger> logger;
    // 设备初始化标志
    bool initialized;
    // 光子脉冲发送计数
    uint64_t pulseCount;
    // 最后发送时间戳
    uint64_t lastSendTimestamp;

    // 内部方法：检查FPGA连接
    bool checkFPGAConnection() const;
    // 内部方法：编码订单为光子脉冲
    void encodeOrderToPulse(const Order& order, uint8_t* pulseBuffer, size_t& bufferSize);
    // 内部方法：发送光子脉冲
    bool sendPulse(const uint8_t* pulseBuffer, size_t bufferSize);

public:
    PhotonicEngine();
    ~PhotonicEngine();

    // 初始化光子引擎
    bool initialize(const std::string& devicePath = "/dev/tera_phy");
    // 发送订单
    bool sendOrder(const Order& order);
    // 获取最后发送延迟（纳秒）
    uint64_t getLastSendLatency() const;
    // 获取总发送计数
    uint64_t getTotalSendCount() const;
    // 关闭光子引擎
    void shutdown();
};