#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "execution/Order.h"
#include "execution/OrderTypes.h"
#include "market/MarketData.h"
#include "core/TimeManager.h"
#include "utils/LockFreeQueue.h"

namespace hft {
namespace execution {

// 高级订单执行器基类
class AdvancedOrderExecutor {
public:
    AdvancedOrderExecutor(core::TimeManager* time_manager);
    virtual ~AdvancedOrderExecutor() = default;

    // 初始化执行器
    virtual bool initialize() = 0;
    // 执行订单
    virtual void execute(std::shared_ptr<Order> order) = 0;
    // 处理市场数据更新
    virtual void onMarketDataUpdate(const market::MarketData& data) = 0;
    // 取消订单
    virtual bool cancelOrder(uint64_t order_id) = 0;
    // 暂停执行
    virtual void pause() = 0;
    // 恢复执行
    virtual void resume() = 0;

protected:
    core::TimeManager* m_time_manager;
    bool m_is_paused;
    std::mutex m_mutex;
};

// 冰山订单执行器
class IcebergOrderExecutor : public AdvancedOrderExecutor {
public:
    IcebergOrderExecutor(core::TimeManager* time_manager);
    bool initialize() override;
    void execute(std::shared_ptr<Order> order) override;
    void onMarketDataUpdate(const market::MarketData& data) override;
    bool cancelOrder(uint64_t order_id) override;
    void pause() override;
    void resume() override;

private:
    struct IcebergOrderState {
        std::shared_ptr<Order> original_order;
        std::shared_ptr<IcebergOrderParams> params;
        double remaining_quantity;
        double visible_quantity;
        uint64_t last_execution_time;
        bool active;
    };

    std::unordered_map<uint64_t, IcebergOrderState> m_orders;
    void updateVisibleQuantity(IcebergOrderState& state);
    void sendVisibleOrder(const IcebergOrderState& state);
};

// TWAP订单执行器
class TwapOrderExecutor : public AdvancedOrderExecutor {
public:
    TwapOrderExecutor(core::TimeManager* time_manager);
    bool initialize() override;
    void execute(std::shared_ptr<Order> order) override;
    void onMarketDataUpdate(const market::MarketData& data) override;
    bool cancelOrder(uint64_t order_id) override;
    void pause() override;
    void resume() override;

private:
    struct TwapOrderState {
        std::shared_ptr<Order> original_order;
        std::shared_ptr<TwapOrderParams> params;
        double remaining_quantity;
        uint64_t start_time;
        uint64_t next_execution_time;
        uint32_t interval_count;
        bool active;
    };

    std::unordered_map<uint64_t, TwapOrderState> m_orders;
    utils::LockFreeQueue<std::shared_ptr<Order>> m_order_queue;
    std::thread m_execution_thread;
    std::atomic<bool> m_running;

    void executionLoop();
    double calculateOrderSize(const TwapOrderState& state);
    uint64_t calculateNextExecutionTime(const TwapOrderState& state);
};

// VWAP订单执行器
class VwapOrderExecutor : public AdvancedOrderExecutor {
public:
    VwapOrderExecutor(core::TimeManager* time_manager);
    bool initialize() override;
    void execute(std::shared_ptr<Order> order) override;
    void onMarketDataUpdate(const market::MarketData& data) override;
    bool cancelOrder(uint64_t order_id) override;
    void pause() override;
    void resume() override;

private:
    struct VwapOrderState {
        std::shared_ptr<Order> original_order;
        std::shared_ptr<VwapOrderParams> params;
        double remaining_quantity;
        uint64_t start_time;
        std::vector<double> historical_volumes;
        std::vector<double> historical_prices;
        bool active;
    };

    std::unordered_map<uint64_t, VwapOrderState> m_orders;
    std::unordered_map<std::string, std::vector<market::MarketData>> m_market_data_history;

    double calculateVolumeWeightedPrice(const VwapOrderState& state);
    double calculateOrderSize(const VwapOrderState& state);
    void updateMarketDataHistory(const market::MarketData& data);
};

// 高级订单执行引擎
class AdvancedOrderExecutionEngine {
public:
    AdvancedOrderExecutionEngine(core::TimeManager* time_manager);
    ~AdvancedOrderExecutionEngine();

    // 初始化引擎
    bool initialize();
    // 执行高级订单
    uint64_t executeOrder(std::shared_ptr<Order> order);
    // 处理市场数据更新
    void onMarketDataUpdate(const market::MarketData& data);
    // 取消订单
    bool cancelOrder(uint64_t order_id);
    // 暂停所有执行
    void pauseAll();
    // 恢复所有执行
    void resumeAll();

private:
    core::TimeManager* m_time_manager;
    std::unordered_map<AdvancedOrderType, std::shared_ptr<AdvancedOrderExecutor>> m_executors;
    std::unordered_map<uint64_t, AdvancedOrderType> m_order_type_map;
    std::mutex m_mutex;
};

} // namespace execution
} // namespace hft