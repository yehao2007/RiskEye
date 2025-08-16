#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "../core/Types.h"

namespace hft {
namespace visualization {

class ChartGenerator {
public:
    // 基础图表类型
    enum class ChartType {
        LINE,
        CANDLESTICK,
        BAR,
        SCATTER,
        HEATMAP,
        SURFACE
    };
    
    struct ChartData {
        std::vector<double> x;
        std::vector<double> y;
        std::vector<double> z;  // 用于3D图表
        std::string label;
        std::string color;
    };
    
    struct ChartConfig {
        std::string title;
        std::string xLabel;
        std::string yLabel;
        bool showLegend{true};
        bool interactive{true};
        std::map<std::string, std::string> customOptions;
    };
    
    // 交易相关图表
    static std::string generatePnLChart(const std::vector<double>& equity,
                                      const std::vector<std::string>& dates);
                                      
    static std::string generateVolatilitySurface(
        const std::vector<std::vector<double>>& surface,
        const std::vector<double>& strikes,
        const std::vector<double>& maturities);
        
    // 高级分析图表
    static std::string generateCorrelationMatrix(
        const std::vector<std::vector<double>>& correlations,
        const std::vector<std::string>& labels);
        
    static std::string generateRiskDecomposition(
        const std::map<std::string, double>& riskFactors);
        
    // 实时监控图表
    static std::string generatePerformanceMetrics(
        const std::vector<double>& metrics,
        const std::vector<std::string>& metricNames);
        
    static std::string generateOrderFlowImbalance(
        const std::vector<double>& buyVolume,
        const std::vector<double>& sellVolume);
        
private:
    static std::string generatePlotlyJS(
        const std::vector<ChartData>& data,
        const ChartConfig& config,
        ChartType type);
        
    static std::string generateD3JS(
        const std::vector<ChartData>& data,
        const ChartConfig& config,
        ChartType type);
};

// 高级风险分析指标计算器
class RiskAnalyzer {
public:
    struct ExtendedRiskMetrics {
        // 基础风险指标
        double valueAtRisk;          // 在险价值
        double expectedShortfall;    // 期望损失
        double maxDrawdown;          // 最大回撤
        
        // 波动率指标
        double impliedVolatility;    // 隐含波动率
        double realizedVolatility;   // 实现波动率
        double volatilitySkew;       // 波动率偏斜
        
        // 相关性指标
        double beta;                 // 贝塔系数
        double correlationMatrix;    // 相关性矩阵
        
        // 流动性指标
        double bidAskSpread;        // 买卖价差
        double marketDepth;         // 市场深度
        double turnoverRatio;       // 换手率
        
        // 压力测试指标
        double stressTestLoss;      // 压力测试损失
        double scenarioImpact;      // 场景影响
        
        // 尾部风险指标
        double tailRisk;            // 尾部风险
        double extremeValueMetric;  // 极值指标
    };
    
    RiskAnalyzer();
    
    // 风险计算方法
    ExtendedRiskMetrics calculateRiskMetrics(
        const std::vector<double>& returns,
        const std::vector<double>& prices,
        const std::vector<double>& volumes);
        
    // 压力测试
    std::vector<double> runStressTest(
        const std::vector<double>& positions,
        const std::vector<std::string>& scenarios);
        
    // 敏感性分析
    std::map<std::string, double> calculateSensitivities(
        const std::vector<double>& positions,
        const std::vector<double>& marketFactors);
        
private:
    // 内部计算方法
    double calculateImpliedVolatility(double price, double strike,
                                    double timeToMaturity, double riskFreeRate);
                                    
    std::vector<double> estimateVolatilitySurface(
        const std::vector<double>& strikes,
        const std::vector<double>& maturities);
        
    double calculateTailRisk(const std::vector<double>& returns);
};

// 实时市场监控系统
class MarketMonitor {
public:
    struct MarketMetrics {
        double orderBookImbalance;
        double volumeProfile;
        double priceImpact;
        double liquidityScore;
        double marketEfficiency;
        std::vector<double> microstructureMetrics;
    };
    
    void initialize();
    void start();
    void stop();
    
    // 市场数据处理
    void processMarketData(const market::MarketData& data);
    void analyzeOrderFlow(const std::vector<market::Order>& orders);
    
    // 实时指标计算
    MarketMetrics calculateRealTimeMetrics();
    
    // 异常检测
    bool detectMarketAnomaly(const MarketMetrics& metrics);
    void generateAlert(const std::string& message);
    
private:
    std::unique_ptr<visualization::ChartGenerator> chartGen_;
    std::unique_ptr<RiskAnalyzer> riskAnalyzer_;
};

} // namespace visualization
} // namespace hft
