#pragma once
#include "../core/Types.h"
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace hft {
namespace risk {

// 风险指标计算器
class RiskMetricsCalculator {
public:
    // 波动率计算
    double calculateVolatility(const std::vector<types::Price>& prices,
                             size_t windowSize = 20) const;
    
    // Value at Risk (VaR)计算
    double calculateVaR(const std::vector<types::Price>& returns,
                       double confidenceLevel = 0.95) const;
    
    // 最大回撤计算
    double calculateMaxDrawdown(const std::vector<types::Price>& prices) const;
    
    // Sharpe比率计算
    double calculateSharpeRatio(const std::vector<types::Price>& returns,
                              double riskFreeRate = 0.02) const;
    
    // Beta系数计算
    double calculateBeta(const std::vector<types::Price>& returns,
                        const std::vector<types::Price>& marketReturns) const;
    
private:
    std::vector<double> calculateReturns(const std::vector<types::Price>& prices) const;
    double calculateMean(const std::vector<double>& values) const;
    double calculateStdDev(const std::vector<double>& values) const;
};

// 压力测试场景
struct StressTestScenario {
    std::string name;
    std::map<std::string, double> priceShocks;  // 价格冲击
    std::map<std::string, double> volumeShocks; // 成交量冲击
    std::map<std::string, double> spreadShocks; // 价差冲击
    double marketVolatility;                    // 市场波动率
    double correlationShock;                    // 相关性冲击
};

// 压力测试引擎
class StressTestEngine {
public:
    void addScenario(const StressTestScenario& scenario);
    void removeScenario(const std::string& name);
    
    // 运行压力测试
    struct TestResult {
        std::string scenarioName;
        double potentialLoss;
        double capitalAdequacy;
        std::map<std::string, double> positionImpact;
        std::map<std::string, double> riskMetrics;
    };
    
    TestResult runTest(const StressTestScenario& scenario);
    std::vector<TestResult> runAllTests();
    
private:
    double simulateMarketImpact(const StressTestScenario& scenario);
    double calculateCapitalImpact(const StressTestScenario& scenario);
    
    std::vector<StressTestScenario> scenarios_;
    RiskMetricsCalculator calculator_;
};

// 信用风险评估器
class CreditRiskEvaluator {
public:
    struct CounterpartyInfo {
        std::string name;
        double creditRating;
        double exposureLimit;
        double currentExposure;
        std::map<std::string, double> positions;
    };
    
    void addCounterparty(const CounterpartyInfo& info);
    void updateExposure(const std::string& counterparty, double amount);
    bool checkCreditLimit(const std::string& counterparty, double amount);
    double calculateTotalExposure() const;
    
private:
    std::map<std::string, CounterpartyInfo> counterparties_;
    std::mutex mutex_;
};

// 风险分析引擎
class RiskAnalysisEngine {
public:
    RiskAnalysisEngine();
    ~RiskAnalysisEngine();
    
    void initialize();
    void start();
    void stop();
    
    // 实时风险分析
    void analyzeMarketRisk();
    void analyzeCreditRisk();
    void analyzeOperationalRisk();
    
    // 定期风险报告
    void generateDailyRiskReport();
    void generateWeeklyRiskReport();
    void generateMonthlyRiskReport();
    
    // 风险预警
    void setRiskAlertHandler(std::function<void(const std::string&)> handler);
    void checkRiskLimits();
    
private:
    struct RiskState {
        double totalExposure;
        double marketRisk;
        double creditRisk;
        double operationalRisk;
        std::map<std::string, double> positionRisks;
    };
    
    void updateRiskState();
    void monitorRiskMetrics();
    void handleRiskAlert(const std::string& message);
    
    std::unique_ptr<RiskMetricsCalculator> calculator_;
    std::unique_ptr<StressTestEngine> stressTest_;
    std::unique_ptr<CreditRiskEvaluator> creditEval_;
    
    RiskState currentState_;
    std::atomic<bool> running_{false};
    std::thread monitorThread_;
    std::mutex stateMutex_;
    
    std::function<void(const std::string&)> alertHandler_;
};

} // namespace risk
} // namespace hft
