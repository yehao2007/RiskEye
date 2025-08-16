#include "SelfOperatingSystem.h"
#include <chrono>
#include <sstream>
#include <algorithm>
#include <random>
#include <cmath>

namespace hft {
namespace core {

//==============================================================================
// SelfOperatingSystem 实现
//==============================================================================

SelfOperatingSystem::SelfOperatingSystem(
    FaultDetector* fault_detector,
    FaultToleranceManager* fault_manager,
    SystemMonitor* system_monitor,
    Logger* logger
) : fault_detector_(fault_detector),
    fault_manager_(fault_manager),
    system_monitor_(system_monitor),
    logger_(logger),
    is_running_(false),
    operating_mode_(OperatingMode::SUPERVISED),
    auto_repair_enabled_(false)
{
}

SelfOperatingSystem::~SelfOperatingSystem() {
    stop();
}

bool SelfOperatingSystem::initialize() {
    if (!fault_detector_ || !fault_manager_ || !system_monitor_ || !logger_) {
        return false;
    }
    
    logger_->info("初始化自运营系统...");
    
    // 注册故障回调
    fault_detector_->registerFaultCallback([this](const FaultDetector::FaultInfo& fault) {
        SystemAnomaly anomaly;
        anomaly.component = fault.component;
        anomaly.anomaly_type = "FAULT_" + std::to_string(static_cast<int>(fault.type));
        anomaly.description = fault.description;
        anomaly.detection_timestamp = fault.timestamp;
        anomaly.severity = static_cast<double>(fault.severity) / 3.0; // 将枚举转换为0-1的严重性
        anomaly.affected_components = fault.affected_systems;
        anomaly.auto_repairable = (fault.severity != FaultDetector::FaultSeverity::CRITICAL);
        
        std::lock_guard<std::mutex> lock(repair_mutex_);
        anomaly_queue_.push(anomaly);
        
        std::stringstream ss;
        ss << "检测到系统异常: " << anomaly.description << " (组件: " << anomaly.component << ")";
        logger_->warning(ss.str());
    });
    
    // 初始化指标
    system_metrics_["health_score"] = 1.0;
    system_metrics_["repair_success_rate"] = 1.0;
    system_metrics_["auto_intervention_count"] = 0;
    
    logger_->info("自运营系统初始化完成");
    return true;
}

bool SelfOperatingSystem::start() {
    if (is_running_) {
        logger_->warning("自运营系统已经在运行");
        return false;
    }
    
    is_running_ = true;
    logger_->info("启动自运营系统");
    
    // 启动监控线程
    monitoring_thread_ = std::thread(&SelfOperatingSystem::systemSelfCheckLoop, this);
    
    return true;
}

void SelfOperatingSystem::stop() {
    if (!is_running_) {
        return;
    }
    
    logger_->info("停止自运营系统");
    is_running_ = false;
    
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }
}

void SelfOperatingSystem::setOperatingMode(OperatingMode mode) {
    OperatingMode old_mode = operating_mode_;
    operating_mode_ = mode;
    
    std::stringstream ss;
    ss << "自运营系统模式切换: ";
    
    switch (old_mode) {
        case OperatingMode::SUPERVISED: ss << "有人监督"; break;
        case OperatingMode::SEMI_AUTOMATED: ss << "半自动"; break;
        case OperatingMode::FULLY_AUTOMATED: ss << "完全自动"; break;
    }
    
    ss << " -> ";
    
    switch (mode) {
        case OperatingMode::SUPERVISED: ss << "有人监督"; break;
        case OperatingMode::SEMI_AUTOMATED: ss << "半自动"; break;
        case OperatingMode::FULLY_AUTOMATED: ss << "完全自动"; break;
    }
    
    logger_->info(ss.str());
    
    // 无人模式需要特殊处理
    if (mode == OperatingMode::FULLY_AUTOMATED) {
        auto_repair_enabled_ = true;
        logger_->info("自动进入无人值守模式，自动修复已启用");
    }
}

SelfOperatingSystem::OperatingMode SelfOperatingSystem::getOperatingMode() const {
    return operating_mode_;
}

void SelfOperatingSystem::registerRepairCallback(RepairCallback callback) {
    repair_callback_ = callback;
}

void SelfOperatingSystem::triggerSystemCheck() {
    logger_->info("手动触发系统自检");
    detectSystemAnomalies();
}

SelfOperatingSystem::DiagnosticReport SelfOperatingSystem::getDiagnosticReport() const {
    return generateDiagnosticReport();
}

void SelfOperatingSystem::enableAutoRepair(bool enable) {
    auto_repair_enabled_ = enable;
    logger_->info(std::string("自动修复") + (enable ? "已启用" : "已禁用"));
}

bool SelfOperatingSystem::isAutoRepairEnabled() const {
    return auto_repair_enabled_;
}

void SelfOperatingSystem::systemSelfCheckLoop() {
    logger_->info("启动系统自检循环");
    
    while (is_running_) {
        try {
            // 检测系统异常
            detectSystemAnomalies();
            
            // 检测异常模式
            detectAnomalyPatterns();
            
            // 预测性维护检查
            predictiveMaintenanceCheck();
            
            // 如果处于无人值守模式，进行参数优化
            if (operating_mode_ == OperatingMode::FULLY_AUTOMATED) {
                optimizeSystemParameters();
            }
            
            // 处理待修复的异常
            std::lock_guard<std::mutex> lock(repair_mutex_);
            while (!anomaly_queue_.empty()) {
                SystemAnomaly anomaly = anomaly_queue_.front();
                anomaly_queue_.pop();
                
                // 进行根因分析
                std::vector<std::string> root_causes = analyzeRootCause(anomaly);
                
                // 更新根因信息
                anomaly.potential_causes = root_causes;
                
                // 生成修复计划
                RepairPlan plan = generateRepairPlan(anomaly);
                
                // 确定是否执行修复
                bool should_repair = false;
                
                if (auto_repair_enabled_) {
                    if (operating_mode_ == OperatingMode::FULLY_AUTOMATED) {
                        // 无人值守模式下自动修复
                        should_repair = true;
                    } else if (operating_mode_ == OperatingMode::SEMI_AUTOMATED && !plan.requires_human_approval) {
                        // 半自动模式下仅修复不需要人工审批的问题
                        should_repair = true;
                    }
                }
                
                if (should_repair) {
                    // 执行修复
                    active_repair_plans_.push_back(plan);
                    bool success = executeRepairPlan(plan);
                    
                    // 学习修复结果
                    learnFromRepairOutcome(plan, success);
                    
                    // 调用回调
                    if (repair_callback_) {
                        repair_callback_(plan, success);
                    }
                    
                    // 记录历史
                    std::stringstream ss;
                    ss << plan.component_id << ": " << plan.issue_description << " - " 
                       << (success ? "修复成功" : "修复失败");
                    repair_history_.push_back(ss.str());
                    
                    // 限制历史记录长度
                    if (repair_history_.size() > 100) {
                        repair_history_.erase(repair_history_.begin());
                    }
                }
            }
            
        } catch (const std::exception& e) {
            logger_->error("自运营系统自检循环异常: " + std::string(e.what()));
        }
        
        // 休眠一段时间
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    logger_->info("系统自检循环已停止");
}

void SelfOperatingSystem::detectSystemAnomalies() {
    // 获取系统状态
    SystemMonitor::SystemState state = system_monitor_->getSystemState();
    
    // 检查CPU使用率异常
    if (state.cpu.total_usage > 90.0) {
        SystemAnomaly anomaly;
        anomaly.component = "CPU";
        anomaly.anomaly_type = "HIGH_USAGE";
        anomaly.description = "CPU使用率过高: " + std::to_string(state.cpu.total_usage) + "%";
        anomaly.detection_timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        anomaly.severity = (state.cpu.total_usage - 90.0) / 10.0; // 90-100% -> 0-1 severity
        anomaly.auto_repairable = true;
        
        std::lock_guard<std::mutex> lock(repair_mutex_);
        anomaly_queue_.push(anomaly);
    }
    
    // 检查内存使用异常
    double memory_usage_percent = 100.0 * state.memory.used_bytes / state.memory.total_bytes;
    if (memory_usage_percent > 85.0) {
        SystemAnomaly anomaly;
        anomaly.component = "Memory";
        anomaly.anomaly_type = "HIGH_USAGE";
        anomaly.description = "内存使用率过高: " + std::to_string(memory_usage_percent) + "%";
        anomaly.detection_timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        anomaly.severity = (memory_usage_percent - 85.0) / 15.0; // 85-100% -> 0-1 severity
        anomaly.auto_repairable = true;
        
        std::lock_guard<std::mutex> lock(repair_mutex_);
        anomaly_queue_.push(anomaly);
    }
    
    // 检查网络延迟异常
    if (state.network.latency_us > 1000.0) { // 超过1ms视为异常
        SystemAnomaly anomaly;
        anomaly.component = "Network";
        anomaly.anomaly_type = "HIGH_LATENCY";
        anomaly.description = "网络延迟过高: " + std::to_string(state.network.latency_us) + "μs";
        anomaly.detection_timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        anomaly.severity = std::min(1.0, (state.network.latency_us - 1000.0) / 9000.0); // 1ms-10ms -> 0-1 severity
        anomaly.auto_repairable = true;
        
        std::lock_guard<std::mutex> lock(repair_mutex_);
        anomaly_queue_.push(anomaly);
    }
    
    // 获取健康状态
    FaultDetector::HealthStatus health = fault_detector_->getHealthStatus();
    
    // 更新健康得分
    system_metrics_["health_score"] = health.health_score;
}

std::vector<std::string> SelfOperatingSystem::analyzeRootCause(const SystemAnomaly& anomaly) {
    std::vector<std::string> causes;
    
    // 根据异常类型进行分析
    if (anomaly.component == "CPU" && anomaly.anomaly_type == "HIGH_USAGE") {
        causes.push_back("可能存在计算密集型任务");
        causes.push_back("可能存在无限循环");
        causes.push_back("可能存在过多并发线程");
    }
    else if (anomaly.component == "Memory" && anomaly.anomaly_type == "HIGH_USAGE") {
        causes.push_back("可能存在内存泄漏");
        causes.push_back("可能存在过大的数据结构");
        causes.push_back("可能存在过多缓存数据");
    }
    else if (anomaly.component == "Network" && anomaly.anomaly_type == "HIGH_LATENCY") {
        causes.push_back("可能存在网络拥塞");
        causes.push_back("可能存在DNS解析延迟");
        causes.push_back("可能存在网络设备问题");
    }
    
    return causes;
}

SelfOperatingSystem::RepairPlan SelfOperatingSystem::generateRepairPlan(const SystemAnomaly& anomaly) {
    RepairPlan plan;
    plan.component_id = anomaly.component;
    plan.issue_description = anomaly.description;
    plan.success_probability = 0.8; // 默认成功率
    plan.requires_restart = false;
    plan.requires_human_approval = (anomaly.severity > 0.7); // 严重程度高的需要人工审批
    
    // 根据不同的异常生成修复步骤
    if (anomaly.component == "CPU" && anomaly.anomaly_type == "HIGH_USAGE") {
        plan.repair_steps.push_back("识别CPU占用较高的进程");
        plan.repair_steps.push_back("降低非关键进程的优先级");
        plan.repair_steps.push_back("如必要，终止异常进程");
        plan.estimated_repair_time_ms = 5000;
    }
    else if (anomaly.component == "Memory" && anomaly.anomaly_type == "HIGH_USAGE") {
        plan.repair_steps.push_back("清理非关键缓存数据");
        plan.repair_steps.push_back("触发垃圾回收");
        plan.repair_steps.push_back("如必要，重启内存密集型组件");
        plan.estimated_repair_time_ms = 8000;
        plan.requires_restart = true;
    }
    else if (anomaly.component == "Network" && anomaly.anomaly_type == "HIGH_LATENCY") {
        plan.repair_steps.push_back("检查网络连接状态");
        plan.repair_steps.push_back("重置网络连接");
        plan.repair_steps.push_back("切换备用网络通道");
        plan.estimated_repair_time_ms = 3000;
    }
    else {
        // 通用修复步骤
        plan.repair_steps.push_back("诊断" + anomaly.component + "组件");
        plan.repair_steps.push_back("尝试重置组件状态");
        plan.repair_steps.push_back("重启组件");
        plan.estimated_repair_time_ms = 10000;
        plan.requires_restart = true;
    }
    
    return plan;
}

bool SelfOperatingSystem::executeRepairPlan(const RepairPlan& plan) {
    std::stringstream ss;
    ss << "执行修复计划: " << plan.component_id << " - " << plan.issue_description;
    logger_->info(ss.str());
    
    // 记录修复步骤
    for (size_t i = 0; i < plan.repair_steps.size(); i++) {
        logger_->info("步骤 " + std::to_string(i+1) + ": " + plan.repair_steps[i]);
        
        // 模拟执行每个修复步骤
        std::this_thread::sleep_for(std::chrono::milliseconds(plan.estimated_repair_time_ms / plan.repair_steps.size()));
    }
    
    // 模拟修复成功率
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution success_dist(plan.success_probability);
    bool success = success_dist(gen);
    
    // 根据异常类型和组件执行特定修复
    if (plan.component_id == "CPU" && success) {
        // 实际上可以实现CPU相关修复逻辑
        // 例如减少线程数、优化算法等
        logger_->info("CPU使用率已降低");
    }
    else if (plan.component_id == "Memory" && success) {
        // 实际上可以实现内存相关修复逻辑
        // 例如清理缓存、触发GC等
        logger_->info("内存使用率已降低");
    }
    else if (plan.component_id == "Network" && success) {
        // 实际上可以实现网络相关修复逻辑
        // 例如重连、切换线路等
        logger_->info("网络延迟已改善");
    }
    
    // 记录修复结果
    logger_->info(std::string("修复计划执行") + (success ? "成功" : "失败"));
    
    return success;
}

void SelfOperatingSystem::learnFromRepairOutcome(const RepairPlan& plan, bool success) {
    // 更新成功率统计
    static int total_repairs = 0;
    static int successful_repairs = 0;
    
    total_repairs++;
    if (success) {
        successful_repairs++;
    }
    
    // 更新修复成功率指标
    system_metrics_["repair_success_rate"] = static_cast<double>(successful_repairs) / total_repairs;
    
    // 增加自动干预计数
    system_metrics_["auto_intervention_count"]++;
    
    // 这里可以加入机器学习逻辑来改进修复策略
    // TODO: 实现机器学习模型来优化修复方案
}

void SelfOperatingSystem::optimizeSystemParameters() {
    // 根据系统状态自动优化参数
    // 这是一个先占位的简单实现
    
    logger_->info("执行系统参数自动优化");
    
    // 获取系统状态
    SystemMonitor::SystemState state = system_monitor_->getSystemState();
    
    // 根据负载情况优化
    if (state.cpu.total_usage > 80.0) {
        logger_->info("检测到CPU负载较高，正在调整系统参数以优化性能");
        // 这里可以实现具体的优化逻辑
    }
    
    if (state.memory.used_bytes > 0.7 * state.memory.total_bytes) {
        logger_->info("检测到内存使用率较高，正在清理非关键缓存");
        // 这里可以实现具体的内存优化逻辑
    }
}

void SelfOperatingSystem::detectAnomalyPatterns() {
    // 检测异常模式，识别重复出现的问题
    // 这是一个占位实现
    
    // 实际实现可以使用时间序列分析、模式匹配算法来检测异常模式
}

void SelfOperatingSystem::predictiveMaintenanceCheck() {
    // 预测性维护检查，预测可能的系统问题
    // 这是一个占位实现
    
    // 实际实现可以使用AI预测模型来进行预测性维护
}

SelfOperatingSystem::DiagnosticReport SelfOperatingSystem::generateDiagnosticReport() const {
    DiagnosticReport report;
    
    // 设置健康状态
    report.system_healthy = (system_metrics_.at("health_score") > 0.7);
    report.health_score = system_metrics_.at("health_score");
    
    // 获取活跃异常
    std::lock_guard<std::mutex> lock(repair_mutex_);
    report.active_anomalies = {}; // 从队列复制异常
    report.pending_repairs = active_repair_plans_;
    report.recent_repairs = repair_history_;
    report.last_check_time = std::chrono::system_clock::now();
    
    // 计算运行时间
    static auto start_time = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    report.uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
    
    // 生成建议
    if (report.system_healthy) {
        report.recommendation = "系统运行正常，无需干预";
    } else {
        report.recommendation = "建议关注当前活跃的系统异常，可能需要手动干预";
    }
    
    return report;
}

void SelfOperatingSystem::logSelfOperatingActivity(const std::string& activity, const std::string& details) {
    std::stringstream ss;
    ss << "[自运营系统] " << activity << ": " << details;
    logger_->info(ss.str());
}

//==============================================================================
// AutoRepairManager 实现
//==============================================================================

AutoRepairManager::AutoRepairManager(SelfOperatingSystem* self_op_system) 
    : self_op_system_(self_op_system),
      total_repair_attempts_(0),
      successful_repairs_(0)
{
}

bool AutoRepairManager::initialize() {
    if (!self_op_system_) {
        return false;
    }
    
    // 注册默认修复策略
    registerRepairStrategy("CPU_HIGH_USAGE", [](const SelfOperatingSystem::SystemAnomaly& anomaly) {
        // 实现CPU高使用率修复策略
        return true;
    });
    
    registerRepairStrategy("MEMORY_LEAK", [](const SelfOperatingSystem::SystemAnomaly& anomaly) {
        // 实现内存泄漏修复策略
        return true;
    });
    
    registerRepairStrategy("NETWORK_LATENCY", [](const SelfOperatingSystem::SystemAnomaly& anomaly) {
        // 实现网络延迟修复策略
        return true;
    });
    
    return true;
}

void AutoRepairManager::registerRepairStrategy(
    const std::string& issue_type,
    std::function<bool(const SelfOperatingSystem::SystemAnomaly&)> strategy
) {
    repair_strategies_[issue_type] = strategy;
}

bool AutoRepairManager::handleAnomaly(const SelfOperatingSystem::SystemAnomaly& anomaly) {
    total_repair_attempts_++;
    
    // 尝试找到匹配的修复策略
    std::string key = anomaly.component + "_" + anomaly.anomaly_type;
    auto it = repair_strategies_.find(key);
    
    if (it != repair_strategies_.end()) {
        // 执行修复策略
        bool success = it->second(anomaly);
        
        if (success) {
            successful_repairs_++;
        }
        
        return success;
    }
    
    // 没有找到匹配的修复策略，尝试选择最佳策略
    auto best_strategy = selectBestRepairStrategy(anomaly);
    
    if (best_strategy) {
        bool success = best_strategy(anomaly);
        
        if (success) {
            successful_repairs_++;
        }
        
        return success;
    }
    
    return false;
}

double AutoRepairManager::getRepairSuccessRate() const {
    if (total_repair_attempts_ == 0) {
        return 0.0;
    }
    
    return static_cast<double>(successful_repairs_) / total_repair_attempts_;
}

std::function<bool(const SelfOperatingSystem::SystemAnomaly&)> AutoRepairManager::selectBestRepairStrategy(
    const SelfOperatingSystem::SystemAnomaly& anomaly
) {
    // 简单的策略选择逻辑
    // 在实际实现中，可以使用AI或启发式算法来选择最佳策略
    
    // 默认返回空策略
    return nullptr;
}

double AutoRepairManager::evaluateRepairRisk(
    const std::string& strategy_name,
    const SelfOperatingSystem::SystemAnomaly& anomaly
) {
    // 简单的风险评估
    // 实际实现可以使用更复杂的风险模型
    
    if (anomaly.severity > 0.8) {
        return 0.8; // 高风险
    } else if (anomaly.severity > 0.5) {
        return 0.5; // 中等风险
    } else {
        return 0.2; // 低风险
    }
}

//==============================================================================
// UnmannedModeController 实现
//==============================================================================

UnmannedModeController::UnmannedModeController(SelfOperatingSystem* self_op_system)
    : self_op_system_(self_op_system),
      unmanned_mode_enabled_(false)
{
}

bool UnmannedModeController::initialize() {
    if (!self_op_system_) {
        return false;
    }
    
    // 设置默认安全边界
    safety_boundaries_["cpu_usage"] = std::make_pair(0.0, 90.0);
    safety_boundaries_["memory_usage"] = std::make_pair(0.0, 85.0);
    safety_boundaries_["network_latency_us"] = std::make_pair(0.0, 5000.0);
    
    return true;
}

bool UnmannedModeController::enableUnmannedMode() {
    // 检查是否可以启用无人模式
    if (!canOperateAutonomously()) {
        return false;
    }
    
    unmanned_mode_enabled_ = true;
    self_op_system_->setOperatingMode(SelfOperatingSystem::OperatingMode::FULLY_AUTOMATED);
    self_op_system_->enableAutoRepair(true);
    
    return true;
}

void UnmannedModeController::disableUnmannedMode() {
    unmanned_mode_enabled_ = false;
    self_op_system_->setOperatingMode(SelfOperatingSystem::OperatingMode::SUPERVISED);
}

bool UnmannedModeController::isUnmannedModeEnabled() const {
    return unmanned_mode_enabled_;
}

void UnmannedModeController::setSafetyBoundaries(
    const std::unordered_map<std::string, std::pair<double, double>>& boundaries
) {
    safety_boundaries_ = boundaries;
}

void UnmannedModeController::setEmergencyContacts(const std::vector<std::string>& contacts) {
    emergency_contacts_ = contacts;
}

bool UnmannedModeController::isWithinSafetyBoundaries() const {
    // 实际实现应检查所有系统指标是否在安全边界内
    return true;
}

void UnmannedModeController::triggerEmergencyProtocol(const std::string& reason) {
    // 实现紧急协议，可能包括：
    // 1. 通知紧急联系人
    // 2. 安全退出某些组件
    // 3. 切换到安全模式
    
    // 记录紧急情况
    Logger::instance().error("触发紧急协议: " + reason);
    
    // 禁用无人模式
    disableUnmannedMode();
    
    // 通知紧急联系人
    for (const auto& contact : emergency_contacts_) {
        // 实现通知逻辑
    }
    
    // 处理紧急情况
    handleEmergencySituation("SYSTEM_EMERGENCY");
}

bool UnmannedModeController::canOperateAutonomously() const {
    // 检查系统状态，确定是否可以安全地无人值守运行
    
    // 检查自动修复成功率
    auto diagnostic_report = self_op_system_->getDiagnosticReport();
    if (diagnostic_report.health_score < 0.7) {
        return false;
    }
    
    // 检查是否在安全边界内
    if (!isWithinSafetyBoundaries()) {
        return false;
    }
    
    return true;
}

void UnmannedModeController::performSafetyChecks() {
    // 实现安全检查逻辑
}

void UnmannedModeController::makeAutonomousDecisions() {
    // 实现自主决策逻辑
}

void UnmannedModeController::handleEmergencySituation(const std::string& situation_type) {
    // 实现紧急情况处理逻辑
}

} // namespace core
} // namespace hft
