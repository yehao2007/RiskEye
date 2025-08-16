#pragma once

#include "DiagnosticCore.h"
#include <finance/trading_analytics.h>
#include <compliance/regulatory.h>
#include <risk/advanced_risk.h>
#include <economics/market_impact.h>

namespace hft {
namespace diagnostics {

// 31. 高频交易专用监控器
class HFTSpecializedMonitor {
public:
    struct TradingMetrics {
        double order_latency_us;          // 订单延迟（微秒）
        double fill_ratio;                // 成交率
        double market_impact;             // 市场冲击
        double slippage;                  // 滑点
        double sharpe_ratio;              // 夏普比率
        double max_drawdown;              // 最大回撤
        double pnl_volatility;            // 盈亏波动率
        std::map<std::string, double> per_symbol_metrics;
        std::chrono::microseconds tick_to_trade_latency;
    };
    
    struct MarketMicrostructure {
        double bid_ask_spread;
        double market_depth;
        double order_book_imbalance;
        double volatility;
        double correlation_matrix[100][100];  // 符号间相关性
        json level2_data;
        std::vector<double> trade_size_distribution;
    };
    
    // 交易性能监控
    TradingMetrics calculateTradingMetrics(const std::string& strategy_id);
    void trackOrderLatency(const std::string& order_id, 
                          std::chrono::microseconds latency);
    void monitorSlippage(const std::string& symbol, double expected_price, 
                        double actual_price);
    
    // 市场微观结构分析
    MarketMicrostructure analyzeMarketMicrostructure(const std::string& symbol);
    double calculateMarketImpact(const std::string& symbol, double order_size);
    std::vector<std::string> detectMarketRegimeChanges();
    
    // 风险监控
    double calculateRealTimeVaR(const std::string& portfolio_id);
    std::vector<std::string> detectConcentrationRisks();
    json generateRiskReport();
    
    // 合规监控
    std::vector<std::string> detectSuspiciousActivity();
    bool validateOrderCompliance(const json& order_data);
    json generateComplianceReport();

private:
    std::unique_ptr<finance::TradingAnalyticsEngine> trading_engine_;
    std::map<std::string, TradingMetrics> strategy_metrics_;
    std::map<std::string, MarketMicrostructure> market_data_;
};

// 32. 算法策略性能分析器
class AlgorithmStrategyAnalyzer {
public:
    struct StrategyPerformance {
        std::string strategy_id;
        double total_return;
        double annualized_return;
        double volatility;
        double information_ratio;
        double calmar_ratio;
        double sortino_ratio;
        std::map<std::string, double> factor_exposures;
        json attribution_analysis;
        std::vector<double> daily_returns;
    };
    
    struct AlgorithmMetrics {
        std::string algorithm_id;
        double cpu_efficiency;           // CPU使用效率
        double memory_efficiency;        // 内存使用效率
        double network_efficiency;       // 网络使用效率
        std::chrono::nanoseconds avg_execution_time;
        double error_rate;
        json optimization_suggestions;
    };
    
    // 策略分析
    StrategyPerformance analyzeStrategyPerformance(const std::string& strategy_id);
    std::vector<std::string> compareStrategies(const std::vector<std::string>& strategy_ids);
    json performAttributionAnalysis(const std::string& strategy_id);
    
    // 算法效率分析
    AlgorithmMetrics analyzeAlgorithmEfficiency(const std::string& algorithm_id);
    std::vector<std::string> identifyBottlenecks(const std::string& algorithm_id);
    json suggestOptimizations(const std::string& algorithm_id);
    
    // 实时监控
    void trackStrategyHealth(const std::string& strategy_id);
    std::vector<DiagnosticIssue> detectStrategyAnomalies();
    void generatePerformanceAlerts();

private:
    std::map<std::string, StrategyPerformance> strategy_performance_;
    std::map<std::string, AlgorithmMetrics> algorithm_metrics_;
    std::unique_ptr<analytics::PerformanceEngine> performance_engine_;
};

// 33. 监管合规自动化器
class RegulatoryComplianceAutomator {
public:
    struct ComplianceRule {
        std::string rule_id;
        std::string regulation_name;     // MiFID II, Dodd-Frank, etc.
        std::string rule_description;
        std::function<bool(const json&)> validation_function;
        Severity violation_severity;
        std::vector<std::string> required_actions;
        json metadata;
    };
    
