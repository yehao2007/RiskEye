#ifndef NEUROMORPHIC_DECISION_INTERFACE_H
#define NEUROMORPHIC_DECISION_INTERFACE_H

#include <vector>
#include <string>
#include <memory>

namespace hft {
namespace ai {

// 神经形态决策接口
class NeuromorphicDecisionInterface {
public:
    NeuromorphicDecisionInterface(const std::string& weights_path = "quantum_trained_weights.npy");
    ~NeuromorphicDecisionInterface();

    // 初始化接口
    bool initialize();

    // 基于市场数据做出交易决策
    // 输入: 市场数据向量
    // 输出: 交易动作 (0: 无动作, 1: 买入, 2: 卖出)
    int makeDecision(const std::vector<float>& market_data);

    // 获取上次决策的延迟(微秒)
    uint64_t getLastDecisionLatency() const;

private:
    // 实现细节封装
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace ai
} // namespace hft

#endif // NEUROMORPHIC_DECISION_INTERFACE_H