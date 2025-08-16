#pragma once
#include <memory>
#include <atomic>
#include <chrono>
#include "core/EventLoop.h"
#include "core/Logger.h"
#include "hardware/AsicDriver.h"
#include "market/FeatureExtractor.h"
#include "market/MarketEnvironmentAnalyzer.h"
#include "core/TimeManager.h"
#include "market/LiquidityEvaluator.h"
#include "execution/OrderValidator.h"
#include "ai/Model.h"
#include "strategy/CustomStrategy.h"
#include "strategy/StrategyFactory.h"
#include "risk/AdvancedRiskManager.h"
#include "core/Configuration.h"
#include "execution/AdvancedOrderExecutionEngine.h"
#include "market/MarketDataSubscriber.h"
#include "market/MarketDataDistributor.h"
#include "market/MarketDataAggregator.h"
#include "persistence/DataStore.h"
#include "backtest/BacktestEngine.h"

namespace hft {
namespace core {

// 添加新组件的前向声明
namespace utils {
    class PerformanceMonitor;
}

enum class SystemStatus {
    NOT_INITIALIZED,
    INITIALIZING,
    RUNNING,
    SHUTTING_DOWN,
    SHUT_DOWN
};

class System {
public:
    System();
    ~System();

    // 初始化系统
    bool initialize();
    bool initialize(const Configuration& config);

    // 启动系统
    bool start();

    // 运行系统
    void run();

    // 关闭系统
    void shutdown();

    // 优雅关闭系统
    void gracefulShutdown(const std::chrono::seconds& timeout = std::chrono::seconds(5));

    // 获取系统状态
    SystemStatus getStatus() const;

    // 获取事件循环
    EventLoopPtr getEventLoop() const;

    // 获取日志器
    LoggerPtr getLogger() const;

    // 获取配置
    const Configuration& getConfiguration() const;

    // 获取ASIC驱动
    hardware::AsicDriverPtr getAsicDriver() const;

    // 获取特征提取器
    market::FeatureExtractorPtr getFeatureExtractor() const;

    // 获取市场环境分析器
    market::MarketEnvironmentAnalyzerPtr getMarketEnvironmentAnalyzer() const;

    // 获取时间管理器
    TimeManagerPtr getTimeManager() const;

    // 获取高级订单执行引擎
    execution::AdvancedOrderExecutionEngine* getAdvancedOrderExecutionEngine();
    // 获取市场数据订阅器
    market::MarketDataSubscriber* getMarketDataSubscriber();
    // 获取市场数据分发器
    market::MarketDataDistributor* getMarketDataDistributor();
    // 获取市场数据聚合器
    market::MarketDataAggregator* getMarketDataAggregator();

private:
    // 初始化组件
    bool initializeComponents();

    // 注册默认策略
    void registerDefaultStrategies();

    EventLoopPtr m_event_loop;
    LoggerPtr m_logger;
    Configuration m_configuration;
    hardware::AsicDriverPtr m_asic_driver;
    market::FeatureExtractorPtr m_feature_extractor;
    market::MarketEnvironmentAnalyzerPtr m_market_env_analyzer;
    TimeManagerPtr m_time_manager;
    market::LiquidityEvaluatorPtr m_liquidity_evaluator;
    execution::OrderValidatorPtr m_order_validator;
    ai::ModelPtr m_model;
    strategy::StrategyFactoryPtr m_strategy_factory;
    risk::AdvancedRiskManagerPtr m_risk_manager;
    bool m_initialized;
    std::atomic<SystemStatus> m_status;

    // 高级订单执行引擎
    std::unique_ptr<execution::AdvancedOrderExecutionEngine> m_advanced_order_execution_engine;
    // 市场数据订阅器
    std::unique_ptr<market::MarketDataSubscriber> m_market_data_subscriber;
    // 市场数据分发器
    std::unique_ptr<market::MarketDataDistributor> m_market_data_distributor;
    // 市场数据聚合器
    std::unique_ptr<market::MarketDataAggregator> m_market_data_aggregator;

    // 获取性能监控器
    std::shared_ptr<utils::PerformanceMonitor> getPerformanceMonitor() const;

    // 生成系统报告
    std::string generateSystemReport() const;

private:
    // 性能监控器
    std::shared_ptr<utils::PerformanceMonitor> m_performance_monitor;

    // 获取数据存储
    std::shared_ptr<persistence::DataStore> getDataStore() const;

    // 获取回测引擎
    std::shared_ptr<backtest::BacktestEngine> getBacktestEngine() const;

private:
    // 数据存储
    std::shared_ptr<persistence::DataStore> m_data_store;

    // 回测引擎
    std::shared_ptr<backtest::BacktestEngine> m_backtest_engine;
};

} // namespace core
} // namespace hft
using SystemPtr = std::shared_ptr<hft::core::System>;
// core/System.h 的改进
class System {
public:
    bool initialize(const Configuration& config);
    void run();
    void shutdown();
    
    // 添加系统状态监控
    SystemStatus getStatus() const;
    
    // 添加优雅关闭机制
    void gracefulShutdown(const std::chrono::seconds& timeout = std::chrono::seconds(5));
    
private:
    std::atomic<bool> running_{false};
    std::unique_ptr<EventLoop> eventLoop_;
    Logger logger_;
    // ... 其他成员
};