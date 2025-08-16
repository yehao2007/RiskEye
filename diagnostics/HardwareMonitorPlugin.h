#pragma once

#include "DiagnosticPlugin.h"
#include <map>
#include <chrono>

namespace hft {
namespace diagnostics {

// 硬件监控插件
class HardwareMonitorPlugin : public IDiagnosticPlugin {
public:
    void initialize(const PluginConfig& config) override;
    void execute() override;
    void shutdown() override;
    
    std::string getName() const override { return "HardwareMonitor"; }
    std::string getVersion() const override { return "1.0.0"; }
    std::string getAuthor() const override { return "HFT Team"; }
    std::string getDescription() const override {
        return "监控系统硬件状态，包括CPU、内存、磁盘和网络";
    }
    
    std::vector<std::unique_ptr<IDiagnosticRule>> getRules() override;
    void addRule(std::unique_ptr<IDiagnosticRule> rule) override;
    void removeRule(const std::string& ruleName) override;
    IDiagnosticRule* findRule(const std::string& ruleName) override;
    
    void configure(const std::map<std::string, std::string>& params) override;
    std::map<std::string, std::string> getConfiguration() const override;
    
    bool isEnabled() const override { return enabled_; }
    PluginStats getStatistics() const override { return stats_; }
    
    void onRuleTriggered(const IDiagnosticRule* rule,
                         const SystemState& state) override;
    void onError(const std::string& error) override;

private:
    bool enabled_{false};
    PluginStats stats_;
    std::map<std::string, std::string> config_;
    std::map<std::string, std::unique_ptr<IDiagnosticRule>> rules_;
    
    // 内部方法
    void monitorCPU();
    void monitorMemory();
    void monitorDisk();
    void monitorNetwork();
};

// CPU温度监控规则
class CPUTemperatureRule : public IDiagnosticRule {
public:
    explicit CPUTemperatureRule(double threshold = 80.0);
    
    bool evaluate(const SystemState& state) override;
    std::string getRuleName() const override { return "CPUTemperature"; }
    std::string getDescription() const override {
        return "监控CPU温度，超过阈值时触发告警";
    }
    
    RuleType getType() const override { return RuleType::Hardware; }
    RulePriority getPriority() const override { return RulePriority::High; }
    
    void configure(const std::map<std::string, std::string>& params) override;
    std::vector<RuleAction> getActions() const override;
    
    bool isEnabled() const override { return enabled_; }
    void setEnabled(bool enabled) override { enabled_ = enabled; }
    
    int getTriggerCount() const override { return trigger_count_; }
    std::chrono::system_clock::time_point getLastTriggerTime() const override {
        return last_trigger_time_;
    }

private:
    double threshold_;
    bool enabled_{true};
    int trigger_count_{0};
    std::chrono::system_clock::time_point last_trigger_time_;
};

} // namespace diagnostics
} // namespace hft
