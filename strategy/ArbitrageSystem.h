#pragma once

#include <memory>
#include <vector>
#include "../core/Types.h"
#include "../market/MarketData.h"

namespace hft {
namespace strategy {

class ArbitrageSystem {
public:
    // 套利类型
    enum class ArbitrageType {
        STATISTICAL,     // 统计套利
        LATENCY,        // 延迟套利
        INDEX,          // 指数套利
        ETF,            // ETF套利
        CROSS_EXCHANGE, // 跨市场套利
        OPTION_PUT_CALL, // 期权跨式套利
        FUTURES_SPOT    // 期现套利
    };

    // 套利机会
    struct ArbitrageOpportunity {
        ArbitrageType type;
        std::vector<std::string> instruments;
        double expected_profit;
        double execution_probability;
        double risk_score;
        std::chrono::microseconds time_window;
    };

    // 初始化套利系统
    bool initialize() {
        try {
            // 初始化价格模型
            initializePriceModels();
            
            // 启动市场监控
            startMarketMonitoring();
            
            // 初始化执行引擎
            initializeExecutionEngine();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Arbitrage system initialization failed: {}", e.what());
            return false;
        }
    }

    // 检测套利机会
    std::vector<ArbitrageOpportunity> detectOpportunities(
        const std::vector<market::MarketData>& market_data) {
        
        std::vector<ArbitrageOpportunity> opportunities;
        
        // 统计套利检测
        detectStatisticalArbitrage(market_data, opportunities);
        
        // 延迟套利检测
        detectLatencyArbitrage(market_data, opportunities);
        
        // 其他套利检测
        detectIndexArbitrage(market_data, opportunities);
        detectETFArbitrage(market_data, opportunities);
        
        // 过滤和排序机会
        filterOpportunities(opportunities);
        rankOpportunities(opportunities);
        
        return opportunities;
    }

    // 执行套利
    struct ArbitrageResult {
        bool success;
        double realized_profit;
        double execution_cost;
        std::chrono::microseconds execution_time;
        std::vector<std::string> executed_orders;
    };
    ArbitrageResult executeArbitrage(const ArbitrageOpportunity& opportunity);

    // 风险管理
    struct RiskMetrics {
        double var_95;
        double max_drawdown;
        double sharpe_ratio;
        double correlation_risk;
    };
    RiskMetrics calculateRisk(const ArbitrageOpportunity& opportunity);

private:
    // 统计套利模型
    class StatisticalArbitrageModel {
    public:
        void train(const std::vector<market::MarketData>& data);
        std::vector<ArbitrageOpportunity> findOpportunities(
            const std::vector<market::MarketData>& data);
        
    private:
        torch::nn::Sequential cointegration_model_;
        std::vector<std::pair<std::string, std::string>> pairs_;
    };

    // 延迟套利模型
    class LatencyArbitrageModel {
    public:
        void initialize(const std::vector<std::string>& venues);
        std::vector<ArbitrageOpportunity> findOpportunities(
            const std::vector<market::MarketData>& data);
        
    private:
        struct VenueLatency {
            double mean_latency_us;
            double jitter_us;
            bool is_reliable;
        };
        std::unordered_map<std::string, VenueLatency> venue_latencies_;
    };

    // ETF套利模型
    class ETFArbitrageModel {
    public:
        void initialize(const std::vector<std::string>& etfs);
        std::vector<ArbitrageOpportunity> findOpportunities(
            const std::vector<market::MarketData>& data);
        
    private:
        struct ETFData {
            std::vector<std::string> components;
            std::vector<double> weights;
            double creation_unit_size;
        };
        std::unordered_map<std::string, ETFData> etf_data_;
    };

private:
    // 套利检测方法
    void detectStatisticalArbitrage(
        const std::vector<market::MarketData>& data,
        std::vector<ArbitrageOpportunity>& opportunities);
        
    void detectLatencyArbitrage(
        const std::vector<market::MarketData>& data,
        std::vector<ArbitrageOpportunity>& opportunities);
        
    void detectIndexArbitrage(
        const std::vector<market::MarketData>& data,
        std::vector<ArbitrageOpportunity>& opportunities);
        
    void detectETFArbitrage(
        const std::vector<market::MarketData>& data,
        std::vector<ArbitrageOpportunity>& opportunities);
    
    // 机会评估方法
    void filterOpportunities(std::vector<ArbitrageOpportunity>& opportunities);
    void rankOpportunities(std::vector<ArbitrageOpportunity>& opportunities);
    double calculateProfitability(const ArbitrageOpportunity& opportunity);
    double calculateExecutionRisk(const ArbitrageOpportunity& opportunity);

private:
    std::unique_ptr<StatisticalArbitrageModel> stat_arb_model_;
    std::unique_ptr<LatencyArbitrageModel> latency_arb_model_;
    std::unique_ptr<ETFArbitrageModel> etf_arb_model_;
    
    // 套利参数
    struct ArbitrageConfig {
        double min_profit_threshold;
        double max_risk_threshold;
        int max_concurrent_trades;
        std::chrono::microseconds max_execution_time;
    } config_;
    
    // 执行状态
    struct ExecutionState {
        std::atomic<int> active_trades{0};
        std::atomic<double> total_profit{0.0};
        std::atomic<int> successful_trades{0};
    } state_;
};

} // namespace strategy
} // namespace hft
