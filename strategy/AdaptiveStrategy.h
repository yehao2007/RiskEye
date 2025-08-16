#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Strategy.h"
#include "MarketMaking.h"
#include "StatisticalArbitrage.h"

namespace hft {
namespace strategy {

// 市场状态枚举
enum class MarketState {
    LOW_VOLATILITY,
    HIGH_VOLATILITY,
    TRENDING,
    RANGING,
    CRASH,
    RALLY
};

class AdaptiveStrategy : public Strategy {
public:
    AdaptiveStrategy();
    ~AdaptiveStrategy();

    // 初始化策略
    bool initialize() override;
    // 执行策略
    void execute(const MarketData& data) override;
    // 更新策略参数
    void updateParameters(const Parameters& params) override;

    // 设置策略池
    void setStrategyPool(const std::vector<std::unique_ptr<Strategy>>& strategies);
    // 识别当前市场状态
    MarketState identifyMarketState(const MarketData& data);
    // 基于市场状态选择策略
    Strategy* selectStrategy(MarketState state);

private:
    std::vector<std::unique_ptr<Strategy>> m_strategyPool;
    Strategy* m_currentStrategy;
    MarketState m_currentMarketState;

    // 市场状态分类器
    std::unique_ptr<MarketStateClassifier> m_stateClassifier;
    // 强化学习代理用于参数优化
    std::unique_ptr<RLAgent> m_rlAgent;
};

} // namespace strategy
} // namespace hft