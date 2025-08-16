#pragma once

/**
 * 🎯 专业配置生成器 - 为不同交易场景定制最优配置
 * 
 * 支持场景：
 * - 🏎️ 超高频套利 (Ultra-HFT Arbitrage)
 * - 📊 量化对冲基金 (Quantitative Hedge Fund)
 * - 🏦 投资银行自营 (Investment Bank Proprietary)
 * - 💰 加密货币交易 (Cryptocurrency Trading)
 * - 🌍 跨市场套利 (Cross-Market Arbitrage)
 * - 🎲 算法交易 (Algorithmic Trading)
 * - 🛡️ 风险平价 (Risk Parity)
 * - ⚡ 市场做市 (Market Making)
 */

#include "GlobalRiskPredictionModel.h"
#include "RiskPredictionFactory.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

namespace hft {
namespace ai {
namespace config {

/**
 * 🎯 交易场景枚举
 */
enum class TradingScenario {
    ULTRA_HFT_ARBITRAGE,        // 超高频套利
    QUANTITATIVE_HEDGE_FUND,    // 量化对冲基金
    INVESTMENT_BANK_PROP,       // 投行自营交易
    CRYPTOCURRENCY_TRADING,     // 加密货币交易
    CROSS_MARKET_ARBITRAGE,     // 跨市场套利
    ALGORITHMIC_TRADING,        // 算法交易
    RISK_PARITY_FUND,          // 风险平价基金
    MARKET_MAKING,             // 市场做市
    VOLATILITY_TRADING,        // 波动率交易
    STATISTICAL_ARBITRAGE      // 统计套利
};

/**
 * 🏗️ 硬件配置规格
 */
struct HardwareProfile {
    std::string name;
    int cpu_cores;
    double memory_gb;
    int gpu_count;
    std::string gpu_model;
    bool has_quantum_processor;
    double network_latency_us;
    int storage_iops;
    
    static HardwareProfile getRecommendedProfile(TradingScenario scenario) {
        switch (scenario) {
            case TradingScenario::ULTRA_HFT_ARBITRAGE:
                return {"UltraHFT", 64, 128.0, 4, "RTX 4090", true, 0.1, 1000000};
            case TradingScenario::QUANTITATIVE_HEDGE_FUND:
                return {"QuantFund", 128, 512.0, 8, "A100", true, 1.0, 500000};
            case TradingScenario::CRYPTOCURRENCY_TRADING:
                return {"CryptoTrader", 32, 64.0, 2, "RTX 4080", false, 5.0, 200000};
            default:
                return {"Standard", 16, 32.0, 1, "RTX 3080", false, 10.0, 100000};
        }
    }
};

/**
 * 🎯 专业配置生成器
 */
class ProfessionalConfigGenerator {
public:
    ProfessionalConfigGenerator();
    
    // 核心配置生成方法
    json generateConfig(
        TradingScenario scenario,
        const HardwareProfile& hardware,
        const json& custom_overrides = {}
    );
    
    // 预设配置
    json getUltraHFTConfig();
    json getQuantHedgeFundConfig();
    json getInvestmentBankConfig();
    json getCryptocurrencyConfig();
    json getCrossMarketConfig();
    json getAlgorithmicTradingConfig();
    json getRiskParityConfig();
    json getMarketMakingConfig();
    json getVolatilityTradingConfig();
    json getStatisticalArbitrageConfig();
    
    // 配置优化
    json optimizeForLatency(const json& base_config, double target_latency_us);
    json optimizeForAccuracy(const json& base_config, double target_accuracy);
    json optimizeForThroughput(const json& base_config, int target_tps);
    json optimizeForMemory(const json& base_config, double memory_limit_gb);
    
    // 动态配置调整
    json adaptConfigToMarketConditions(
        const json& base_config,
        const json& market_conditions
    );
    
    // 风险配置
    json generateRiskManagementConfig(
        TradingScenario scenario,
        double risk_tolerance,
        const std::vector<std::string>& risk_factors
    );
    
    // 合规配置
    json generateComplianceConfig(
        const std::string& jurisdiction,
        const std::vector<std::string>& regulations
    );
    