    struct ComplianceViolation {
        std::string violation_id;
        std::string rule_id;
        std::string entity_id;
        std::string violation_description;
        Severity severity;
        std::chrono::system_clock::time_point detection_time;
        json evidence;
        std::vector<std::string> recommended_actions;
    };
    
    // 规则管理
    void addComplianceRule(const ComplianceRule& rule);
    void updateRegulation(const std::string& regulation_name, const json& updates);
    std::vector<ComplianceRule> getApplicableRules(const std::string& entity_type);
    
    // 合规检查
    std::vector<ComplianceViolation> performComplianceCheck(const json& transaction_data);
    bool validateRealTimeCompliance(const json& order_data);
    json generateComplianceScore(const std::string& entity_id);
    
    // 报告生成
    json generateRegulatoryReport(const std::string& regulation_name);
    void submitAutomaticReports();
    std::vector<std::string> getUpcomingDeadlines();
    
    // 培训和意识
    std::vector<std::string> identifyTrainingNeeds(const std::string& user_id);
    json generateComplianceTraining();

private:
    std::map<std::string, ComplianceRule> compliance_rules_;
    std::vector<ComplianceViolation> violations_;
    std::unique_ptr<compliance::RegulatoryEngine> regulatory_engine_;
};

// 34. 多资产类别风险引擎
class MultiAssetRiskEngine {
public:
    struct AssetClassRisk {
        std::string asset_class;         // equity, fixed_income, fx, commodity, crypto
        double var_1d;                   // 1日VaR
        double var_10d;                  // 10日VaR
        double expected_shortfall;       // 期望损失
        double maximum_drawdown;         // 最大回撤
        std::map<std::string, double> risk_factors;
        json stress_test_results;
    };
    
    struct PortfolioRisk {
        std::string portfolio_id;
        double total_var;
        double component_var;
        double marginal_var;
        double incremental_var;
        std::map<std::string, double> sector_concentrations;
        std::map<std::string, double> geographical_exposures;
        json correlation_analysis;
    };
    
    // 风险计算
    AssetClassRisk calculateAssetClassRisk(const std::string& asset_class);
    PortfolioRisk calculatePortfolioRisk(const std::string& portfolio_id);
    double calculateCrossAssetCorrelation(const std::string& asset1, const std::string& asset2);
    
    // 压力测试
    json performStressTest(const std::string& scenario_name);
    json runMonteCarloSimulation(int num_simulations);
    std::vector<std::string> identifyWorstCaseScenarios();
    
    // 风险归因
    json performRiskAttribution(const std::string& portfolio_id);
    std::map<std::string, double> calculateRiskContributions();
    
    // 动态对冲
    json suggestHedgingStrategies(const std::string& portfolio_id);
    double calculateOptimalHedgeRatio(const std::string& underlying, const std::string& hedge);

private:
    std::map<std::string, AssetClassRisk> asset_risks_;
    std::map<std::string, PortfolioRisk> portfolio_risks_;
    std::unique_ptr<risk::RiskEngine> risk_engine_;
};

// 35. 市场数据质量监控器
class MarketDataQualityMonitor {
public:
    struct DataQualityMetrics {
        std::string data_source;
        double completeness_score;       // 数据完整性
        double accuracy_score;           // 数据准确性
        double timeliness_score;         // 数据及时性
        double consistency_score;        // 数据一致性
        double validity_score;           // 数据有效性
        std::vector<std::string> quality_issues;
        json quality_trend;
    };
    
    struct DataAnomalyDetection {
        std::string symbol;
        std::string anomaly_type;        // price_spike, volume_anomaly, missing_data
        double anomaly_score;
        std::chrono::system_clock::time_point detection_time;
        json anomaly_details;
        std::vector<std::string> potential_causes;
    };
    
    // 数据质量评估
    DataQualityMetrics assessDataQuality(const std::string& data_source);
    std::vector<DataAnomalyDetection> detectDataAnomalies();
    json generateDataQualityReport();
    
    // 数据验证
    bool validatePriceData(const std::string& symbol, const json& price_data);
    bool validateVolumeData(const std::string& symbol, const json& volume_data);
    std::vector<std::string> crossValidateDataSources();
    
    // 数据修正
    json suggestDataCorrections(const std::vector<DataAnomalyDetection>& anomalies);
    void applyDataCleaning(const std::string& symbol, const json& cleaning_rules);
    
