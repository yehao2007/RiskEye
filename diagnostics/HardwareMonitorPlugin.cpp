#include "HardwareMonitorPlugin.h"
#include "../core/Logger.h"
#include <thread>
#include <chrono>

namespace hft {
namespace diagnostics {

void HardwareMonitorPlugin::initialize(const PluginConfig& config) {
    // 配置插件
    config_ = config.parameters;
    enabled_ = true;
    
    // 初始化统计信息
    stats_.start_time = std::chrono::system_clock::now();
    stats_.rules_evaluated = 0;
    stats_.rules_triggered = 0;
    stats_.avg_evaluation_time = 0.0;
    
    // 添加默认规则
    auto cpu_rule = std::make_unique<CPUTemperatureRule>();
    addRule(std::move(cpu_rule));
    
    Logger::info("HardwareMonitor plugin initialized");
}

void HardwareMonitorPlugin::execute() {
    if (!enabled_) return;
    
    try {
        // 监控硬件状态
        monitorCPU();
        monitorMemory();
        monitorDisk();
        monitorNetwork();
        
        // 更新统计信息
        stats_.rules_evaluated += rules_.size();
        
    } catch (const std::exception& e) {
        Logger::error("HardwareMonitor execution failed: {}", e.what());
        onError(e.what());
    }
}

void HardwareMonitorPlugin::shutdown() {
    enabled_ = false;
    rules_.clear();
    Logger::info("HardwareMonitor plugin shut down");
}

std::vector<std::unique_ptr<IDiagnosticRule>> 
HardwareMonitorPlugin::getRules() {
    std::vector<std::unique_ptr<IDiagnosticRule>> result;
    for (const auto& [name, rule] : rules_) {
        // 创建规则副本
        auto new_rule = std::make_unique<CPUTemperatureRule>();
        new_rule->configure(rule->getConfiguration());
        result.push_back(std::move(new_rule));
    }
    return result;
}

void HardwareMonitorPlugin::addRule(
    std::unique_ptr<IDiagnosticRule> rule) {
    
    if (!rule) {
        throw std::invalid_argument("Rule cannot be null");
    }
    
    const auto& name = rule->getRuleName();
    if (rules_.find(name) != rules_.end()) {
        throw std::runtime_error("Rule " + name + " already exists");
    }
    
    rules_[name] = std::move(rule);
    stats_.active_rules.push_back(name);
}

void HardwareMonitorPlugin::removeRule(const std::string& ruleName) {
    auto it = rules_.find(ruleName);
    if (it != rules_.end()) {
        rules_.erase(it);
        auto& active_rules = stats_.active_rules;
        active_rules.erase(
            std::remove(active_rules.begin(), 
                       active_rules.end(), 
                       ruleName),
            active_rules.end());
    }
}

IDiagnosticRule* HardwareMonitorPlugin::findRule(
    const std::string& ruleName) {
    
    auto it = rules_.find(ruleName);
    return it != rules_.end() ? it->second.get() : nullptr;
}

void HardwareMonitorPlugin::configure(
    const std::map<std::string, std::string>& params) {
    
    config_ = params;
    
    // 配置所有规则
    for (auto& [_, rule] : rules_) {
        rule->configure(params);
    }
}

std::map<std::string, std::string> 
HardwareMonitorPlugin::getConfiguration() const {
    return config_;
}

void HardwareMonitorPlugin::onRuleTriggered(
    const IDiagnosticRule* rule,
    const SystemState& state) {
    
    if (!rule) return;
    
    // 更新统计信息
    stats_.rules_triggered++;
    
    // 获取规则动作并执行
    for (const auto& action : rule->getActions()) {
        try {
            switch (action.type) {
                case RuleAction::ActionType::Log:
                    Logger::warn("Rule {} triggered: {}",
                               rule->getRuleName(),
                               rule->getDescription());
                    break;
                    
                case RuleAction::ActionType::Alert:
                    // 发送告警
                    // TODO: 实现告警发送逻辑
                    break;
                    
                case RuleAction::ActionType::Callback:
                    if (action.callback) {
                        action.callback(state);
                    }
                    break;
                    
                default:
                    break;
            }
        } catch (const std::exception& e) {
            Logger::error("Failed to execute action for rule {}: {}",
                         rule->getRuleName(), e.what());
        }
    }
}

void HardwareMonitorPlugin::onError(const std::string& error) {
    Logger::error("HardwareMonitor error: {}", error);
}

void HardwareMonitorPlugin::monitorCPU() {
    // TODO: 实现CPU监控逻辑
    // 1. 收集CPU信息
    // 2. 评估规则
    // 3. 触发必要的动作
}

void HardwareMonitorPlugin::monitorMemory() {
    // TODO: 实现内存监控逻辑
}

void HardwareMonitorPlugin::monitorDisk() {
    // TODO: 实现磁盘监控逻辑
}

void HardwareMonitorPlugin::monitorNetwork() {
    // TODO: 实现网络监控逻辑
}

// CPUTemperatureRule 实现
CPUTemperatureRule::CPUTemperatureRule(double threshold)
    : threshold_(threshold) {
}

bool CPUTemperatureRule::evaluate(const SystemState& state) {
    if (!enabled_) return false;
    
    // 检查CPU温度
    if (state.hardware_health.cpu.temperature > threshold_) {
        trigger_count_++;
        last_trigger_time_ = std::chrono::system_clock::now();
        return true;
    }
    
    return false;
}

void CPUTemperatureRule::configure(
    const std::map<std::string, std::string>& params) {
    
    auto it = params.find("cpu_temperature_threshold");
    if (it != params.end()) {
        try {
            threshold_ = std::stod(it->second);
        } catch (const std::exception& e) {
            Logger::error("Failed to parse CPU temperature threshold: {}",
                         e.what());
        }
    }
}

std::vector<RuleAction> CPUTemperatureRule::getActions() const {
    std::vector<RuleAction> actions;
    
    // 添加日志动作
    actions.push_back({
        .type = RuleAction::ActionType::Log,
        .target = "system.log"
    });
    
    // 添加告警动作
    actions.push_back({
        .type = RuleAction::ActionType::Alert,
        .target = "admin@example.com"
    });
    
    // 添加回调动作
    actions.push_back({
        .type = RuleAction::ActionType::Callback,
        .callback = [](const SystemState& state) {
            Logger::warn("CPU temperature is too high: {}°C",
                        state.hardware_health.cpu.temperature);
        }
    });
    
    return actions;
}

} // namespace diagnostics
} // namespace hft
