#include "AdvancedRiskManager.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <mutex>
#include <filesystem>

#include "AdvancedRiskManager.h"
#include "RiskMetrics.h"
#include "core/Configuration.h"
#include "execution/Order.h"

namespace hft {
namespace risk {

AdvancedRiskManager::AdvancedRiskManager(const core::Configuration& config) : 
    m_logLevel(LogLevel::INFO), m_logFile("risk_manager.log") {
    // 从配置中加载风险参数
    m_maxPosition = config.get<double>("risk.max_position", 1000000.0);
    m_maxDailyLoss = config.get<double>("risk.max_daily_loss", 50000.0);
    m_var95Threshold = config.get<double>("risk.var_95_threshold", 20000.0);

    // 初始化其他成员
    m_currentMetrics = RiskMetrics();

    log(LogLevel::INFO, "AdvancedRiskManager initialized");
}

void AdvancedRiskManager::setLogLevel(LogLevel level) {
    m_logLevel = level;
    log(LogLevel::INFO, "Log level set to " + std::to_string(static_cast<int>(level)));
}

void AdvancedRiskManager::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logFile = filename;
    log(LogLevel::INFO, "Log file set to " + filename);
}

void AdvancedRiskManager::log(LogLevel level, const std::string& message) {
    // 检查日志级别
    if (level < m_logLevel) {
        return;
    }

    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time);

    // 日志级别字符串
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:
            levelStr = "DEBUG";
            break;
        case LogLevel::INFO:
            levelStr = "INFO";
            break;
        case LogLevel::WARNING:
            levelStr = "WARNING";
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
    }

    // 构建日志消息
    std::stringstream ss;
    ss << "[" 
       << (local_tm.tm_year + 1900) << "-"
       << std::setw(2) << std::setfill('0') << (local_tm.tm_mon + 1) << "-"
       << std::setw(2) << std::setfill('0') << local_tm.tm_mday << " "
       << std::setw(2) << std::setfill('0') << local_tm.tm_hour << ":"
       << std::setw(2) << std::setfill('0') << local_tm.tm_min << ":"
       << std::setw(2) << std::setfill('0') << local_tm.tm_sec << "] ["
       << levelStr << "] " << message << "\n";

    std::string logMessage = ss.str();

    // 写入日志文件
    { // 作用域限制锁的生命周期
        std::lock_guard<std::mutex> lock(m_logMutex);
        std::ofstream logStream(m_logFile, std::ios::app);
        if (logStream.is_open()) {
            logStream << logMessage;
            logStream.close();
        } else {
            std::cerr << "Failed to open log file: " << m_logFile << std::endl;
        }
    }

    // 同时输出到控制台
    if (level >= LogLevel::WARNING) {
        std::cout << logMessage;
    }
}

bool AdvancedRiskManager::checkOrder(const execution::Order& order, const RiskMetrics& metrics) {
    log(LogLevel::DEBUG, "Checking order for symbol: " + order.symbol + ", quantity: " + std::to_string(order.quantity));
    // 检查订单是否违反风险规则
    bool isValid = true;

    // 1. 检查持仓限额
    double potentialPosition = metrics.currentPosition + order.quantity;
    if (std::abs(potentialPosition) > m_maxPosition) {
        m_alerts.push_back("Order rejected: Position limit exceeded");
        isValid = false;
    }

    // 2. 检查品种持仓限额
    auto it = m_positionLimits.find(order.symbol);
    if (it != m_positionLimits.end()) {
        double currentSymbolPosition = 0.0;
        auto symbolIt = metrics.positionBySymbol.find(order.symbol);
        if (symbolIt != metrics.positionBySymbol.end()) {
            currentSymbolPosition = symbolIt->second;
        }
        double potentialSymbolPosition = currentSymbolPosition + order.quantity;
        if (std::abs(potentialSymbolPosition) > it->second) {
            m_alerts.push_back("Order rejected: Symbol position limit exceeded for " + order.symbol);
            isValid = false;
        }
    }

    // 3. 检查亏损限额
    if (metrics.dailyPnl < -m_maxDailyLoss) {
        m_alerts.push_back("Order rejected: Daily loss limit exceeded");
        isValid = false;
    }

    // 4. 检查VAR限额
    if (!metrics.recentReturns.empty() && metrics.volatility > 0) {
        double var = calculateVAR(metrics.recentReturns, 0.95);
        if (var > m_var95Threshold) {
            m_alerts.push_back("Order rejected: VAR threshold exceeded");
            isValid = false;
        }
    }

    return isValid;
}

