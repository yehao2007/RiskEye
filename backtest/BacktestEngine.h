#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "core/Configuration.h"
#include "market/MarketData.h"
#include "strategy/Strategy.h"
#include "execution/Order.h"
#include "utils/PerformanceMetrics.h"

namespace hft {
namespace backtest {

class BacktestEngine {
public:
    BacktestEngine(const core::Configuration& config);
    ~BacktestEngine();

    // 初始化回测引擎
    bool initialize();
    
    // 设置策略
    void setStrategy(std::shared_ptr<strategy::Strategy> strategy);
    
    // 加载历史数据
    bool loadHistoricalData(const std::string& data_path);
    
    // 运行回测
    void run();
    
    // 暂停回测
    void pause();
    
    // 恢复回测
    void resume();
    
    // 停止回测
    void stop();
    
    // 获取回测结果
    PerformanceMetrics getResults() const;
    
    // 设置回测参数
    void setParameters(const std::map<std::string, double>& params);

private:
    core::Configuration m_config;
    std::shared_ptr<strategy::Strategy> m_strategy;
    std::vector<market::MarketData> m_historical_data;
    PerformanceMetrics m_performance_metrics;
    std::atomic<bool> m_running;
    std::atomic<bool> m_paused;
    double m_initial_capital;
    double m_current_capital;
    double m_commission_rate;

    // 处理市场数据
    void processMarketData(const market::MarketData& data);
    
    // 处理订单执行
    void processOrderExecution(const execution::Order& order, double fill_price);
    
    // 更新性能指标
    void updatePerformanceMetrics();
};

} // namespace backtest
} // namespace hft