    // A/B测试配置
    std::vector<json> generateABTestConfigs(
        const json& base_config,
        const std::vector<std::string>& test_parameters
    );
    
private:
    // 配置模板
    json base_model_config_;
    json base_quantum_config_;
    json base_performance_config_;
    json base_monitoring_config_;
    
    // 场景特定参数
    std::unordered_map<TradingScenario, json> scenario_parameters_;
    
    // 辅助方法
    json mergeConfigs(const json& base, const json& override);
    json calculateOptimalBatchSize(const HardwareProfile& hardware);
    json selectOptimalModels(TradingScenario scenario);
    json configureCaching(const HardwareProfile& hardware);
    json setupDistributedComputing(const HardwareProfile& hardware);
};

/**
 * ⚡ 超高频交易配置 - 极致延迟优化
 */
class UltraHFTConfigBuilder {
public:
    static json build() {
        return {
            {"scenario", "ULTRA_HFT_ARBITRAGE"},
            {"target_latency_ns", 10000},  // 10微秒
            {"target_accuracy", 0.88},     // 适当降低精度换取速度
            
            {"model", {
                {"architecture", "LIGHTWEIGHT_TRANSFORMER"},
                {"hidden_dim", 256},       // 减小模型尺寸
                {"num_layers", 4},         // 减少层数
                {"num_heads", 8},
                {"dropout_rate", 0.05},    // 降低dropout
                {"use_quantization", true}, // 启用量化
                {"mixed_precision", true}
            }},
            
            {"quantum", {
                {"enabled", false},        // 禁用量子计算降低延迟
                {"fallback_classical", true}
            }},
            
            {"performance", {
                {"batch_size", 1},         // 单样本推理
                {"prefetch_batches", 4},
                {"parallel_workers", 16},
                {"gpu_memory_fraction", 0.3},
                {"enable_cuda_graphs", true},
                {"tensor_rt_optimization", true}
            }},
            
            {"features", {
                {"price_features", true},
                {"orderbook_features", true},
                {"news_features", false},  // 禁用慢特征
                {"macro_features", false},
                {"quantum_features", false}
            }},
            
            {"risk_management", {
                {"max_position_size", 1000000},
                {"stop_loss_threshold", 0.001},
                {"circuit_breaker", 0.005}
            }},
            
            {"monitoring", {
                {"enable_real_time", true},
                {"metrics_interval_us", 100},
                {"alert_latency_threshold_us", 15}
            }}
        };
    }
};

/**
 * 🧠 量化对冲基金配置 - 平衡精度与性能
 */
class QuantHedgeFundConfigBuilder {
public:
    static json build() {
        return {
            {"scenario", "QUANTITATIVE_HEDGE_FUND"},
            {"target_latency_ns", 100000},  // 100微秒
            {"target_accuracy", 0.96},      // 高精度要求
            
            {"model", {
                {"architecture", "MULTI_MODAL_TRANSFORMER"},
                {"hidden_dim", 1024},
                {"num_layers", 12},
                {"num_heads", 16},
                {"dropout_rate", 0.1},
                {"use_ensemble", true},
                {"ensemble_size", 5}
            }},
            
            {"quantum", {
                {"enabled", true},
                {"num_qubits", 32},
                {"quantum_algorithms", {"QFT", "VQE", "QAOA"}},
                {"error_correction", true}
            }},
            
            {"performance", {
                {"batch_size", 32},
                {"prefetch_batches", 8},
                {"parallel_workers", 32},
                {"gpu_memory_fraction", 0.8},
                {"distributed_inference", true}
            }},
            
            {"features", {
                {"price_features", true},
                {"orderbook_features", true},
                {"news_features", true},
                {"macro_features", true},
                {"sentiment_features", true},
                {"alternative_data", true},
                {"quantum_features", true}
            }},
            
            {"risk_management", {
                {"portfolio_var_limit", 0.02},
                {"sector_concentration_limit", 0.1},
                {"correlation_monitoring", true},
                {"stress_testing", true}
            }},
            
            {"strategies", {
                {"long_short_equity", true},
                {"relative_value", true},
                {"event_driven", true},
                {"macro_global", true}
            }}
        };
    }
};

/**
 * 💰 加密货币交易配置 - 适应高波动性
 */
class CryptocurrencyConfigBuilder {
public:
    static json build() {
        return {
            {"scenario", "CRYPTOCURRENCY_TRADING"},
            {"target_latency_ns", 50000},   // 50微秒
            {"target_accuracy", 0.92},
            
            {"model", {
                {"architecture", "CRYPTO_SPECIALIZED_TRANSFORMER"},
                {"hidden_dim", 512},
                {"num_layers", 8},
                {"num_heads", 12},
                {"dropout_rate", 0.15},     // 更高dropout应对噪声
                {"volatility_adaptation", true}
            }},
            
            {"quantum", {
                {"enabled", false},         // 加密市场变化太快
                {"classical_fallback", true}
            }},
            
            {"performance", {
                {"batch_size", 16},
                {"update_frequency_ms", 100},
                {"model_refresh_hours", 4}   // 频繁更新模型
            }},
            
            {"features", {
                {"price_features", true},
                {"volume_features", true},
                {"social_sentiment", true},  // 社交媒体重要
                {"whale_tracking", true},    // 巨鲸监控
                {"defi_metrics", true},      // DeFi指标
                {"fear_greed_index", true}
            }},
            
            {"risk_management", {
                {"max_drawdown_limit", 0.15},
                {"volatility_scaling", true},
                {"correlation_with_btc", 0.7},
                {"liquidity_buffer", 0.1}
            }},
            
            {"exchanges", {
                {"supported", ["binance", "coinbase", "kraken", "ftx"]},
                {"arbitrage_detection", true},
                {"cross_exchange_risk", true}
            }}
        };
    }
};

/**
 * 🌍 跨市场套利配置 - 全球市场覆盖
 */
class CrossMarketConfigBuilder {
public:
    static json build() {
        return {
            {"scenario", "CROSS_MARKET_ARBITRAGE"},
            {"target_latency_ns", 200000},  // 200微秒
            {"target_accuracy", 0.94},
            
            {"model", {
                {"architecture", "MULTI_MARKET_TRANSFORMER"},
                {"hidden_dim", 768},
                {"num_layers", 10},
                {"cross_attention_layers", 4},
                {"market_embedding_dim", 128}
            }},
            
            {"markets", {
                {"equity_markets", ["NYSE", "NASDAQ", "LSE", "TSE", "SSE"]},
                {"fx_markets", ["EUR/USD", "GBP/USD", "USD/JPY", "USD/CNY"]},
                {"commodity_markets", ["GOLD", "OIL", "COPPER", "WHEAT"]},
                {"crypto_markets", ["BTC", "ETH", "ADA", "SOL"]},
                {"time_zones", ["America/New_York", "Europe/London", "Asia/Tokyo", "Asia/Shanghai"]}
            }},
            
            {"features", {
                {"price_spreads", true},
                {"currency_hedging", true},
                {"time_zone_effects", true},
                {"regulatory_differences", true},
                {"liquidity_differences", true}
            }},
            
            {"risk_management", {
                {"currency_exposure_limit", 0.05},
                {"market_correlation_monitoring", true},
                {"regulatory_risk_assessment", true},
                {"execution_risk_control", true}
            }}
        };
    }
};

/**
 * 🛡️ 风险平价基金配置 - 风险均衡
 */
class RiskParityConfigBuilder {
public:
    static json build() {
        return {
            {"scenario", "RISK_PARITY_FUND"},
            {"target_latency_ns", 1000000}, // 1毫秒
            {"target_accuracy", 0.98},      // 极高精度
            
            {"model", {
                {"architecture", "RISK_FOCUSED_TRANSFORMER"},
                {"hidden_dim", 1024},
                {"num_layers", 16},
                {"risk_attention_heads", 8},
                {"portfolio_embedding_dim", 256}
            }},
            
            {"quantum", {
                {"enabled", true},
                {"portfolio_optimization", true},
                {"risk_decomposition", true}
            }},
            
            {"risk_factors", {
                {"equity_risk", true},
                {"interest_rate_risk", true},
                {"credit_risk", true},
                {"commodity_risk", true},
                {"currency_risk", true},
                {"volatility_risk", true}
            }},
            
            {"portfolio_construction", {
                {"equal_risk_contribution", true},
                {"rebalancing_frequency", "daily"},
                {"transaction_cost_optimization", true},
                {"leverage_management", true}
            }},
            
            {"risk_management", {
                {"target_volatility", 0.12},
                {"max_component_risk", 0.25},
                {"correlation_monitoring", true},
                {"regime_detection", true}
            }}
        };
    }
};

/**
 * ⚡ 市场做市配置 - 流动性提供
 */
class MarketMakingConfigBuilder {
public:
    static json build() {
        return {
            {"scenario", "MARKET_MAKING"},
            {"target_latency_ns", 25000},   // 25微秒
            {"target_accuracy", 0.90},
            
            {"model", {
                {"architecture", "MARKET_MAKING_TRANSFORMER"},
                {"hidden_dim", 384},
                {"num_layers", 6},
                {"bid_ask_prediction", true},
                {"inventory_management", true}
            }},
            
            {"features", {
                {"orderbook_imbalance", true},
                {"trade_flow_toxicity", true},
                {"inventory_position", true},
                {"adverse_selection", true},
                {"microstructure_features", true}
            }},
            
            {"spread_management", {
                {"min_spread_bps", 1},
                {"max_spread_bps", 50},
                {"dynamic_adjustment", true},
                {"competition_awareness", true}
            }},
            
            {"inventory_management", {
                {"max_position", 10000},
                {"target_position", 0},
                {"inventory_penalty", 0.001},
                {"skew_adjustment", true}
            }},
            
            {"risk_management", {
                {"adverse_selection_protection", true},
                {"latency_arbitrage_detection", true},
                {"order_flow_analysis", true}
            }}
        };
    }
};

/**
 * 🎲 配置优化器 - 动态参数调整
 */
class ConfigOptimizer {
public:
    // 基于历史表现优化配置
    static json optimizeFromBacktest(
        const json& base_config,
        const json& backtest_results,
        const json& optimization_targets
    ) {
        json optimized = base_config;
        
        // 根据回测结果调整参数
        if (backtest_results["sharpe_ratio"].get<double>() < 1.5) {
            // 提高模型复杂度
            optimized["model"]["hidden_dim"] = optimized["model"]["hidden_dim"].get<int>() * 1.2;
            optimized["model"]["num_layers"] = optimized["model"]["num_layers"].get<int>() + 2;
        }
        
        if (backtest_results["max_drawdown"].get<double>() > 0.05) {
            // 加强风险控制
            optimized["risk_management"]["position_limit"] *= 0.8;
            optimized["risk_management"]["stop_loss_threshold"] *= 0.8;
        }
        
        return optimized;
    }
    
