#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include "execution/Order.h"

namespace hft {
namespace execution {

// 高级订单类型枚举
enum class AdvancedOrderType {
    ICEBERG,             // 冰山订单
    TWAP,                // 时间加权平均价格订单
    VWAP,                // 成交量加权平均价格订单
    PEGGED,              // 钉住订单
    TRAILING_STOP,       // 追踪止损订单
    CONDITIONAL,         // 条件订单
    OCO,                 // 二选一订单
    OTO                  // 触发订单
};

// 订单时间条件
enum class TimeCondition {
    DAY,                 // 当日有效
    GTC,                 // 撤销前有效
    IOC,                 // 立即成交或取消
    FOK                  // 全部成交或取消
};

// 高级订单参数基类
struct AdvancedOrderParams {
    virtual ~AdvancedOrderParams() = default;
    AdvancedOrderType type;
    TimeCondition time_condition;
    uint64_t expiration_time;  // 过期时间戳(微秒)
};

// 冰山订单参数
struct IcebergOrderParams : public AdvancedOrderParams {
    double visible_size;  // 可见数量
    double peak_size;     // 峰值数量
    double minimum_size;  // 最小显示数量
};

// TWAP订单参数
struct TwapOrderParams : public AdvancedOrderParams {
    uint32_t time_interval_ms;  // 时间间隔(毫秒)
    uint32_t total_duration_ms; // 总持续时间(毫秒)
    bool randomized;            // 是否随机化执行时间
    double random_factor;       // 随机因子(0-1)
};

// VWAP订单参数
struct VwapOrderParams : public AdvancedOrderParams {
    uint32_t lookback_period;   // 回溯周期(秒)
    bool use_historical_volume; // 是否使用历史成交量
};

// 钉住订单参数
struct PeggedOrderParams : public AdvancedOrderParams {
    enum class PegType {
        BEST_BID,
        BEST_ASK,
        MID_PRICE,
        LAST_PRICE
    };
    PegType peg_type;
    double offset;              // 偏移量
    bool dynamic_offset;        // 是否动态调整偏移量
};

// 追踪止损订单参数
struct TrailingStopOrderParams : public AdvancedOrderParams {
    double trail_amount;        // 追踪金额
    double trail_percentage;    // 追踪百分比
    bool use_percentage;        // 是否使用百分比
    double minimum_move;        // 最小移动单位
};

// 条件订单参数
struct ConditionalOrderParams : public AdvancedOrderParams {
    enum class ConditionType {
        PRICE_ABOVE,
        PRICE_BELOW,
        VOLUME_ABOVE,
        VOLUME_BELOW,
        TIME_AFTER,
        TIME_BEFORE
    };
    ConditionType condition_type;
    double condition_value;     // 条件值
    std::string reference_symbol; // 参考标的
};

// 二选一订单参数
struct OcoOrderParams : public AdvancedOrderParams {
    std::shared_ptr<Order> primary_order;
    std::shared_ptr<Order> secondary_order;
};

// 触发订单参数
struct OtoOrderParams : public AdvancedOrderParams {
    std::shared_ptr<Order> trigger_order;
    std::vector<std::shared_ptr<Order>> subsequent_orders;
};

// 订单类型工厂
class OrderTypeFactory {
public:
    static std::shared_ptr<AdvancedOrderParams> createOrderParams(AdvancedOrderType type);
    static std::shared_ptr<Order> createAdvancedOrder(
        const std::string& symbol,
        OrderSide side,
        double quantity,
        double price,
        std::shared_ptr<AdvancedOrderParams> params);
};

} // namespace execution
} // namespace hft