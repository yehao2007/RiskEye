#ifndef QUANTUM_RISK_VALIDATOR_INTERFACE_H
#define QUANTUM_RISK_VALIDATOR_INTERFACE_H

#include "execution/Order.h"
#include "market_data/MarketData.h"
#include <string>
#include <memory>

namespace hft {
namespace risk {

struct RiskMetrics {
    float volatility;
    float liquidity;
    float correlation;
    float margin_requirement;
};

// 量子风险验证器接口
class QuantumRiskValidatorInterface {
public:
    QuantumRiskValidatorInterface(float risk_threshold = -1.0f);
    ~QuantumRiskValidatorInterface();

    // 初始化接口
    bool initialize();

    // 验证订单
    bool validateOrder(const execution::Order& order, const market_data::MarketData& market_data, const RiskMetrics& risk_metrics);

    // 批量验证订单
    std::vector<bool> batchValidateOrders(const std::vector<execution::Order>& orders, const market_data::MarketData& market_data, const RiskMetrics& risk_metrics);

    // 获取上次验证的延迟(微秒)
    uint64_t getLastValidationLatency() const;

private:
    // 实现细节封装
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace risk
} // namespace hft

#endif // QUANTUM_RISK_VALIDATOR_INTERFACE_H