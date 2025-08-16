#pragma once

#include "Strategy.h"
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include "execution/Order.h"
#include "market/MarketData.h"

namespace hft {
namespace strategy {

// 策略参数类型
struct StrategyParameter {
    std::string name;
    std::string type;
    double value;
    double min_value;
    double max_value;
    double default_value;
};

// 自定义策略基类
class CustomStrategy : public Strategy {
public:
    CustomStrategy(const std::string& name);
    ~CustomStrategy() override = default;

    // 初始化策略
    bool initialize(const std::vector<StrategyParameter>& parameters) override;

    // 设置参数
    bool setParameter(const std::string& name, double value);

    // 获取参数
    double getParameter(const std::string& name) const;

    // 获取所有参数
    std::vector<StrategyParameter> getParameters() const;

protected:
    std::unordered_map<std::string, double> m_parameters;
    std::vector<StrategyParameter> m_parameter_definitions;
};

// 策略工厂
class StrategyFactory {
public:
    StrategyFactory() = default;
    ~StrategyFactory() = default;

    // 注册策略创建函数
    void registerStrategy(const std::string& name,
                         std::function<StrategyPtr()> creator);

    // 创建策略
    StrategyPtr createStrategy(const std::string& name);

    // 获取所有可用策略名称
    std::vector<std::string> getAvailableStrategies() const;

private:
    std::unordered_map<std::string, std::function<StrategyPtr()>> m_strategy_creators;
};

} // namespace strategy
} // namespace hft