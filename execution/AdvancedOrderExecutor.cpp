#include "AdvancedOrderExecutor.h"
#include <iostream>
#include <thread>
#include <random>
#include <algorithm>
#include "execution/OrderManager.h"
#include "market/MarketDataManager.h"

namespace hft {
namespace execution {

// AdvancedOrderExecutor 基类实现
AdvancedOrderExecutor::AdvancedOrderExecutor(core::TimeManager* time_manager)
    : m_time_manager(time_manager),
      m_is_paused(false) {
}

// IcebergOrderExecutor 实现
IcebergOrderExecutor::IcebergOrderExecutor(core::TimeManager* time_manager)
    : AdvancedOrderExecutor(time_manager) {
}

bool IcebergOrderExecutor::initialize() {
    std::cout << "IcebergOrderExecutor initialized" << std::endl;
    return true;
}

void IcebergOrderExecutor::execute(std::shared_ptr<Order> order) {
    if (!order || !order->advanced_params ||
        order->advanced_params->type != AdvancedOrderType::ICEBERG) {
        std::cerr << "Invalid order for IcebergOrderExecutor" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto params = std::static_pointer_cast<IcebergOrderParams>(order->advanced_params);

    IcebergOrderState state;
    state.original_order = order;
    state.params = params;
    state.remaining_quantity = order->quantity;
    state.last_execution_time = m_time_manager->getCurrentTimestamp();
    state.active = true;

    // 初始可见数量
    updateVisibleQuantity(state);

    m_orders[order->id] = state;
    std::cout << "Iceberg order " << order->id << " started, total quantity: " << order->quantity << std::endl;

    // 发送第一个可见订单
    sendVisibleOrder(state);
}

void IcebergOrderExecutor::onMarketDataUpdate(const market::MarketData& data) {
    if (m_is_paused) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t current_time = m_time_manager->getCurrentTimestamp();

    for (auto& [order_id, state] : m_orders) {
        if (!state.active) {
            continue;
        }

        // 检查订单是否过期
        if (state.params->expiration_time > 0 && current_time > state.params->expiration_time) {
            cancelOrder(order_id);
            continue;
        }

        // 检查是否需要更新可见数量
        if (current_time - state.last_execution_time > 10000) { // 10秒
            updateVisibleQuantity(state);
            sendVisibleOrder(state);
            state.last_execution_time = current_time;
        }
    }
}

bool IcebergOrderExecutor::cancelOrder(uint64_t order_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_orders.find(order_id);
    if (it == m_orders.end()) {
        return false;
    }

    it->second.active = false;
    std::cout << "Iceberg order " << order_id << " canceled" << std::endl;
    return true;
}

void IcebergOrderExecutor::pause() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_is_paused = true;
    std::cout << "IcebergOrderExecutor paused" << std::endl;
}

void IcebergOrderExecutor::resume() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_is_paused = false;
    std::cout << "IcebergOrderExecutor resumed" << std::endl;
}

void IcebergOrderExecutor::updateVisibleQuantity(IcebergOrderState& state) {
    // 计算可见数量
    double visible_size = std::min(state.params->visible_size, state.remaining_quantity);
    visible_size = std::max(visible_size, state.params->minimum_size);

    // 随机化可见数量以隐藏真实意图
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.8, 1.2);
    visible_size *= dis(gen);

    state.visible_quantity = visible_size;
}

void IcebergOrderExecutor::sendVisibleOrder(const IcebergOrderState& state) {
    // 创建可见订单
    auto visible_order = std::make_shared<Order>();
    visible_order->id = m_time_manager->getCurrentTimestamp(); // 临时ID
    visible_order->symbol = state.original_order->symbol;
    visible_order->side = state.original_order->side;
    visible_order->quantity = state.visible_quantity;
    visible_order->price = state.original_order->price;
    visible_order->time_condition = state.params->time_condition;

    // 发送订单到订单管理器
    // 这里简化处理，实际应调用OrderManager
    std::cout << "Sending iceberg visible order: " << visible_order->id
              << " quantity: " << visible_order->quantity << std::endl;
}

// TWAPOrderExecutor 实现
TwapOrderExecutor::TwapOrderExecutor(core::TimeManager* time_manager)
    : AdvancedOrderExecutor(time_manager),
      m_running(false) {
}

bool TwapOrderExecutor::initialize() {
    m_running = true;
    m_execution_thread = std::thread(&TwapOrderExecutor::executionLoop, this);
    std::cout << "TwapOrderExecutor initialized" << std::endl;
    return true;
}

