#include "StrategyManager.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>
#include "TrendFollowingStrategy.h"

namespace hft {
namespace strategy {

using json = nlohmann::json;

StrategyManager::StrategyManager() {
    m_eventLoop = std::make_shared<core::EventLoop>();
}

StrategyManager::~StrategyManager() {
    stopAllStrategies();
}

bool StrategyManager::initialize(const std::string& configPath) {
    // 从配置文件加载策略
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << configPath << std::endl;
            return false;
        }

        json config;
        file >> config;

        // 加载策略
        if (config.contains("strategies")) {
            for (const auto& strategyConfig : config["strategies"]) {
                std::string name = strategyConfig.value("name", "");
                std::string type = strategyConfig.value("type", "");
                std::string strategyConfigPath = strategyConfig.value("config_path", "");

                if (name.empty() || type.empty()) {
                    std::cerr << "Invalid strategy configuration: missing name or type" << std::endl;
                    continue;
                }

                std::shared_ptr<Strategy> strategy;
                if (type == "trend_following") {
                    strategy = std::make_shared<TrendFollowingStrategy>(name);
                }
                // 可以添加其他类型的策略
                else {
                    std::cerr << "Unknown strategy type: " << type << std::endl;
                    continue;
                }

                // 初始化策略
                if (!strategy->initialize(strategyConfigPath)) {
                    std::cerr << "Failed to initialize strategy: " << name << std::endl;
                    continue;
                }

                // 添加策略
                if (!addStrategy(strategy)) {
                    std::cerr << "Failed to add strategy: " << name << std::endl;
                    continue;
                }
            }
        }

        // 启动事件循环
        m_eventLoop->start();

        // 安排定期执行所有策略的任务
        m_eventLoop->scheduleAtFixedRate(
            std::bind(&StrategyManager::executeAllStrategies, this),
            0, // 立即开始
            100000); // 100ms执行一次

        std::cout << "StrategyManager initialized with " << m_strategies.size() << " strategies" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize StrategyManager: " << e.what() << std::endl;
        return false;
    }
}

void StrategyManager::startAllStrategies() {
    for (auto& [name, strategy] : m_strategies) {
        strategy->start();
    }
    std::cout << "All strategies started" << std::endl;
}

void StrategyManager::stopAllStrategies() {
    for (auto& [name, strategy] : m_strategies) {
        strategy->stop();
    }
    m_eventLoop->stop();
    std::cout << "All strategies stopped" << std::endl;
}

bool StrategyManager::addStrategy(std::shared_ptr<Strategy> strategy) {
    if (!strategy) {
        return false;
    }

    const std::string& name = strategy->getName();
    if (m_strategies.find(name) != m_strategies.end()) {
        std::cerr << "Strategy already exists: " << name << std::endl;
        return false;
    }

    m_strategies[name] = strategy;
    std::cout << "Strategy added: " << name << std::endl;
    return true;
}

std::shared_ptr<Strategy> StrategyManager::getStrategy(const std::string& name) const {
    auto it = m_strategies.find(name);
    if (it != m_strategies.end()) {
        return it->second;
    }
    return nullptr;
}

bool StrategyManager::removeStrategy(const std::string& name) {
    auto it = m_strategies.find(name);
    if (it == m_strategies.end()) {
        std::cerr << "Strategy not found: " << name << std::endl;
        return false;
    }

    it->second->stop();
    m_strategies.erase(it);
    std::cout << "Strategy removed: " << name << std::endl;
    return true;
}

void StrategyManager::registerMarketDataCallback() {
    // 在实际应用中，这里会注册市场数据回调
    // 示例：
    // m_marketDataFeed->registerCallback(
    //     std::bind(&StrategyManager::onMarketData, this, std::placeholders::_1)
    // );
}

void StrategyManager::registerPatternCallback() {
    // 在实际应用中，这里会注册模式识别回调
    // 示例：
    // m_patternRecognizer->registerPatternCallback(
    //     std::bind(&StrategyManager::onMarketPattern, this, std::placeholders::_1)
    // );
}

void StrategyManager::executeAllStrategies() {
    try {
        for (auto& [name, strategy] : m_strategies) {
            if (strategy->isActive()) {
                std::vector<execution::Order> orders = strategy->execute();
                if (!orders.empty() && m_orderExecution) {
                    for (const auto& order : orders) {
                        m_orderExecution->executeOrder(order);
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error executing strategies: " << e.what() << std::endl;
    }
}

} // namespace strategy
} // namespace hft