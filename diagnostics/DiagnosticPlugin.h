#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <map>
#include <chrono>
#include "../core/Types.h"

namespace hft {
namespace diagnostics {

// 诊断规则优先级
enum class RulePriority {
    Low,
    Medium,
    High,
    Critical
};

// 诊断规则类型
enum class RuleType {
    Hardware,      // 硬件相关规则
    Network,       // 网络相关规则
    Performance,   // 性能相关规则
    Security,      // 安全相关规则
    Custom        // 自定义规则
};

// 规则触发动作
struct RuleAction {
    enum class ActionType {
        Log,            // 记录日志
        Alert,          // 发送告警
        Callback,       // 回调函数
        AutoFix,        // 自动修复
        Shutdown        // 紧急关闭
    };

    ActionType type;
    std::string target;  // 目标（如告警接收者）
    std::function<void(const SystemState&)> callback;  // 回调函数
};

// 诊断规则接口
class IDiagnosticRule {
public:
    virtual ~IDiagnosticRule() = default;
    
    // 规则评估
    virtual bool evaluate(const SystemState& state) = 0;
    
    // 规则元数据
    virtual std::string getRuleName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual RuleType getType() const = 0;
    virtual RulePriority getPriority() const = 0;
    
    // 规则配置
    virtual void configure(const std::map<std::string, std::string>& params) = 0;
    
    // 获取规则触发动作
    virtual std::vector<RuleAction> getActions() const = 0;
    
    // 规则状态
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    
    // 规则统计
    virtual int getTriggerCount() const = 0;
    virtual std::chrono::system_clock::time_point getLastTriggerTime() const = 0;
};

// 插件配置
struct PluginConfig {
    std::string name;                    // 插件名称
    std::string version;                 // 插件版本
    std::string author;                  // 插件作者
    std::string description;             // 插件描述
    std::vector<std::string> dependencies;  // 依赖项
    std::map<std::string, std::string> parameters;  // 配置参数
};

// 插件统计信息
struct PluginStats {
    std::chrono::system_clock::time_point start_time;  // 启动时间
    uint64_t rules_evaluated;           // 规则评估次数
    uint64_t rules_triggered;           // 规则触发次数
    double avg_evaluation_time;         // 平均评估时间
    std::vector<std::string> active_rules;  // 活动规则列表
};

// 诊断插件接口
class IDiagnosticPlugin {
public:
    virtual ~IDiagnosticPlugin() = default;
    
    // 基本操作
    virtual void initialize(const PluginConfig& config) = 0;
    virtual void execute() = 0;
    virtual void shutdown() = 0;
    
    // 插件信息
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getAuthor() const = 0;
    virtual std::string getDescription() const = 0;
    
    // 规则管理
    virtual std::vector<std::unique_ptr<IDiagnosticRule>> getRules() = 0;
    virtual void addRule(std::unique_ptr<IDiagnosticRule> rule) = 0;
    virtual void removeRule(const std::string& ruleName) = 0;
    virtual IDiagnosticRule* findRule(const std::string& ruleName) = 0;
    
    // 配置管理
    virtual void configure(const std::map<std::string, std::string>& params) = 0;
    virtual std::map<std::string, std::string> getConfiguration() const = 0;
    
    // 状态查询
    virtual bool isEnabled() const = 0;
    virtual PluginStats getStatistics() const = 0;
    
    // 事件处理
    virtual void onRuleTriggered(const IDiagnosticRule* rule, 
                                const SystemState& state) = 0;
    virtual void onError(const std::string& error) = 0;
};

// 插件事件监听器
class IPluginEventListener {
public:
    virtual ~IPluginEventListener() = default;
    virtual void onPluginLoaded(const std::string& pluginName) = 0;
    virtual void onPluginUnloaded(const std::string& pluginName) = 0;
    virtual void onRuleTriggered(const std::string& pluginName,
                                const std::string& ruleName,
                                const SystemState& state) = 0;
    virtual void onError(const std::string& pluginName,
                        const std::string& error) = 0;
};

// 插件管理器
class PluginManager {
public:
    // 插件生命周期管理
    void loadPlugin(std::unique_ptr<IDiagnosticPlugin> plugin,
                   const PluginConfig& config);
    void unloadPlugin(const std::string& pluginName);
    void enablePlugin(const std::string& pluginName);
    void disablePlugin(const std::string& pluginName);
    
    // 插件查询
    std::vector<std::string> getLoadedPlugins() const;
    IDiagnosticPlugin* getPlugin(const std::string& pluginName);
    PluginStats getPluginStats(const std::string& pluginName) const;
    
    // 规则管理
    void addRuleToPlugin(const std::string& pluginName,
                        std::unique_ptr<IDiagnosticRule> rule);
    void removeRuleFromPlugin(const std::string& pluginName,
                            const std::string& ruleName);
    std::vector<std::string> getPluginRules(const std::string& pluginName) const;
    
    // 配置管理
    void configurePlugin(const std::string& pluginName,
                        const std::map<std::string, std::string>& params);
    PluginConfig getPluginConfig(const std::string& pluginName) const;
    
    // 事件监听
    void addEventListener(std::shared_ptr<IPluginEventListener> listener);
    void removeEventListener(std::shared_ptr<IPluginEventListener> listener);
    
    // 批量操作
    void executeAll();
    void shutdownAll();
    
private:
    std::map<std::string, std::unique_ptr<IDiagnosticPlugin>> plugins_;
    std::vector<std::shared_ptr<IPluginEventListener>> listeners_;
    std::map<std::string, PluginConfig> configs_;
    std::map<std::string, PluginStats> stats_;
    
    // 远程监控和可视化组件
    std::unique_ptr<RemoteMonitorServer> remote_monitor_;
    std::unique_ptr<IDataVisualizer> visualizer_;
    std::map<std::string, std::unique_ptr<IDiagnosticDataSource>> data_sources_;
    VisualizationConfig viz_config_;
};

} // namespace diagnostics
} // namespace hft
