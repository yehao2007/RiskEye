#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace hft {
namespace risk {

// 风险指标结构体
struct RiskMetrics {
    // 基本风险指标
    double currentPosition;              // 当前总持仓
    double currentValue;                 // 当前持仓价值
    double dailyPnl;                     // 当日盈亏
    double totalPnl;                     // 总盈亏
    double maxDrawdown;                  // 最大回撤
    double sharpeRatio;                  // 夏普比率
    double sortinoRatio;                 // 索提诺比率
    double volatility;                   // 波动率
    double beta;                         // 贝塔系数
    double alpha;                        // 阿尔法系数

    // 按品种持仓
    std::unordered_map<std::string, double> positionBySymbol; // 品种持仓
    std::unordered_map<std::string, double> valueBySymbol;    // 品种价值
    std::unordered_map<std::string, double> pnlBySymbol;      // 品种盈亏

    // 订单流指标
    uint64_t totalOrders;                // 总订单数
    uint64_t filledOrders;               // 已成交订单数
    double fillRate;                     // 成交率
    double avgOrderSize;                 // 平均订单大小

    // 市场风险指标
    std::vector<double> recentReturns;   // 最近收益序列
    double var95;                        // 95%置信度VAR
    double var99;                        // 99%置信度VAR
    double cvVar95;                      // 95%置信度条件VAR
    double liquidityRisk;                // 流动性风险
    double marketImpact;                 // 市场冲击成本

    // 时间戳
    uint64_t timestamp;                  // 指标计算时间戳

    // 构造函数
    RiskMetrics() : currentPosition(0), currentValue(0), dailyPnl(0), totalPnl(0),
                   maxDrawdown(0), sharpeRatio(0), sortinoRatio(0), volatility(0),
                   beta(0), alpha(0), totalOrders(0), filledOrders(0), fillRate(0),
                   avgOrderSize(0), var95(0), var99(0), cvVar95(0), liquidityRisk(0),
                   marketImpact(0), timestamp(0) {}
};

typedef std::shared_ptr<RiskMetrics> RiskMetricsPtr;

} // namespace risk
} // namespace hft