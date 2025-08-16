#include "StrategyFactory.h"
#include <iostream>

namespace hft {
namespace strategy {

std::unordered_map<std::string, std::function<std::shared_ptr<Strategy>()>> StrategyFactory::m_strategy_creators;

void StrategyFactory::registerStrategy(const std::string& name, 
                                      std::function<std::shared_ptr<Strategy>()> creator) {
    m_strategy_creators[name] = creator;
    std::cout << "Strategy registered: " << name << std::endl;
}

std::shared_ptr<Strategy> StrategyFactory::createStrategy(const std::string& name) {
    auto it = m_strategy_creators.find(name);
    if (it != m_strategy_creators.end()) {
        return it->second();
    }
    std::cerr << "Strategy not found: " << name << std::endl;
    return nullptr;
}

std::vector<std::string> StrategyFactory::getAvailableStrategies() {
    std::vector<std::string> strategies;
    for (const auto& pair : m_strategy_creators) {
        strategies.push_back(pair.first);
    }
    return strategies;
}

} // namespace strategy
} // namespace hft