bool AdvancedRiskManager::checkHFTOrderRate(const std::vector<execution::Order>& recentOrders, int timeWindowSeconds) const {
    if (recentOrders.empty() || timeWindowSeconds <= 0) {
        return true; // 没有订单或时间窗口无效，默认为通过
    }

    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto timeWindow = std::chrono::seconds(timeWindowSeconds);
    auto windowStart = now - timeWindow;

    // 统计时间窗口内的订单数
    int orderCount = 0;
    for (const auto& order : recentOrders) {
        // 假设Order有一个timestamp字段表示订单创建时间
        auto orderTime = std::chrono::system_clock::from_time_t(order.timestamp / 1000000000);
        if (orderTime >= windowStart) {
            orderCount++;
        }
    }

    // 检查是否超过最大订单速率
    // 注意：这里的m_maxOrderRatePerSecond是假设的成员变量，需要在类中定义
    int maxOrdersInWindow = m_maxOrderRatePerSecond * timeWindowSeconds;
    if (orderCount > maxOrdersInWindow) {
        log(LogLevel::WARNING, "Order rate exceeded: " + std::to_string(orderCount) + " orders in " + 
            std::to_string(timeWindowSeconds) + " seconds");
        return false;
    }

    return true;
}

bool AdvancedRiskManager::checkOrderSizeConsistency(const execution::Order& order) const {
    // 检查订单大小是否与历史订单一致
    if (m_currentMetrics.avgOrderSize > 0) {
        double orderSizeRatio = std::abs(order.quantity) / m_currentMetrics.avgOrderSize;
        // 如果订单大小是平均订单大小的3倍以上，触发警告
        if (orderSizeRatio > 3.0) {
            log(LogLevel::WARNING, "Order size inconsistency detected for " + order.symbol + ": " + std::to_string(order.quantity) + " vs avg " + std::to_string(m_currentMetrics.avgOrderSize));
            return false;
        }
    }

    return true;
}

void AdvancedRiskManager::updateRiskMetrics(const RiskMetrics& metrics) {
    m_currentMetrics = metrics;

    // 检查是否触发风险预警
    m_alerts.clear();

    // 持仓超限预警
    if (std::abs(metrics.currentPosition) > 0.8 * m_maxPosition) {
        m_alerts.push_back("Warning: Approaching position limit");
    }

    // 亏损预警
    if (metrics.dailyPnl < -0.8 * m_maxDailyLoss) {
        m_alerts.push_back("Warning: Approaching daily loss limit");
    }

    // VAR预警
    if (!metrics.recentReturns.empty()) {
        double var = calculateVAR(metrics.recentReturns, 0.95);
        if (var > 0.8 * m_var95Threshold) {
            m_alerts.push_back("Warning: Approaching VAR threshold");
        }
    }

    // 流动性风险预警
    if (metrics.liquidityRisk > 0.7) {
        m_alerts.push_back("Warning: High liquidity risk");
    }

    // 市场冲击风险预警
    if (metrics.marketImpact > 0.02) {
        m_alerts.push_back("Warning: High market impact cost");
    }
}

void AdvancedRiskManager::setRiskLimits(double maxPosition, double maxLoss, double varThreshold) {
    m_maxPosition = maxPosition;
    m_maxDailyLoss = maxLoss;
    m_var95Threshold = varThreshold;
    log(LogLevel::INFO, "Risk limits updated");
}

// 添加单个品种的持仓限额
void AdvancedRiskManager::setSymbolPositionLimit(const std::string& symbol, double limit) {
    m_positionLimits[symbol] = limit;
    log(LogLevel::INFO, "Position limit set for symbol " + symbol + ": " + std::to_string(limit));
}

// 移除品种的持仓限额
void AdvancedRiskManager::removeSymbolPositionLimit(const std::string& symbol) {
    m_positionLimits.erase(symbol);
    log(LogLevel::INFO, "Position limit removed for symbol " + symbol);
}

// 获取品种持仓限额
std::optional<double> AdvancedRiskManager::getSymbolPositionLimit(const std::string& symbol) const {
    auto it = m_positionLimits.find(symbol);
    if (it != m_positionLimits.end()) {
        return it->second;
    }
    return std::nullopt;
}

// 计算条件VAR(CVaR)
double AdvancedRiskManager::calculateCVaR(const std::vector<double>& returns, double confidenceLevel) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }

    // 历史模拟法CVaR
    std::vector<double> sortedReturns = returns;
    std::sort(sortedReturns.begin(), sortedReturns.end());

    int index = static_cast<int>((1.0 - confidenceLevel) * sortedReturns.size());
    if (index < 0) index = 0;
    if (index >= sortedReturns.size()) index = sortedReturns.size() - 1;

    // 计算尾部预期损失
    double cvar = 0.0;
    int tailCount = sortedReturns.size() - index;
    for (int i = 0; i < tailCount; ++i) {
        cvar += sortedReturns[index + i];
    }
    cvar = -cvar / tailCount; // CVaR通常表示为正数

    return cvar;
}

// 参数化VAR计算
double AdvancedRiskManager::calculateParametricVAR(const std::vector<double>& returns, double confidenceLevel) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }

    // 计算均值和标准差
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    for (double ret : returns) {
        variance += std::pow(ret - mean, 2);
    }
    variance /= returns.size();
    double stdDev = std::sqrt(variance);

    // 计算Z值 (假设正态分布)
    double zValue = 0.0;
    if (confidenceLevel == 0.95) {
        zValue = 1.645;
    } else if (confidenceLevel == 0.99) {
        zValue = 2.33;
    } else if (confidenceLevel == 0.90) {
        zValue = 1.282;
    } else {
        // 对于其他置信水平，默认使用95%
        zValue = 1.645;
        log(LogLevel::WARNING, "Unsupported confidence level, using 95% instead");
    }

    // 计算参数VAR
    return mean - zValue * stdDev;
}

