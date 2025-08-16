#pragma once

/**
 * 🧙‍♂️ 智能配置向导 - 交互式配置生成器
 * 
 * 功能：
 * - 🎯 交互式场景选择
 * - 💻 硬件配置检测与建议
 * - ⚖️ 性能 vs 精度权衡分析
 * - 📊 实时配置验证
 * - 🔮 智能参数推荐
 * - 💰 成本效益分析
 * - 📈 预期性能预测
 */

#include "ProfessionalConfigGenerator.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace hft {
namespace ai {
namespace config {

/**
 * 🎯 用户需求分析
 */
struct UserRequirements {
    TradingScenario primary_scenario;
    std::vector<TradingScenario> secondary_scenarios;
    
    // 性能要求
    double max_acceptable_latency_us;
    double min_required_accuracy;
    int min_throughput_tps;
    double max_memory_usage_gb;
    
    // 风险偏好
    double risk_tolerance;              // 0.0 - 1.0
    bool enable_aggressive_optimization;
    bool prefer_conservative_approach;
    
    // 预算约束
    double hardware_budget_usd;
    double operational_cost_limit_monthly;
    bool cloud_deployment_preferred;
    
    // 监管要求
    std::vector<std::string> jurisdictions;
    std::vector<std::string> compliance_requirements;
    
    // 技术偏好
    bool quantum_computing_available;
    bool prefer_open_source;
    bool require_explainable_ai;
};

/**
 * 💻 系统环境检测器
 */
class SystemProfiler {
public:
    static HardwareProfile detectHardware() {
        HardwareProfile profile;
        
        // 检测CPU
        profile.cpu_cores = detectCPUCores();
        
        // 检测内存
        profile.memory_gb = detectMemorySize();
        
        // 检测GPU
        auto gpu_info = detectGPUs();
        profile.gpu_count = gpu_info.first;
        profile.gpu_model = gpu_info.second;
        
        // 检测网络延迟
        profile.network_latency_us = measureNetworkLatency();
        
        // 检测存储性能
        profile.storage_iops = measureStorageIOPS();
        
        // 量子处理器检测
        profile.has_quantum_processor = detectQuantumProcessor();
        
        profile.name = "Auto_Detected";
        
        return profile;
    }
    
private:
    static int detectCPUCores();
    static double detectMemorySize();
    static std::pair<int, std::string> detectGPUs();
    static double measureNetworkLatency();
    static int measureStorageIOPS();
    static bool detectQuantumProcessor();
};

/**
 * 🧙‍♂️ 智能配置向导
 */
class IntelligentConfigWizard {
public:
    IntelligentConfigWizard();
    ~IntelligentConfigWizard();
    
    // 主要入口点
    json runInteractiveWizard();
    
    // 自动化模式
    json generateOptimalConfig(const UserRequirements& requirements);
    
    // 专家模式
    json runExpertMode();
    
private:
    std::unique_ptr<ProfessionalConfigGenerator> generator_;
    std::unique_ptr<SystemProfiler> profiler_;
    
    // 交互式步骤
    UserRequirements collectUserRequirements();
    TradingScenario selectTradingScenario();
    HardwareProfile configureHardware();
    void setPerformanceTargets(UserRequirements& req);
    void configureRiskManagement(UserRequirements& req);
    void selectComplianceRequirements(UserRequirements& req);
    
    // 智能分析
    json analyzeTradeoffs(const json& config);
    std::vector<std::string> generateRecommendations(const json& config);
    double estimatePerformance(const json& config);
    double estimateCost(const json& config);
    
    // 配置验证
    bool validateConfiguration(const json& config);
    std::vector<std::string> identifyPotentialIssues(const json& config);
    json suggestImprovements(const json& config);
    
    // 用户界面辅助
    void displayWelcomeMessage();
    void displayConfiguration(const json& config);
    void displayPerformancePrediction(const json& config);
    void displayCostAnalysis(const json& config);
    bool askUserConfirmation(const std::string& message);
    
    // 模板生成
    json generateDeploymentScript(const json& config);
    json generateMonitoringConfig(const json& config);
    json generateDocumentation(const json& config);
};

/**
 * 📊 性能预测器
 */
class PerformancePredictor {
public:
    struct PerformancePrediction {
        double predicted_latency_us;
        double predicted_accuracy;
        int predicted_throughput_tps;
        double predicted_memory_usage_gb;
        double confidence_level;
        
