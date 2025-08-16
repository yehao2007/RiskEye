#include "core/System.h"
#include "core/Configuration.h"
#include "core/Logger.h"
#include "gui/MainWindow.h"
#include "cli/CommandLineInterface.h"
#include "ai/IntelligentDecisionSystem.h"
#include "hardware/PhotonicEngine.h"
#include "strategy/VectorizedStrategyEngine.h"
#include "risk/AdvancedRiskManager.h"
#include "utils/LowLatencyLogger.h"
#include "market_data/MarketDataFeed.h"
#include <QApplication>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include "hardware/PhotonicEngine.h"
#include "strategy/VectorizedStrategyEngine.h"
#include "risk/AdvancedRiskManager.h"
#include "utils/LowLatencyLogger.h"
#include "market_data/MarketDataFeed.h"

void printUsage() {
    std::cout << "Usage: hft_system [--gui|--cli] [--config <config_file>]" << std::endl;
}

int main(int argc, char** argv) {
    // 解析命令行参数
    bool useGui = true;  // 默认使用GUI
    std::string configFile = "config/config.json";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--gui") {
            useGui = true;
        } else if (arg == "--cli") {
            useGui = false;
        } else if (arg == "--config" && i + 1 < argc) {
            configFile = argv[++i];
        } else if (arg == "--help") {
            printUsage();
            return 0;
        }
    }
    
    // 初始化配置
    hft::core::Configuration config;
    if (!config.loadFromFile(configFile)) {
        std::cerr << "Failed to load configuration from file: " << configFile << std::endl;
        std::cerr << "Using default configuration..." << std::endl;
    }
    
    // 初始化系统
    hft::core::System system;
    if (!system.initialize(config)) {
        std::cerr << "System initialization failed." << std::endl;
        return 1;
    }
    
    if (useGui) {
        // GUI模式
        QApplication app(argc, argv);
        
        hft::gui::MainWindow mainWindow;
        mainWindow.setSystem(&system);
        mainWindow.show();
        
        // 启动系统
        system.start();
        
        return app.exec();
    } else {
        // 命令行模式
        hft::cli::CommandLineInterface cli(&system);
        
        // 启动系统
        system.start();
        
        // 运行命令行界面
        cli.run();
        
        // 优雅关闭
        system.gracefulShutdown();
        return 0;
    }
}


#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include "hardware/PhotonicEngine.h"
#include "strategy/VectorizedStrategyEngine.h"
#include "risk/AdvancedRiskManager.h"
#include "utils/LowLatencyLogger.h"
#include "market_data/MarketDataFeed.h"

// 市场数据结构体
struct MarketData {
    std::vector<float> bids;
    std::vector<float> asks;
    uint64_t timestamp;
};

// 市场数据生成器（模拟）
class MarketDataGenerator {
public:
    MarketData generateMarketData() {
        MarketData data;
        data.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        // 生成模拟买卖盘数据
        data.bids.resize(16); // 16个买单价格
        data.asks.resize(16); // 16个卖单价格

        for (int i = 0; i < 16; i++) {
            data.bids[i] = 100.0f - i * 0.1f + (rand() % 100) / 1000.0f;
            data.asks[i] = 100.0f + i * 0.1f + (rand() % 100) / 1000.0f;
        }

        return data;
    }
};

// 系统监控类
class SystemMonitor {
private:
    LowLatencyLogger logger;
    std::atomic<bool> running;
    std::thread monitorThread;
    PhotonicEngine& photonicEngine;
    VectorizedStrategyEngine& strategyEngine;
    AdvancedRiskManager& riskManager;