// Monte Carlo模拟VAR
double AdvancedRiskManager::calculateMonteCarloVAR(const std::vector<double>& returns, double confidenceLevel, int numSimulations) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }

    // 计算均值和标准差
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    for (double ret : returns) {
        variance += std::pow(ret - mean, 2);
    }
    variance /= returns.size();
    double stdDev = std::sqrt(variance);

    // 生成随机数引擎
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(mean, stdDev);

    // 执行Monte Carlo模拟
    std::vector<double> simulatedReturns;
    simulatedReturns.reserve(numSimulations);
    for (int i = 0; i < numSimulations; ++i) {
        simulatedReturns.push_back(dist(gen));
    }

    // 对模拟结果排序并计算VAR
    std::sort(simulatedReturns.begin(), simulatedReturns.end());
    int index = static_cast<int>((1.0 - confidenceLevel) * simulatedReturns.size());
    if (index < 0) index = 0;
    if (index >= simulatedReturns.size()) index = simulatedReturns.size() - 1;

    return -simulatedReturns[index];
}

// 历史模拟法VAR计算
double AdvancedRiskManager::calculateVAR(const std::vector<double>& returns, double confidenceLevel) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }

    // 历史模拟法VAR
    std::vector<double> sortedReturns = returns;
    std::sort(sortedReturns.begin(), sortedReturns.end());

    int index = static_cast<int>((1.0 - confidenceLevel) * sortedReturns.size());
    if (index < 0) index = 0;
    if (index >= sortedReturns.size()) index = sortedReturns.size() - 1;

    return -sortedReturns[index]; // VAR通常表示为正数
}

// 获取风险评分
double AdvancedRiskManager::getRiskScore() const {
    double score = 0.0;

    // 1. 持仓风险 (0-25)
    double positionRisk = std::min(25.0, 25.0 * std::abs(m_currentMetrics.currentPosition) / m_maxPosition);

    // 2. 亏损风险 (0-25)
    double lossRisk = 0.0;
    if (m_currentMetrics.dailyPnl < 0) {
        lossRisk = std::min(25.0, 25.0 * std::abs(m_currentMetrics.dailyPnl) / m_maxDailyLoss);
    }

    // 3. VAR风险 (0-20)
    double varRisk = 0.0;
    if (!m_currentMetrics.recentReturns.empty()) {
        double var = calculateVAR(m_currentMetrics.recentReturns, 0.95);
        varRisk = std::min(20.0, 20.0 * var / m_var95Threshold);
    }

    // 4. 波动率风险 (0-10)
    double volatilityRisk = 0.0;
    // 假设波动率基准为0.02 (2%)
    if (m_currentMetrics.volatility > 0.02) {
        volatilityRisk = std::min(10.0, 10.0 * m_currentMetrics.volatility / 0.05); // 最大5%波动率
    }

    // 5. 流动性风险 (0-10)
    double liquidityRisk = std::min(10.0, 10.0 * m_currentMetrics.liquidityRisk);

    // 6. 集中性风险 (0-10)
    double concentrationRisk = 0.0;
    if (!m_currentMetrics.positionBySymbol.empty()) {
        // 计算赫芬达尔指数 (HHI)
        double hhi = 0.0;
        double totalPosition = std::abs(m_currentMetrics.currentPosition);
        if (totalPosition > 0) {
            for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
                double weight = std::abs(position) / totalPosition;
                hhi += weight * weight;
            }
        }
        // HHI范围在1/N到1之间，转换为0-10分
        concentrationRisk = std::min(10.0, 10.0 * (hhi * m_currentMetrics.positionBySymbol.size() - 1) / (m_currentMetrics.positionBySymbol.size() - 1));
    }

    score = positionRisk + lossRisk + varRisk + volatilityRisk + liquidityRisk + concentrationRisk;
    return std::min(100.0, score); // 限制在100以内
}

// 检查是否有风险预警
bool AdvancedRiskManager::hasRiskAlert() const {
    return !m_alerts.empty();
}

// 获取风险预警列表
std::vector<std::string> AdvancedRiskManager::getRiskAlerts() const {
    return m_alerts;
}

