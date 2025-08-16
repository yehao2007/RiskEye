#ifndef INTELLIGENT_DECISION_SYSTEM_H
#define INTELLIGENT_DECISION_SYSTEM_H

#include "ai/NeuromorphicDecisionInterface.h"
#include "risk/QuantumRiskValidatorInterface.h"
#include "market_data/MarketData.h"
#include "execution/Order.h"
#include <vector>
#include <memory>

namespace hft {
namespace ai {

// 智能决策系统
class IntelligentDecisionSystem {
public:
    IntelligentDecisionSystem();
    ~IntelligentDecisionSystem();

    // 初始化系统
    bool initialize(const std::string& neuromorphic_weights_path = "quantum_trained_weights.npy");

    // 基于市场数据生成交易决策
    std::vector<execution::Order> generateDecisions(const market_data::MarketData& market_data);

    // 验证订单
    bool validateOrder(const execution::Order& order, const market_data::MarketData& market_data);

    // 获取上次决策的延迟(微秒)
    uint64_t getLastDecisionLatency() const;

    // 获取上次验证的延迟(微秒)
    uint64_t getLastValidationLatency() const;

private:
    std::unique_ptr<NeuromorphicDecisionInterface> neuromorphic_interface_;
    std::unique_ptr<risk::QuantumRiskValidatorInterface> quantum_validator_;
    uint64_t last_decision_latency_;
    uint64_t last_validation_latency_;
};

} // namespace ai
} // namespace hft

#endif // INTELLIGENT_DECISION_SYSTEM_H