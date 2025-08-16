#include "SystemDiagnostics.h"
#include "DiagnosticDataSource.h"
#include "HardwareMonitorPlugin.h"
#include "../core/Logger.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace hft::diagnostics;

// 示例客户端实现
class ExampleRemoteClient : public IRemoteClient {
public:
    explicit ExampleRemoteClient(const std::string& id) : client_id_(id) {}
    
    void onConnect() override {
        Logger::info("Client {} connected", client_id_);
        authenticated_ = true;
    }
    
    void onDisconnect() override {
        Logger::info("Client {} disconnected", client_id_);
        authenticated_ = false;
    }
    
    void onMessage(const RemoteMessage& message) override {
        Logger::info("Client {} received message type: {}", 
                    client_id_, static_cast<int>(message.type));
    }
    
    void onError(const std::string& error) override {
        Logger::error("Client {} error: {}", client_id_, error);
    }
    
    std::string getClientId() const override { return client_id_; }
    bool isAuthenticated() const override { return authenticated_; }

private:
    std::string client_id_;
    bool authenticated_{false};
};

// 主程序示例
int main() {
    try {
        Logger::info("Starting HFT Diagnostic System Example");
        
        // 1. 创建诊断系统配置
        DiagnosticSystemConfig config;
        config.update_interval = std::chrono::milliseconds(1000);
        config.max_history_size = 5000;
        config.enable_auto_repair = true;
        config.enable_predictions = true;
        
        // 设置阈值
        config.thresholds.latency_warning = 50.0;
        config.thresholds.latency_critical = 200.0;
        config.thresholds.cpu_warning = 70.0;
        config.thresholds.cpu_critical = 90.0;
        config.thresholds.memory_warning = 80.0;
        config.thresholds.memory_critical = 95.0;
        
        // 2. 创建诊断系统
        auto diagnostic_system = DiagnosticSystemFactory::createSystem(config);
        
        // 3. 配置告警系统
        AlertConfig alert_config;
        
        // CPU告警规则
        AlertConfig::AlertRule cpu_rule;
        cpu_rule.metric = "cpu_usage";
        cpu_rule.threshold = 80.0;
        cpu_rule.severity = Severity::Warning;
        cpu_rule.methods = {AlertConfig::AlertMethod::Console, AlertConfig::AlertMethod::Log};
        cpu_rule.target = "admin@example.com";
        alert_config.rules.push_back(cpu_rule);
        
        // 延迟告警规则
        AlertConfig::AlertRule latency_rule;
        latency_rule.metric = "latency";
        latency_rule.threshold = 100.0;
        latency_rule.severity = Severity::Error;
        latency_rule.methods = {AlertConfig::AlertMethod::Console, AlertConfig::AlertMethod::Webhook};
        latency_rule.target = "https://alerts.example.com/webhook";
        alert_config.rules.push_back(latency_rule);
        
        diagnostic_system->configureAlerts(alert_config);
        diagnostic_system->enableAlerts();
        
        // 4. 加载硬件监控插件
        auto hw_plugin = std::make_unique<HardwareMonitorPlugin>();
        PluginConfig plugin_config{
            .name = "HardwareMonitor",
            .version = "1.0.0",
            .author = "HFT Team",
            .description = "监控CPU、内存、磁盘和网络硬件状态",
            .parameters = {
                {"cpu_temp_threshold", "75.0"},
                {"memory_temp_threshold", "70.0"},
                {"disk_temp_threshold", "60.0"}
            }
        };
        
        diagnostic_system->loadPlugin(std::move(hw_plugin), plugin_config);
        diagnostic_system->enablePlugin("HardwareMonitor");
        
        // 5. 添加自定义诊断规则
        auto custom_rule = std::make_unique<CustomRule>(
            "HighThroughputCheck",
            "检查交易吞吐量是否过低",
            [](const SystemState& state) {
                return state.performance.throughput < 500.0;  // 500 TPS以下触发
            }
        );
        diagnostic_system->addCustomRule(std::move(custom_rule));
        
        // 6. 添加数据源
        auto perf_source = std::make_unique<PerformanceDataSource>(1000);
        auto network_source = std::make_unique<NetworkDataSource>(1000);
        
        diagnostic_system->addDataSource(std::move(perf_source));
        diagnostic_system->addDataSource(std::move(network_source));
        
        // 7. 配置远程监控
        RemoteMonitorConfig remote_config{
            .host = "0.0.0.0",
            .port = 8080,
            .auth_token = "hft-diagnostic-token-2024",
            .enable_ssl = false,
            .max_connections = 50,
            .enable_compression = true,
            .heartbeat_interval = std::chrono::seconds(30)
        };
        
        diagnostic_system->startRemoteMonitor(remote_config);
        
        // 8. 配置可视化
        VisualizationConfig viz_config{
            .theme = "dark",
            .color_scheme = "blue",
            .refresh_interval = 2000,
            .enable_animation = true,
            .chart_width = 900,
            .chart_height = 450,
            .enable_realtime = true
        };
        diagnostic_system->configureVisualization(viz_config);
        
        // 9. 添加事件监听器
        diagnostic_system->addEventListener([](const DiagnosticIssue& issue) {
            Logger::warn("Diagnostic event: {} - Severity: {}", 
                        issue.description, static_cast<int>(issue.severity));
        });
        
        // 10. 启动诊断系统
        diagnostic_system->startDiagnostics();
        
        Logger::info("Diagnostic system started successfully");
        Logger::info("Web dashboard available at: http://localhost:8080");
        Logger::info("Press Enter to generate reports and view statistics...");
        
        std::cin.get();
        
        // 11. 生成和展示报告
        Logger::info("Generating diagnostic reports...");
        
        // 生成完整诊断报告
        auto report = diagnostic_system->generateReport();
        Logger::info("Generated report ID: {}", report.id);
        Logger::info("System health score: {:.1f}%", diagnostic_system->getSystemHealthScore());
        
        // 获取系统状态
        auto current_state = diagnostic_system->getCurrentSystemState();
        Logger::info("Current system metrics:");
        Logger::info("  CPU Usage: {:.1f}%", current_state.resources.cpu_usage);
        Logger::info("  Memory Usage: {:.1f}%", current_state.resources.memory_usage);
        Logger::info("  Latency: {:.2f}ms", current_state.performance.latency);
        Logger::info("  Throughput: {:.0f} ops/sec", current_state.performance.throughput);
        
        // 检测当前问题
        auto issues = diagnostic_system->detectIssues();
        Logger::info("Detected {} issues:", issues.size());
        for (const auto& issue : issues) {
            Logger::info("  - {} (Severity: {})", 
                        issue.description, static_cast<int>(issue.severity));
        }
        
        // 生成仪表板
        std::vector<std::string> metrics = {
            "latency", "throughput", "cpu_usage", "memory_usage"
        };
        std::string dashboard_html = diagnostic_system->generateDashboard(metrics);
        
        // 保存仪表板到文件
        std::ofstream dashboard_file("dashboard.html");
        dashboard_file << dashboard_html;
        dashboard_file.close();
        Logger::info("Dashboard saved to dashboard.html");
        
        // 导出报告
        std::string json_report = diagnostic_system->exportReportAsJson();
        std::ofstream json_file("diagnostic_report.json");
        json_file << json_report;
        json_file.close();
        Logger::info("JSON report saved to diagnostic_report.json");
        
        std::string html_report = diagnostic_system->exportReportAsHtml();
        std::ofstream html_file("diagnostic_report.html");
        html_file << html_report;
        html_file.close();
        Logger::info("HTML report saved to diagnostic_report.html");
        
        // 获取性能趋势
        auto now = std::chrono::system_clock::now();
        auto start = now - std::chrono::hours(1);
        auto trend = diagnostic_system->getPerformanceTrend(start, now);
        
        Logger::info("Performance trend statistics:");
        Logger::info("  Latency - Mean: {:.2f}ms, 95th: {:.2f}ms, 99th: {:.2f}ms",
                    trend.latency_stats.mean,
                    trend.latency_stats.percentile_95,
                    trend.latency_stats.percentile_99);
        
        // 获取诊断指标
        auto metrics_data = diagnostic_system->getMetrics();
        Logger::info("Diagnostic system metrics:");
        Logger::info("  Detection accuracy: {:.1f}%", metrics_data.detection_accuracy * 100);
        Logger::info("  Average response time: {:.2f}ms", metrics_data.avg_response_time);
        Logger::info("  CPU overhead: {:.2f}%", metrics_data.cpu_overhead);
        Logger::info("  Memory overhead: {:.2f}MB", metrics_data.memory_overhead);
        
        // 获取远程监控统计
        if (diagnostic_system->isRemoteMonitorRunning()) {
            auto remote_stats = diagnostic_system->getRemoteMonitorStats();
            Logger::info("Remote monitoring statistics:");
            Logger::info("  Active connections: {}", remote_stats.active_connections);
            Logger::info("  Messages sent: {}", remote_stats.messages_sent);
            Logger::info("  Messages received: {}", remote_stats.messages_received);
            Logger::info("  Average message size: {:.1f} bytes", remote_stats.avg_message_size);
        }
        
        // 获取插件统计
        auto loaded_plugins = diagnostic_system->getLoadedPlugins();
        Logger::info("Loaded plugins: {}", loaded_plugins.size());
        for (const auto& plugin_name : loaded_plugins) {
            auto plugin_stats = diagnostic_system->getPluginStats(plugin_name);
            Logger::info("  Plugin {}: {} rules evaluated, {} triggered",
                        plugin_name,
                        plugin_stats.rules_evaluated,
                        plugin_stats.rules_triggered);
        }
        
        Logger::info("Press Enter to stop the diagnostic system...");
        std::cin.get();
        
        // 12. 停止诊断系统
        Logger::info("Stopping diagnostic system...");
        diagnostic_system->stopDiagnostics();
        
        if (diagnostic_system->isRemoteMonitorRunning()) {
            diagnostic_system->stopRemoteMonitor();
        }
        
        Logger::info("Diagnostic system stopped successfully");
        
    } catch (const std::exception& e) {
        Logger::error("Error in diagnostic system example: {}", e.what());
        return 1;
    }
    
    return 0;
}

// 客户端连接示例（可以在单独的程序中运行）
void runClientExample() {
    try {
        Logger::info("Starting remote monitoring client example");
        
        // 这里可以实现WebSocket客户端连接到远程监控服务器
        // 示例代码展示如何处理实时数据流
        
        // 连接到 ws://localhost:8080
        // 发送认证消息
        // 订阅特定指标
        // 接收实时数据并显示
        
        Logger::info("Client example completed");
        
    } catch (const std::exception& e) {
        Logger::error("Error in client example: {}", e.what());
    }
}