// 计算风险归因
std::unordered_map<std::string, double> AdvancedRiskManager::calculateRiskAttribution() const {
    std::unordered_map<std::string, double> attribution;
    double totalScore = getRiskScore();

    if (totalScore == 0) {
        return attribution;
    }

    // 1. 持仓风险归因
    double positionRisk = std::min(25.0, 25.0 * std::abs(m_currentMetrics.currentPosition) / m_maxPosition);
    attribution["position_risk"] = (positionRisk / totalScore) * 100;

    // 2. 亏损风险归因
    double lossRisk = 0.0;
    if (m_currentMetrics.dailyPnl < 0) {
        lossRisk = std::min(25.0, 25.0 * std::abs(m_currentMetrics.dailyPnl) / m_maxDailyLoss);
    }
    attribution["loss_risk"] = (lossRisk / totalScore) * 100;

    // 3. VAR风险归因
    double varRisk = 0.0;
    if (!m_currentMetrics.recentReturns.empty()) {
        double var = calculateVAR(m_currentMetrics.recentReturns, 0.95);
        varRisk = std::min(20.0, 20.0 * var / m_var95Threshold);
    }
    attribution["var_risk"] = (varRisk / totalScore) * 100;

    // 4. 波动率风险归因
    double volatilityRisk = 0.0;
    if (m_currentMetrics.volatility > 0.02) {
        volatilityRisk = std::min(10.0, 10.0 * m_currentMetrics.volatility / 0.05);
    }
    attribution["volatility_risk"] = (volatilityRisk / totalScore) * 100;

    // 5. 流动性风险归因
    double liquidityRisk = std::min(10.0, 10.0 * m_currentMetrics.liquidityRisk);
    attribution["liquidity_risk"] = (liquidityRisk / totalScore) * 100;

    // 6. 集中性风险归因
    double concentrationRisk = 0.0;
    if (!m_currentMetrics.positionBySymbol.empty()) {
        double hhi = 0.0;
        double totalPosition = std::abs(m_currentMetrics.currentPosition);
        if (totalPosition > 0) {
            for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
                double weight = std::abs(position) / totalPosition;
                hhi += weight * weight;
            }
        }
        concentrationRisk = std::min(10.0, 10.0 * (hhi * m_currentMetrics.positionBySymbol.size() - 1) / (m_currentMetrics.positionBySymbol.size() - 1));
    }
    attribution["concentration_risk"] = (concentrationRisk / totalScore) * 100;

    return attribution;
}

double AdvancedRiskManager::calculateDiversificationScore() const {
    if (m_currentMetrics.positionBySymbol.empty()) {
        return 0.0;
    }

    // 计算赫芬达尔指数 (HHI)
    double hhi = 0.0;
    double totalPosition = std::abs(m_currentMetrics.currentPosition);
    if (totalPosition > 0) {
        for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
            double weight = std::abs(position) / totalPosition;
            hhi += weight * weight;
        }
    }

    // HHI范围在1/N到1之间，转换为0-100分的分散化评分
    int numSymbols = m_currentMetrics.positionBySymbol.size();
    double maxHHI = 1.0;
    double minHHI = 1.0 / numSymbols;
    double normalizedHHI = (hhi - minHHI) / (maxHHI - minHHI);

    // 分散化评分越高越好，HHI越低分散化越好
    return 100.0 * (1.0 - normalizedHHI);
}

std::unordered_map<std::string, double> AdvancedRiskManager::getSectorExposure() const {
    std::unordered_map<std::string, double> sectorExposure;
    double totalValue = m_currentMetrics.currentValue;

    if (totalValue == 0) {
        return sectorExposure;
    }

    // 这里简化处理，假设我们有一个函数可以根据符号获取行业
    // 实际应用中，需要一个行业分类映射表
    for (const auto& [symbol, value] : m_currentMetrics.valueBySymbol) {
        std::string sector = "unknown";
        // 示例：根据符号前缀判断行业
        if (symbol.starts_with("AAPL") || symbol.starts_with("MSFT")) {
            sector = "tech";
        } else if (symbol.starts_with("JPM") || symbol.starts_with("BAC")) {
            sector = "financial";
        } else if (symbol.starts_with("XOM") || symbol.starts_with("CVX")) {
            sector = "energy";
        }
        // 更多行业分类...

        sectorExposure[sector] += value / totalValue * 100;
    }

    return sectorExposure;
}

