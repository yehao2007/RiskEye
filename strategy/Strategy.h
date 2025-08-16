#pragma once
#include <string>
#include <vector>
#include <memory>
#include "core/EventLoop.h"
#include "pattern/MarketPattern.h"
#include "network/MarketDataFeed.h"
#include "execution/Order.h"

namespace hft {
namespace strategy {

// 策略类型枚举
enum class StrategyType {
    TREND_FOLLOWING,
    MEAN_REVERSION,
    ARBITRAGE,
    MOMENTUM,
    MARKET_MAKING
};

// 策略基类
class Strategy {
public:
    Strategy(const std::string& name, StrategyType type);
    virtual ~Strategy();

    // 获取策略名称
    const std::string& getName() const { return m_name; }
    // 获取策略类型
    StrategyType getType() const { return m_type; }
    // 策略是否激活
    bool isActive() const { return m_active; }
    // 设置策略激活状态
    void setActive(bool active) { m_active = active; }

    // 初始化策略
    virtual bool initialize(const std::string& configPath) = 0;
    // 启动策略
    virtual void start() = 0;
    // 停止策略
    virtual void stop() = 0;
    // 处理市场数据
    virtual void onMarketData(const network::MarketData& data) = 0;
    // 处理市场模式
    virtual void onMarketPattern(const pattern::MarketPattern& pattern) = 0;
    // 执行策略逻辑
    virtual std::vector<execution::Order> execute() = 0;

    // 设置风险参数
    virtual void setRiskParameters(double maxPosition, double maxLossPerDay) {
        m_maxPosition = maxPosition;
        m_maxLossPerDay = maxLossPerDay;
    }

protected:
    std::string m_name;
    StrategyType m_type;
    bool m_active;
    double m_maxPosition;
    double m_maxLossPerDay;
    std::shared_ptr<core::EventLoop> m_eventLoop;
};

} // namespace strategy
} // namespace hft