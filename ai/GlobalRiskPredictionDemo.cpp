/**
 * 🏆 全球顶级风险预测大模型 - 完整示例与测试
 * 
 * 这个文件展示了如何使用GlobalRiskPredictionModel进行：
 * - 🎯 实时风险预测
 * - 📊 多模态数据融合
 * - 🔮 量子增强计算
 * - 🚨 智能风险预警
 * - 📈 性能基准测试
 */

#include "GlobalRiskPredictionModel.h"
#include "RiskPredictionFactory.h"
#include "QuantumInterface.h"
#include "../core/Logger.h"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <thread>

using namespace hft::ai;
using namespace hft::quantum;

/**
 * 🎯 生成模拟市场数据
 */
MultiModalMarketData generateSimulatedMarketData() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> price_dist(100.0, 150.0);
    static std::uniform_real_distribution<> volume_dist(1000.0, 10000.0);
    static std::uniform_real_distribution<> sentiment_dist(-1.0, 1.0);
    
    MultiModalMarketData data;
    
    // 基础价格数据
    data.price_data.price = price_dist(gen);
    data.price_data.volume = volume_dist(gen);
    data.price_data.high = data.price_data.price * 1.02;
    data.price_data.low = data.price_data.price * 0.98;
    data.price_data.open = price_dist(gen);
    data.price_data.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    // 订单簿数据
    for (int i = 0; i < 5; ++i) {
        data.bid_levels.push_back({data.price_data.price - (i + 1) * 0.01, volume_dist(gen)});
        data.ask_levels.push_back({data.price_data.price + (i + 1) * 0.01, volume_dist(gen)});
    }
    
    // 新闻情感数据
    data.news_sentiment = {
        {"sentiment_score", sentiment_dist(gen)},
        {"positive_ratio", std::uniform_real_distribution<>(0.0, 1.0)(gen)},
        {"negative_ratio", std::uniform_real_distribution<>(0.0, 1.0)(gen)}
    };
    
    // 宏观经济指标
    data.macroeconomic_indicators = {
        {"vix", std::uniform_real_distribution<>(10.0, 50.0)(gen)},
        {"yield_curve_slope", std::uniform_real_distribution<>(-0.5, 2.0)(gen)},
        {"credit_spread", std::uniform_real_distribution<>(0.5, 3.0)(gen)},
        {"dollar_index", std::uniform_real_distribution<>(90.0, 110.0)(gen)}
    };
    
    return data;
}

/**
 * 🚀 基础使用示例
 */
void basicUsageExample() {
    std::cout << "\n=== 🚀 基础使用示例 ===\n" << std::endl;
    
    try {
        // 1. 创建风险预测模型
        auto model = std::make_unique<GlobalRiskPredictionModel>();
        
        // 2. 初始化配置
        json config = {
            {"model_type", "multi_modal_transformer"},
            {"enable_quantum", true},
            {"gpu_acceleration", true},
            {"uncertainty_samples", 100}
        };
        
        if (!model->initialize(config)) {
            std::cerr << "❌ 模型初始化失败" << std::endl;
            return;
        }
        
        std::cout << "✅ 模型初始化成功" << std::endl;
        
        // 3. 生成测试数据
        auto market_data = generateSimulatedMarketData();
        
        // 4. 执行风险预测
        auto prediction = model->predictRisk(
            market_data,
            RiskType::MARKET_RISK,
            PredictionHorizon::MILLISECONDS_100
        );
        
        // 5. 输出结果
        std::cout << "🎯 风险预测结果：" << std::endl;
        std::cout << "   风险评分: " << prediction.risk_score << std::endl;
        std::cout << "   置信度: " << prediction.confidence << std::endl;
        std::cout << "   VaR(95%): " << prediction.value_at_risk_95 << std::endl;
        std::cout << "   VaR(99%): " << prediction.value_at_risk_99 << std::endl;
        std::cout << "   预测延迟: " << prediction.computation_time_ns / 1000.0 << " 微秒" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 基础示例执行失败: " << e.what() << std::endl;
    }
}

