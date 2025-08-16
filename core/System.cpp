#include "System.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "core/Logger.h"
#include "core/EventLoop.h"
#include "risk/RiskLimits.h"
#include "execution/OrderExecutor.h"
#include "market/OrderBook.h"
#include "market/MarketData.h"
#include "utils/PerformanceMonitor.h"
#include "persistence/DataStore.h"
#include "backtest/BacktestEngine.h"
#include "utils/ExceptionHandler.h"
#include "network/NetworkManager.h"
#include "synchronization/ClockSynchronizer.h"
#include "hardware/AsicDriver.h"
#include "ai/TensorFlowModel.h"
#include "ai/PyTorchModel.h"
#include "ai/OnnxModel.h"
#include "market/AdvancedLiquidityEvaluator.h"
#include "execution/AdvancedOrderValidator.h"
#include "risk/AdvancedRiskManager.h"
#include "strategy/TrendFollowingStrategy.h"
#include "strategy/AdaptiveStrategy.h"
#include "strategy/MarketMakingStrategy.h"
#include "strategy/StatisticalArbitrageStrategy.h"

namespace hft {
namespace core {

System::System()
    : m_initialized(false), m_status(SystemStatus::NOT_INITIALIZED) {
    // 初始化异常处理器
    utils::ExceptionHandler::initialize();
}

System::~System() {
    shutdown();
}

bool System::initialize() {
    m_status = SystemStatus::INITIALIZING;
    try {
        // 初始化配置
        Configuration config;
        if (!config.loadFromFile("config/config.json")) {
            std::cerr << "Failed to load configuration from default file: config/config.json" << std::endl;
            std::cerr << "Using default configuration..." << std::endl;
        }
        return initialize(config);
    } catch (const std::exception& e) {
        std::cerr << "Exception during initialization: " << e.what() << std::endl;
        m_status = SystemStatus::NOT_INITIALIZED;
        return false;
    }
}

bool System::initialize(const Configuration& config) {
    m_status = SystemStatus::INITIALIZING;
    try {
        // 保存配置
        m_configuration = config;

        // 初始化日志器
        m_logger = std::make_shared<Logger>();
        std::string logLevel = m_configuration.getString("system.log_level", "INFO");
        if (!m_logger->initialize(logLevel)) {
            std::cerr << "Failed to initialize Logger" << std::endl;
            return false;
        }

        // 设置性能监控
        m_performance_monitor = std::make_shared<utils::PerformanceMonitor>();
        m_performance_monitor->setLatencyThreshold(std::chrono::microseconds(100));
        m_performance_monitor->setMetricsCollectionInterval(
            std::chrono::milliseconds(m_configuration.getInt("system.metrics_interval_ms", 1000))
        );

        // 初始化事件循环
        m_event_loop = std::make_shared<EventLoop>();
        if (!m_event_loop->initialize()) {
            LOG_ERROR("Failed to initialize EventLoop");
            return false;
        }

        // 初始化其他组件
        if (!initializeComponents()) {
            LOG_ERROR("Failed to initialize components");
            return false;
        }

        m_initialized = true;
        m_status = SystemStatus::NOT_INITIALIZED;
        LOG_INFO("HFT System initialized successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during initialization: {}", e.what());
        m_status = SystemStatus::NOT_INITIALIZED;
        return false;
    }
}

bool System::start() {
    if (!m_initialized) {
        LOG_ERROR("System not initialized");
        return false;
    }

    LOG_INFO("Starting HFT System...");

    try {
        // 启动事件循环
        if (!m_event_loop->start()) {
            LOG_ERROR("Failed to start EventLoop");
            return false;
        }

        // 启动性能监控
        m_performance_monitor->start();

        LOG_INFO("HFT System started successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during startup: {}", e.what());
        return false;
    }
}

void System::shutdown() {
    if (!m_initialized) {
        return;
    }

    LOG_INFO("Shutting down HFT System...");

    try {
        // 停止性能监控并生成报告
        if (m_performance_monitor) {
            m_performance_monitor->stop();
            std::string report = m_performance_monitor->generateReport();
            LOG_INFO("Performance Report:\n{}", report);
        }

        // 停止事件循环
        if (m_event_loop) {
            m_event_loop->stop();
            m_event_loop.reset();
        }

        // 重置其他组件
        m_logger.reset();
        m_config_manager.reset();
        m_asic_driver.reset();
        m_feature_extractor.reset();
        m_market_env_analyzer.reset();
        m_time_manager.reset();
        m_liquidity_evaluator.reset();
        m_order_validator.reset();
        m_model.reset();
        m_strategy_factory.reset();
        m_risk_manager.reset();
        m_data_store.reset();
        m_backtest_engine.reset();

        m_initialized = false;
        std::cout << "HFT System shutdown successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception during shutdown: " << e.what() << std::endl;
    }
}

bool System::initializeComponents() {
    try {
        // 初始化时间管理器
        m_time_manager = std::make_shared<TimeManager>();
        if (!m_time_manager->initialize()) {
            LOG_ERROR("Failed to initialize TimeManager");
            return false;
        }

        // 初始化网络管理器
        auto network_manager = std::make_shared<network::NetworkManager>();
        if (!network_manager->initialize(m_config_manager->getNetworkConfig())) {
            LOG_ERROR("Failed to initialize NetworkManager");
            return false;
        }

        // 与NTP服务器同步时间（替代GPS，更通用）
        auto clock_sync = std::make_shared<synchronization::ClockSynchronizer>(network_manager);
        if (!clock_sync->synchronizeWithNtp(m_config_manager->getString("synchronization.ntp_server", "pool.ntp.org"))) {
            LOG_WARNING("Failed to synchronize with NTP server");
        }

        // 初始化ASIC驱动
        m_asic_driver = std::make_shared<hardware::AsicDriver>();
        hardware::AsicConfig asic_config;
        asic_config.device_type = hardware::AsicDeviceType::DEVICE_TYPE_ASIC;
        asic_config.device_path = m_config_manager->getString("hardware.asic.device_path", "/dev/asic0");
        asic_config.clock_freq_mhz = m_config_manager->getInt("hardware.asic.clock_freq_mhz", 1000);
        asic_config.memory_size_mb = m_config_manager->getInt("hardware.asic.memory_size_mb", 1024);
        asic_config.num_cores = m_config_manager->getInt("hardware.asic.num_cores", 64);
        if (!m_asic_driver->initialize(asic_config)) {
            LOG_WARNING("Failed to initialize ASIC driver, continuing without ASIC acceleration");
        }

        // 初始化特征提取器
        m_feature_extractor = std::make_shared<market::FeatureExtractor>();
        std::vector<market::FeatureConfig> feature_configs = {
            {market::FeatureType::FEATURE_TYPE_PRICE, 1, 0.0},
            {market::FeatureType::FEATURE_TYPE_SPREAD, 1, 0.0},
            {market::FeatureType::FEATURE_TYPE_DEPTH, 5, 0.0},
            {market::FeatureType::FEATURE_TYPE_VOLATILITY, 20, 0.0},
            {market::FeatureType::FEATURE_TYPE_ORDER_FLOW, 10, 0.0}  // 新增订单流特征
        };
        m_feature_extractor->initialize(feature_configs);

        // 初始化市场环境分析器
        m_market_env_analyzer = std::make_shared<market::MarketEnvironmentAnalyzer>();
        m_market_env_analyzer->initialize(
            m_config_manager->getDouble("market.env.volatility_threshold", 0.02),
            m_config_manager->getDouble("market.env.liquidity_threshold", 0.01)
        );

        // 初始化流动性评估器
        m_liquidity_evaluator = std::make_shared<market::AdvancedLiquidityEvaluator>();
        m_liquidity_evaluator->initialize(m_config_manager->getInt("liquidity.depth_levels", 5));

        // 初始化风险管理
        risk::RiskLimits risk_limits;
        risk_limits.max_position = m_config_manager->getDouble("risk.max_position", 1000000.0);
        risk_limits.max_daily_loss = m_config_manager->getDouble("risk.max_daily_loss", 50000.0);
        risk_limits.max_order_size = m_config_manager->getDouble("execution.max_order_size", 100000.0);
        risk_limits.min_order_size = m_config_manager->getDouble("execution.min_order_size", 100.0);
        risk_limits.max_orders_per_second = m_config_manager->getInt("execution.max_orders_per_second", 100);
        risk_limits.max_value_at_risk = m_config_manager->getDouble("risk.max_value_at_risk", 100000.0);
        risk_limits.min_liquidity_score = m_config_manager->getDouble("liquidity.min_score", 0.5);
        risk_limits.max_price_deviation = m_config_manager->getDouble("execution.price_threshold", 0.01);
        risk_limits.max_drawdown = m_config_manager->getDouble("risk.max_drawdown", 0.1);

        m_risk_manager = std::make_shared<risk::AdvancedRiskManager>();
        if (!m_risk_manager->initialize(risk_limits)) {
            LOG_ERROR("Failed to initialize AdvancedRiskManager");
            return false;
        }

        // 初始化订单验证器
        m_order_validator = std::make_shared<execution::AdvancedOrderValidator>(risk_limits);
        m_order_validator->setLiquidityEvaluator(m_liquidity_evaluator);

        // 初始化AI模型 - 支持多种模型类型
        std::string model_type = m_config_manager->getString("ai.model_type", "tensorflow");
        if (model_type == "tensorflow") {
            m_model = std::make_shared<ai::TensorFlowModel>();
        } else if (model_type == "pytorch") {
            m_model = std::make_shared<ai::PyTorchModel>();
        } else if (model_type == "onnx") {
            m_model = std::make_shared<ai::OnnxModel>();
        } else {
            LOG_WARNING("Unknown model type: {}. Using default TensorFlow model.", model_type);
            m_model = std::make_shared<ai::TensorFlowModel>();
        }

        if (!m_model->load(m_config_manager->getString("ai.model_path", "models/hft_model.pb"))) {
            LOG_WARNING("Failed to load AI model, using default strategy");
        } else {
            LOG_INFO("AI model loaded successfully");
        }

        // 初始化策略工厂
        m_strategy_factory = std::make_shared<strategy::StrategyFactory>();
        registerDefaultStrategies();

        // 初始化性能监控器
        m_performance_monitor = std::make_shared<utils::PerformanceMonitor>();
        m_performance_monitor->setMetricsCollectionInterval(
            m_config_manager->getInt("performance.metrics_interval_ms", 1000)
        );

        // 初始化数据存储
        m_data_store = std::make_shared<persistence::DataStore>(m_config_manager->getConfig());
        if (!m_data_store->initialize()) {
            LOG_WARNING("Failed to initialize DataStore");
        } else {
            LOG_INFO("DataStore initialized successfully");
        }

        // 初始化回测引擎
        m_backtest_engine = std::make_shared<backtest::BacktestEngine>(m_config_manager->getConfig());
        if (!m_backtest_engine->initialize()) {
            LOG_WARNING("Failed to initialize BacktestEngine");
        } else {
            LOG_INFO("BacktestEngine initialized successfully");
        }

        // 初始化高级订单执行引擎
        m_advanced_order_execution_engine = std::make_unique<execution::AdvancedOrderExecutionEngine>(m_config_manager->getConfig());
        if (!m_advanced_order_execution_engine->initialize()) {
            LOG_ERROR("Failed to initialize AdvancedOrderExecutionEngine");
            return false;
        }

        // 初始化市场数据订阅器
        m_market_data_subscriber = std::make_unique<market::MarketDataSubscriber>(network_manager);
        if (!m_market_data_subscriber->initialize(m_config_manager->getMarketDataConfig())) {
            LOG_ERROR("Failed to initialize MarketDataSubscriber");
            return false;
        }

        // 初始化市场数据分发器
        m_market_data_distributor = std::make_unique<market::MarketDataDistributor>();
        m_market_data_distributor->initialize();

        // 初始化市场数据聚合器
        m_market_data_aggregator = std::make_unique<market::MarketDataAggregator>();
        m_market_data_aggregator->initialize(m_config_manager->getInt("market.data.aggregation_window_ms", 10));

        // 建立组件之间的连接
        m_market_data_subscriber->subscribeToMarketData(m_market_data_aggregator.get());
        m_market_data_aggregator->subscribeToDataUpdates(m_feature_extractor.get());
        m_feature_extractor->subscribeToFeatureUpdates(m_model.get());
        m_model->subscribeToPredictions(m_advanced_order_execution_engine.get());

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during component initialization: {}", e.what());
        return false;
    }
}

void System::registerDefaultStrategies() {
    // 注册默认策略
    m_strategy_factory->registerStrategy("TrendFollowing", []() {
        return std::make_shared<strategy::TrendFollowingStrategy>();
    });

    m_strategy_factory->registerStrategy("Adaptive", []() {
        return std::make_shared<strategy::AdaptiveStrategy>();
    });

    m_strategy_factory->registerStrategy("MarketMaking", []() {
        return std::make_shared<strategy::MarketMakingStrategy>();
    });

    m_strategy_factory->registerStrategy("StatisticalArbitrage", []() {
        return std::make_shared<strategy::StatisticalArbitrageStrategy>();
    });
}

EventLoopPtr System::getEventLoop() const {
    return m_event_loop;
}

LoggerPtr System::getLogger() const {
    return m_logger;
}

ConfigManagerPtr System::getConfigManager() const {
    return m_config_manager;
}

hardware::AsicDriverPtr System::getAsicDriver() const {
    return m_asic_driver;
}

market::FeatureExtractorPtr System::getFeatureExtractor() const {
    return m_feature_extractor;
}

market::MarketEnvironmentAnalyzerPtr System::getMarketEnvironmentAnalyzer() const {
    return m_market_env_analyzer;
}

TimeManagerPtr System::getTimeManager() const {
    return m_time_manager;
}

std::shared_ptr<utils::PerformanceMonitor> System::getPerformanceMonitor() const {
    return m_performance_monitor;
}

std::string System::generateSystemReport() const {
    std::stringstream report;
    report << "=== HFT System Report ===\n";
    report << "Initialized: " << (m_initialized ? "Yes" : "No") << "\n";
    
    if (m_performance_monitor) {
        report << m_performance_monitor->generateReport();
    }
    
    return report.str();
}

execution::AdvancedOrderExecutionEngine* System::getAdvancedOrderExecutionEngine() {
    return m_advanced_order_execution_engine.get();
}

market::MarketDataSubscriber* System::getMarketDataSubscriber() {
    return m_market_data_subscriber.get();
}

market::MarketDataDistributor* System::getMarketDataDistributor() {
    return m_market_data_distributor.get();
}

market::MarketDataAggregator* System::getMarketDataAggregator() {
    return m_market_data_aggregator.get();
}

} // namespace core
} // namespace hft

// 确保SystemPtr类型定义在命名空间之外
using SystemPtr = std::shared_ptr<hft::core::System>;

SystemStatus System::getStatus() const {
    return m_status.load();
}

const Configuration& System::getConfiguration() const {
    return m_configuration;
}

void System::run() {
    if (!m_initialized) {
        LOG_ERROR("System not initialized");
        return;
    }

    m_status = SystemStatus::RUNNING;
    LOG_INFO("HFT System running...");

    // 主运行循环
    while (m_status == SystemStatus::RUNNING) {
        // 处理系统消息
        m_event_loop->processEvents(std::chrono::milliseconds(10));
    }
}

void System::gracefulShutdown(const std::chrono::seconds& timeout) {
    LOG_INFO("Gracefully shutting down HFT System...");
    m_status = SystemStatus::SHUTTING_DOWN;

    auto startTime = std::chrono::steady_clock::now();
    while (m_status == SystemStatus::SHUTTING_DOWN) {
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (elapsed > timeout) {
            LOG_WARNING("Graceful shutdown timeout, forcing shutdown");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    shutdown();
}

} // namespace core
} // namespace hft

// 确保SystemPtr类型定义在命名空间之外
using SystemPtr = std::shared_ptr<hft::core::System>;