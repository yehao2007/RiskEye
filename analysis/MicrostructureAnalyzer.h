#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "../market/MarketData.h"
#include "../core/Types.h"

namespace hft {
namespace analysis {

class MicrostructureAnalyzer {
public:
    // 分析类型
    enum class AnalysisType {
        ORDER_FLOW,         // 订单流分析
        PRICE_IMPACT,       // 价格冲击
        MARKET_MAKING,      // 做市行为
        HFT_ACTIVITY,       // 高频活动
        TOXICITY,          // 流动性毒性
        RESILIENCE,        // 市场弹性
        FRAGMENTATION      // 市场分散度
    };

    // 市场状态
    struct MarketState {
        // 订单簿状态
        struct OrderBookState {
            double bid_ask_spread;
            double depth_imbalance;
            double price_pressure;
            std::vector<double> bid_depths;
            std::vector<double> ask_depths;
        } orderbook;

        // 交易状态
        struct TradeState {
            double volume_weighted_price;
            double trade_imbalance;
            double tick_size_impact;
            std::vector<double> trade_sizes;
        } trades;

        // 市场质量
        struct MarketQuality {
            double liquidity_score;
            double efficiency_score;
            double stability_score;
            double fairness_score;
        } quality;
    };

    // 初始化分析器
    bool initialize() {
        try {
            // 初始化分析模块
            initializeAnalysisModules();
            
            // 加载历史数据
            loadHistoricalData();
            
            // 启动实时分析
            startRealTimeAnalysis();
            
            return true;
        } catch (const std::exception& e) {
            Logger::error("Microstructure analyzer initialization failed: {}", e.what());
            return false;
        }
    }

    // 分析市场状态
    MarketState analyzeMarketState(const market::MarketData& data) {
        MarketState state;
        
        // 分析订单簿
        analyzeOrderBook(data, state);
        
        // 分析交易流
        analyzeTrades(data, state);
        
        // 评估市场质量
        assessMarketQuality(state);
        
        return state;
    }

    // 计算市场指标
    struct MarketMetrics {
        // 流动性指标
        double effective_spread;    // 有效价差
        double realized_spread;     // 实现价差
        double market_impact;       // 市场冲击
        double kyle_lambda;         // Kyle's Lambda
        
        // 价格发现
        double price_efficiency;    // 价格效率
        double information_share;   // 信息份额
        double price_discovery;     // 价格发现
        
        // 市场质量
        double market_resilience;   // 市场弹性
        double market_stability;    // 市场稳定性
        double market_fairness;     // 市场公平性
    };
    MarketMetrics calculateMetrics(const market::MarketData& data);

    // 检测市场异常
    struct MarketAnomaly {
        std::string type;
        double severity;
        std::string description;
        std::vector<std::string> indicators;
    };
    std::vector<MarketAnomaly> detectAnomalies(const market::MarketData& data);

private:
    // 订单流分析器
    class OrderFlowAnalyzer {
    public:
        void analyze(const market::OrderBook& book);
        double calculateToxicity();
        double calculatePressure();
        
    private:
        struct FlowMetrics {
            double order_imbalance;
            double flow_toxicity;
            double price_pressure;
        };
        std::vector<FlowMetrics> flow_history_;
    };

    // 价格冲击分析器
    class PriceImpactAnalyzer {
    public:
        void analyze(const std::vector<market::Trade>& trades);
        double calculateTemporaryImpact();
        double calculatePermanentImpact();
        
    private:
        struct ImpactMetrics {
            double temporary_impact;
            double permanent_impact;
            double decay_rate;
        };
        std::vector<ImpactMetrics> impact_history_;
    };

    // 做市商行为分析器
    class MarketMakingAnalyzer {
    public:
        void analyze(const market::OrderBook& book);
        double calculateInventoryRisk();
        double calculateQuoteQuality();
        
    private:
        struct MMMetrics {
            double quote_presence;
            double spread_contribution;
            double inventory_position;
        };
        std::vector<MMMetrics> mm_history_;
    };

private:
    // 分析方法
    void analyzeOrderBook(const market::MarketData& data, MarketState& state);
    void analyzeTrades(const market::MarketData& data, MarketState& state);
    void assessMarketQuality(MarketState& state);
    
    // 指标计算
    double calculateEffectiveSpread(const market::Trade& trade);
    double calculateRealizedSpread(const market::Trade& trade);
    double calculateKyleLambda(const std::vector<market::Trade>& trades);
    double calculateMarketResilience(const MarketState& state);
    
    // 异常检测
    void detectPriceManipulation();
    void detectQuoteStuffing();
    void detectMomentumIgnition();
    void detectLayering();

private:
    std::unique_ptr<OrderFlowAnalyzer> flow_analyzer_;
    std::unique_ptr<PriceImpactAnalyzer> impact_analyzer_;
    std::unique_ptr<MarketMakingAnalyzer> mm_analyzer_;
    
    // 分析参数
    struct AnalysisParams {
        int window_size{100};
        double significance_level{0.05};
        int min_sample_size{30};
        double toxicity_threshold{0.7};
    } params_;
    
    // 市场状态历史
    std::vector<MarketState> state_history_;
    std::unordered_map<std::string, std::vector<double>> metric_history_;
};

} // namespace analysis
} // namespace hft
