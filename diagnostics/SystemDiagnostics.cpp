#include "SystemDiagnostics.h"
#include "../core/Logger.h"
#include <algorithm>
#include <fstream>
#include <filesystem>

namespace hft {
namespace diagnostics {

SystemDiagnostics::SystemDiagnostics(const DiagnosticSystemConfig& config)
    : config_(config) {
    
    Logger::info("Initializing comprehensive diagnostic system");
    
    // 创建目录结构
    std::filesystem::create_directories(config_.log_directory);
    std::filesystem::create_directories(config_.report_directory);
    std::filesystem::create_directory(std::filesystem::path(config_.config_file).parent_path());
}

SystemDiagnostics::~SystemDiagnostics() {
    if (is_running_) {
        stopDiagnostics();
    }
    shutdownComponents();
}

void SystemDiagnostics::initialize() {
    Logger::info("Initializing diagnostic system components");
    
    try {
        // 初始化核心组件
        initializeComponents();
        
        // 配置组件
        configureComponents();
        
        // 初始化可视化组件
        VisualizationConfig viz_config;
        visualizer_ = std::make_unique<ChartJSVisualizer>(viz_config);
        
        Logger::info("Diagnostic system initialized successfully");
        
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize diagnostic system: {}", e.what());
        throw;
    }
}

void SystemDiagnostics::startDiagnostics() {
    if (is_running_) {
        Logger::warn("Diagnostic system is already running");
        return;
    }
    
    Logger::info("Starting diagnostic system");
    
    try {
        // 启动系统监控
        system_monitor_->startMonitoring();
        
        // 启动诊断主循环
        is_running_ = true;
        diagnostic_thread_ = std::thread([this]() {
            diagnosticLoop();
        });
        
        Logger::info("Diagnostic system started successfully");
        
    } catch (const std::exception& e) {
        Logger::error("Failed to start diagnostic system: {}", e.what());
        is_running_ = false;
        throw;
    }
}

void SystemDiagnostics::stopDiagnostics() {
    if (!is_running_) {
        Logger::warn("Diagnostic system is not running");
        return;
    }
    
    Logger::info("Stopping diagnostic system");
    
    try {
        // 停止主循环
        is_running_ = false;
        
        if (diagnostic_thread_.joinable()) {
            diagnostic_thread_.join();
        }
        
        // 停止系统监控
        system_monitor_->stopMonitoring();
        
        // 停止远程监控
        if (remote_monitor_ && remote_monitor_->isRunning()) {
            remote_monitor_->stop();
        }
        
        Logger::info("Diagnostic system stopped successfully");
        
    } catch (const std::exception& e) {
        Logger::error("Error stopping diagnostic system: {}", e.what());
    }
}

void SystemDiagnostics::loadPlugin(std::unique_ptr<IDiagnosticPlugin> plugin,
                                  const PluginConfig& config) {
    
    if (!plugin_manager_) {
        throw std::runtime_error("Plugin manager not initialized");
    }
    
    try {
        plugin_manager_->loadPlugin(std::move(plugin), config);
        Logger::info("Plugin {} loaded successfully", config.name);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to load plugin {}: {}", config.name, e.what());
        throw;
    }
}

void SystemDiagnostics::startRemoteMonitor(const RemoteMonitorConfig& config) {
    try {
        if (!remote_monitor_) {
            remote_monitor_ = std::make_unique<RemoteMonitorServer>(config);
        }
        
        remote_monitor_->start();
        
        // 启动数据流
        if (!data_streamer_) {
            data_streamer_ = std::make_unique<RealtimeDataStreamer>(remote_monitor_);
        }
        data_streamer_->startStreaming();
        
        Logger::info("Remote monitoring started on port {}", config.port);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to start remote monitoring: {}", e.what());
        throw;
    }
}

void SystemDiagnostics::configureAlerts(const AlertConfig& config) {
    if (!alert_manager_) {
        throw std::runtime_error("Alert manager not initialized");
    }
    
    try {
        alert_manager_->configure(config);
        Logger::info("Alert system configured with {} rules", config.rules.size());
        
    } catch (const std::exception& e) {
        Logger::error("Failed to configure alerts: {}", e.what());
        throw;
    }
}

void SystemDiagnostics::addDataSource(std::unique_ptr<IDiagnosticDataSource> source) {
    if (!source) {
        throw std::invalid_argument("Data source cannot be null");
    }
    
    std::string name = source->getName();
    data_sources_[name] = std::move(source);
    data_sources_[name]->start();
    
    Logger::info("Data source {} added and started", name);
}

SystemState SystemDiagnostics::getCurrentSystemState() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return current_state_;
}

DiagnosticReport SystemDiagnostics::generateReport() {
    std::lock_guard<std::mutex> lock(report_mutex_);
    
    try {
        DiagnosticReport report;
        report.id = "report_" + std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        report.generated_at = std::chrono::system_clock::now();
        
        // 生成报告各部分
        generateSystemSummary();
        generatePerformanceAnalysis();
        generateHardwareHealth();
        generateNetworkStatus();
        generateRecommendations();
        
        current_report_ = report;
        return report;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to generate diagnostic report: {}", e.what());
        throw;
    }
}

bool SystemDiagnostics::isSystemHealthy() const {
    double health_score = getSystemHealthScore();
    return health_score >= 80.0;  // 80%以上认为健康
}

double SystemDiagnostics::getSystemHealthScore() const {
    return calculateHealthScore();
}

std::vector<DiagnosticIssue> SystemDiagnostics::detectIssues() {
    std::vector<DiagnosticIssue> issues;
    
    try {
        // 获取当前系统状态
        SystemState state = getCurrentSystemState();
        
        // 故障检测
        if (fault_detector_) {
            auto fault_issues = fault_detector_->detectFaults(state);
            issues.insert(issues.end(), fault_issues.begin(), fault_issues.end());
        }
        
        // 性能瓶颈检测
        if (performance_analyzer_) {
            auto bottlenecks = performance_analyzer_->detectBottlenecks(state);
            for (const auto& bottleneck : bottlenecks) {
                DiagnosticIssue issue;
                issue.id = "bottleneck_" + std::to_string(
                    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
                issue.description = bottleneck;
                issue.severity = Severity::Warning;
                issue.type = RuleType::Performance;
                issue.component = "System";
                issue.timestamp = std::chrono::system_clock::now();
                issues.push_back(issue);
            }
        }
        
        // 日志分析
        if (log_analyzer_) {
            auto log_issues = log_analyzer_->analyzeLogs(state.logs);
            issues.insert(issues.end(), log_issues.begin(), log_issues.end());
        }
        
        // 执行插件规则
        if (plugin_manager_) {
            plugin_manager_->executeAll();
        }
        
        current_issues_ = issues;
        return issues;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to detect issues: {}", e.what());
        return {};
    }
}

std::string SystemDiagnostics::generateDashboard(const std::vector<std::string>& metrics) {
    if (!visualizer_) {
        throw std::runtime_error("Visualizer not initialized");
    }
    
    try {
        std::vector<std::string> charts;
        std::vector<std::string> selected_metrics = metrics;
        
        // 如果没有指定指标，使用默认指标
        if (selected_metrics.empty()) {
            selected_metrics = {"latency", "throughput", "cpu_usage", "memory_usage"};
        }
        
        // 生成各个图表
        for (const auto& metric : selected_metrics) {
            std::string chart = generateChart(metric);
            if (!chart.empty()) {
                charts.push_back(chart);
            }
        }
        
        return visualizer_->generateDashboard(charts, 2);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to generate dashboard: {}", e.what());
        return "";
    }
}

std::string SystemDiagnostics::generateChart(const std::string& metric, 
                                           const std::string& chart_type) {
    
    if (!visualizer_) {
        throw std::runtime_error("Visualizer not initialized");
    }
    
    try {
        // 获取性能趋势数据
        auto now = std::chrono::system_clock::now();
        auto start = now - std::chrono::hours(1);  // 最近1小时的数据
        
        PerformanceTrend trend = getPerformanceTrend(start, now);
        
        if (metric == "latency") {
            return visualizer_->generateTimeSeriesChart(
                trend.latencies, trend.timestamps, "Latency Trend", "Latency (ms)");
        } else if (metric == "throughput") {
            return visualizer_->generateTimeSeriesChart(
                trend.throughputs, trend.timestamps, "Throughput Trend", "Ops/sec");
        } else if (metric == "cpu_usage") {
            return visualizer_->generateTimeSeriesChart(
                trend.cpu_usages, trend.timestamps, "CPU Usage", "CPU (%)");
        } else if (metric == "memory_usage") {
            return visualizer_->generateTimeSeriesChart(
                trend.memory_usages, trend.timestamps, "Memory Usage", "Memory (%)");
        }
        
        return "";
        
    } catch (const std::exception& e) {
        Logger::error("Failed to generate chart for metric {}: {}", metric, e.what());
        return "";
    }
}

PerformanceTrend SystemDiagnostics::getPerformanceTrend(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) const {
    
    if (!performance_analyzer_) {
        throw std::runtime_error("Performance analyzer not initialized");
    }
    
    return performance_analyzer_->getPerformanceTrend(start, end);
}

void SystemDiagnostics::diagnosticLoop() {
    Logger::info("Diagnostic loop started");
    
    while (is_running_) {
        try {
            // 更新系统状态
            updateSystemState();
            
            // 分析系统
            analyzeSystem();
            
            // 检测和处理问题
            detectAndHandleIssues();
            
            // 更新报告
            updateReport();
            
            // 清理旧数据
            archiveOldData();
            
            // 等待下一个周期
            std::this_thread::sleep_for(config_.update_interval);
            
        } catch (const std::exception& e) {
            Logger::error("Error in diagnostic loop: {}", e.what());
            std::this_thread::sleep_for(std::chrono::seconds(5));  // 错误恢复等待
        }
    }
    
    Logger::info("Diagnostic loop stopped");
}

void SystemDiagnostics::updateSystemState() {
    if (!system_monitor_) return;
    
    try {
        SystemState new_state = system_monitor_->collectSystemState();
        
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            current_state_ = new_state;
        }
        
        // 推送实时数据
        if (data_streamer_ && data_streamer_->isStreaming()) {
            data_streamer_->pushSystemState(new_state);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Failed to update system state: {}", e.what());
    }
}

void SystemDiagnostics::analyzeSystem() {
    SystemState state = getCurrentSystemState();
    
    // 性能分析
    if (performance_analyzer_) {
        performance_analyzer_->analyzePerformance(state);
    }
    
    // 处理数据源数据
    processPerformanceData();
    processLogData();
}

void SystemDiagnostics::detectAndHandleIssues() {
    std::vector<DiagnosticIssue> issues = detectIssues();
    
    for (const auto& issue : issues) {
        // 通知监听器
        notifyListeners(issue);
        
        // 处理告警
        if (alert_manager_ && shouldTriggerAlert(issue)) {
            SystemState state = getCurrentSystemState();
            alert_manager_->processAlert(issue, state);
        }
        
        // 推送告警到远程客户端
        if (data_streamer_ && data_streamer_->isStreaming()) {
            data_streamer_->pushAlert(issue);
        }
        
        // 自动修复（如果启用）
        if (config_.enable_auto_repair) {
            auto strategies = getRepairStrategies(issue);
            for (const auto& strategy : strategies) {
                if (strategy.is_automatic) {
                    executeRepair(strategy);
                    break;  // 只执行第一个自动修复策略
                }
            }
        }
    }
}

void SystemDiagnostics::initializeComponents() {
    // 初始化核心组件
    performance_analyzer_ = std::make_unique<PerformanceAnalyzer>();
    performance_analyzer_->initialize();
    
    fault_detector_ = std::make_unique<FaultDetector>();
    fault_detector_->initialize();
    
    log_analyzer_ = std::make_unique<LogAnalyzer>();
    log_analyzer_->initialize();
    
    system_monitor_ = std::make_unique<SystemMonitor>();
    system_monitor_->initialize();
    
    alert_manager_ = std::make_unique<AlertManager>();
    
    plugin_manager_ = std::make_unique<PluginManager>();
    
    Logger::info("Core diagnostic components initialized");
}

double SystemDiagnostics::calculateHealthScore() const {
    SystemState state = getCurrentSystemState();
    
    double score = 100.0;
    
    // CPU健康评分
    if (state.resources.cpu_usage > config_.thresholds.cpu_critical) {
        score -= 30.0;
    } else if (state.resources.cpu_usage > config_.thresholds.cpu_warning) {
        score -= 15.0;
    }
    
    // 内存健康评分
    if (state.resources.memory_usage > config_.thresholds.memory_critical) {
        score -= 25.0;
    } else if (state.resources.memory_usage > config_.thresholds.memory_warning) {
        score -= 10.0;
    }
    
    // 延迟健康评分
    if (state.performance.latency > config_.thresholds.latency_critical) {
        score -= 25.0;
    } else if (state.performance.latency > config_.thresholds.latency_warning) {
        score -= 10.0;
    }
    
    // 错误率影响
    if (state.performance.error_rate > 0.05) {  // 5%以上错误率
        score -= 20.0;
    } else if (state.performance.error_rate > 0.01) {  // 1%以上错误率
        score -= 10.0;
    }
    
    return std::max(0.0, score);
}

void SystemDiagnostics::notifyListeners(const DiagnosticIssue& issue) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    
    for (const auto& listener : event_listeners_) {
        try {
            listener(issue);
        } catch (const std::exception& e) {
            Logger::error("Error in event listener: {}", e.what());
        }
    }
}

bool SystemDiagnostics::shouldTriggerAlert(const DiagnosticIssue& issue) const {
    // 可以根据问题严重程度、类型等决定是否触发告警
    return issue.severity >= Severity::Warning;
}

void SystemDiagnostics::archiveOldData() {
    // 清理超过最大历史记录数的数据
    // 这里可以实现数据归档逻辑
}

// DiagnosticSystemFactory 实现
std::unique_ptr<SystemDiagnostics> DiagnosticSystemFactory::createSystem(
    const DiagnosticSystemConfig& config) {
    
    auto system = std::make_unique<SystemDiagnostics>(config);
    system->initialize();
    return system;
}

DiagnosticSystemConfig DiagnosticSystemFactory::getDefaultConfig() {
    DiagnosticSystemConfig config;
    
    // 设置默认阈值
    config.thresholds.latency_warning = 100.0;
    config.thresholds.latency_critical = 500.0;
    config.thresholds.cpu_warning = 80.0;
    config.thresholds.cpu_critical = 95.0;
    config.thresholds.memory_warning = 85.0;
    config.thresholds.memory_critical = 95.0;
    
    return config;
}

} // namespace diagnostics
} // namespace hft
}

