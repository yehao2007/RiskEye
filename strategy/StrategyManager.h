#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Strategy.h"
#include "core/EventLoop.h"
#include "network/MarketDataFeed.h"
#include "pattern/PatternRecognizer.h"
#include "execution/OrderExecution.h"

namespace hft {
namespace strategy {

class StrategyManager {
public:
    StrategyManager();
    ~StrategyManager();

    // 初始化
    bool initialize(const std::string& configPath);
    // 启动所有策略
    void startAllStrategies();
    // 停止所有策略
    void stopAllStrategies();
    // 添加策略
    bool addStrategy(std::shared_ptr<Strategy> strategy);
    // 获取策略
    std::shared_ptr<Strategy> getStrategy(const std::string& name) const;
    // 移除策略
    bool removeStrategy(const std::string& name);

    // 注册市场数据回调
    void registerMarketDataCallback();
    // 注册模式识别回调
    void registerPatternCallback();

    // 设置执行器
    void setOrderExecution(std::shared_ptr<execution::OrderExecution> orderExecution) {
        m_orderExecution = orderExecution;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Strategy>> m_strategies;
    std::shared_ptr<core::EventLoop> m_eventLoop;
    std::shared_ptr<pattern::PatternRecognizer> m_patternRecognizer;
    std::shared_ptr<execution::OrderExecution> m_orderExecution;

    // 定期执行所有策略
    void executeAllStrategies();
};

} // namespace strategy
} // namespace hft