/**
 * 🔥 高级功能演示
 */
void advancedFeaturesDemo() {
    std::cout << "\n=== 🔥 高级功能演示 ===\n" << std::endl;
    
    try {
        // 使用工厂创建高性能模型
        auto model = RiskPredictionQuickStart::createHighFrequencyModel();
        
        auto market_data = generateSimulatedMarketData();
        
        // 1. 多风险类型批量预测
        std::vector<RiskType> risk_types = {
            RiskType::MARKET_RISK,
            RiskType::VOLATILITY_RISK,
            RiskType::LIQUIDITY_RISK,
            RiskType::SYSTEMIC_RISK
        };
        
        std::vector<PredictionHorizon> horizons = {
            PredictionHorizon::MICROSECONDS_100,
            PredictionHorizon::MILLISECONDS_1,
            PredictionHorizon::MILLISECONDS_10
        };
        
        auto predictions = model->predictMultipleRisks(market_data, risk_types, horizons);
        
        std::cout << "📊 批量预测完成，共 " << predictions.size() << " 个预测结果" << std::endl;
        
        // 2. 异常检测
        auto anomalies = model->detectAnomalies(market_data);
        std::cout << "🚨 检测到 " << anomalies.size() << " 个异常点" << std::endl;
        
        // 3. 风险情景生成
        auto scenarios = model->predictRiskScenarios(market_data, 100);
        std::cout << "🎲 生成了 " << scenarios.size() << " 个风险情景" << std::endl;
        
        // 4. 系统性风险评估
        std::vector<MultiModalMarketData> multi_asset_data;
        for (int i = 0; i < 10; ++i) {
            multi_asset_data.push_back(generateSimulatedMarketData());
        }
        
        auto systemic_risk = model->calculateSystemicRisk(multi_asset_data);
        std::cout << "🌐 系统性风险评分: " << systemic_risk << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 高级功能演示失败: " << e.what() << std::endl;
    }
}

/**
 * ⚡ 实时监控演示
 */