void SystemDiagnostics::initializeComponents() {
    // 性能分析器
    perf_analyzer_ = std::make_unique<PerformanceAnalyzer>();
    perf_analyzer_->initialize();
    
    // 故障检测器
    fault_detector_ = std::make_unique<FaultDetector>();
    fault_detector_->initialize();
    
    // 日志分析器
    log_analyzer_ = std::make_unique<LogAnalyzer>();
    log_analyzer_->initialize();
    
    // 系统监控器
    system_monitor_ = std::make_unique<SystemMonitor>();
    system_monitor_->initialize();
}

void SystemDiagnostics::startDiagnostics() {
    is_running_ = true;
    
    // 启动诊断线程
    diagnostic_thread_ = std::thread([this] {
        while (is_running_) {
            try {
                // 1. 收集系统状态
                auto system_state = collectSystemState();
                
                // 2. 分析系统性能
                analyzeSystemPerformance(system_state);
                
                // 3. 检测潜在问题
                detectPotentialIssues(system_state);
                
                // 4. 更新诊断报告
                updateDiagnosticReport(system_state);
                
                // 等待下一个诊断周期
                std::this_thread::sleep_for(params_.diagnostic_interval);
                
            } catch (const std::exception& e) {
                Logger::error("Diagnostic error: {}", e.what());
            }
        }
    });
}