void TwapOrderExecutor::execute(std::shared_ptr<Order> order) {
    if (!order || !order->advanced_params ||
        order->advanced_params->type != AdvancedOrderType::TWAP) {
        std::cerr << "Invalid order for TwapOrderExecutor" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto params = std::static_pointer_cast<TwapOrderParams>(order->advanced_params);

    TwapOrderState state;
    state.original_order = order;
    state.params = params;
    state.remaining_quantity = order->quantity;
    state.start_time = m_time_manager->getCurrentTimestamp();
    state.next_execution_time = state.start_time + params->time_interval_ms * 1000;
    state.interval_count = 0;
    state.active = true;

    m_orders[order->id] = state;
    std::cout << "TWAP order " << order->id << " started, total quantity: " << order->quantity
              << " duration: " << params->total_duration_ms << "ms" << std::endl;
}

void TwapOrderExecutor::onMarketDataUpdate(const market::MarketData& data) {
    // TWAP主要基于时间执行，市场数据更新可能用于调整价格
    if (m_is_paused) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [order_id, state] : m_orders) {
        if (!state.active) {
            continue;
        }
        // 这里可以根据市场数据调整订单价格
    }
}

bool TwapOrderExecutor::cancelOrder(uint64_t order_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_orders.find(order_id);
    if (it == m_orders.end()) {
        return false;
    }

    it->second.active = false;
    std::cout << "TWAP order " << order_id << " canceled" << std::endl;
    return true;
}

void TwapOrderExecutor::pause() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_is_paused = true;
    std::cout << "TwapOrderExecutor paused" << std::endl;
}

void TwapOrderExecutor::resume() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_is_paused = false;
    std::cout << "TwapOrderExecutor resumed" << std::endl;
}

void TwapOrderExecutor::executionLoop() {
    while (m_running) {
        if (!m_is_paused) {
            uint64_t current_time = m_time_manager->getCurrentTimestamp();
            std::vector<uint64_t> completed_orders;

            { // 加锁作用域
                std::lock_guard<std::mutex> lock(m_mutex);
                for (auto& [order_id, state] : m_orders) {
                    if (!state.active) {
                        continue;
                    }

                    // 检查订单是否过期
                    if (state.params->expiration_time > 0 && current_time > state.params->expiration_time) {
                        completed_orders.push_back(order_id);
                        continue;
                    }

                    // 检查是否到了下一次执行时间
                    if (current_time >= state.next_execution_time) {
                        double order_size = calculateOrderSize(state);
                        if (order_size > 0) {
                            // 创建并发送订单
                            auto twap_order = std::make_shared<Order>();
                            twap_order->id = current_time + state.interval_count;
                            twap_order->symbol = state.original_order->symbol;
                            twap_order->side = state.original_order->side;
                            twap_order->quantity = order_size;
                            twap_order->price = state.original_order->price; // 实际应用中可能根据市场数据调整
                            twap_order->time_condition = state.params->time_condition;

                            // 发送订单到队列
                            m_order_queue.push(twap_order);
                            std::cout << "Sending TWAP order slice: " << twap_order->id
                                      << " quantity: " << twap_order->quantity << std::endl;

                            // 更新剩余数量
                            state.remaining_quantity -= order_size;
                            state.interval_count++;

                            // 计算下一次执行时间
                            state.next_execution_time = calculateNextExecutionTime(state);
                        }

                        // 检查是否完成
                        if (state.remaining_quantity <= 0 ||
                            current_time >= state.start_time + state.params->total_duration_ms * 1000) {
                            completed_orders.push_back(order_id);
                        }
                    }
                }
            }

            // 处理完成的订单
            for (uint64_t order_id : completed_orders) {
                cancelOrder(order_id);
            }

            // 处理订单队列中的订单
            std::shared_ptr<Order> order;
            while (m_order_queue.pop(order)) {
                // 实际应用中这里应该调用OrderManager发送订单
                std::cout << "Executing TWAP order slice: " << order->id << std::endl;
            }
        }

        // 短暂休眠以避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

double TwapOrderExecutor::calculateOrderSize(const TwapOrderState& state) {
    // 计算剩余时间间隔数
    uint64_t current_time = m_time_manager->getCurrentTimestamp();
    uint64_t remaining_time = state.start_time + state.params->total_duration_ms * 1000 - current_time;
    uint32_t remaining_intervals = remaining_time / (state.params->time_interval_ms * 1000) + 1;

    // 平均分配剩余数量
    double average_size = state.remaining_quantity / remaining_intervals;

    // 如果启用随机化，调整订单大小
    if (state.params->randomized) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(1.0 - state.params->random_factor,
                                             1.0 + state.params->random_factor);
        average_size *= dis(gen);
    }

    return std::min(average_size, state.remaining_quantity);
}

uint64_t TwapOrderExecutor::calculateNextExecutionTime(const TwapOrderState& state) {
    uint64_t next_time = state.next_execution_time + state.params->time_interval_ms * 1000;

    // 如果启用随机化，调整下一次执行时间
    if (state.params->randomized) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-state.params->random_factor,
                                             state.params->random_factor);
        int64_t random_offset = static_cast<int64_t>(state.params->time_interval_ms * 1000 * dis(gen));
        next_time += random_offset;
    }

    return next_time;
}