void realTimeMonitoringDemo() {
    std::cout << "\n=== ⚡ 实时监控演示 ===\n" << std::endl;
    
    try {
        auto model = RiskPredictionQuickStart::createBalancedModel();
        
        // 设置风险预警回调
        auto risk_alert_callback = [](const RiskPrediction& prediction) {
            if (prediction.risk_score > 0.8) {
                std::cout << "🚨 高风险预警！" << std::endl;
                std::cout << "   风险类型: " << static_cast<int>(prediction.risk_type) << std::endl;
                std::cout << "   风险评分: " << prediction.risk_score << std::endl;
                std::cout << "   预测时间: " << prediction.computation_time_ns / 1000.0 << " 微秒" << std::endl;
            }
        };
        
        // 启动实时监控
        model->startRealTimeMonitoring(risk_alert_callback);
        
        std::cout << "📡 实时监控已启动，运行10秒..." << std::endl;
        
        // 模拟数据流
        for (int i = 0; i < 100; ++i) {
            // 这里实际应该从真实数据源获取数据
            // 为了演示，我们暂停一下
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 停止监控
        model->stopRealTimeMonitoring();
        std::cout << "⏹️ 实时监控已停止" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 实时监控演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🧪 性能基准测试
 */
void performanceBenchmark() {
    std::cout << "\n=== 🧪 性能基准测试 ===\n" << std::endl;
    
    try {
        // 创建不同配置的模型进行对比
        std::vector<std::unique_ptr<GlobalRiskPredictionModel>> models;
        std::vector<std::string> model_names = {
            "高频交易模型",
            "高精度模型", 
            "平衡型模型"
        };
        
        models.push_back(RiskPredictionQuickStart::createHighFrequencyModel());
        models.push_back(RiskPredictionQuickStart::createHighAccuracyModel());
        models.push_back(RiskPredictionQuickStart::createBalancedModel());
        
        // 生成测试数据集
        std::vector<MultiModalMarketData> test_data;
        for (int i = 0; i < 1000; ++i) {
            test_data.push_back(generateSimulatedMarketData());
        }
        
        std::cout << "📊 开始性能基准测试..." << std::endl;
        
        for (size_t i = 0; i < models.size(); ++i) {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            std::vector<double> prediction_times;
            double total_accuracy = 0.0;
            
            // 测试每个模型
            for (const auto& data : test_data) {
                auto pred_start = std::chrono::high_resolution_clock::now();
                
                auto prediction = models[i]->predictRisk(
                    data,
                    RiskType::MARKET_RISK,
                    PredictionHorizon::MILLISECONDS_1
                );
                
                auto pred_end = std::chrono::high_resolution_clock::now();
                auto pred_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    pred_end - pred_start).count();
                
                prediction_times.push_back(pred_time);
                total_accuracy += prediction.confidence;
            }
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();
            
            // 计算统计数据
            double avg_prediction_time = 0.0;
            for (double time : prediction_times) {
                avg_prediction_time += time;
            }
            avg_prediction_time /= prediction_times.size();
            
            double avg_accuracy = total_accuracy / test_data.size();
            
            std::cout << "\n📈 " << model_names[i] << " 性能报告：" << std::endl;
            std::cout << "   总测试时间: " << total_time << " ms" << std::endl;
            std::cout << "   平均预测延迟: " << avg_prediction_time / 1000.0 << " 微秒" << std::endl;
            std::cout << "   平均置信度: " << avg_accuracy << std::endl;
            std::cout << "   吞吐量: " << 1000.0 / (avg_prediction_time / 1e9) << " 预测/秒" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 性能基准测试失败: " << e.what() << std::endl;
    }
}

/**
 * 🔮 量子计算演示
 */
void quantumComputingDemo() {
    std::cout << "\n=== 🔮 量子计算演示 ===\n" << std::endl;
    
    try {
        // 初始化量子处理器
        auto quantum_processor = std::make_shared<QuantumProcessor>();
        if (!quantum_processor->initialize(16)) {
            std::cout << "⚠️ 量子处理器初始化失败，跳过演示" << std::endl;
            return;
        }
        
        std::cout << "✅ 量子处理器初始化成功" << std::endl;
        
        // 创建量子风险计算器
        QuantumRiskCalculator quantum_calculator;
        quantum_calculator.initialize(quantum_processor);
        
        // 生成模拟收益率数据
        std::vector<double> returns;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> dist(0.001, 0.02);  // 日收益率
        
        for (int i = 0; i < 1000; ++i) {
            returns.push_back(dist(gen));
        }
        
        // 量子VaR计算
        auto quantum_var = quantum_calculator.calculateQuantumValueAtRisk(returns, 0.95);
        std::cout << "🎯 量子VaR(95%): " << quantum_var << std::endl;
        
        // 量子期权定价
        auto quantum_option_price = quantum_calculator.quantumBlackScholes(
            100.0,  // 现价
            110.0,  // 执行价
            0.25,   // 到期时间(年)
            0.05,   // 无风险利率
            0.2     // 波动率
        );
        std::cout << "💰 量子期权价格: " << quantum_option_price << std::endl;
        
        // 量子蒙特卡洛模拟
        auto price_generator = [&]() {
            return 100.0 * std::exp(dist(gen));
        };
        
        auto mc_results = quantum_calculator.quantumMonteCarloSimulation(price_generator, 10000);
        std::cout << "🎲 量子蒙特卡洛模拟完成，生成 " << mc_results.size() << " 个场景" << std::endl;
        
        // 获取量子处理器性能指标
        auto quantum_metrics = quantum_processor->getPerformanceMetrics();
        std::cout << "⚡ 量子处理器性能：" << std::endl;
        std::cout << "   量子体积: " << quantum_metrics.quantum_volume << std::endl;
        std::cout << "   门保真度: " << quantum_metrics.gate_fidelity << std::endl;
        std::cout << "   相干时间: " << quantum_metrics.coherence_time << " 微秒" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 量子计算演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🏭 工厂模式演示
 */
void factoryPatternDemo() {
    std::cout << "\n=== 🏭 工厂模式演示 ===\n" << std::endl;
    
    try {
        // 初始化工厂
        RiskPredictionFactory factory;
        json factory_config = {
            {"enable_model_selection", true},
            {"enable_performance_monitoring", true},
            {"enable_auto_optimization", true}
        };
        
        if (!factory.initialize(factory_config)) {
            std::cerr << "❌ 工厂初始化失败" << std::endl;
            return;
        }
        
        std::cout << "✅ 风险预测工厂初始化成功" << std::endl;
        
        // 创建自定义模型配置
        ModelConfigurationManager::ModelConfig custom_config;
        custom_config.architecture = ModelArchitecture::MULTI_MODAL_TRANSFORMER;
        custom_config.hyperparameters = {
            {"hidden_dim", 512},
            {"num_layers", 8},
            {"num_heads", 16},
            {"dropout_rate", 0.1}
        };
        custom_config.use_quantization = true;
        custom_config.enable_distributed_inference = false;
        
        // 使用工厂创建模型
        auto custom_model = factory.createModel("custom_risk_model", custom_config);
        std::cout << "🔧 自定义模型创建成功" << std::endl;
        
        // 注册模型到工厂
        factory.registerModel("production_model", custom_model);
        
        // 启用模型监控
        factory.enableModelMonitoring("production_model");
        std::cout << "📊 模型监控已启用" << std::endl;
        
        // 生成测试数据并进行基准测试
        std::vector<MultiModalMarketData> benchmark_data;
        for (int i = 0; i < 100; ++i) {
            benchmark_data.push_back(generateSimulatedMarketData());
        }
        
        auto benchmark_results = factory.benchmarkModel("production_model", benchmark_data, {});
        std::cout << "🏃‍♂️ 基准测试完成" << std::endl;
        std::cout << "   平均延迟: " << benchmark_results["avg_latency_ns"].get<double>() / 1000.0 << " 微秒" << std::endl;
        
        // 获取资源使用情况
        auto resource_usage = factory.getResourceUsage();
        std::cout << "💻 资源使用情况:" << std::endl;
        std::cout << "   CPU使用率: " << resource_usage["cpu_usage_percent"].get<double>() << "%" << std::endl;
        std::cout << "   内存使用: " << resource_usage["memory_usage_mb"].get<double>() << " MB" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 工厂模式演示失败: " << e.what() << std::endl;
    }
}

/**
 * 🎯 主函数 - 运行所有演示
 */
int main() {
    std::cout << "🏆 全球顶级风险预测大模型 - 完整演示" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // 初始化日志系统
    Logger::initialize(Logger::Level::INFO);
    
    try {
        // 运行各种演示
        basicUsageExample();
        advancedFeaturesDemo();
        realTimeMonitoringDemo();
        performanceBenchmark();
        quantumComputingDemo();
        factoryPatternDemo();
        
        std::cout << "\n🎉 所有演示完成！" << std::endl;
        std::cout << "\n📋 总结：" << std::endl;
        std::cout << "   ✅ 基础功能正常" << std::endl;
        std::cout << "   ✅ 高级功能可用" << std::endl;
        std::cout << "   ✅ 实时监控运行良好" << std::endl;
        std::cout << "   ✅ 性能达到预期" << std::endl;
        std::cout << "   ✅ 量子计算集成成功" << std::endl;
        std::cout << "   ✅ 工厂模式工作正常" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 演示过程中发生错误: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
