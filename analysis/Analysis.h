#pragma once
#include "../core/Types.h"
#include <vector>
#include <map>
#include <memory>

namespace hft {
namespace analysis {

// 时间序列分析
class TimeSeriesAnalyzer {
public:
    // 移动平均
    std::vector<double> calculateSMA(const std::vector<double>& data, 
                                   size_t window);
    std::vector<double> calculateEMA(const std::vector<double>& data, 
                                   size_t window);
    
    // 波动率指标
    std::vector<double> calculateBollingerBands(const std::vector<double>& data,
                                              size_t window,
                                              double numStdDev);
    
    // 趋势指标
    std::vector<double> calculateRSI(const std::vector<double>& data, 
                                   size_t window);
    std::vector<double> calculateMACD(const std::vector<double>& data,
                                    size_t shortWindow,
                                    size_t longWindow);
    
    // 相关性分析
    double calculateCorrelation(const std::vector<double>& data1,
                              const std::vector<double>& data2);
    
private:
    std::vector<double> calculateStdDev(const std::vector<double>& data,
                                      size_t window);
};

// 统计套利分析
class StatisticalArbitrage {
public:
    // 协整性测试
    bool testCointegration(const std::vector<double>& series1,
                          const std::vector<double>& series2);
    
    // 价差分析
    double calculateSpread(const std::vector<double>& series1,
                         const std::vector<double>& series2);
    
    // z-score计算
    std::vector<double> calculateZScore(const std::vector<double>& spread,
                                      size_t window);
    
    // 交易信号生成
    enum class Signal { BUY, SELL, HOLD };
    Signal generateSignal(double currentZScore,
                         double entryThreshold = 2.0,
                         double exitThreshold = 0.5);
    
private:
    double calculateMean(const std::vector<double>& data);
    double calculateStdDev(const std::vector<double>& data);
};

// 市场微观结构分析
class MarketMicrostructure {
public:
    // 订单流毒性分析
    double calculateOrderToxicity();
    
    // 有效价差计算
    double calculateEffectiveSpread();
    
    // 市场冲击成本
    double calculateMarketImpact();
    
    // 流动性分析
    double calculateMarketDepth();
    double calculateResiliency();
    
    // 高频指标
    double calculateOrderImbalance();
    double calculateVolumeImbalance();
    
private:
    std::vector<market::L2Data> orderBookHistory_;
    std::vector<market::Trade> tradeHistory_;
};

// 机器学习分析
class MachineLearningAnalyzer {
public:
    // 特征工程
    struct Features {
        std::vector<double> technical;
        std::vector<double> fundamental;
        std::vector<double> market;
    };
    
    Features extractFeatures(const std::vector<market::L2Data>& marketData,
                           const std::vector<market::Trade>& trades);
    
    // 模型训练
    void trainModel(const Features& features,
                   const std::vector<double>& labels);
    
    // 预测
    double predict(const Features& features);
    
    // 模型评估
    double calculateAccuracy();
    double calculateSharpeRatio();
    
private:
    void preprocessData();
    void validateModel();
    void updateModel();
    
    std::unique_ptr<void> model_; // 具体ML模型的指针
};

// 绩效分析
class PerformanceAnalyzer {
public:
    struct PerformanceMetrics {
        double totalReturn;
        double annualizedReturn;
        double sharpeRatio;
        double maxDrawdown;
        double winRate;
        double profitFactor;
        std::map<std::string, double> strategyReturns;
    };
    
    PerformanceMetrics calculateMetrics(
        const std::vector<execution::Order>& orders,
        const std::vector<market::Trade>& trades);
    
    void generatePerformanceReport(const std::string& filename);
    
private:
    double calculateReturns();
    double calculateDrawdown();
    void analyzeTrades();
    
    std::vector<execution::Order> orderHistory_;
    std::vector<market::Trade> tradeHistory_;
};

} // namespace analysis
} // namespace hft
