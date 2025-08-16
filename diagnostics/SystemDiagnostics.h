#pragma once

#include "DiagnosticTool.h"
#include "DiagnosticCore.h"
#include "VisualizationAndMonitoring.h"
#include "DiagnosticPlugin.h"
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <string>

namespace hft {
namespace diagnostics {

// 诊断系统配置
struct DiagnosticSystemConfig {
    // 基本配置
    std::chrono::milliseconds update_interval{1000};  // 更新间隔
    size_t max_history_size{10000};                   // 最大历史记录数
    bool enable_auto_repair{true};                    // 启用自动修复
    bool enable_predictions{true};                    // 启用预测功能
    
    // 阈值配置
    struct {
        double latency_warning{100.0};      // 延迟警告阈值(ms)
        double latency_critical{500.0};     // 延迟严重阈值(ms)
        double cpu_warning{80.0};           // CPU警告阈值(%)
        double cpu_critical{95.0};          // CPU严重阈值(%)
        double memory_warning{85.0};        // 内存警告阈值(%)
        double memory_critical{95.0};       // 内存严重阈值(%)
    } thresholds;
    
    // 文件路径配置
    std::string log_directory{"./logs"};
    std::string report_directory{"./reports"};
    std::string config_file{"./config/diagnostics.json"};
};

// 主诊断系统类
class SystemDiagnostics {
public:
    // 构造函数和析构函数
    explicit SystemDiagnostics(const DiagnosticSystemConfig& config = {});
    ~SystemDiagnostics();

    // 系统控制
    void initialize();
    void startDiagnostics();
    void stopDiagnostics();
    bool isRunning() const { return is_running_; }
    
    // 配置管理
    void configure(const DiagnosticSystemConfig& config);
    DiagnosticSystemConfig getConfiguration() const;
    void loadConfiguration(const std::string& config_file);
    void saveConfiguration(const std::string& config_file) const;

    // 插件管理
    void loadPlugin(std::unique_ptr<IDiagnosticPlugin> plugin,
                   const PluginConfig& config);
    void unloadPlugin(const std::string& plugin_name);
    void enablePlugin(const std::string& plugin_name);
    void disablePlugin(const std::string& plugin_name);
    std::vector<std::string> getLoadedPlugins() const;
    PluginStats getPluginStats(const std::string& plugin_name) const;

    // 自定义规则管理
    void addCustomRule(std::unique_ptr<IDiagnosticRule> rule);
    void removeRule(const std::string& rule_name);
    void enableRule(const std::string& rule_name);
    void disableRule(const std::string& rule_name);
    std::vector<std::string> getActiveRules() const;

    // 远程监控
    void startRemoteMonitor(const RemoteMonitorConfig& config);
    void stopRemoteMonitor();
    bool isRemoteMonitorRunning() const;
    RemoteMonitorStats getRemoteMonitorStats() const;

    // 可视化
    void configureVisualization(const VisualizationConfig& config);
    std::string generateDashboard(const std::vector<std::string>& metrics = {});
    std::string generateChart(const std::string& metric, 
                            const std::string& chart_type = "line");
    std::string exportDashboardAsHTML() const;
    std::string exportDashboardAsJSON() const;

    // 告警配置
    void configureAlerts(const AlertConfig& config);
    void addAlertRule(const AlertConfig::AlertRule& rule);
    void removeAlertRule(const std::string& metric);
    void enableAlerts();
    void disableAlerts();
    std::vector<std::string> getActiveAlerts() const;

    // 数据源管理
    void addDataSource(std::unique_ptr<IDiagnosticDataSource> source);
    void removeDataSource(const std::string& source_name);
    std::vector<std::string> getAvailableDataSources() const;
    json getDataSourceData(const std::string& source_name) const;

    // 诊断功能
    SystemState getCurrentSystemState() const;
    DiagnosticReport generateReport();
    DiagnosticMetrics getMetrics() const;
    bool isSystemHealthy() const;
    double getSystemHealthScore() const;