// 运行压力测试
void AdvancedRiskManager::runStressTest() {
    log(LogLevel::INFO, "Running stress test");

    // 1. 大幅下跌场景
    RiskMetrics stressMetrics = m_currentMetrics;
    stressMetrics.dailyPnl = -m_maxDailyLoss * 1.5; // 150%的最大亏损
    updateRiskMetrics(stressMetrics);
    log(LogLevel::INFO, "Stress test 1 (Market crash): Risk score = " + std::to_string(getRiskScore()));
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            log(LogLevel::WARNING, "Stress test alert: " + alert);
        }
    }

    // 2. 高波动场景
    stressMetrics = m_currentMetrics;
    stressMetrics.volatility = 0.05; // 5%波动率
    updateRiskMetrics(stressMetrics);
    log(LogLevel::INFO, "Stress test 2 (High volatility): Risk score = " + std::to_string(getRiskScore()));
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            log(LogLevel::WARNING, "Stress test alert: " + alert);
        }
    }

    // 3. 流动性危机场景
    stressMetrics = m_currentMetrics;
    stressMetrics.liquidityRisk = 0.8; // 高流动性风险
    stressMetrics.marketImpact = 0.03; // 3%市场冲击成本
    updateRiskMetrics(stressMetrics);
    log(LogLevel::INFO, "Stress test 3 (Liquidity Crisis): Risk score = " + std::to_string(getRiskScore()));
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            log(LogLevel::WARNING, "Stress test alert: " + alert);
        }
    }

    // 4. 跳空缺口场景
    stressMetrics = m_currentMetrics;
    // 假设5%的跳空缺口
    for (auto& [symbol, _] : stressMetrics.positionBySymbol) {
        stressMetrics.pnlBySymbol[symbol] *= 0.95; // 5%下跌
    }
    stressMetrics.dailyPnl *= 0.95;
    updateRiskMetrics(stressMetrics);
    log(LogLevel::INFO, "Stress test 4 (Gap opening): Risk score = " + std::to_string(getRiskScore()));
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            log(LogLevel::WARNING, "Stress test alert: " + alert);
        }
    }

    // 5. 极端波动场景
    stressMetrics = m_currentMetrics;
    stressMetrics.volatility = 0.10; // 10%波动率
    stressMetrics.beta = 2.0; // 贝塔系数加倍
    updateRiskMetrics(stressMetrics);
    log(LogLevel::INFO, "Stress test 5 (Extreme volatility): Risk score = " + std::to_string(getRiskScore()));
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            log(LogLevel::WARNING, "Stress test alert: " + alert);
        }
    }

    // 恢复原始指标
    updateRiskMetrics(m_currentMetrics);
    log(LogLevel::INFO, "Stress test completed");
}

// 生成风险报告
void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        log(LogLevel::ERROR, "Failed to open risk report file: " + filename);
        return;
    }

    // 报告头部
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time);

    std::stringstream ss;
    ss << (local_tm.tm_year + 1900) << "-"
       << std::setw(2) << std::setfill('0') << (local_tm.tm_mon + 1) << "-"
       << std::setw(2) << std::setfill('0') << local_tm.tm_mday << " "
       << std::setw(2) << std::setfill('0') << local_tm.tm_hour << ":"
       << std::setw(2) << std::setfill('0') << local_tm.tm_min << ":"
       << std::setw(2) << std::setfill('0') << local_tm.tm_sec;

    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << ss.str() << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = 0.0;
        auto valueIt = m_currentMetrics.valueBySymbol.find(symbol);
        if (valueIt != m_currentMetrics.valueBySymbol.end()) {
            value = valueIt->second;
        }

        double pnl = 0.0;
        auto pnlIt = m_currentMetrics.pnlBySymbol.find(symbol);
        if (pnlIt != m_currentMetrics.pnlBySymbol.end()) {
            pnl = pnlIt->second;
        }

        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    log(LogLevel::INFO, "Risk report generated: " + filename);
}

// 根据市场条件动态调整风险限额
void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 市场波动率因子
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.02) {
        // 波动率越高，风险限额越低
        volatilityFactor = std::max(0.5, 1.0 - (metrics.volatility - 0.02) * 10);
    }

    // 流动性风险因子
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.5) {
        // 流动性风险越高，风险限额越低
        liquidityFactor = std::max(0.6, 1.0 - (metrics.liquidityRisk - 0.5) * 0.8);
    }

    // 风险评分因子
    double riskScoreFactor = 1.0;
    double currentScore = getRiskScore();
    if (currentScore > 70) {
        // 风险评分高，降低风险限额
        riskScoreFactor = std::max(0.7, 1.0 - (currentScore - 70) * 0.01);
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * riskScoreFactor;
    // 限制调整幅度，防止过度调整
    totalFactor = std::max(0.5, std::min(1.5, totalFactor));

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxDailyLoss = m_maxDailyLoss;
    double originalVar95Threshold = m_var95Threshold;

    // 调整风险限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    log(LogLevel::INFO, "Risk limits adjusted based on market conditions");
    log(LogLevel::INFO, "Volatility factor: " + std::to_string(volatilityFactor));
    log(LogLevel::INFO, "Liquidity factor: " + std::to_string(liquidityFactor));
    log(LogLevel::INFO, "Risk score factor: " + std::to_string(riskScoreFactor));
    log(LogLevel::INFO, "Total adjustment factor: " + std::to_string(totalFactor));
    log(LogLevel::INFO, "Max position: " + std::to_string(originalMaxPosition) + " -> " + std::to_string(m_maxPosition));
    log(LogLevel::INFO, "Max daily loss: " + std::to_string(originalMaxDailyLoss) + " -> " + std::to_string(m_maxDailyLoss));
    log(LogLevel::INFO, "VAR 95 threshold: " + std::to_string(originalVar95Threshold) + " -> " + std::to_string(m_var95Threshold));
}
} // namespace risk
} // namespace hft


void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        std::cerr << "Failed to open risk report file: " << filename << std::endl;
        return;
    }

    // 报告头部
    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << std::time(nullptr) << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = m_currentMetrics.valueBySymbol[symbol];
        double pnl = m_currentMetrics.pnlBySymbol[symbol];
        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    std::cout << "Risk report generated: " << filename << std::endl;
}