    // 供应商管理
    double rankDataVendor(const std::string& vendor_id);
    json compareDataVendors(const std::vector<std::string>& vendor_ids);

private:
    std::map<std::string, DataQualityMetrics> data_quality_;
    std::vector<DataAnomalyDetection> anomaly_history_;
    std::unique_ptr<data::DataQualityEngine> quality_engine_;
};

// 36. 智能订单路由优化器
class IntelligentOrderRoutingOptimizer {
public:
    struct VenueCharacteristics {
        std::string venue_id;
        double average_latency;
        double fill_probability;
        double market_impact;
        double fee_structure;
        std::map<std::string, double> symbol_liquidity;
        json historical_performance;
        bool is_dark_pool;
    };
    
    struct RoutingDecision {
        std::string order_id;
        std::vector<std::pair<std::string, double>> venue_allocations;  // venue_id, percentage
        double expected_execution_quality;
        json routing_rationale;
        std::chrono::microseconds estimated_execution_time;
    };
    
    // 场所分析
    void analyzeVenueCharacteristics(const std::string& venue_id);
    double calculateVenueScore(const std::string& venue_id, const json& order_parameters);
    std::vector<std::string> rankVenues(const std::string& symbol, double order_size);
    
    // 智能路由
    RoutingDecision optimizeOrderRouting(const json& order_parameters);
    void updateRoutingStrategy(const std::string& strategy_name, const json& parameters);
    
    // 执行质量分析
    json analyzeExecutionQuality(const std::string& order_id);
    double calculateImplementationShortfall(const std::string& order_id);
    
    // 机器学习优化
    void trainRoutingModel(const std::vector<json>& historical_executions);
    RoutingDecision predictOptimalRouting(const json& order_parameters);

private:
    std::map<std::string, VenueCharacteristics> venue_data_;
    std::vector<RoutingDecision> routing_history_;
    std::unique_ptr<routing::OrderRoutingEngine> routing_engine_;
};

// 37. 流动性聚合分析器
class LiquidityAggregationAnalyzer {
public:
    struct LiquiditySnapshot {
        std::string symbol;
        std::chrono::system_clock::time_point timestamp;
        std::map<std::string, json> venue_books;  // venue_id -> order_book
        double aggregated_bid_volume;
        double aggregated_ask_volume;
        double weighted_mid_price;
        double effective_spread;
        json liquidity_distribution;
    };
    
    struct LiquidityMetrics {
        std::string symbol;
        double total_liquidity;
        double bid_ask_spread;
        double market_depth;
        double liquidity_imbalance;
        std::map<std::string, double> venue_contributions;
        double fragmentation_index;
        json intraday_patterns;
    };
    
    // 流动性聚合
    LiquiditySnapshot aggregateLiquidity(const std::string& symbol);
    json createConsolidatedOrderBook(const std::string& symbol);
    double calculateBestExecutionPrice(const std::string& symbol, double quantity, bool is_buy);
    
    // 流动性分析
    LiquidityMetrics analyzeLiquidityMetrics(const std::string& symbol);
    double calculateFragmentationIndex(const std::string& symbol);
    std::vector<std::string> identifyLiquidityProviders(const std::string& symbol);
    
    // 流动性预测
    double predictLiquidity(const std::string& symbol, 
                           std::chrono::system_clock::time_point future_time);
    json identifyLiquidityPatterns(const std::string& symbol);
    
    // 交易成本分析
    double estimateMarketImpact(const std::string& symbol, double order_size);
    json calculateExecutionCosts(const json& order_parameters);

private:
    std::map<std::string, LiquiditySnapshot> liquidity_snapshots_;
    std::map<std::string, LiquidityMetrics> liquidity_metrics_;
    std::unique_ptr<liquidity::LiquidityEngine> liquidity_engine_;
};

// 38. 交易后分析引擎
class PostTradeAnalysisEngine {
public:
    struct TradeExecution {
        std::string trade_id;
        std::string symbol;
        double executed_quantity;
        double executed_price;
        std::chrono::system_clock::time_point execution_time;
        std::string venue;
        json execution_details;
    };
    
    struct ExecutionAnalysis {
        std::string trade_id;
        double implementation_shortfall;
        double market_impact;
        double timing_cost;
        double opportunity_cost;
        double total_transaction_cost;
        json benchmark_comparison;
        std::vector<std::string> improvement_suggestions;
    };
    
