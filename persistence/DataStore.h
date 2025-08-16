#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "core/Configuration.h"
#include "market/MarketData.h"
#include "execution/Order.h"
#include "risk/RiskMetrics.h"

namespace hft {
namespace persistence {

class DataStore {
public:
    DataStore(const core::Configuration& config);
    ~DataStore();

    // 初始化数据存储
    bool initialize();
    
    // 保存市场数据
    bool saveMarketData(const market::MarketData& data);
    
    // 批量保存市场数据
    bool saveMarketDataBatch(const std::vector<market::MarketData>& data);
    
    // 查询市场数据
    std::vector<market::MarketData> queryMarketData(const std::string& symbol,
                                                  uint64_t start_time,
                                                  uint64_t end_time);
    
    // 保存订单
    bool saveOrder(const execution::Order& order);
    
    // 更新订单状态
    bool updateOrderStatus(const std::string& order_id, execution::OrderStatus status);
    
    // 查询订单
    std::vector<execution::Order> queryOrders(const std::string& symbol,
                                            uint64_t start_time,
                                            uint64_t end_time);
    
    // 保存风险指标
    bool saveRiskMetrics(const risk::RiskMetrics& metrics);
    
    // 查询风险指标
    std::vector<risk::RiskMetrics> queryRiskMetrics(uint64_t start_time,
                                                  uint64_t end_time);
    
    // 关闭数据存储
    void close();

private:
    core::Configuration m_config;
    std::string m_connection_string;
    bool m_initialized;
    std::mutex m_mutex;

    // 实现具体存储逻辑的内部方法
    bool connect();
    bool createTables();
};

} // namespace persistence
} // namespace hft