void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 根据波动率调整风险限额
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.05) { // 高波动
        volatilityFactor = 0.7; // 降低30%的风险限额
    } else if (metrics.volatility < 0.02) { // 低波动
        volatilityFactor = 1.2; // 增加20%的风险限额
    }

    // 根据流动性风险调整
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.7) { // 高流动性风险
        liquidityFactor = 0.8; // 降低20%的风险限额
    }

    // 根据市场冲击成本调整
    double impactFactor = 1.0;
    if (metrics.marketImpact > 0.02) { // 高市场冲击
        impactFactor = 0.9; // 降低10%的风险限额
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * impactFactor;

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxLoss = m_maxDailyLoss;
    double originalVarThreshold = m_var95Threshold;

    // 调整限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    std::cout << "Risk limits adjusted based on market conditions:" << std::endl;
    std::cout << "- Max Position: " << originalMaxPosition << " -> " << m_maxPosition << std::endl;
    std::cout << "- Max Daily Loss: " << originalMaxLoss << " -> " << m_maxDailyLoss << std::endl;
    std::cout << "- VAR 95 Threshold: " << originalVarThreshold << " -> " << m_var95Threshold << std::endl;
}
} // namespace risk
} // namespace hft


void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        log(LogLevel::ERROR, "Failed to open risk report file: " + filename);
        return;
    }

    // 报告头部
    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << std::time(nullptr) << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = 0.0;
        auto valueIt = m_currentMetrics.valueBySymbol.find(symbol);
        if (valueIt != m_currentMetrics.valueBySymbol.end()) {
            value = valueIt->second;
        }

        double pnl = 0.0;
        auto pnlIt = m_currentMetrics.pnlBySymbol.find(symbol);
        if (pnlIt != m_currentMetrics.pnlBySymbol.end()) {
            pnl = pnlIt->second;
        }

        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    log(LogLevel::INFO, "Risk report generated: " + filename);
}

// 根据市场条件动态调整风险限额
void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 根据波动率调整风险限额
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.05) { // 高波动
        volatilityFactor = 0.7; // 降低30%的风险限额
    } else if (metrics.volatility < 0.02) { // 低波动
        volatilityFactor = 1.2; // 增加20%的风险限额
    }

    // 根据流动性风险调整
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.7) { // 高流动性风险
        liquidityFactor = 0.8; // 降低20%的风险限额
    }

    // 根据市场冲击成本调整
    double impactFactor = 1.0;
    if (metrics.marketImpact > 0.02) { // 高市场冲击
        impactFactor = 0.9; // 降低10%的风险限额
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * impactFactor;

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxLoss = m_maxDailyLoss;
    double originalVarThreshold = m_var95Threshold;

    // 调整限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    log(LogLevel::INFO, "Risk limits adjusted based on market conditions");
    log(LogLevel::INFO, "- Max Position: " + std::to_string(originalMaxPosition) + " -> " + std::to_string(m_maxPosition));
    log(LogLevel::INFO, "- Max Daily Loss: " + std::to_string(originalMaxLoss) + " -> " + std::to_string(m_maxDailyLoss));
    log(LogLevel::INFO, "- VAR 95 Threshold: " + std::to_string(originalVarThreshold) + " -> " + std::to_string(m_var95Threshold));
}
} // namespace risk
} // namespace hft


void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        log(LogLevel::ERROR, "Failed to open risk report file: " + filename);
        return;
    }

    // 报告头部
    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << std::time(nullptr) << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = m_currentMetrics.valueBySymbol[symbol];
        double pnl = m_currentMetrics.pnlBySymbol[symbol];
        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    log(LogLevel::INFO, "Risk report generated: " + filename);
}

void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 根据波动率调整风险限额
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.05) { // 高波动
        volatilityFactor = 0.7; // 降低30%的风险限额
    } else if (metrics.volatility < 0.02) { // 低波动
        volatilityFactor = 1.2; // 增加20%的风险限额
    }

    // 根据流动性风险调整
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.7) { // 高流动性风险
        liquidityFactor = 0.8; // 降低20%的风险限额
    }

    // 根据市场冲击成本调整
    double impactFactor = 1.0;
    if (metrics.marketImpact > 0.02) { // 高市场冲击
        impactFactor = 0.9; // 降低10%的风险限额
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * impactFactor;

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxLoss = m_maxDailyLoss;
    double originalVarThreshold = m_var95Threshold;

    // 调整限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    log(LogLevel::INFO, "Risk limits adjusted based on market conditions");
    log(LogLevel::INFO, "- Max Position: " + std::to_string(originalMaxPosition) + " -> " + std::to_string(m_maxPosition));
    log(LogLevel::INFO, "- Max Daily Loss: " + std::to_string(originalMaxLoss) + " -> " + std::to_string(m_maxDailyLoss));
    log(LogLevel::INFO, "- VAR 95 Threshold: " + std::to_string(originalVarThreshold) + " -> " + std::to_string(m_var95Threshold));
}
} // namespace risk
} // namespace hft