    // 趋势分析
    PerformanceTrend getPerformanceTrend(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const;
    std::vector<std::string> predictBottlenecks() const;
    std::map<std::string, double> getSLACompliance() const;

    // 故障检测和修复
    std::vector<DiagnosticIssue> detectIssues();
    std::vector<RepairStrategy> getRepairStrategies(const DiagnosticIssue& issue);
    bool executeRepair(const RepairStrategy& strategy);
    std::vector<DiagnosticIssue> predictFaults() const;

    // 报告生成和导出
    std::string exportReportAsJson() const;
    std::string exportReportAsHtml() const;
    std::string exportReportAsCsv() const;
    std::string exportReportAsXml() const;
    void saveReportToFile(const std::string& filename, 
                         const std::string& format = "json");

    // 统计和查询
    std::map<std::string, int> getErrorCounts() const;
    std::map<std::string, double> getAverageMetrics() const;
    std::vector<std::string> getRecentAlerts(int count = 10) const;
    std::chrono::system_clock::time_point getLastUpdateTime() const;

    // 事件监听
    void addEventListener(std::function<void(const DiagnosticIssue&)> listener);
    void removeEventListener(std::function<void(const DiagnosticIssue&)> listener);

private:
    // 配置
    DiagnosticSystemConfig config_;
    
    // 核心组件
    std::unique_ptr<PerformanceAnalyzer> performance_analyzer_;
    std::unique_ptr<FaultDetector> fault_detector_;
    std::unique_ptr<LogAnalyzer> log_analyzer_;
    std::unique_ptr<SystemMonitor> system_monitor_;
    std::unique_ptr<AlertManager> alert_manager_;
    std::unique_ptr<PluginManager> plugin_manager_;
    
    // 可视化和监控
    std::unique_ptr<IDataVisualizer> visualizer_;
    std::unique_ptr<RemoteMonitorServer> remote_monitor_;
    std::unique_ptr<RealtimeDataStreamer> data_streamer_;
    
    // 数据源
    std::map<std::string, std::unique_ptr<IDiagnosticDataSource>> data_sources_;
    
    // 状态管理
    SystemState current_state_;
    DiagnosticReport current_report_;
    std::vector<DiagnosticIssue> current_issues_;
    
    // 线程控制
    std::thread diagnostic_thread_;
    std::atomic<bool> is_running_{false};
    mutable std::mutex state_mutex_;
    mutable std::mutex report_mutex_;
    
    // 事件监听器
    std::vector<std::function<void(const DiagnosticIssue&)>> event_listeners_;
    std::mutex listeners_mutex_;

    // 内部方法
    void diagnosticLoop();
    void updateSystemState();
    void analyzeSystem();
    void detectAndHandleIssues();
    void updateReport();
    void notifyListeners(const DiagnosticIssue& issue);
    
    // 组件初始化
    void initializeComponents();
    void configureComponents();
    void shutdownComponents();
    
    // 数据处理
    void processPerformanceData();
    void processLogData();
    void processAlerts();
    
    // 报告生成
    void generateSystemSummary();
    void generatePerformanceAnalysis();
    void generateHardwareHealth();
    void generateNetworkStatus();
    void generateRecommendations();
    
    // 辅助方法
    double calculateHealthScore() const;
    std::vector<std::string> generateRecommendations() const;
    bool shouldTriggerAlert(const DiagnosticIssue& issue) const;
    void archiveOldData();
    void cleanup();
};

// 诊断系统工厂类
class DiagnosticSystemFactory {
public:
    static std::unique_ptr<SystemDiagnostics> createSystem(
        const DiagnosticSystemConfig& config = {});
    
    static std::unique_ptr<SystemDiagnostics> createSystemFromConfig(
        const std::string& config_file);
    
    static DiagnosticSystemConfig getDefaultConfig();
    static DiagnosticSystemConfig loadConfigFromFile(const std::string& filename);
    static void saveConfigToFile(const DiagnosticSystemConfig& config, 
                                const std::string& filename);
};

} // namespace diagnostics
} // namespace hft

// 系统状态结构
struct SystemState {
    struct {
        double latency;          // 延迟指标
        double throughput;       // 吞吐量
        double success_rate;     // 成功率
    } performance;
    
    struct {
        double cpu_usage;        // CPU使用率
        double memory_usage;     // 内存使用率
        double network_usage;    // 网络使用率
    } resources;
    
    std::vector<std::string> logs;   // 系统日志
    std::vector<std::string> errors; // 错误记录
};

// 诊断问题结构
struct DiagnosticIssue {
    enum class Severity {
        Low,
        Medium,
        High,
        Critical
    };
    
    std::string description;     // 问题描述
    Severity severity;           // 严重程度
    std::string component;       // 相关组件
    std::chrono::system_clock::time_point timestamp;  // 发现时间
};

// 修复策略结构
struct RepairStrategy {
    std::vector<std::string> steps;  // 修复步骤
    bool requires_restart;            // 是否需要重启
    std::string verification_method;  // 验证方法
};

// 诊断报告结构
struct DiagnosticReport {
    struct {
        std::string status;          // 系统状态
        std::vector<std::string> issues;  // 发现的问题
        HardwareHealth hardware;     // 硬件健康状态
        NetworkState network;        // 网络状态
        DiskState disk;             // 磁盘状态
    } system_summary;
    
    struct {
        double overall_performance;   // 整体性能
        std::vector<std::string> bottlenecks;  // 瓶颈点
        PerformanceTrend trends;     // 性能趋势
    } performance_analysis;
    
    std::vector<DiagnosticIssue> issue_diagnosis;  // 问题诊断
    std::vector<std::string> recommendations;      // 建议措施
    