    void monitorLoop() {
        while (running) {
            // 记录系统状态
            uint64_t photonLatency = photonicEngine.getLastSendLatency();
            uint64_t totalOrders = photonicEngine.getTotalSendCount();

            logger.log(LogLevel::INFO, "System status - Photon latency: " + std::to_string(photonLatency) + " ns, "
                      "Total orders: " + std::to_string(totalOrders));

            // 检查风险状态
            if (riskManager.hasRiskAlert()) {
                logger.log(LogLevel::WARNING, "Risk alert detected!");
                auto alerts = riskManager.getRiskAlerts();
                for (const auto& alert : alerts) {
                    logger.log(LogLevel::WARNING, "Alert: " + alert);
                }
            }

            // 每秒监控一次
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

public:
    SystemMonitor(PhotonicEngine& engine, VectorizedStrategyEngine& strategy, AdvancedRiskManager& risk)
        : logger("system_monitor.log", LogLevel::INFO), running(false),
          photonicEngine(engine), strategyEngine(strategy), riskManager(risk) {}

    void start() {
        running = true;
        monitorThread = std::thread(&SystemMonitor::monitorLoop, this);
        logger.log(LogLevel::INFO, "System monitor started");
    }

    void stop() {
        running = false;
        if (monitorThread.joinable()) {
            monitorThread.join();
        }
        logger.log(LogLevel::INFO, "System monitor stopped");
    }
};

int main() {
    try {
        // 初始化随机数种子
        srand(static_cast<unsigned int>(time(nullptr)));

        // 初始化光子引擎
        PhotonicEngine photonicEngine;
        if (!photonicEngine.initialize()) {
            std::cerr << "Failed to initialize photonic engine" << std::endl;
            return 1;
        }

        // 初始化向量化策略引擎
        VectorizedStrategyEngine strategyEngine(photonicEngine, 1.5f);

        // 初始化高级风险管理器
        AdvancedRiskManager riskManager;
        riskManager.setRiskLimits(100000, 0.05, 1000); // 设置风险限额

        // 初始化智能决策系统 (阶段二)
        ai::IntelligentDecisionSystem decision_system;
        if (!decision_system.initialize("config/neuromorphic_weights.npy")) {
            std::cerr << "Failed to initialize intelligent decision system" << std::endl;
            return 1;
        }

        // 初始化阶段三模块
        // 1. 进化引擎
        evolution::EvolutionEngine evolution_engine(&system, &decision_system, &risk_manager);
        if (!evolution_engine.initialize("config/evolution_config.json")) {
            std::cerr << "Failed to initialize evolution engine" << std::endl;
            return 1;
        }

        // 2. 多模态数据融合
        market::MultimodalDataFusion data_fusion;
        if (!data_fusion.initialize("config/fusion_model.json")) {
            std::cerr << "Failed to initialize multimodal data fusion" << std::endl;
            return 1;
        }

        // 3. 容错管理器
        core::FaultToleranceManager fault_tolerance(&system, &alert_manager, &communicator);
        if (!fault_tolerance.initialize()) {
            std::cerr << "Failed to initialize fault tolerance manager" << std::endl;
            return 1;
        }

        // 启动阶段三模块
        evolution_engine.startLearning();
        fault_tolerance.startFaultDetection();

        // 初始化系统监控
        SystemMonitor systemMonitor(photonicEngine, strategyEngine, riskManager);
        systemMonitor.start();

        // 初始化市场数据生成器
        MarketDataGenerator dataGenerator;

        std::cout << "HFT system initialized successfully" << std::endl;
        std::cout << "Target latency: < 300ns" << std::endl;

        // 主交易循环
        const int RUN_DURATION_SECONDS = 60; // 运行60秒
        auto start_time = std::chrono::steady_clock::now();

        while (std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::steady_clock::now() - start_time).count() < RUN_DURATION_SECONDS) {
            // 生成市场数据
            MarketData market_data = dataGenerator.generateMarketData();

            // 使用智能决策系统生成订单 (阶段二新增)
            auto orders = decisionSystem.generateDecisions(market_data);

            for (const auto& order : orders) {
                // 验证订单
                if (decisionSystem.validateOrder(order, market_data)) {
                    // 发送订单
                    uint64_t send_latency = photonicEngine.sendOrder(order);
                    std::cout << "Order sent with latency: " << send_latency << " ns" << std::endl;
                } else {
                    std::cout << "Order rejected by quantum risk validator" << std::endl;
                }
            }

            // 短暂休眠以模拟实际交易节奏
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }

        // 停止系统监控
        systemMonitor.stop();

        std::cout << "HFT system run completed" << std::endl;
        std::cout << "Total orders sent: " << photonicEngine.getTotalSendCount() << std::endl;
        std::cout << "Average send latency: " << photonicEngine.getAverageSendLatency() << " ns" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
#include <iostream>

int main() {
    // 初始化订单引擎
    hft::execution::LowLatencyOrderEngine orderEngine;
    if (!orderEngine.initialize("config/order_engine.json")) {
        std::cerr << "Failed to initialize order engine" << std::endl;
        return 1;
    }

    // 初始化风险管理器
    hft::risk::AdvancedRiskManager riskManager;
    riskManager.setRiskLimits(1000000, 50000, 20000); // 设置风险限额
    riskManager.setMaxOrderRatePerSecond(1000); // 设置最大订单速率

    // 初始化市场数据 feed
    hft::market_data::MarketDataFeed marketDataFeed;
    marketDataFeed.subscribe("ESZ5"); // 订阅合约

    // 设置市场数据更新回调
    marketDataFeed.setOnDataCallback([&](const hft::market_data::MarketData& data) {
        // 更新订单引擎的市场数据
        orderEngine.onMarketDataUpdate(data);

        // 在实际系统中，这里会触发交易策略
        // 例如: generateOrderFromStrategy(data);
    });

    // 启动市场数据接收
    if (!marketDataFeed.start()) {
        std::cerr << "Failed to start market data feed" << std::endl;
        return 1;
    }

    std::cout << "HFT system started successfully" << std::endl;

    // 主循环
    while (true) {
        // 定期输出系统状态
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // 获取并打印延迟统计
        auto latencyStats = riskManager.getLatencyStats();
        for (const auto& [op, stats] : latencyStats) {
            std::cout << "Operation: " << op << ", Avg Latency: " << stats.first << "us, Max Latency: " << stats.second << "us" << std::endl;
        }
    }

    return 0;
}
#include "evolution/EvolutionEngine.h"
#include "market/MultimodalDataFusion.h"
#include "core/FaultToleranceManager.h"