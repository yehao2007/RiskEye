#pragma once
#include "Strategy.h"
#include <vector>
#include <deque>

namespace hft {
namespace strategy {

class MeanReversionStrategy : public Strategy {
public:
    MeanReversionStrategy();
    ~MeanReversionStrategy() override = default;

    // 初始化策略
    bool initialize(const std::string& configPath) override;

    // 启动策略
    void start() override;

    // 停止策略
    void stop() override;

    // 处理市场数据
    void onMarketData(const network::MarketData& data) override;

    // 处理市场模式
    void onMarketPattern(const pattern::MarketPattern& pattern) override;

    // 执行策略逻辑
    std::vector<execution::Order> execute() override;

private:
    std::deque<double> m_price_history;
    uint32_t m_lookback_period;
    double m_std_dev_threshold;

    // 计算移动平均线
    double calculateMovingAverage() const;

    // 计算标准差
    double calculateStdDev() const;

    // 计算Z分数
    double calculateZScore() const;
};

} // namespace strategy
} // namespace hft