void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        log(LogLevel::ERROR, "Failed to open risk report file: " + filename);
        return;
    }

    // 报告头部
    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << std::time(nullptr) << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = m_currentMetrics.valueBySymbol[symbol];
        double pnl = m_currentMetrics.pnlBySymbol[symbol];
        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    log(LogLevel::INFO, "Risk report generated: " + filename);
}

void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 根据波动率调整风险限额
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.05) { // 高波动
        volatilityFactor = 0.7; // 降低30%的风险限额
    } else if (metrics.volatility < 0.02) { // 低波动
        volatilityFactor = 1.2; // 增加20%的风险限额
    }

    // 根据流动性风险调整
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.7) { // 高流动性风险
        liquidityFactor = 0.8; // 降低20%的风险限额
    }

    // 根据市场冲击成本调整
    double impactFactor = 1.0;
    if (metrics.marketImpact > 0.02) { // 高市场冲击
        impactFactor = 0.9; // 降低10%的风险限额
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * impactFactor;

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxLoss = m_maxDailyLoss;
    double originalVarThreshold = m_var95Threshold;

    // 调整限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    log(LogLevel::INFO, "Risk limits adjusted based on market conditions");
    log(LogLevel::INFO, "- Max Position: " + std::to_string(originalMaxPosition) + " -> " + std::to_string(m_maxPosition));
    log(LogLevel::INFO, "- Max Daily Loss: " + std::to_string(originalMaxLoss) + " -> " + std::to_string(m_maxDailyLoss));
    log(LogLevel::INFO, "- VAR 95 Threshold: " + std::to_string(originalVarThreshold) + " -> " + std::to_string(m_var95Threshold));
}
} // namespace risk
} // namespace hft


void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        log(LogLevel::ERROR, "Failed to open risk report file: " + filename);
        return;
    }

    // 报告头部
    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << std::time(nullptr) << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = m_currentMetrics.valueBySymbol[symbol];
        double pnl = m_currentMetrics.pnlBySymbol[symbol];
        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    log(LogLevel::INFO, "Risk report generated: " + filename);
}

void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 根据波动率调整风险限额
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.05) { // 高波动
        volatilityFactor = 0.7; // 降低30%的风险限额
    } else if (metrics.volatility < 0.02) { // 低波动
        volatilityFactor = 1.2; // 增加20%的风险限额
    }

    // 根据流动性风险调整
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.7) { // 高流动性风险
        liquidityFactor = 0.8; // 降低20%的风险限额
    }

    // 根据市场冲击成本调整
    double impactFactor = 1.0;
    if (metrics.marketImpact > 0.02) { // 高市场冲击
        impactFactor = 0.9; // 降低10%的风险限额
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * impactFactor;

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxLoss = m_maxDailyLoss;
    double originalVarThreshold = m_var95Threshold;

    // 调整限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    log(LogLevel::INFO, "Risk limits adjusted based on market conditions");
    log(LogLevel::INFO, "- Max Position: " + std::to_string(originalMaxPosition) + " -> " + std::to_string(m_maxPosition));
    log(LogLevel::INFO, "- Max Daily Loss: " + std::to_string(originalMaxLoss) + " -> " + std::to_string(m_maxDailyLoss));
    log(LogLevel::INFO, "- VAR 95 Threshold: " + std::to_string(originalVarThreshold) + " -> " + std::to_string(m_var95Threshold));
}
} // namespace risk
} // namespace hft


void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        log(LogLevel::ERROR, "Failed to open risk report file: " + filename);
        return;
    }

    // 报告头部
    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << std::time(nullptr) << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = m_currentMetrics.valueBySymbol[symbol];
        double pnl = m_currentMetrics.pnlBySymbol[symbol];
        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    log(LogLevel::INFO, "Risk report generated: " + filename);
}

void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 根据波动率调整风险限额
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.05) { // 高波动
        volatilityFactor = 0.7; // 降低30%的风险限额
    } else if (metrics.volatility < 0.02) { // 低波动
        volatilityFactor = 1.2; // 增加20%的风险限额
    }

    // 根据流动性风险调整
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.7) { // 高流动性风险
        liquidityFactor = 0.8; // 降低20%的风险限额
    }

    // 根据市场冲击成本调整
    double impactFactor = 1.0;
    if (metrics.marketImpact > 0.02) { // 高市场冲击
        impactFactor = 0.9; // 降低10%的风险限额
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * impactFactor;

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxLoss = m_maxDailyLoss;
    double originalVarThreshold = m_var95Threshold;

    // 调整限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    log(LogLevel::INFO, "Risk limits adjusted based on market conditions");
    log(LogLevel::INFO, "- Max Position: " + std::to_string(originalMaxPosition) + " -> " + std::to_string(m_maxPosition));
    log(LogLevel::INFO, "- Max Daily Loss: " + std::to_string(originalMaxLoss) + " -> " + std::to_string(m_maxDailyLoss));
    log(LogLevel::INFO, "- VAR 95 Threshold: " + std::to_string(originalVarThreshold) + " -> " + std::to_string(m_var95Threshold));
}
} // namespace risk
} // namespace hft