    // 执行分析
    ExecutionAnalysis analyzeTradeExecution(const TradeExecution& trade);
    json compareExecutionStrategies(const std::vector<std::string>& strategy_ids);
    double calculateTWAP(const std::string& symbol, 
                        std::chrono::system_clock::time_point start,
                        std::chrono::system_clock::time_point end);
    
    // 基准比较
    json benchmarkAgainstTWAP(const std::string& trade_id);
    json benchmarkAgainstVWAP(const std::string& trade_id);
    json benchmarkAgainstArrival(const std::string& trade_id);
    
    // 成本归因
    json attributeTransactionCosts(const std::string& trade_id);
    std::map<std::string, double> calculateCostComponents();
    
    // 性能报告
    json generateExecutionReport(const std::vector<std::string>& trade_ids);
    json calculateBestExecutionMetrics();

private:
    std::vector<TradeExecution> trade_history_;
    std::map<std::string, ExecutionAnalysis> execution_analyses_;
    std::unique_ptr<analysis::PostTradeEngine> post_trade_engine_;
};

// 39. 算法交易策略回测引擎
class AlgorithmicBacktestEngine {
public:
    struct BacktestConfiguration {
        std::string strategy_id;
        std::chrono::system_clock::time_point start_date;
        std::chrono::system_clock::time_point end_date;
        double initial_capital;
        std::vector<std::string> symbols;
        json strategy_parameters;
        json market_data_config;
        json cost_model;
    };
    
    struct BacktestResults {
        std::string strategy_id;
        double total_return;
        double sharpe_ratio;
        double maximum_drawdown;
        double win_rate;
        double profit_factor;
        std::vector<double> daily_pnl;
        json trade_statistics;
        json risk_metrics;
        std::vector<std::string> performance_insights;
    };
    
    // 回测执行
    BacktestResults runBacktest(const BacktestConfiguration& config);
    void validateBacktestData(const BacktestConfiguration& config);
    json simulateMarketConditions(const std::string& scenario);
    
    // 参数优化
    BacktestResults optimizeStrategyParameters(const BacktestConfiguration& base_config);
    json performWalkForwardAnalysis(const BacktestConfiguration& config);
    
    // 结果分析
    json compareBacktestResults(const std::vector<BacktestResults>& results);
    std::vector<std::string> identifyOverfitting(const BacktestResults& results);
    
    // 风险分析
    json calculateBacktestRisks(const BacktestResults& results);
    double assessStrategyRobustness(const std::string& strategy_id);

private:
    std::map<std::string, BacktestResults> backtest_results_;
    std::unique_ptr<backtest::BacktestEngine> backtest_engine_;
    std::unique_ptr<optimization::ParameterOptimizer> parameter_optimizer_;
};

// 40. 实时风险限额管理器
class RealTimeRiskLimitManager {
public:
    struct RiskLimit {
        std::string limit_id;
        std::string limit_type;          // position, var, exposure, concentration
        std::string entity_id;           // trader, desk, portfolio
        double limit_value;
        double current_value;
        double utilization_percentage;
        Severity breach_severity;
        std::vector<std::string> breach_actions;
        json limit_metadata;
    };
    
    struct LimitBreach {
        std::string breach_id;
        std::string limit_id;
        double breach_amount;
        std::chrono::system_clock::time_point breach_time;
        std::string breach_description;
        json breach_context;
        std::vector<std::string> actions_taken;
        bool is_resolved;
    };
    
    // 限额管理
    void setRiskLimit(const RiskLimit& limit);
    void updateRiskLimit(const std::string& limit_id, double new_value);
    std::vector<RiskLimit> getRiskLimits(const std::string& entity_id);
    
    // 实时监控
    std::vector<LimitBreach> checkRiskLimits();
    void monitorPositionLimits(const std::string& entity_id);
    void monitorVaRLimits(const std::string& portfolio_id);
    
    // 限额预警
    std::vector<std::string> generateLimitAlerts();
    void configureAlertThresholds(const std::string& limit_id, 
                                 const std::vector<double>& thresholds);
    
    // 自动化响应
    void executeBreachActions(const LimitBreach& breach);
    void implementEmergencyStops(const std::string& entity_id);

private:
    std::map<std::string, RiskLimit> risk_limits_;
    std::vector<LimitBreach> breach_history_;
    std::unique_ptr<risk::LimitEngine> limit_engine_;
};

} // namespace diagnostics
} // namespace hft