    // 基于实时性能调整
    static json adaptToPerformance(
        const json& current_config,
        const json& performance_metrics
    ) {
        json adapted = current_config;
        
        double avg_latency = performance_metrics["avg_latency_ns"].get<double>();
        double target_latency = current_config["target_latency_ns"].get<double>();
        
        if (avg_latency > target_latency * 1.5) {
            // 降低模型复杂度
            adapted["model"]["hidden_dim"] = adapted["model"]["hidden_dim"].get<int>() * 0.8;
            adapted["performance"]["batch_size"] = std::max(1, adapted["performance"]["batch_size"].get<int>() / 2);
        }
        
        return adapted;
    }
    
    // 市场状态自适应
    static json adaptToMarketRegime(
        const json& base_config,
        const std::string& market_regime
    ) {
        json adapted = base_config;
        
        if (market_regime == "high_volatility") {
            adapted["model"]["dropout_rate"] = 0.2;
            adapted["risk_management"]["position_limit"] *= 0.7;
            adapted["monitoring"]["alert_threshold"] = 0.7;
        } else if (market_regime == "low_volatility") {
            adapted["model"]["dropout_rate"] = 0.05;
            adapted["risk_management"]["position_limit"] *= 1.3;
            adapted["monitoring"]["alert_threshold"] = 0.9;
        }
        
        return adapted;
    }
};

} // namespace config
} // namespace ai
} // namespace hft
