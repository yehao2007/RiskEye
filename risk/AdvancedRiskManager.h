#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <optional>
#include "RiskMetrics.h"
#include "core/Configuration.h"
#include "execution/Order.h"
#include "LowLatencyLogger.h"

namespace hft {
namespace risk {

// 添加日志级别枚举
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// 风险限制结构
struct RiskLimits {
    uint32_t maxPositionSize;       // 最大持仓
    double maxDailyLoss;            // 最大日亏损
    double varThreshold;            // VAR阈值
    uint32_t maxOrderRate;          // 最大订单速率(每秒)
};

class AdvancedRiskManager {
public:
    AdvancedRiskManager(const core::Configuration& config);

    // 检查订单是否符合风险规则
    bool checkOrder(const execution::Order& order, const RiskMetrics& metrics);

    // 更新风险指标
    void updateRiskMetrics(const RiskMetrics& metrics);

    // 设置风险限制
    void setRiskLimits(double maxPosition, double maxLoss, double varThreshold);

    // 设置单个品种的持仓限额
    void setSymbolPositionLimit(const std::string& symbol, double limit);

    // 移除品种的持仓限额
    void removeSymbolPositionLimit(const std::string& symbol);

    // 获取品种持仓限额
    std::optional<double> getSymbolPositionLimit(const std::string& symbol) const;

    // 获取风险评分
    double getRiskScore() const;

    // 风险预警
    bool hasRiskAlert() const;
    std::vector<std::string> getRiskAlerts() const;

    // 设置日志级别
    void setLogLevel(LogLevel level);

    // 设置日志文件
    void setLogFile(const std::string& filename);

    // 运行压力测试
    void runStressTest();

    // 生成风险报告
    void generateRiskReport(const std::string& filename);

    // 根据市场条件调整风险限额
    void adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics);

    // VAR计算方法
    double calculateVAR(const std::vector<double>& returns, double confidenceLevel);
    double calculateParametricVAR(const std::vector<double>& returns, double confidenceLevel);
    double calculateMonteCarloVAR(const std::vector<double>& returns, double confidenceLevel, int numSimulations = 10000);
    double calculateCVaR(const std::vector<double>& returns, double confidenceLevel);

    // 风险分散化分析
    double calculateDiversificationScore() const;
    std::unordered_map<std::string, double> getSectorExposure() const;
    std::unordered_map<std::string, double> calculateRiskAttribution() const;

    // 风险可视化数据导出
    void exportRiskDataForVisualization(const std::string& filename) const;

    // 设置最大订单速率
    void setMaxOrderRatePerSecond(int rate);
    // 检查高频交易订单速率
    bool checkHFTOrderRate(const std::vector<execution::Order>& recentOrders, int timeWindowSeconds) const;
    // 获取风险评分及各成分
    std::pair<double, std::unordered_map<std::string, double>> getRiskScoreWithComponents() const;
    // 延迟监控方法
    void monitorLatency(const std::string& operation, double latencyUs);
    // 获取延迟统计
    std::unordered_map<std::string, std::pair<double, double>> getLatencyStats() const;

private:
    double m_maxPosition;         // 最大持仓限额
    double m_maxDailyLoss;        // 最大日亏损限额
    double m_var95Threshold;      // 95% VAR限额
    RiskMetrics m_currentMetrics; // 当前风险指标
    std::vector<std::string> m_alerts; // 风险预警
    std::unordered_map<std::string, double> m_positionLimits; // 品种持仓限额

    // 日志相关
    LogLevel m_logLevel;          // 日志级别
    std::string m_logFile;        // 日志文件
    std::mutex m_logMutex;        // 日志互斥锁

    // 日志记录方法
    void log(LogLevel level, const std::string& message);
    // 高频交易风险监控
    bool checkHFTOrderRate(const std::vector<execution::Order>& recentOrders, int timeWindowSeconds) const;
    // 延迟统计
    std::unordered_map<std::string, std::deque<double>> m_latencyHistory;
    std::unordered_map<std::string, std::pair<double, double>> m_latencyStats; // <avg, max>
    mutable std::mutex m_latencyMutex;
    // 缓存最近计算的风险成分
    mutable std::unordered_map<std::string, double> m_cachedRiskComponents;
    mutable bool m_riskComponentsValid = false;
    int m_maxOrderRatePerSecond = 1000; // 默认每秒最多1000个订单
    // 延迟统计
    std::unordered_map<std::string, std::deque<double>> m_latencyHistory;
    std::unordered_map<std::string, std::pair<double, double>> m_latencyStats; // <avg, max>
    mutable std::mutex m_latencyMutex;
    // 缓存最近计算的风险成分
    mutable std::unordered_map<std::string, double> m_cachedRiskComponents;
    mutable bool m_riskComponentsValid = false;
};

} // namespace risk
} // namespace hft