void AdvancedRiskManager::generateRiskReport(const std::string& filename) {
    std::ofstream reportFile(filename);
    if (!reportFile.is_open()) {
        log(LogLevel::ERROR, "Failed to open risk report file: " + filename);
        return;
    }

    // 报告头部
    reportFile << "==================== HFT System Risk Report ====================" << std::endl;
    reportFile << "Report generated on: " << std::time(nullptr) << std::endl;
    reportFile << "==============================================================" << std::endl;

    // 风险概览
    reportFile << "\n=== Risk Overview ===" << std::endl;
    reportFile << "Risk Score: " << std::fixed << std::setprecision(2) << getRiskScore() << std::endl;
    reportFile << "Current Position: " << std::fixed << std::setprecision(2) << m_currentMetrics.currentPosition << std::endl;
    reportFile << "Current Value: $" << std::fixed << std::setprecision(2) << m_currentMetrics.currentValue << std::endl;
    reportFile << "Daily PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.dailyPnl << std::endl;
    reportFile << "Total PnL: $" << std::fixed << std::setprecision(2) << m_currentMetrics.totalPnl << std::endl;
    reportFile << "Max Drawdown: " << std::fixed << std::setprecision(2) << m_currentMetrics.maxDrawdown * 100 << "%" << std::endl;
    reportFile << "Volatility: " << std::fixed << std::setprecision(2) << m_currentMetrics.volatility * 100 << "%" << std::endl;
    reportFile << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << m_currentMetrics.sharpeRatio << std::endl;

    // VAR指标
    reportFile << "\n=== VAR Metrics ===" << std::endl;
    reportFile << "95% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var95 << std::endl;
    reportFile << "99% Historical VAR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.var99 << std::endl;
    reportFile << "95% Parametric VAR: $" << std::fixed << std::setprecision(2) << calculateParametricVAR(m_currentMetrics.recentReturns, 0.95) << std::endl;
    reportFile << "95% Monte Carlo VAR: $" << std::fixed << std::setprecision(2) << calculateMonteCarloVAR(m_currentMetrics.recentReturns, 0.95, 10000) << std::endl;
    reportFile << "95% CVaR: $" << std::fixed << std::setprecision(2) << m_currentMetrics.cvVar95 << std::endl;

    // 持仓分布
    reportFile << "\n=== Position Distribution ===" << std::endl;
    reportFile << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Position" << std::setw(15) << "Value" << std::setw(15) << "PnL" << std::endl;
    reportFile << std::string(55, '-') << std::endl;
    for (const auto& [symbol, position] : m_currentMetrics.positionBySymbol) {
        double value = m_currentMetrics.valueBySymbol[symbol];
        double pnl = m_currentMetrics.pnlBySymbol[symbol];
        reportFile << std::left << std::setw(10) << symbol << std::setw(15) << position << std::setw(15) << value << std::setw(15) << pnl << std::endl;
    }

    // 风险预警
    reportFile << "\n=== Risk Alerts ===" << std::endl;
    if (hasRiskAlert()) {
        for (const auto& alert : getRiskAlerts()) {
            reportFile << "- " << alert << std::endl;
        }
    } else {
        reportFile << "No risk alerts at this time." << std::endl;
    }

    reportFile.close();
    log(LogLevel::INFO, "Risk report generated: " + filename);
}

void AdvancedRiskManager::adjustRiskLimitsBasedOnMarketConditions(const RiskMetrics& metrics) {
    // 根据波动率调整风险限额
    double volatilityFactor = 1.0;
    if (metrics.volatility > 0.05) { // 高波动
        volatilityFactor = 0.7; // 降低30%的风险限额
    } else if (metrics.volatility < 0.02) { // 低波动
        volatilityFactor = 1.2; // 增加20%的风险限额
    }

    // 根据流动性风险调整
    double liquidityFactor = 1.0;
    if (metrics.liquidityRisk > 0.7) { // 高流动性风险
        liquidityFactor = 0.8; // 降低20%的风险限额
    }

    // 根据市场冲击成本调整
    double impactFactor = 1.0;
    if (metrics.marketImpact > 0.02) { // 高市场冲击
        impactFactor = 0.9; // 降低10%的风险限额
    }

    // 综合调整因子
    double totalFactor = volatilityFactor * liquidityFactor * impactFactor;

    // 保存原始限额用于日志
    double originalMaxPosition = m_maxPosition;
    double originalMaxLoss = m_maxDailyLoss;
    double originalVarThreshold = m_var95Threshold;

    // 调整限额
    m_maxPosition *= totalFactor;
    m_maxDailyLoss *= totalFactor;
    m_var95Threshold *= totalFactor;

    // 记录调整
    log(LogLevel::INFO, "Risk limits adjusted based on market conditions");
    log(LogLevel::INFO, "- Max Position: " + std::to_string(originalMaxPosition) + " -> " + std::to_string(m_maxPosition));
    log(LogLevel::INFO, "- Max Daily Loss: " + std::to_string(originalMaxLoss) + " -> " + std::