void SystemDiagnostics::stopDiagnostics() {
    is_running_ = false;
    if (diagnostic_thread_.joinable()) {
        diagnostic_thread_.join();
    }
}

SystemState SystemDiagnostics::collectSystemState() {
    SystemState state;
    
    // 1. 收集性能指标
    state.performance = collectPerformanceMetrics();
    
    // 2. 收集资源使用情况
    state.resources = collectResourceUsage();
    
    // 3. 收集系统日志
    state.logs = collectSystemLogs();
    
    // 4. 收集错误统计
    state.errors = collectErrorStatistics();
    
    return state;
}

void SystemDiagnostics::analyzeSystemPerformance(
    const SystemState& state) {
    
    // 1. 分析延迟
    analyzeLatecy(state.performance);
    
    // 2. 分析吞吐量
    analyzeThroughput(state.performance);
    
    // 3. 分析资源使用
    analyzeResourceUsage(state.resources);
    
    // 4. 分析系统稳定性
    analyzeSystemStability(state);
}

void SystemDiagnostics::detectPotentialIssues(
    const SystemState& state) {
    
    // 1. 性能问题检测
    detectPerformanceIssues(state.performance);
    
    // 2. 资源问题检测
    detectResourceIssues(state.resources);
    
    // 3. 系统错误检测
    detectSystemErrors(state.errors);
    
    // 4. 异常模式检测
    detectAnomalousPatterns(state);
}

