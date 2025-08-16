#pragma once
#include "Order.h"
#include "../marketdata/MarketDataFeed.h"
#include <vector>
#include <functional>

namespace hft {

class ExecutionOptimizer {
public:
    ExecutionOptimizer(MarketDataFeed* marketDataFeed);
    ~ExecutionOptimizer();

    // 设置优化参数
    void setParameters(double urgency, double riskAversion, double maxSlippage);

    // 优化订单执行
    void optimizeOrderExecution(OrderPtr order, const std::function<void(const OrderPtr&)>& executionCallback);

    // 基于当前市场条件调整订单
    void adjustOrderBasedOnMarketConditions(OrderPtr order);

private:
    // 计算最优下单时机
    bool shouldExecuteNow(const OrderPtr& order);

    // 计算最优下单价格
    double calculateOptimalPrice(const OrderPtr& order);

    // 计算最优下单量
    uint64_t calculateOptimalQuantity(const OrderPtr& order);

    // 订单拆分策略
    std::vector<OrderPtr> splitOrder(const OrderPtr& order);

    MarketDataFeed* m_marketDataFeed;
    double m_urgency;          // 0-1, 1表示最紧急
    double m_riskAversion;     // 0-1, 1表示最厌恶风险
    double m_maxSlippage;      // 最大允许滑点
};

} // namespace hft