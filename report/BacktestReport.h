#pragma once
#include "../core/Types.h"
#include "../analysis/TechnicalIndicators.h"
#include <string>
#include <vector>
#include <map>

namespace hft {
namespace report {

struct TradeStatistics {
    int totalTrades;
    int winningTrades;
    int losingTrades;
    double winRate;
    double profitFactor;
    double averageWin;
    double averageLoss;
    double largestWin;
    double largestLoss;
    double averageHoldingTime;
};

struct RiskMetrics {
    double sharpeRatio;
    double sortinoRatio;
    double maxDrawdown;
    double maxDrawdownDuration;
    double valueAtRisk;
    double expectedShortfall;
    double beta;
    double alpha;
    double informationRatio;
};

struct PerformanceMetrics {
    double totalReturn;
    double annualizedReturn;
    double volatility;
    double annualizedVolatility;
    double calmarRatio;
    double omega;
    double treynorRatio;
    std::vector<double> monthlyReturns;
    std::vector<double> dailyReturns;
};

class BacktestReport {
public:
    void initialize(const std::string& strategyName,
                   const std::string& period,
                   const std::map<std::string, double>& parameters);
                   
    void addTrade(const Trade& trade);
    void addDailyStats(const std::map<std::string, double>& stats);
    void calculateMetrics();
    void generateReport(const std::string& outputPath);
    
private:
    // 报告生成辅助方法
    void generateSummary();
    void generateTradeAnalysis();
    void generateRiskAnalysis();
    void generatePerformanceCharts();
    void generateMonthlyAnalysis();
    void generateDrawdownAnalysis();
    
    // HTML报告生成
    std::string generateHTML();
    std::string generateCSS();
    std::string generateChartJS();
    
    // 数据成员
    std::string strategyName_;
    std::string period_;
    std::map<std::string, double> parameters_;
    std::vector<Trade> trades_;
    std::vector<std::map<std::string, double>> dailyStats_;
    
    TradeStatistics tradeStats_;
    RiskMetrics riskMetrics_;
    PerformanceMetrics perfMetrics_;
};

// 可视化报告生成器
class ReportVisualizer {
public:
    // 图表生成方法
    static std::string generateEquityCurve(const std::vector<double>& equity);
    static std::string generateDrawdownChart(const std::vector<double>& drawdowns);
    static std::string generateMonthlyReturnsHeatmap(const std::vector<double>& returns);
    static std::string generateTradeDistribution(const std::vector<Trade>& trades);
    static std::string generateVolatilityCone(const std::vector<double>& returns);
    
private:
    static std::string getChartTemplate();
    static std::string formatDataForChart(const std::vector<double>& data);
};

// 风险分析报告
class RiskReport {
public:
    void initialize(const std::vector<Trade>& trades,
                   const std::vector<double>& returns);
                   
    void calculateRiskMetrics();
    void generateRiskReport(const std::string& outputPath);
    
private:
    // 风险计算方法
    double calculateValueAtRisk(double confidence);
    double calculateExpectedShortfall(double confidence);
    double calculateBeta(const std::vector<double>& marketReturns);
    
    // 压力测试
    void performStressTest();
    void analyzeLeverageImpact();
    void analyzeCorrelations();
    
    std::vector<Trade> trades_;
    std::vector<double> returns_;
    RiskMetrics riskMetrics_;
};

} // namespace report
} // namespace hft
