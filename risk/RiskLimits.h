#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>
#include <memory>

namespace hft {
namespace risk {

struct RiskLimits {
    // 总仓位限额
    uint64_t maxTotalPosition;      // 最大总持仓量
    double maxTotalValue;           // 最大总持仓价值
    double maxSinglePosition;       // 单个品种最大持仓比例
    double maxDailyLoss;            // 单日最大亏损
    double maxDrawdown;             // 最大回撤比例
    uint64_t maxOrderSize;          // 单笔最大下单量
    uint64_t maxOrdersPerSecond;    // 每秒最大下单数量

    RiskLimits() : maxTotalPosition(1000000), maxTotalValue(10000000.0),
                  maxSinglePosition(0.2), maxDailyLoss(50000.0),
                  maxDrawdown(0.1), maxOrderSize(100000),
                  maxOrdersPerSecond(100) {}
};

typedef std::shared_ptr<RiskLimits> RiskLimitsPtr;

} // namespace risk
} // namespace hft