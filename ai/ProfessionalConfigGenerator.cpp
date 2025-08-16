#include "ProfessionalConfigGenerator.h"
#include "../core/Logger.h"
#include <algorithm>
#include <cmath>

namespace hft {
namespace ai {
namespace config {

ProfessionalConfigGenerator::ProfessionalConfigGenerator() {
    // 初始化基础配置模板
    base_model_config_ = {
        {"architecture", "MULTI_MODAL_TRANSFORMER"},
        {"hidden_dim", 512},
        {"num_layers", 8},
        {"num_heads", 8},
        {"dropout_rate", 0.1},
        {"activation", "GELU"},
        {"layer_norm", true},
        {"residual_connections", true}
    };
    
    base_quantum_config_ = {
        {"enabled", false},
        {"num_qubits", 16},
        {"quantum_algorithms", json::array({"QFT", "VQE"})},
        {"error_correction", false},
        {"decoherence_time_us", 100}
    };
    
    base_performance_config_ = {
        {"batch_size", 16},
        {"prefetch_batches", 4},
        {"parallel_workers", 8},
        {"gpu_memory_fraction", 0.5},
        {"mixed_precision", false},
        {"tensor_rt_optimization", false}
    };
    
    base_monitoring_config_ = {
        {"enable_real_time", true},
        {"metrics_interval_ms", 1000},
        {"alert_threshold", 0.8},
        {"logging_level", "INFO"}
    };
    
    // 初始化场景特定参数
    initializeScenarioParameters();
}

void ProfessionalConfigGenerator::initializeScenarioParameters() {
    scenario_parameters_[TradingScenario::ULTRA_HFT_ARBITRAGE] = {
        {"priority", "latency"},
        {"acceptable_accuracy_loss", 0.05},
        {"target_latency_percentile", 99},
        {"risk_tolerance", "low"}
    };
    
    scenario_parameters_[TradingScenario::QUANTITATIVE_HEDGE_FUND] = {
        {"priority", "accuracy"},
        {"min_accuracy_requirement", 0.95},
        {"acceptable_latency_ms", 10},
        {"risk_tolerance", "medium"}
    };
    
    scenario_parameters_[TradingScenario::CRYPTOCURRENCY_TRADING] = {
        {"priority", "adaptability"},
        {"volatility_adjustment", true},
        {"update_frequency_hours", 2},
        {"risk_tolerance", "high"}
    };
}

json ProfessionalConfigGenerator::generateConfig(
    TradingScenario scenario,
    const HardwareProfile& hardware,
    const json& custom_overrides) {
    
    Logger::info("Generating configuration for scenario: {}", static_cast<int>(scenario));
    
    json config;
    
    // 1. 选择基础配置
    switch (scenario) {
        case TradingScenario::ULTRA_HFT_ARBITRAGE:
            config = getUltraHFTConfig();
            break;
        case TradingScenario::QUANTITATIVE_HEDGE_FUND:
            config = getQuantHedgeFundConfig();
            break;
        case TradingScenario::CRYPTOCURRENCY_TRADING:
            config = getCryptocurrencyConfig();
            break;
        case TradingScenario::CROSS_MARKET_ARBITRAGE:
            config = getCrossMarketConfig();
            break;
        case TradingScenario::RISK_PARITY_FUND:
            config = getRiskParityConfig();
            break;
        case TradingScenario::MARKET_MAKING:
            config = getMarketMakingConfig();
            break;
        default:
            config = getAlgorithmicTradingConfig();
    }
    
    // 2. 根据硬件优化配置
    config = optimizeForHardware(config, hardware);
    
    // 3. 应用自定义覆盖
    if (!custom_overrides.empty()) {
        config = mergeConfigs(config, custom_overrides);
    }
    
    // 4. 验证配置合理性
    validateConfig(config, scenario);
    
    Logger::info("Configuration generated successfully");
    return config;
}

json ProfessionalConfigGenerator::getUltraHFTConfig() {
    return UltraHFTConfigBuilder::build();
}

json ProfessionalConfigGenerator::getQuantHedgeFundConfig() {
    return QuantHedgeFundConfigBuilder::build();
}

json ProfessionalConfigGenerator::getCryptocurrencyConfig() {
    return CryptocurrencyConfigBuilder::build();
}

json ProfessionalConfigGenerator::getCrossMarketConfig() {
    return CrossMarketConfigBuilder::build();
}

json ProfessionalConfigGenerator::getRiskParityConfig() {
    return RiskParityConfigBuilder::build();
}

json ProfessionalConfigGenerator::getMarketMakingConfig() {
    return MarketMakingConfigBuilder::build();
}

json ProfessionalConfigGenerator::getAlgorithmicTradingConfig() {
    return {
        {"scenario", "ALGORITHMIC_TRADING"},
        {"target_latency_ns", 500000},  // 500微秒
        {"target_accuracy", 0.93},
        
        {"model", {
            {"architecture", "HYBRID_TRANSFORMER_CNN"},
            {"hidden_dim", 768},
            {"num_layers", 10},
            {"num_heads", 12},
            {"cnn_layers", 3},
            {"kernel_sizes", json::array({3, 5, 7})},
            {"dropout_rate", 0.12}
        }},
        
        {"quantum", {
            {"enabled", true},
            {"num_qubits", 24},
            {"quantum_feature_enhancement", true}
        }},
        
        {"features", {
            {"technical_indicators", true},
            {"pattern_recognition", true},
            {"momentum_features", true},
            {"mean_reversion_features", true},
            {"seasonality_features", true}
        }},
        
        {"strategies", {
            {"trend_following", true},
            {"mean_reversion", true},
            {"momentum", true},
            {"breakout", true}
        }},
        
        {"risk_management", {
            {"position_sizing", "kelly_criterion"},
            {"stop_loss_method", "adaptive_atr"},
            {"portfolio_heat", 0.02}
        }}
    };
}

json ProfessionalConfigGenerator::getVolatilityTradingConfig() {
    return {
        {"scenario", "VOLATILITY_TRADING"},
        {"target_latency_ns", 300000},  // 300微秒
        {"target_accuracy", 0.94},
        
        {"model", {
            {"architecture", "VOLATILITY_SPECIALIZED_TRANSFORMER"},
            {"hidden_dim", 512},
            {"num_layers", 8},
            {"volatility_prediction_head", true},
            {"garch_integration", true},
            {"realized_volatility_features", true}
        }},
        
        {"features", {
            {"intraday_volatility", true},
            {"overnight_gap_risk", true},
            {"vix_term_structure", true},
            {"options_flow", true},
            {"gamma_exposure", true}
        }},
        
        {"strategies", {
            {"long_volatility", true},
            {"short_volatility", true},
            {"volatility_surface_arbitrage", true},
            {"gamma_scalping", true}
        }},
        
        {"risk_management", {
            {"vega_limit", 50000},
            {"gamma_limit", 10000},
            {"theta_decay_management", true}
        }}
    };
}

json ProfessionalConfigGenerator::getStatisticalArbitrageConfig() {
    return {
        {"scenario", "STATISTICAL_ARBITRAGE"},
        {"target_latency_ns", 1000000}, // 1毫秒
        {"target_accuracy", 0.95},
        
        {"model", {
            {"architecture", "PAIRS_TRADING_TRANSFORMER"},
            {"hidden_dim", 768},
            {"num_layers", 12},
            {"cross_asset_attention", true},
            {"cointegration_detection", true}
        }},
        
        {"features", {
            {"price_ratios", true},
            {"spread_features", true},
            {"cointegration_residuals", true},
            {"half_life_estimation", true},
            {"regime_detection", true}
        }},
        
        {"strategies", {
            {"pairs_trading", true},
            {"basket_arbitrage", true},
            {"index_arbitrage", true},
            {"cross_sectional_momentum", true}
        }},
        
        {"risk_management", {
            {"correlation_monitoring", true},
            {"spread_widening_limits", 3.0},
            {"position_concentration", 0.05}
        }}
    };
}

json ProfessionalConfigGenerator::optimizeForLatency(
    const json& base_config,
    double target_latency_us) {
    
    json optimized = base_config;
    
    // 基于目标延迟调整模型参数
    if (target_latency_us < 50) {  // 极低延迟 < 50微秒
        optimized["model"]["hidden_dim"] = 256;
        optimized["model"]["num_layers"] = 4;
        optimized["model"]["num_heads"] = 4;
        optimized["quantum"]["enabled"] = false;
        optimized["performance"]["batch_size"] = 1;
        optimized["performance"]["tensor_rt_optimization"] = true;
        
    } else if (target_latency_us < 200) {  // 低延迟 < 200微秒
        optimized["model"]["hidden_dim"] = 512;
        optimized["model"]["num_layers"] = 6;
        optimized["model"]["num_heads"] = 8;
        optimized["performance"]["batch_size"] = 4;
        optimized["performance"]["mixed_precision"] = true;
        
    } else if (target_latency_us < 1000) {  // 中等延迟 < 1毫秒
        optimized["model"]["hidden_dim"] = 768;
        optimized["model"]["num_layers"] = 10;
        optimized["model"]["num_heads"] = 12;
        optimized["performance"]["batch_size"] = 16;
        
    } else {  // 高延迟 >= 1毫秒
        optimized["model"]["hidden_dim"] = 1024;
        optimized["model"]["num_layers"] = 16;
        optimized["model"]["num_heads"] = 16;
        optimized["performance"]["batch_size"] = 32;
        optimized["quantum"]["enabled"] = true;
    }
    
    optimized["target_latency_ns"] = static_cast<int64_t>(target_latency_us * 1000);
    
    return optimized;
}

json ProfessionalConfigGenerator::optimizeForAccuracy(
    const json& base_config,
    double target_accuracy) {
    
    json optimized = base_config;
    
    if (target_accuracy > 0.98) {  // 极高精度
        optimized["model"]["hidden_dim"] = 1536;
        optimized["model"]["num_layers"] = 20;
        optimized["model"]["num_heads"] = 24;
        optimized["model"]["use_ensemble"] = true;
        optimized["model"]["ensemble_size"] = 7;
        optimized["quantum"]["enabled"] = true;
        optimized["quantum"]["num_qubits"] = 64;
        
    } else if (target_accuracy > 0.95) {  // 高精度
        optimized["model"]["hidden_dim"] = 1024;
        optimized["model"]["num_layers"] = 16;
        optimized["model"]["num_heads"] = 16;
        optimized["model"]["use_ensemble"] = true;
        optimized["model"]["ensemble_size"] = 5;
        
    } else if (target_accuracy > 0.90) {  // 中等精度
        optimized["model"]["hidden_dim"] = 768;
        optimized["model"]["num_layers"] = 12;
        optimized["model"]["num_heads"] = 12;
        optimized["model"]["use_ensemble"] = true;
        optimized["model"]["ensemble_size"] = 3;
        
    } else {  // 基础精度
        optimized["model"]["hidden_dim"] = 512;
        optimized["model"]["num_layers"] = 8;
        optimized["model"]["num_heads"] = 8;
    }
    
    optimized["target_accuracy"] = target_accuracy;
    
    return optimized;
}

json ProfessionalConfigGenerator::optimizeForThroughput(
    const json& base_config,
    int target_tps) {
    
    json optimized = base_config;
    
    if (target_tps > 50000) {  // 超高吞吐量
        optimized["performance"]["batch_size"] = 128;
        optimized["performance"]["parallel_workers"] = 64;
        optimized["performance"]["gpu_memory_fraction"] = 0.9;
        optimized["performance"]["distributed_inference"] = true;
        
    } else if (target_tps > 10000) {  // 高吞吐量
        optimized["performance"]["batch_size"] = 64;
        optimized["performance"]["parallel_workers"] = 32;
        optimized["performance"]["gpu_memory_fraction"] = 0.8;
        
    } else if (target_tps > 1000) {  // 中等吞吐量
        optimized["performance"]["batch_size"] = 32;
        optimized["performance"]["parallel_workers"] = 16;
        optimized["performance"]["gpu_memory_fraction"] = 0.6;
        
    } else {  // 基础吞吐量
        optimized["performance"]["batch_size"] = 16;
        optimized["performance"]["parallel_workers"] = 8;
        optimized["performance"]["gpu_memory_fraction"] = 0.4;
    }
    
    return optimized;
}

json ProfessionalConfigGenerator::optimizeForMemory(
    const json& base_config,
    double memory_limit_gb) {
    
    json optimized = base_config;
    
    if (memory_limit_gb < 8) {  // 严格内存限制
        optimized["model"]["hidden_dim"] = 256;
        optimized["model"]["num_layers"] = 4;
        optimized["performance"]["batch_size"] = 4;
        optimized["performance"]["gpu_memory_fraction"] = 0.3;
        optimized["quantum"]["enabled"] = false;
        
    } else if (memory_limit_gb < 16) {  // 中等内存限制
        optimized["model"]["hidden_dim"] = 512;
        optimized["model"]["num_layers"] = 8;
        optimized["performance"]["batch_size"] = 16;
        optimized["performance"]["gpu_memory_fraction"] = 0.5;
        
    } else if (memory_limit_gb < 64) {  // 宽松内存限制
        optimized["model"]["hidden_dim"] = 1024;
        optimized["model"]["num_layers"] = 12;
        optimized["performance"]["batch_size"] = 32;
        optimized["performance"]["gpu_memory_fraction"] = 0.7;
        
    } else {  // 充足内存
        optimized["model"]["hidden_dim"] = 1536;
        optimized["model"]["num_layers"] = 20;
        optimized["performance"]["batch_size"] = 64;
        optimized["performance"]["gpu_memory_fraction"] = 0.9;
        optimized["quantum"]["enabled"] = true;
    }
    
    return optimized;
}

json ProfessionalConfigGenerator::adaptConfigToMarketConditions(
    const json& base_config,
    const json& market_conditions) {
    
    json adapted = base_config;
    
    // 根据市场波动率调整
    double volatility = market_conditions.value("volatility", 0.2);
    if (volatility > 0.4) {  // 高波动
        adapted["model"]["dropout_rate"] = 0.2;
        adapted["risk_management"]["position_limit"] = 
            adapted["risk_management"]["position_limit"].get<double>() * 0.7;
        adapted["monitoring"]["alert_threshold"] = 0.7;
    } else if (volatility < 0.1) {  // 低波动
        adapted["model"]["dropout_rate"] = 0.05;
        adapted["risk_management"]["position_limit"] = 
            adapted["risk_management"]["position_limit"].get<double>() * 1.3;
        adapted["monitoring"]["alert_threshold"] = 0.9;
    }
    
    // 根据流动性调整
    double liquidity = market_conditions.value("liquidity", 1.0);
    if (liquidity < 0.5) {  // 低流动性
        adapted["execution"]["max_order_size"] = 
            adapted["execution"].value("max_order_size", 1000.0) * 0.5;
        adapted["risk_management"]["liquidity_buffer"] = 0.2;
    }
    
    // 根据市场情绪调整
    std::string sentiment = market_conditions.value("sentiment", "neutral");
    if (sentiment == "fearful") {
        adapted["model"]["conservative_mode"] = true;
        adapted["risk_management"]["risk_multiplier"] = 0.8;
    } else if (sentiment == "greedy") {
        adapted["model"]["aggressive_mode"] = true;
        adapted["risk_management"]["risk_multiplier"] = 1.2;
    }
    
    return adapted;
}

json ProfessionalConfigGenerator::generateRiskManagementConfig(
    TradingScenario scenario,
    double risk_tolerance,
    const std::vector<std::string>& risk_factors) {
    
    json risk_config;
    
    // 基础风险参数
    risk_config["risk_tolerance"] = risk_tolerance;
    risk_config["max_portfolio_var"] = risk_tolerance * 0.02;
    risk_config["max_individual_position"] = risk_tolerance * 0.1;
    
    // 场景特定风险控制
    switch (scenario) {
        case TradingScenario::ULTRA_HFT_ARBITRAGE:
            risk_config["max_holding_time_ms"] = 1000;
            risk_config["inventory_limit"] = 1000000;
            risk_config["adverse_selection_protection"] = true;
            break;
            
        case TradingScenario::CRYPTOCURRENCY_TRADING:
            risk_config["max_drawdown_limit"] = 0.15;
            risk_config["volatility_scaling"] = true;
            risk_config["correlation_monitoring"] = true;
            break;
            
        case TradingScenario::RISK_PARITY_FUND:
            risk_config["equal_risk_contribution"] = true;
            risk_config["rebalancing_frequency"] = "daily";
            risk_config["risk_budget_allocation"] = {
                {"equity", 0.25},
                {"fixed_income", 0.25},
                {"commodities", 0.25},
                {"alternatives", 0.25}
            };
            break;
    }
    
    // 风险因子特定控制
    for (const auto& factor : risk_factors) {
        if (factor == "market_risk") {
            risk_config["market_risk"]["beta_limit"] = 1.5;
            risk_config["market_risk"]["sector_concentration"] = 0.1;
        } else if (factor == "credit_risk") {
            risk_config["credit_risk"]["rating_limit"] = "BBB-";
            risk_config["credit_risk"]["concentration_limit"] = 0.05;
        } else if (factor == "liquidity_risk") {
            risk_config["liquidity_risk"]["min_daily_volume"] = 1000000;
            risk_config["liquidity_risk"]["bid_ask_spread_limit"] = 0.002;
        }
    }
    
    return risk_config;
}

json ProfessionalConfigGenerator::generateComplianceConfig(
    const std::string& jurisdiction,
    const std::vector<std::string>& regulations) {
    
    json compliance_config;
    
    compliance_config["jurisdiction"] = jurisdiction;
    compliance_config["regulations"] = regulations;
    
    // 地区特定合规要求
    if (jurisdiction == "US") {
        compliance_config["sec_regulations"] = {
            {"reg_sho", true},
            {"pattern_day_trader", true},
            {"market_maker_exemption", false}
        };
        compliance_config["finra_rules"] = {
            {"rule_3110", true},  // 监督要求
            {"rule_4511", true}   // 记录保存
        };
    } else if (jurisdiction == "EU") {
        compliance_config["mifid_ii"] = {
            {"best_execution", true},
            {"algorithmic_trading_controls", true},
            {"market_making_obligations", true}
        };
    } else if (jurisdiction == "APAC") {
        compliance_config["local_regulations"] = {
            {"position_limits", true},
            {"foreign_ownership_limits", true}
        };
    }
    
    // 监管报告配置
    compliance_config["reporting"] = {
        {"trade_reporting", true},
        {"position_reporting", true},
        {"risk_reporting", true},
        {"frequency", "daily"}
    };
    
    return compliance_config;
}

std::vector<json> ProfessionalConfigGenerator::generateABTestConfigs(
    const json& base_config,
    const std::vector<std::string>& test_parameters) {
    
    std::vector<json> test_configs;
    
    for (const auto& param : test_parameters) {
        json test_config = base_config;
        
        if (param == "learning_rate") {
            // 测试不同学习率
            std::vector<double> learning_rates = {0.0001, 0.001, 0.01};
            for (double lr : learning_rates) {
                test_config["model"]["learning_rate"] = lr;
                test_config["test_id"] = "lr_" + std::to_string(lr);
                test_configs.push_back(test_config);
            }
        } else if (param == "batch_size") {
            // 测试不同批次大小
            std::vector<int> batch_sizes = {8, 16, 32, 64};
            for (int bs : batch_sizes) {
                test_config["performance"]["batch_size"] = bs;
                test_config["test_id"] = "bs_" + std::to_string(bs);
                test_configs.push_back(test_config);
            }
        } else if (param == "model_architecture") {
            // 测试不同架构
            std::vector<std::string> architectures = {
                "MULTI_MODAL_TRANSFORMER",
                "HYBRID_TRANSFORMER_CNN",
                "QUANTUM_ENHANCED_LSTM"
            };
            for (const auto& arch : architectures) {
                test_config["model"]["architecture"] = arch;
                test_config["test_id"] = "arch_" + arch;
                test_configs.push_back(test_config);
            }
        }
    }
    
    return test_configs;
}

json ProfessionalConfigGenerator::optimizeForHardware(
    const json& config,
    const HardwareProfile& hardware) {
    
    json optimized = config;
    
    // 根据CPU核心数调整并行度
    int optimal_workers = std::min(hardware.cpu_cores, 64);
    optimized["performance"]["parallel_workers"] = optimal_workers;
    
    // 根据内存大小调整批次和模型大小
    if (hardware.memory_gb >= 128) {
        optimized["performance"]["batch_size"] = std::max(
            optimized["performance"]["batch_size"].get<int>(), 32
        );
    } else if (hardware.memory_gb < 32) {
        optimized["performance"]["batch_size"] = std::min(
            optimized["performance"]["batch_size"].get<int>(), 8
        );
    }
    
    // 根据GPU配置调整
    if (hardware.gpu_count > 0) {
        optimized["performance"]["gpu_acceleration"] = true;
        optimized["performance"]["gpu_memory_fraction"] = 
            std::min(0.9, hardware.memory_gb / (hardware.gpu_count * 12.0));
        
        if (hardware.gpu_count > 1) {
            optimized["performance"]["distributed_inference"] = true;
        }
        
        if (hardware.gpu_model.find("A100") != std::string::npos ||
            hardware.gpu_model.find("4090") != std::string::npos) {
            optimized["performance"]["tensor_rt_optimization"] = true;
            optimized["performance"]["mixed_precision"] = true;
        }
    } else {
        optimized["performance"]["gpu_acceleration"] = false;
    }
    
    // 根据网络延迟调整
    if (hardware.network_latency_us < 1.0) {
        optimized["network"]["high_frequency_optimized"] = true;
    }
    
    // 根据存储性能调整
    if (hardware.storage_iops > 500000) {
        optimized["data"]["enable_fast_loading"] = true;
        optimized["data"]["cache_size_gb"] = std::min(16.0, hardware.memory_gb * 0.2);
    }
    
    return optimized;
}

json ProfessionalConfigGenerator::mergeConfigs(const json& base, const json& override) {
    json result = base;
    
    for (auto& [key, value] : override.items()) {
        if (result.contains(key) && result[key].is_object() && value.is_object()) {
            result[key] = mergeConfigs(result[key], value);
        } else {
            result[key] = value;
        }
    }
    
    return result;
}

void ProfessionalConfigGenerator::validateConfig(const json& config, TradingScenario scenario) {
    // 验证必需字段
    if (!config.contains("target_latency_ns")) {
        throw std::runtime_error("Missing target_latency_ns in configuration");
    }
    
    if (!config.contains("target_accuracy")) {
        throw std::runtime_error("Missing target_accuracy in configuration");
    }
    
    // 验证参数合理性
    double accuracy = config["target_accuracy"].get<double>();
    if (accuracy < 0.5 || accuracy > 1.0) {
        throw std::runtime_error("Invalid target_accuracy: must be between 0.5 and 1.0");
    }
    
    int64_t latency = config["target_latency_ns"].get<int64_t>();
    if (latency < 1000 || latency > 1000000000) {  // 1微秒到1秒
        throw std::runtime_error("Invalid target_latency_ns: must be between 1000 and 1000000000");
    }
    
    // 场景特定验证
    if (scenario == TradingScenario::ULTRA_HFT_ARBITRAGE && latency > 100000) {
        Logger::warn("High latency target for ultra-HFT scenario: {} ns", latency);
    }
    
    if (scenario == TradingScenario::QUANTITATIVE_HEDGE_FUND && accuracy < 0.9) {
        Logger::warn("Low accuracy target for quantitative hedge fund: {}", accuracy);
    }
}

} // namespace config
} // namespace ai
} // namespace hft