void SystemDiagnostics::handleIssue(
    const DiagnosticIssue& issue) {
    
    // 1. 评估问题严重性
    auto severity = assessIssueSeverity(issue);
    
    // 2. 确定处理策略
    auto strategy = determineHandlingStrategy(issue, severity);
    
    // 3. 执行修复操作
    executeRepairAction(strategy);
    
    // 4. 记录问题处理
    recordIssueHandling(issue, strategy);
}

void SystemDiagnostics::executeRepairAction(
    const RepairStrategy& strategy) {
    
    try {
        // 1. 准备修复
        prepareRepair(strategy);
        
        // 2. 执行修复步骤
        for (const auto& step : strategy.steps) {
            executeRepairStep(step);
        }
        
        // 3. 验证修复结果
        verifyRepair(strategy);
        
        // 4. 更新系统状态
        updateSystemStatus();
        
    } catch (const std::exception& e) {
        Logger::error("Repair action failed: {}", e.what());
        handleRepairFailure(strategy);
    }
}

DiagnosticReport SystemDiagnostics::generateReport() {
    DiagnosticReport report;
    
    // 1. 系统状态摘要
    report.system_summary = generateSystemSummary();
    
    // 2. 性能分析
    report.performance_analysis = generatePerformanceAnalysis();
    
    // 3. 问题诊断
    report.issue_diagnosis = generateIssueDiagnosis();
    
    // 4. 建议措施
    report.recommendations = generateRecommendations();
    
    return report;
}

