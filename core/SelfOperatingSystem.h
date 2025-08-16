#pragma once

#include <vector>
#include <chrono>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <queue>

#include "../core/FaultDetector.h"
#include "../core/FaultToleranceManager.h"
#include "../core/SystemMonitor.h"
#include "../core/Logger.h"
#include "../ai/GlobalRiskPredictionModel.h"

namespace hft {
namespace core {

/**
 * 🔄 自运营系统 - SelfOperatingSystem 🔄
 * 
 * 核心特性：
 * - 🛠️ 自动检测与修复系统故障和异常
 * - 🧠 自适应优化系统配置
 * - 🔒 无人值守模式支持
 * - 📊 实时健康监控与自动干预
 * - 🔍 智能根因分析
 * - 📈 系统性能自优化
 * - 🔄 持续自我学习和进化
 */
class SelfOperatingSystem {
public:
    // 自运营系统状态
    enum class OperatingMode {
        SUPERVISED,      // 有人监督模式
        SEMI_AUTOMATED,  // 半自动模式
        FULLY_AUTOMATED  // 完全自动（无人值守）模式
    };
    
    // 修复计划结构
    struct RepairPlan {
        std::string component_id;
        std::string issue_description;
        std::vector<std::string> repair_steps;
        double success_probability;
        uint64_t estimated_repair_time_ms;
        bool requires_restart;
        bool requires_human_approval;
    };
    
    // 系统异常结构
    struct SystemAnomaly {
        std::string component;
        std::string anomaly_type;
        std::string description;
        uint64_t detection_timestamp;
        double severity;
        std::vector<std::string> affected_components;
        std::vector<std::string> potential_causes;
        bool auto_repairable;
    };
    
    // 构造函数
    SelfOperatingSystem(
        FaultDetector* fault_detector,
        FaultToleranceManager* fault_manager,
        SystemMonitor* system_monitor,
        Logger* logger
    );
    
    // 析构函数
    ~SelfOperatingSystem();
    
    // 初始化自运营系统
    bool initialize();
    
    // 启动自运营服务
    bool start();
    
    // 停止自运营服务
    void stop();
    
    // 设置运行模式
    void setOperatingMode(OperatingMode mode);
    
    // 获取当前运行模式
    OperatingMode getOperatingMode() const;
    
    // 注册系统修复完成回调
    using RepairCallback = std::function<void(const RepairPlan&, bool)>;
    void registerRepairCallback(RepairCallback callback);
    
    // 手动触发系统检查
    void triggerSystemCheck();
    
    // 获取系统自我诊断报告
    struct DiagnosticReport {
        bool system_healthy;
        double health_score;
        std::vector<SystemAnomaly> active_anomalies;
        std::vector<RepairPlan> pending_repairs;
        std::vector<std::string> recent_repairs;
        std::chrono::system_clock::time_point last_check_time;
        uint64_t uptime_seconds;
        std::string recommendation;
    };
    DiagnosticReport getDiagnosticReport() const;
    
    // 启用/禁用自动修复
    void enableAutoRepair(bool enable);
    
    // 自动修复是否启用
    bool isAutoRepairEnabled() const;

private:
    // 系统组件
    FaultDetector* fault_detector_;
    FaultToleranceManager* fault_manager_;
    SystemMonitor* system_monitor_;
    Logger* logger_;
    
    // 内部状态
    std::atomic<bool> is_running_;
    std::atomic<OperatingMode> operating_mode_;
    std::atomic<bool> auto_repair_enabled_;
    
    // 监控线程
    std::thread monitoring_thread_;
    std::mutex repair_mutex_;
    
    // 异常和修复队列
    std::queue<SystemAnomaly> anomaly_queue_;
    std::vector<RepairPlan> active_repair_plans_;
    std::vector<std::string> repair_history_;
    
    // 回调
    RepairCallback repair_callback_;
    
    // 系统关键指标
    std::unordered_map<std::string, double> system_metrics_;
    
    // 系统自检
    void systemSelfCheckLoop();
    
    // 异常检测
    void detectSystemAnomalies();
    
    // 根因分析
    std::vector<std::string> analyzeRootCause(const SystemAnomaly& anomaly);
    
    // 生成修复计划
    RepairPlan generateRepairPlan(const SystemAnomaly& anomaly);
    
    // 执行修复
    bool executeRepairPlan(const RepairPlan& plan);
    
    // 学习修复结果
    void learnFromRepairOutcome(const RepairPlan& plan, bool success);
    
    // 优化系统参数
    void optimizeSystemParameters();
    
    // 检查异常模式
    void detectAnomalyPatterns();
    
    // 预测性维护
    void predictiveMaintenanceCheck();
    
    // 生成诊断报告
    DiagnosticReport generateDiagnosticReport() const;
    
    // 记录自运营活动
    void logSelfOperatingActivity(const std::string& activity, const std::string& details);
};

// 自动修复管理器
class AutoRepairManager {
public:
    AutoRepairManager(SelfOperatingSystem* self_op_system);
    
    // 初始化
    bool initialize();
    
    // 注册修复策略
    void registerRepairStrategy(
        const std::string& issue_type,
        std::function<bool(const SelfOperatingSystem::SystemAnomaly&)> strategy
    );
    
    // 处理异常
    bool handleAnomaly(const SelfOperatingSystem::SystemAnomaly& anomaly);
    
    // 获取修复成功率
    double getRepairSuccessRate() const;
    
private:
    SelfOperatingSystem* self_op_system_;
    std::unordered_map<std::string, std::function<bool(const SelfOperatingSystem::SystemAnomaly&)>> repair_strategies_;
    std::atomic<uint64_t> total_repair_attempts_;
    std::atomic<uint64_t> successful_repairs_;
    
    // 智能选择修复策略
    std::function<bool(const SelfOperatingSystem::SystemAnomaly&)> selectBestRepairStrategy(
        const SelfOperatingSystem::SystemAnomaly& anomaly
    );
    
    // 评估修复风险
    double evaluateRepairRisk(
        const std::string& strategy_name,
        const SelfOperatingSystem::SystemAnomaly& anomaly
    );
};

// 无人模式控制器
class UnmannedModeController {
public:
    UnmannedModeController(SelfOperatingSystem* self_op_system);
    
    // 初始化
    bool initialize();
    
    // 启用无人模式
    bool enableUnmannedMode();
    
    // 禁用无人模式
    void disableUnmannedMode();
    
    // 无人模式是否启用
    bool isUnmannedModeEnabled() const;
    
    // 设置安全边界
    void setSafetyBoundaries(const std::unordered_map<std::string, std::pair<double, double>>& boundaries);
    
    // 设置紧急联系人
    void setEmergencyContacts(const std::vector<std::string>& contacts);
    
    // 检查系统状态是否在安全边界内
    bool isWithinSafetyBoundaries() const;
    
    // 触发紧急程序
    void triggerEmergencyProtocol(const std::string& reason);
    
private:
    SelfOperatingSystem* self_op_system_;
    std::atomic<bool> unmanned_mode_enabled_;
    std::unordered_map<std::string, std::pair<double, double>> safety_boundaries_;
    std::vector<std::string> emergency_contacts_;
    
    // 决策逻辑
    bool canOperateAutonomously() const;
    
    // 安全检查
    void performSafetyChecks();
    
    // 自动化决策
    void makeAutonomousDecisions();
    
    // 紧急状态处理
    void handleEmergencySituation(const std::string& situation_type);
};

} // namespace core
} // namespace hft
