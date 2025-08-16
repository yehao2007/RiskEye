#ifndef EVOLUTION_ENGINE_H
#define EVOLUTION_ENGINE_H

#include "core/System.h"
#include "ai/IntelligentDecisionSystem.h"
#include "risk/AdvancedRiskManager.h"
#include <memory>
#include <vector>
#include <tuple>

namespace hft {
namespace evolution {

class EvolutionEngine {
public:
    EvolutionEngine(core::System* system,
                    ai::IntelligentDecisionSystem* decision_system,
                    risk::AdvancedRiskManager* risk_manager);

    // 初始化进化引擎
    bool initialize(const std::string& config_path);

    // 启动自适应学习过程
    void startLearning();

    // 停止学习过程
    void stopLearning();

    // 获取当前学习状态
    bool isLearning() const;

    // 设置学习速率
    void setLearningRate(float rate);

private:
    core::System* system_;
    ai::IntelligentDecisionSystem* decision_system_;
    risk::AdvancedRiskManager* risk_manager_;
    std::atomic<bool> is_learning_;
    std::thread learning_thread_;
    float learning_rate_;

    // 学习循环
    void learningLoop();

    // 评估当前系统性能
    float evaluatePerformance();

    // 优化系统参数
    void optimizeParameters(const std::vector<float>& performance_metrics);

    // 保存优化后的参数
    void saveOptimizedParameters();
};

} // namespace evolution
} // namespace hft

#endif // EVOLUTION_ENGINE_H
#include <string>
#include <functional>
#include <memory>

// 自进化代码架构接口
class EvolutionEngine {
public:
    virtual ~EvolutionEngine() = default;

    // 进化策略代码
    virtual std::string evolveStrategy(const std::string& parentCode) = 0;

    // 热部署新策略
    virtual bool hotDeploy(const std::string& newCode) = 0;

    // 设置适应度评估函数
    virtual void setFitnessFunction(std::function<double(const std::string&)> fitnessFunc) = 0;
};

// 时空压缩风控接口
class TemporalSpatialRiskController {
public:
    virtual ~TemporalSpatialRiskController() = default;

    // 并行风险扫描
    virtual bool riskScan(const std::vector<Order*>& orders) = 0;

    // 更新风险限制
    virtual void updateRiskLimits(const RiskLimits& limits) = 0;

    // 获取风控状态
    virtual RiskState getRiskState() const = 0;
};