// VWAPOrderExecutor 实现
VwapOrderExecutor::VwapOrderExecutor(core::TimeManager* time_manager)
    : AdvancedOrderExecutor(time_manager) {
}

bool VwapOrderExecutor::initialize() {
    std::cout << "VwapOrderExecutor initialized" << std::endl;
    return true;
}

void VwapOrderExecutor::execute(std::shared_ptr<Order> order) {
    if (!order || !order->advanced_params ||
        order->advanced_params->type != AdvancedOrderType::VWAP) {
        std::cerr << "Invalid order for VwapOrderExecutor" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto params = std::static_pointer_cast<VwapOrderParams>(order->advanced_params);

    VwapOrderState state;
    state.original_order = order;
    state.params = params;
    state.remaining_quantity = order->quantity;
    state.start_time = m_time_manager->getCurrentTimestamp();
    state.active = true;

    // 初始化历史数据
    if (params->use_historical_volume) {
        // 这里应该从MarketDataManager获取历史数据
        // 简化处理，假设已经获取
    }

    m_orders[order->id] = state;
    std::cout << "VWAP order " << order->id << " started, total quantity: " << order->quantity << std::endl;
}

void VwapOrderExecutor::onMarketDataUpdate(const market::MarketData& data) {
    if (m_is_paused) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t current_time = m_time_manager->getCurrentTimestamp();

    // 更新市场数据历史
    updateMarketDataHistory(data);

    // 处理每个VWAP订单
    for (auto& [order_id, state] : m_orders) {
        if (!state.active) {
            continue;
        }

        // 检查订单是否过期
        if (state.params->expiration_time > 0 && current_time > state.params->expiration_time) {
            cancelOrder(order_id);
            continue;
        }

        // 计算VWAP价格
        double vwap_price = calculateVolumeWeightedPrice(state);
        if (vwap_price <= 0) {
            continue;
        }

        // 计算订单大小
        double order_size = calculateOrderSize(state);
        if (order_size <= 0) {
            continue;
        }

        // 创建并发送订单
        auto vwap_order = std::make_shared<Order>();
        vwap_order->id = current_time;
        vwap_order->symbol = state.original_order->symbol;
        vwap_order->side = state.original_order->side;
        vwap_order->quantity = order_size;
        vwap_order->price = vwap_price;
        vwap_order->time_condition = state.params->time_condition;

        // 发送订单
        std::cout << "Sending VWAP order: " << vwap_order->id
                  << " quantity: " << vwap_order->quantity
                  << " price: " << vwap_order->price << std::endl;

        // 更新剩余数量
        state.remaining_quantity -= order_size;

        // 检查是否完成
        if (state.remaining_quantity <= 0) {
            cancelOrder(order_id);
        }
    }
}

bool VwapOrderExecutor::cancelOrder(uint64_t order_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_orders.find(order_id);
    if (it == m_orders.end()) {
        return false;
    }

    it->second.active = false;
    std::cout << "VWAP order " << order_id << " canceled" << std::endl;
    return true;
}

void VwapOrderExecutor::pause() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_is_paused = true;
    std::cout << "VwapOrderExecutor paused" << std::endl;
}

void VwapOrderExecutor::resume() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_is_paused = false;
    std::cout << "VwapOrderExecutor resumed" << std::endl;
}

