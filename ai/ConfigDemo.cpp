/**
 * 🎯 专业配置演示 - 展示不同交易场景的定制配置
 * 
 * 本文件演示如何为不同的交易场景生成和优化配置：
 * - 超高频套利配置
 * - 量化对冲基金配置  
 * - 加密货币交易配置
 * - 跨市场套利配置
 * - 风险平价基金配置
 * - 市场做市配置
 * - 动态配置优化
 */

#include "ProfessionalConfigGenerator.h"
#include "GlobalRiskPredictionModel.h"
#include "RiskPredictionFactory.h"
#include "../core/Logger.h"
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace hft::ai;
using namespace hft::ai::config;

/**
 * 🏎️ 超高频套利配置演示
 */
void demonstrateUltraHFTConfig() {
    std::cout << "\n=== 🏎️ 超高频套利配置演示 ===\n" << std::endl;
    
    try {
        ProfessionalConfigGenerator generator;
        
        // 定义硬件配置
        HardwareProfile hft_hardware = {
            .name = "UltraHFT_Rig",
            .cpu_cores = 64,
            .memory_gb = 128.0,
            .gpu_count = 4,
            .gpu_model = "RTX 4090",
            .has_quantum_processor = true,
            .network_latency_us = 0.05,  // 50纳秒网络延迟
            .storage_iops = 2000000
        };
        
        // 生成配置
        auto config = generator.generateConfig(
            TradingScenario::ULTRA_HFT_ARBITRAGE,
            hft_hardware
        );
        
        std::cout << "✅ 超高频配置生成成功" << std::endl;
        std::cout << "🎯 目标延迟: " << config["target_latency_ns"].get<int64_t>() / 1000.0 << " 微秒" << std::endl;
        std::cout << "🎯 目标准确率: " << config["target_accuracy"].get<double>() * 100 << "%" << std::endl;
        std::cout << "🧠 模型架构: " << config["model"]["architecture"].get<std::string>() << std::endl;
        std::cout << "⚡ 批次大小: " << config["performance"]["batch_size"].get<int>() << std::endl;
        
        // 延迟优化
        auto latency_optimized = generator.optimizeForLatency(config, 25.0);  // 25微秒
        std::cout << "\n🚀 延迟优化后:" << std::endl;
        std::cout << "   隐藏层维度: " << latency_optimized["model"]["hidden_dim"].get<int>() << std::endl;
        std::cout << "   网络层数: " << latency_optimized["model"]["num_layers"].get<int>() << std::endl;
        
        // 保存配置
        std::ofstream file("ultra_hft_config.json");
        file << std::setw(2) << latency_optimized << std::endl;
        std::cout << "💾 配置已保存到 ultra_hft_config.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 超高频配置演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🏦 量化对冲基金配置演示
 */
void demonstrateQuantHedgeFundConfig() {
    std::cout << "\n=== 🏦 量化对冲基金配置演示 ===\n" << std::endl;
    
    try {
        ProfessionalConfigGenerator generator;
        
        // 机构级硬件配置
        HardwareProfile institutional_hardware = {
            .name = "QuantFund_Cluster",
            .cpu_cores = 256,
            .memory_gb = 1024.0,
            .gpu_count = 16,
            .gpu_model = "A100 80GB",
            .has_quantum_processor = true,
            .network_latency_us = 1.0,
            .storage_iops = 5000000
        };
        
        // 生成基础配置
        auto config = generator.generateConfig(
            TradingScenario::QUANTITATIVE_HEDGE_FUND,
            institutional_hardware
        );
        
        // 精度优化
        auto accuracy_optimized = generator.optimizeForAccuracy(config, 0.98);
        
        std::cout << "✅ 量化对冲基金配置生成成功" << std::endl;
        std::cout << "🎯 目标准确率: " << accuracy_optimized["target_accuracy"].get<double>() * 100 << "%" << std::endl;
        std::cout << "🧠 集成模型数量: " << accuracy_optimized["model"]["ensemble_size"].get<int>() << std::endl;
        std::cout << "🔮 量子比特数: " << accuracy_optimized["quantum"]["num_qubits"].get<int>() << std::endl;
        
        // 风险管理配置
        auto risk_config = generator.generateRiskManagementConfig(
            TradingScenario::QUANTITATIVE_HEDGE_FUND,
            0.15,  // 风险容忍度
            {"market_risk", "credit_risk", "liquidity_risk"}
        );
        
        std::cout << "\n🛡️ 风险管理配置:" << std::endl;
        std::cout << "   最大组合VaR: " << risk_config["max_portfolio_var"].get<double>() * 100 << "%" << std::endl;
        std::cout << "   单笔持仓限制: " << risk_config["max_individual_position"].get<double>() * 100 << "%" << std::endl;
        
        // 保存完整配置
        json complete_config = accuracy_optimized;
        complete_config["risk_management"] = risk_config;
        
        std::ofstream file("quant_hedge_fund_config.json");
        file << std::setw(2) << complete_config << std::endl;
        std::cout << "💾 配置已保存到 quant_hedge_fund_config.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 量化对冲基金配置演示失败: " << e.what() << std::endl;
    }
}

/**
 * 💰 加密货币交易配置演示
 */
void demonstrateCryptocurrencyConfig() {
    std::cout << "\n=== 💰 加密货币交易配置演示 ===\n" << std::endl;
    
    try {
        ProfessionalConfigGenerator generator;
        
        // 加密货币交易硬件
        HardwareProfile crypto_hardware = {
            .name = "CryptoTrader_Setup",
            .cpu_cores = 32,
            .memory_gb = 64.0,
            .gpu_count = 2,
            .gpu_model = "RTX 4080",
            .has_quantum_processor = false,
            .network_latency_us = 5.0,
            .storage_iops = 500000
        };
        
        // 生成配置
        auto config = generator.generateConfig(
            TradingScenario::CRYPTOCURRENCY_TRADING,
            crypto_hardware
        );
        
        std::cout << "✅ 加密货币交易配置生成成功" << std::endl;
        std::cout << "🎯 模型更新频率: " << config["performance"]["model_refresh_hours"].get<int>() << " 小时" << std::endl;
        std::cout << "📊 波动率适应: " << (config["model"]["volatility_adaptation"].get<bool>() ? "启用" : "禁用") << std::endl;
        std::cout << "🐋 巨鲸监控: " << (config["features"]["whale_tracking"].get<bool>() ? "启用" : "禁用") << std::endl;
        
        // 模拟市场状况自适应
        json market_conditions = {
            {"volatility", 0.8},        // 高波动
            {"liquidity", 0.6},         // 中等流动性
            {"sentiment", "fearful"}    // 恐慌情绪
        };
        
        auto adapted_config = generator.adaptConfigToMarketConditions(config, market_conditions);
        
        std::cout << "\n📈 市场自适应调整:" << std::endl;
        std::cout << "   调整后Dropout: " << adapted_config["model"]["dropout_rate"].get<double>() << std::endl;
        std::cout << "   保守模式: " << (adapted_config["model"]["conservative_mode"].get<bool>() ? "启用" : "禁用") << std::endl;
        std::cout << "   风险倍数: " << adapted_config["risk_management"]["risk_multiplier"].get<double>() << std::endl;
        
        // 保存配置
        std::ofstream file("cryptocurrency_config.json");
        file << std::setw(2) << adapted_config << std::endl;
        std::cout << "💾 配置已保存到 cryptocurrency_config.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 加密货币配置演示失败: " << e.what() << std::endl;
    }
}

/**
 * ⚡ 市场做市配置演示
 */
void demonstrateMarketMakingConfig() {
    std::cout << "\n=== ⚡ 市场做市配置演示 ===\n" << std::endl;
    
    try {
        ProfessionalConfigGenerator generator;
        
        // 市场做市硬件
        HardwareProfile mm_hardware = {
            .name = "MarketMaker_System",
            .cpu_cores = 48,
            .memory_gb = 96.0,
            .gpu_count = 2,
            .gpu_model = "RTX 4090",
            .has_quantum_processor = false,
            .network_latency_us = 0.1,  // 极低延迟
            .storage_iops = 1500000
        };
        
        // 生成配置
        auto config = generator.generateConfig(
            TradingScenario::MARKET_MAKING,
            mm_hardware
        );
        
        std::cout << "✅ 市场做市配置生成成功" << std::endl;
        std::cout << "📊 最小价差: " << config["spread_management"]["min_spread_bps"].get<int>() << " 基点" << std::endl;
        std::cout << "📊 最大价差: " << config["spread_management"]["max_spread_bps"].get<int>() << " 基点" << std::endl;
        std::cout << "📦 最大持仓: " << config["inventory_management"]["max_position"].get<int>() << std::endl;
        std::cout << "🛡️ 逆向选择保护: " << (config["risk_management"]["adverse_selection_protection"].get<bool>() ? "启用" : "禁用") << std::endl;
        
        // 吞吐量优化
        auto throughput_optimized = generator.optimizeForThroughput(config, 25000);  // 25K TPS
        
        std::cout << "\n🚀 吞吐量优化后:" << std::endl;
        std::cout << "   批次大小: " << throughput_optimized["performance"]["batch_size"].get<int>() << std::endl;
        std::cout << "   并行工作者: " << throughput_optimized["performance"]["parallel_workers"].get<int>() << std::endl;
        std::cout << "   分布式推理: " << (throughput_optimized["performance"]["distributed_inference"].get<bool>() ? "启用" : "禁用") << std::endl;
        
        // 保存配置
        std::ofstream file("market_making_config.json");
        file << std::setw(2) << throughput_optimized << std::endl;
        std::cout << "💾 配置已保存到 market_making_config.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 市场做市配置演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🧪 A/B测试配置演示
 */
void demonstrateABTestingConfigs() {
    std::cout << "\n=== 🧪 A/B测试配置演示 ===\n" << std::endl;
    
    try {
        ProfessionalConfigGenerator generator;
        
        // 基础配置
        auto base_config = generator.getAlgorithmicTradingConfig();
        
        // 生成A/B测试配置
        std::vector<std::string> test_params = {"learning_rate", "batch_size", "model_architecture"};
        auto ab_configs = generator.generateABTestConfigs(base_config, test_params);
        
        std::cout << "✅ 生成了 " << ab_configs.size() << " 个A/B测试配置" << std::endl;
        
        // 展示部分配置
        for (size_t i = 0; i < std::min(size_t(5), ab_configs.size()); ++i) {
            const auto& config = ab_configs[i];
            std::cout << "\n📋 测试配置 " << (i + 1) << ":" << std::endl;
            std::cout << "   测试ID: " << config["test_id"].get<std::string>() << std::endl;
            
            if (config.contains("model") && config["model"].contains("learning_rate")) {
                std::cout << "   学习率: " << config["model"]["learning_rate"].get<double>() << std::endl;
            }
            if (config.contains("performance") && config["performance"].contains("batch_size")) {
                std::cout << "   批次大小: " << config["performance"]["batch_size"].get<int>() << std::endl;
            }
            if (config.contains("model") && config["model"].contains("architecture")) {
                std::cout << "   架构: " << config["model"]["architecture"].get<std::string>() << std::endl;
            }
        }
        
        // 保存A/B测试配置
        json ab_test_suite = {
            {"test_name", "algorithmic_trading_optimization"},
            {"total_configs", ab_configs.size()},
            {"configs", ab_configs}
        };
        
        std::ofstream file("ab_test_configs.json");
        file << std::setw(2) << ab_test_suite << std::endl;
        std::cout << "\n💾 A/B测试配置已保存到 ab_test_configs.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ A/B测试配置演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🔧 配置优化器演示
 */
void demonstrateConfigOptimizer() {
    std::cout << "\n=== 🔧 配置优化器演示 ===\n" << std::endl;
    
    try {
        // 模拟回测结果
        json backtest_results = {
            {"sharpe_ratio", 1.2},          // 低于预期
            {"max_drawdown", 0.08},         // 高于预期
            {"total_return", 0.15},
            {"volatility", 0.12},
            {"winning_rate", 0.58}
        };
        
        json optimization_targets = {
            {"target_sharpe", 2.0},
            {"max_acceptable_drawdown", 0.05}
        };
        
        // 基础配置
        auto base_config = QuantHedgeFundConfigBuilder::build();
        
        // 基于回测结果优化
        auto optimized_config = ConfigOptimizer::optimizeFromBacktest(
            base_config,
            backtest_results,
            optimization_targets
        );
        
        std::cout << "✅ 基于回测结果的配置优化完成" << std::endl;
        std::cout << "🧠 优化后隐藏层维度: " << optimized_config["model"]["hidden_dim"].get<int>() << std::endl;
        std::cout << "📊 优化后网络层数: " << optimized_config["model"]["num_layers"].get<int>() << std::endl;
        
        // 模拟性能指标
        json performance_metrics = {
            {"avg_latency_ns", 450000},     // 高于目标的300微秒
            {"accuracy", 0.94},
            {"throughput_tps", 1200}
        };
        
        // 基于性能自适应
        auto adapted_config = ConfigOptimizer::adaptToPerformance(
            optimized_config,
            performance_metrics
        );
        
        std::cout << "\n⚡ 基于性能的自适应调整:" << std::endl;
        std::cout << "   调整后隐藏层维度: " << adapted_config["model"]["hidden_dim"].get<int>() << std::endl;
        std::cout << "   调整后批次大小: " << adapted_config["performance"]["batch_size"].get<int>() << std::endl;
        
        // 市场状态自适应
        auto regime_adapted = ConfigOptimizer::adaptToMarketRegime(
            adapted_config,
            "high_volatility"
        );
        
        std::cout << "\n📈 高波动市场适应:" << std::endl;
        std::cout << "   Dropout率: " << regime_adapted["model"]["dropout_rate"].get<double>() << std::endl;
        std::cout << "   持仓限制倍数: " << 
            regime_adapted["risk_management"]["position_limit"].get<double>() / 
            adapted_config["risk_management"]["position_limit"].get<double>() << std::endl;
        
        // 保存优化后的配置
        std::ofstream file("optimized_config.json");
        file << std::setw(2) << regime_adapted << std::endl;
        std::cout << "\n💾 优化配置已保存到 optimized_config.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 配置优化器演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🌍 合规配置演示
 */
void demonstrateComplianceConfig() {
    std::cout << "\n=== 🌍 合规配置演示 ===\n" << std::endl;
    
    try {
        ProfessionalConfigGenerator generator;
        
        // 美国市场合规
        auto us_compliance = generator.generateComplianceConfig(
            "US",
            {"SEC Rule 15c3-5", "FINRA Rule 3110", "Reg SHO"}
        );
        
        std::cout << "🇺🇸 美国市场合规配置:" << std::endl;
        std::cout << "   SEC监管: " << (us_compliance["sec_regulations"]["reg_sho"].get<bool>() ? "启用" : "禁用") << std::endl;
        std::cout << "   FINRA规则3110: " << (us_compliance["finra_rules"]["rule_3110"].get<bool>() ? "启用" : "禁用") << std::endl;
        
        // 欧盟市场合规
        auto eu_compliance = generator.generateComplianceConfig(
            "EU",
            {"MiFID II", "EMIR", "CSDR"}
        );
        
        std::cout << "\n🇪🇺 欧盟市场合规配置:" << std::endl;
        std::cout << "   最佳执行: " << (eu_compliance["mifid_ii"]["best_execution"].get<bool>() ? "启用" : "禁用") << std::endl;
        std::cout << "   算法交易控制: " << (eu_compliance["mifid_ii"]["algorithmic_trading_controls"].get<bool>() ? "启用" : "禁用") << std::endl;
        
        // 保存合规配置
        json compliance_suite = {
            {"us_compliance", us_compliance},
            {"eu_compliance", eu_compliance}
        };
        
        std::ofstream file("compliance_config.json");
        file << std::setw(2) << compliance_suite << std::endl;
        std::cout << "\n💾 合规配置已保存到 compliance_config.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 合规配置演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🎯 主函数 - 运行所有配置演示
 */
int main() {
    std::cout << "🎯 专业配置生成器演示程序" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // 初始化日志系统
    Logger::initialize(Logger::Level::INFO);
    
    try {
        // 运行各种配置演示
        demonstrateUltraHFTConfig();
        demonstrateQuantHedgeFundConfig();
        demonstrateCryptocurrencyConfig();
        demonstrateMarketMakingConfig();
        demonstrateABTestingConfigs();
        demonstrateConfigOptimizer();
        demonstrateComplianceConfig();
        
        std::cout << "\n🎉 所有配置演示完成！" << std::endl;
        
        std::cout << "\n📋 生成的配置文件:" << std::endl;
        std::cout << "   📄 ultra_hft_config.json - 超高频套利配置" << std::endl;
        std::cout << "   📄 quant_hedge_fund_config.json - 量化对冲基金配置" << std::endl;
        std::cout << "   📄 cryptocurrency_config.json - 加密货币交易配置" << std::endl;
        std::cout << "   📄 market_making_config.json - 市场做市配置" << std::endl;
        std::cout << "   📄 ab_test_configs.json - A/B测试配置套件" << std::endl;
        std::cout << "   📄 optimized_config.json - 动态优化配置" << std::endl;
        std::cout << "   📄 compliance_config.json - 合规配置" << std::endl;
        
        std::cout << "\n💡 使用建议:" << std::endl;
        std::cout << "   1. 根据实际硬件配置调整参数" << std::endl;
        std::cout << "   2. 定期基于回测结果优化配置" << std::endl;
        std::cout << "   3. 根据市场状况动态调整配置" << std::endl;
        std::cout << "   4. 确保符合当地监管要求" << std::endl;
        std::cout << "   5. 通过A/B测试验证配置效果" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 配置演示过程中发生错误: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