        struct CostEstimate {
            double hardware_cost_usd;
            double monthly_operational_cost;
            double development_time_weeks;
            double maintenance_effort_hours_monthly;
        } cost_estimate;
        
        struct RiskAssessment {
            double implementation_risk;     // 0.0 - 1.0
            double performance_risk;
            double regulatory_risk;
            double technology_risk;
        } risk_assessment;
    };
    
    static PerformancePrediction predictPerformance(
        const json& config,
        const HardwareProfile& hardware
    );
    
private:
    static double estimateLatency(const json& config, const HardwareProfile& hardware);
    static double estimateAccuracy(const json& config);
    static int estimateThroughput(const json& config, const HardwareProfile& hardware);
    static double estimateMemoryUsage(const json& config);
    static PerformancePrediction::CostEstimate estimateCosts(const json& config);
    static PerformancePrediction::RiskAssessment assessRisks(const json& config);
};

/**
 * 🎯 配置比较器
 */
class ConfigurationComparator {
public:
    struct ComparisonResult {
        std::string config_name;
        json config;
        PerformancePredictor::PerformancePrediction performance;
        double overall_score;
        std::vector<std::string> pros;
        std::vector<std::string> cons;
    };
    
    static std::vector<ComparisonResult> compareConfigurations(
        const std::vector<std::pair<std::string, json>>& configs,
        const UserRequirements& requirements
    );
    
    static void displayComparison(const std::vector<ComparisonResult>& results);
    
private:
    static double calculateOverallScore(
        const PerformancePredictor::PerformancePrediction& perf,
        const UserRequirements& req
    );
    
    static std::vector<std::string> identifyPros(
        const json& config,
        const PerformancePredictor::PerformancePrediction& perf
    );
    
    static std::vector<std::string> identifyCons(
        const json& config,
        const PerformancePredictor::PerformancePrediction& perf
    );
};

/**
 * 🚀 快速配置生成器
 */
class QuickConfigGenerator {
public:
    // 一键生成常用配置
    static json generateForDay1Trading() {
        // 适合初学者的基础配置
        return json{
            {"scenario", "beginner_friendly"},
            {"target_latency_ns", 10000000},    // 10ms
            {"target_accuracy", 0.85},
            {"model", {
                {"architecture", "SIMPLE_TRANSFORMER"},
                {"hidden_dim", 256},
                {"num_layers", 4}
            }},
            {"quantum", {"enabled", false}},
            {"features", {
                {"basic_price_features", true},
                {"advanced_features", false}
            }}
        };
    }
    
    static json generateForProduction() {
        // 生产环境标准配置
        return json{
            {"scenario", "production_ready"},
            {"target_latency_ns", 500000},      // 500微秒
            {"target_accuracy", 0.93},
            {"model", {
                {"architecture", "MULTI_MODAL_TRANSFORMER"},
                {"hidden_dim", 768},
                {"num_layers", 10}
            }},
            {"quantum", {"enabled", true}},
            {"monitoring", {
                {"enable_all", true},
                {"alert_levels", {"warning", "critical"}}
            }}
        };
    }
    
    static json generateForResearch() {
        // 研究和实验配置
        return json{
            {"scenario", "research_experimental"},
            {"target_latency_ns", 5000000},     // 5ms
            {"target_accuracy", 0.98},
            {"model", {
                {"architecture", "EXPERIMENTAL_HYBRID"},
                {"hidden_dim", 1536},
                {"num_layers", 24},
                {"enable_experimental_features", true}
            }},
            {"quantum", {
                {"enabled", true},
                {"experimental_algorithms", true}
            }}
        };
    }
    
    static json generateForBacktesting() {
        // 回测专用配置
        return json{
            {"scenario", "backtesting"},
            {"target_latency_ns", 1000000},     // 1ms
            {"target_accuracy", 0.95},
            {"backtesting", {
                {"enable_paper_trading", true},
                {"historical_data_years", 5},
                {"walk_forward_optimization", true}
            }},
            {"model", {
                {"architecture", "BACKTESTING_OPTIMIZED"},
                {"batch_processing", true}
            }}
        };
    }
};

} // namespace config
} // namespace ai
} // namespace hft
