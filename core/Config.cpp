#include "Config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace hft {
namespace core {

Config::Config() {
}

Config::~Config() {
}

bool Config::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;

        // 加载系统配置
        if (j.contains("system")) {
            const auto& system_config = j["system"];
            m_systemConfig.logLevel = system_config.value("log_level", "INFO");
            m_systemConfig.maxThreads = system_config.value("max_threads", 4);
            m_systemConfig.memoryLimitMB = system_config.value("memory_limit_mb", 4096);
        }

        // 加载网络配置
        if (j.contains("network")) {
            const auto& network_config = j["network"];
            m_networkConfig.marketDataFeedPort = network_config.value("market_data_feed_port", 5555);
            m_networkConfig.orderRoutingPort = network_config.value("order_routing_port", 5556);
            m_networkConfig.lowLatencyMode = network_config.value("low_latency_mode", true);
        }

        // 加载执行配置
        if (j.contains("execution")) {
            const auto& execution_config = j["execution"];
            m_executionConfig.maxOrderSize = execution_config.value("max_order_size", 1000);
            m_executionConfig.defaultTimeoutMs = execution_config.value("default_timeout_ms", 5000);
            m_executionConfig.slippageTolerance = execution_config.value("slippage_tolerance", 0.1);
        }

        // 加载风险配置
        if (j.contains("risk")) {
            const auto& risk_config = j["risk"];
            m_riskConfig.maxPosition = risk_config.value("max_position", 10000);
            m_riskConfig.dailyLossLimit = risk_config.value("daily_loss_limit", 50000);
            m_riskConfig.riskThreshold = risk_config.value("risk_threshold", 0.05);
        }

        // 加载策略配置
        if (j.contains("strategy")) {
            const auto& strategy_config = j["strategy"];
            m_strategyConfig.defaultStrategy = strategy_config.value("default_strategy", "trend_following");
            m_strategyConfig.backtestingMode = strategy_config.value("backtesting_mode", false);
            m_strategyConfig.capital = strategy_config.value("capital", 1000000.0);
        }

        std::cout << "Config loaded successfully from " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        return false;
    }
}

bool Config::saveToFile(const std::string& filename) const {
    try {
        nlohmann::json j;

        // 保存系统配置
        j["system"]["log_level"] = m_systemConfig.logLevel;
        j["system"]["max_threads"] = m_systemConfig.maxThreads;
        j["system"]["memory_limit_mb"] = m_systemConfig.memoryLimitMB;

        // 保存网络配置
        j["network"]["market_data_feed_port"] = m_networkConfig.marketDataFeedPort;
        j["network"]["order_routing_port"] = m_networkConfig.orderRoutingPort;
        j["network"]["low_latency_mode"] = m_networkConfig.lowLatencyMode;

        // 保存执行配置
        j["execution"]["max_order_size"] = m_executionConfig.maxOrderSize;
        j["execution"]["default_timeout_ms"] = m_executionConfig.defaultTimeoutMs;
        j["execution"]["slippage_tolerance"] = m_executionConfig.slippageTolerance;

        // 保存风险配置
        j["risk"]["max_position"] = m_riskConfig.maxPosition;
        j["risk"]["daily_loss_limit"] = m_riskConfig.dailyLossLimit;
        j["risk"]["risk_threshold"] = m_riskConfig.riskThreshold;

        // 保存策略配置
        j["strategy"]["default_strategy"] = m_strategyConfig.defaultStrategy;
        j["strategy"]["backtesting_mode"] = m_strategyConfig.backtestingMode;
        j["strategy"]["capital"] = m_strategyConfig.capital;

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << filename << std::endl;
            return false;
        }

        file << j.dump(4); // 格式化输出，缩进4个空格
        std::cout << "Config saved successfully to " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save config: " << e.what() << std::endl;
        return false;
    }
}

// 获取配置的方法
const SystemConfig& Config::getSystemConfig() const {
    return m_systemConfig;
}

const NetworkConfig& Config::getNetworkConfig() const {
    return m_networkConfig;
}

const ExecutionConfig& Config::getExecutionConfig() const {
    return m_executionConfig;
}

const RiskConfig& Config::getRiskConfig() const {
    return m_riskConfig;
}

const StrategyConfig& Config::getStrategyConfig() const {
    return m_strategyConfig;
}

// 设置配置的方法
void Config::setSystemConfig(const SystemConfig& config) {
    m_systemConfig = config;
}

void Config::setNetworkConfig(const NetworkConfig& config) {
    m_networkConfig = config;
}

void Config::setExecutionConfig(const ExecutionConfig& config) {
    m_executionConfig = config;
}

void Config::setRiskConfig(const RiskConfig& config) {
    m_riskConfig = config;
}

void Config::setStrategyConfig(const StrategyConfig& config) {
    m_strategyConfig = config;
}

} // namespace core
} // namespace hft