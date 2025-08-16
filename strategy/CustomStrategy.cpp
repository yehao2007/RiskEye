#include "CustomStrategy.h"
#include <stdexcept>

namespace hft {
namespace strategy {

CustomStrategy::CustomStrategy(const std::string& name)
    : Strategy(name) {
}

bool CustomStrategy::initialize(const std::vector<StrategyParameter>& parameters) {
    m_parameter_definitions = parameters;

    // 设置默认参数值
    for (const auto& param : parameters) {
        m_parameters[param.name] = param.default_value;
    }

    return true;
}

bool CustomStrategy::setParameter(const std::string& name, double value) {
    auto it = m_parameters.find(name);
    if (it == m_parameters.end()) {
        return false;
    }

    // 检查参数范围
    for (const auto& param : m_parameter_definitions) {
        if (param.name == name) {
            if (value < param.min_value || value > param.max_value) {
                return false;
            }
            break;
        }
    }

    m_parameters[name] = value;
    return true;
}

double CustomStrategy::getParameter(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it == m_parameters.end()) {
        throw std::runtime_error("Parameter not found: " + name);
    }
    return it->second;
}

std::vector<StrategyParameter> CustomStrategy::getParameters() const {
    return m_parameter_definitions;
}

// 策略工厂实现
void StrategyFactory::registerStrategy(const std::string& name,
                                      std::function<StrategyPtr()> creator) {
    m_strategy_creators[name] = creator;
}

StrategyPtr StrategyFactory::createStrategy(const std::string& name) {
    auto it = m_strategy_creators.find(name);
    if (it == m_strategy_creators.end()) {
        return nullptr;
    }
    return it->second();
}

std::vector<std::string> StrategyFactory::getAvailableStrategies() const {
    std::vector<std::string> strategies;
    for (const auto& pair : m_strategy_creators) {
        strategies.push_back(pair.first);
    }
    return strategies;
}

} // namespace strategy
} // namespace hft