double VwapOrderExecutor::calculateVolumeWeightedPrice(const VwapOrderState& state) {
    const std::string& symbol = state.original_order->symbol;
    auto it = m_market_data_history.find(symbol);
    if (it == m_market_data_history.end() || it->second.empty()) {
        return 0.0;
    }

    const auto& history = it->second;
    uint32_t lookback = std::min(state.params->lookback_period, static_cast<uint32_t>(history.size()));

    double total_price_volume = 0.0;
    double total_volume = 0.0;

    for (uint32_t i = history.size() - lookback; i < history.size(); ++i) {
        const auto& data = history[i];
        double price = (data.best_bid + data.best_ask) / 2.0;
        total_price_volume += price * data.volume;
        total_volume += data.volume;
    }

    if (total_volume == 0) {
        return 0.0;
    }

    return total_price_volume / total_volume;
}

double VwapOrderExecutor::calculateOrderSize(const VwapOrderState& state) {
    // 这里简化处理，实际应用中应根据当前成交量和目标比例计算
    // 假设目标是在剩余时间内均匀分配成交量
    return state.remaining_quantity / 10; // 分成10份
}

void VwapOrderExecutor::updateMarketDataHistory(const market::MarketData& data) {
    const std::string& symbol = data.symbol;
    auto& history = m_market_data_history[symbol];

    // 添加新数据
    history.push_back(data);

    // 保持历史数据在合理范围内
    if (history.size() > 1000) { // 最多保留1000条
        history.erase(history.begin());
    }
}

// AdvancedOrderExecutionEngine 实现
AdvancedOrderExecutionEngine::AdvancedOrderExecutionEngine(core::TimeManager* time_manager)
    : m_time_manager(time_manager) {
}

AdvancedOrderExecutionEngine::~AdvancedOrderExecutionEngine() {
    // 清理资源
}

bool AdvancedOrderExecutionEngine::initialize() {
    // 创建各种订单执行器
    m_executors[AdvancedOrderType::ICEBERG] = std::make_shared<IcebergOrderExecutor>(m_time_manager);
    m_executors[AdvancedOrderType::TWAP] = std::make_shared<TwapOrderExecutor>(m_time_manager);
    m_executors[AdvancedOrderType::VWAP] = std::make_shared<VwapOrderExecutor>(m_time_manager);
    // 可以添加更多执行器...

    // 初始化所有执行器
    for (auto& [type, executor] : m_executors) {
        if (!executor->initialize()) {
            std::cerr << "Failed to initialize executor for type: " << static_cast<int>(type) << std::endl;
            return false;
        }
    }

    std::cout << "AdvancedOrderExecutionEngine initialized" << std::endl;
    return true;
}

uint64_t AdvancedOrderExecutionEngine::executeOrder(std::shared_ptr<Order> order) {
    if (!order || !order->advanced_params) {
        std::cerr << "Invalid advanced order" << std::endl;
        return 0;
    }

    AdvancedOrderType type = order->advanced_params->type;
    auto it = m_executors.find(type);
    if (it == m_executors.end()) {
        std::cerr << "No executor found for order type: " << static_cast<int>(type) << std::endl;
        return 0;
    }

    // 生成订单ID
    uint64_t order_id = m_time_manager->getCurrentTimestamp();
    order->id = order_id;

    // 记录订单类型映射
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_order_type_map[order_id] = type;
    }

    // 执行订单
    it->second->execute(order);
    return order_id;
}

void AdvancedOrderExecutionEngine::onMarketDataUpdate(const market::MarketData& data) {
    // 将市场数据更新传递给所有执行器
    for (auto& [type, executor] : m_executors) {
        executor->onMarketDataUpdate(data);
    }
}

bool AdvancedOrderExecutionEngine::cancelOrder(uint64_t order_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_order_type_map.find(order_id);
    if (it == m_order_type_map.end()) {
        return false;
    }

    AdvancedOrderType type = it->second;
    auto executor_it = m_executors.find(type);
    if (executor_it == m_executors.end()) {
        return false;
    }

    bool result = executor_it->second->cancelOrder(order_id);
    if (result) {
        m_order_type_map.erase(it);
    }
    return result;
}

void AdvancedOrderExecutionEngine::pauseAll() {
    for (auto& [type, executor] : m_executors) {
        executor->pause();
    }
    std::cout << "All advanced order executors paused" << std::endl;
}

void AdvancedOrderExecutionEngine::resumeAll() {
    for (auto& [type, executor] : m_executors) {
        executor->resume();
    }
    std::cout << "All advanced order executors resumed" << std::endl;
}

} // namespace execution
} // namespace hft