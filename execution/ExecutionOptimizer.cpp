#include "ExecutionOptimizer.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace hft {

ExecutionOptimizer::ExecutionOptimizer(MarketDataFeed* marketDataFeed)
    : m_marketDataFeed(marketDataFeed), m_urgency(0.5), m_riskAversion(0.5), m_maxSlippage(0.1) {
}

ExecutionOptimizer::~ExecutionOptimizer() {
}

void ExecutionOptimizer::setParameters(double urgency, double riskAversion, double maxSlippage) {
    m_urgency = std::clamp(urgency, 0.0, 1.0);
    m_riskAversion = std::clamp(riskAversion, 0.0, 1.0);
    m_maxSlippage = std::clamp(maxSlippage, 0.0, 10.0);
}

void ExecutionOptimizer::optimizeOrderExecution(OrderPtr order, const std::function<void(const OrderPtr&)>& executionCallback) {
    if (!order) return;

    // 基于市场条件调整订单
    adjustOrderBasedOnMarketConditions(order);

    // 如果订单是限价单，检查是否应该立即执行
    if (order->type == OrderType::LIMIT) {
        if (shouldExecuteNow(order)) {
            executionCallback(order);
        } else {
            // 否则，可能需要拆分订单或等待更好的价格
            std::vector<OrderPtr> splitOrders = splitOrder(order);
            for (const auto& splitOrder : splitOrders) {
                executionCallback(splitOrder);
            }
        }
    } else {
        // 市价单或其他类型的订单立即执行
        executionCallback(order);
    }
}

void ExecutionOptimizer::adjustOrderBasedOnMarketConditions(OrderPtr order) {
    if (!order || !m_marketDataFeed) return;

    // 获取最新的市场数据
    auto marketData = m_marketDataFeed->getLastMarketData(order->symbol);
    if (!marketData) return;

    double bidPrice = marketData->bidPrice;
    double askPrice = marketData->askPrice;
    double lastPrice = marketData->lastPrice;

    // 根据订单方向和市场条件调整价格
    if (order->side == OrderSide::BUY) {
        if (order->type == OrderType::LIMIT) {
            // 买单的限价不应低于最佳卖价太多
            order->price = std::max(order->price, askPrice * (1.0 - m_maxSlippage / 100.0));
        }
    } else if (order->side == OrderSide::SELL) {
        if (order->type == OrderType::LIMIT) {
            // 卖单的限价不应高于最佳买价太多
            order->price = std::min(order->price, bidPrice * (1.0 + m_maxSlippage / 100.0));
        }
    }

    // 根据市场波动性调整订单价格
    double volatility = m_marketDataFeed->getVolatility(order->symbol);
    if (volatility > 0) {
        double priceAdjustment = volatility * m_riskAversion * (1.0 - m_urgency);
        if (order->side == OrderSide::BUY) {
            order->price += priceAdjustment;
        } else {
            order->price -= priceAdjustment;
        }
    }
}

bool ExecutionOptimizer::shouldExecuteNow(const OrderPtr& order) {
    if (!order || !m_marketDataFeed) return true;

    auto marketData = m_marketDataFeed->getLastMarketData(order->symbol);
    if (!marketData) return true;

    // 检查当前市场价格是否满足订单条件
    if (order->side == OrderSide::BUY && order->price >= marketData->askPrice) {
        return true;
    }
    if (order->side == OrderSide::SELL && order->price <= marketData->bidPrice) {
        return true;
    }

    // 根据紧迫性调整执行决策
    if (m_urgency > 0.8) {
        // 高紧迫性，接近价格就执行
        double priceDiffPercent = 0.1; // 0.1%
        if (order->side == OrderSide::BUY) {
            return order->price >= marketData->askPrice * (1.0 - priceDiffPercent / 100.0);
        } else {
            return order->price <= marketData->bidPrice * (1.0 + priceDiffPercent / 100.0);
        }
    }

    return false;
}

double ExecutionOptimizer::calculateOptimalPrice(const OrderPtr& order) {
    if (!order || !m_marketDataFeed) return order->price;

    auto marketData = m_marketDataFeed->getLastMarketData(order->symbol);
    if (!marketData) return order->price;

    double referencePrice = (marketData->bidPrice + marketData->askPrice) / 2.0;

    // 根据订单方向和市场条件计算最优价格
    if (order->side == OrderSide::BUY) {
        // 买单价格 = 参考价格 + 滑点调整 - 风险调整
        return referencePrice * (1.0 + m_maxSlippage / 100.0 * (1.0 - m_riskAversion)) * m_urgency;
    } else {
        // 卖单价格 = 参考价格 - 滑点调整 + 风险调整
        return referencePrice * (1.0 - m_maxSlippage / 100.0 * (1.0 - m_riskAversion)) * m_urgency;
    }
}

uint64_t ExecutionOptimizer::calculateOptimalQuantity(const OrderPtr& order) {
    if (!order || !m_marketDataFeed) return order->quantity;

    // 获取市场深度
    auto marketDepth = m_marketDataFeed->getMarketDepth(order->symbol);
    if (marketDepth.empty()) return order->quantity;

    // 基于市场深度和流动性计算最优下单量
    uint64_t availableLiquidity = 0;
    double averagePrice = 0.0;

    if (order->side == OrderSide::BUY) {
        // 买单看卖盘深度
        for (const auto& level : marketDepth) {
            if (level.price <= order->price) {
                availableLiquidity += level.quantity;
                averagePrice += level.price * level.quantity;
            } else {
                break;
            }
        }
    } else {
        // 卖单看买盘深度
        for (const auto& level : marketDepth) {
            if (level.price >= order->price) {
                availableLiquidity += level.quantity;
                averagePrice += level.price * level.quantity;
            } else {
                break;
            }
        }
    }

    if (availableLiquidity > 0) {
        averagePrice /= availableLiquidity;
    }

    // 根据市场流动性和订单紧迫性调整下单量
    uint64_t optimalQuantity = std::min(order->quantity, availableLiquidity * m_urgency);

    return optimalQuantity > 0 ? optimalQuantity : order->quantity;
}

std::vector<OrderPtr> ExecutionOptimizer::splitOrder(const OrderPtr& order) {
    std::vector<OrderPtr> splitOrders;

    // 简单的订单拆分策略：根据市场流动性拆分
    uint64_t optimalQuantity = calculateOptimalQuantity(order);
    if (optimalQuantity >= order->quantity) {
        splitOrders.push_back(order);
        return splitOrders;
    }

    // 拆分订单为多个小订单
    uint64_t remainingQuantity = order->quantity;
    while (remainingQuantity > 0) {
        uint64_t currentQuantity = std::min(optimalQuantity, remainingQuantity);

        OrderPtr splitOrder = std::make_shared<Order>(*order);
        splitOrder->quantity = currentQuantity;
        splitOrder->orderId = 0; // 会在发送时重新生成

        splitOrders.push_back(splitOrder);
        remainingQuantity -= currentQuantity;

        // 如果还有剩余数量，可能需要调整价格以获得更好的执行
        if (remainingQuantity > 0) {
            if (order->side == OrderSide::BUY) {
                splitOrder->price += 0.01; // 略微提高买单价格
            } else {
                splitOrder->price -= 0.01; // 略微降低卖单价格
            }
        }
    }

    return splitOrders;
}

} // namespace hft