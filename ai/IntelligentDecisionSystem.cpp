#include "IntelligentDecisionSystem.h"
#include "execution/Order.h"
#include "risk/QuantumRiskValidatorInterface.h"
#include <iostream>
#include <chrono>

namespace hft {
namespace ai {

IntelligentDecisionSystem::IntelligentDecisionSystem()
    : last_decision_latency_(0), last_validation_latency_(0) {
}

IntelligentDecisionSystem::~IntelligentDecisionSystem() = default;

bool IntelligentDecisionSystem::initialize(const std::string& neuromorphic_weights_path) {
    // 初始化神经形态决策接口
    neuromorphic_interface_ = std::make_unique<NeuromorphicDecisionInterface>(neuromorphic_weights_path);
    if (!neuromorphic_interface_->initialize()) {
        std::cerr << "Failed to initialize neuromorphic decision interface" << std::endl;
        return false;
    }

    // 初始化量子风险验证器接口
    quantum_validator_ = std::make_unique<risk::QuantumRiskValidatorInterface>();
    if (!quantum_validator_->initialize()) {
        std::cerr << "Failed to initialize quantum risk validator interface" << std::endl;
        return false;
    }

    return true;
}

std::vector<execution::Order> IntelligentDecisionSystem::generateDecisions(const market_data::MarketData& market_data) {
    std::vector<execution::Order> orders;

    // 提取市场数据特征
    std::vector<float> features;
    // (省略特征提取逻辑...)

    // 使用神经形态决策接口生成决策
    int action = neuromorphic_interface_->makeDecision(features);
    last_decision_latency_ = neuromorphic_interface_->getLastDecisionLatency();

    // 根据决策生成订单
    execution::Order order;
    switch (action) {
        case 1: // 买入
            order.type = execution::OrderType::MARKET_BUY;
            order.size = 100; // 示例订单大小
            order.symbol = "AAPL"; // 示例股票代码
            orders.push_back(order);
            break;
        case 2: // 卖出
            order.type = execution::OrderType::MARKET_SELL;
            order.size = 100;
            order.symbol = "AAPL";
            orders.push_back(order);
            break;
        default: // 无动作
            break;
    }

    return orders;
}

bool IntelligentDecisionSystem::validateOrder(const execution::Order& order, const market_data::MarketData& market_data) {
    // 准备风险指标
    risk::RiskMetrics risk_metrics;
    // (省略风险指标计算逻辑...)

    // 使用量子风险验证器验证订单
    auto start = std::chrono::high_resolution_clock::now();
    bool is_valid = quantum_validator_->validateOrder(order, market_data, risk_metrics);
    auto end = std::chrono::high_resolution_clock::now();
    last_validation_latency_ = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    return is_valid;
}

uint64_t IntelligentDecisionSystem::getLastDecisionLatency() const {
    return last_decision_latency_;
}

uint64_t IntelligentDecisionSystem::getLastValidationLatency() const {
    return last_validation_latency_;
}

} // namespace ai
} // namespace hft