void SystemDiagnostics::configureDiagnostics() {
    // 设置诊断参数
    params_.diagnostic_interval = std::chrono::seconds(1);
    params_.performance_threshold = 0.8;
    params_.error_threshold = 0.01;
    params_.resource_threshold = 0.9;
}

bool SystemDiagnostics::isSystemHealthy() const {
    // 检查系统健康状态
    return checkPerformanceHealth() &&
           checkResourceHealth() &&
           checkErrorHealth() &&
           checkSystemStability();
}

void SystemDiagnostics::updateDiagnosticReport(
    const SystemState& state) {
    
    std::lock_guard<std::mutex> lock(report_mutex_);
    
    // 1. 更新系统状态
    current_report_.system_state = state;
    
    // 2. 更新性能指标
    updatePerformanceMetrics(state);
    
    // 3. 更新问题诊断
    updateIssueDiagnosis(state);
    
    // 4. 更新建议
    updateRecommendations(state);
}

DiagnosticMetrics SystemDiagnostics::getMetrics() const {
    DiagnosticMetrics metrics;
    
    // 收集诊断准确度指标
    metrics.accuracy = calculateAccuracyMetrics();
    
    // 收集响应时间指标
    metrics.response_time = calculateResponseTimeMetrics();
    
    // 收集修复效果指标
    metrics.repair_effectiveness = calculateRepairEffectiveness();
    
    // 收集系统健康度指标
    metrics.system_health = calculateSystemHealthMetrics();
    
    return metrics;
}

} // namespace diagnostics
} // namespace hft