    // 报告格式转换
    virtual std::string toJson() const;
    virtual std::string toHtml() const;
    virtual std::string toCsv() const;
};

// 诊断指标结构
struct DiagnosticMetrics {
    double accuracy;              // 诊断准确度
    double response_time;         // 响应时间
    double repair_effectiveness;  // 修复效果
    double system_health;         // 系统健康度
};

// 诊断参数结构
struct DiagnosticParameters {
    std::chrono::seconds diagnostic_interval;  // 诊断间隔
    double performance_threshold;              // 性能阈值
    double error_threshold;                    // 错误阈值
    double resource_threshold;                 // 资源阈值
};

// 诊断数据源接口
class IDiagnosticDataSource {
public:
    virtual ~IDiagnosticDataSource() = default;
    virtual std::string getName() const = 0;
    virtual json getData() const = 0;
    virtual std::vector<std::string> getMetrics() const = 0;
};

// 诊断可视化配置
struct VisualizationConfig {
    std::string theme{"light"};          // 主题
    std::string colorScheme{"default"};  // 配色方案
    int refreshInterval{5000};           // 刷新间隔(ms)
    bool enableAnimation{true};          // 启用动画
    std::string dateFormat{"%Y-%m-%d %H:%M:%S"};  // 日期格式
};

class SystemDiagnostics {
public:
    // 构造函数和析构函数
    SystemDiagnostics();
    ~SystemDiagnostics() {
        if (is_running_) stopDiagnostics();
    }
    
    // 远程监控
    void startRemoteMonitor(const RemoteMonitorConfig& config);
    void stopRemoteMonitor();
    RemoteMonitorStats getRemoteMonitorStats() const;
    
    // 可视化相关
    void configureVisualization(const VisualizationConfig& config);
    std::string generateDashboard(const std::vector<std::string>& metrics);
    std::string generateChart(const std::string& metric,
                            const std::string& chartType);
    std::string exportDashboardAsHTML() const;
    
    // 数据源管理
    void addDataSource(std::unique_ptr<IDiagnosticDataSource> source);
    void removeDataSource(const std::string& sourceName);
    std::vector<std::string> getAvailableDataSources() const;
    
    // 插件管理
    void loadPlugin(std::unique_ptr<IDiagnosticPlugin> plugin);
    void unloadPlugin(const std::string& pluginName);
    std::vector<std::string> getLoadedPlugins() const;
    
    // 自定义规则管理
    void addCustomRule(std::unique_ptr<CustomRule> rule);
    void removeRule(const std::string& ruleName);
    std::vector<std::string> getActiveRules() const;
    
    // 远程监控配置
    void configureRemoteMonitoring(const RemoteMonitoringConfig& config);
    void startRemoteMonitoring();
    void stopRemoteMonitoring();
    
    // 告警配置
    void configureAlerts(const AlertConfig& config);
    void enableAlerts();
    void disableAlerts();
    
    // 趋势分析
    PerformanceTrend getPerformanceTrend(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end);
        
    // 报告导出
    std::string exportReportAsJson() const;
    std::string exportReportAsHtml() const;
    std::string exportReportAsCsv() const;
    
    // 系统诊断接口
    void initialize();                                    // 初始化诊断系统
    void startDiagnostics();                             // 启动诊断
    void stopDiagnostics();                              // 停止诊断
    bool isSystemHealthy() const;                        // 检查系统健康状态
    DiagnosticReport generateReport();                   // 生成诊断报告
    DiagnosticMetrics getMetrics() const;               // 获取诊断指标
    
private:
    // 组件初始化
    void initializeComponents();                         // 初始化组件
    void configureMonitoring();                         // 配置监控
    void configureDiagnostics();                        // 配置诊断参数
    void initializeFaultDetection();                    // 初始化故障检测
    
    // 系统状态收集
    SystemState collectSystemState();                    // 收集系统状态
    void analyzeSystemPerformance(const SystemState&);   // 分析系统性能
    void detectPotentialIssues(const SystemState&);      // 检测潜在问题
    
    // 问题处理
    void handleIssue(const DiagnosticIssue&);           // 处理问题
    void executeRepairAction(const RepairStrategy&);     // 执行修复
    
    // 报告更新
    void updateDiagnosticReport(const SystemState&);     // 更新诊断报告
    
    // 内部实现方法
    bool checkPerformanceHealth() const;                 // 检查性能健康
    bool checkResourceHealth() const;                    // 检查资源健康
    bool checkErrorHealth() const;                       // 检查错误健康
    bool checkSystemStability() const;                   // 检查系统稳定性
    
private:
    // 诊断组件
    std::unique_ptr<PerformanceAnalyzer> perf_analyzer_;
    std::unique_ptr<FaultDetector> fault_detector_;
    std::unique_ptr<LogAnalyzer> log_analyzer_;
    std::unique_ptr<SystemMonitor> system_monitor_;
    
    // 诊断参数
    DiagnosticParameters params_;
    
    // 诊断报告
    DiagnosticReport current_report_;
    std::mutex report_mutex_;
    
    // 诊断线程控制
    std::thread diagnostic_thread_;
    bool is_running_{false};
};

} // namespace diagnostics
} // namespace hft
