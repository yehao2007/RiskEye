#pragma once

/**
 * 🏆 世界最强风险控制系统 - 终极风险管理 🏆
 * 
 * 特性：
 * - 🛡️ 毫秒级风险监控
 * - 🧠 AI预测性风险分析
 * - ⚡ 自动风险响应
 * - 🔮 量子风险计算
 * - 🌐 全球风险感知
 * - 🎯 精准风险定位
 * - 🚨 多维度预警系统
 * - 🔒 智能风险隔离
 */

#include <memory>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <functional>
#include <queue>
#include <algorithm>
#include <cmath>

#include <nlohmann/json.hpp>
#include <eigen3/Eigen/Dense>

using json = nlohmann::json;
using namespace std::chrono;

namespace hft::risk {

/**
 * ⚡ 超级实时风险引擎
 */
class UltraRealTimeRiskEngine {
public:
    enum class RiskType {
        MARKET_RISK,
        CREDIT_RISK,
        LIQUIDITY_RISK,
        OPERATIONAL_RISK,
        SYSTEMIC_RISK,
        COUNTERPARTY_RISK,
        MODEL_RISK,
        REGULATORY_RISK,
        CYBER_RISK,
        CLIMATE_RISK,
        POLITICAL_RISK,
        TECHNOLOGY_RISK,
        QUANTUM_RISK        // 量子计算风险
    };

    enum class RiskLevel {
        NEGLIGIBLE = 0,
        LOW = 1,
        MEDIUM = 2,
        HIGH = 3,
        CRITICAL = 4,
        CATASTROPHIC = 5    // 灾难级风险
    };

    struct RiskMetrics {
        double value_at_risk_1d;         // 1日VaR
        double value_at_risk_10d;        // 10日VaR
        double expected_shortfall;       // 期望损失
        double maximum_drawdown;         // 最大回撤
        double sharpe_ratio;             // 夏普比率
        double sortino_ratio;            // 索提诺比率
        double calmar_ratio;             // 卡尔玛比率
        double current_leverage;         // 当前杠杆
        double beta_exposure;            // Beta敞口
        double correlation_risk;         // 相关性风险
        double concentration_risk;       // 集中度风险
        double liquidity_risk_score;     // 流动性风险评分
        double stress_test_score;        // 压力测试评分
        double model_uncertainty;        // 模型不确定性
        milliseconds calculation_time;   // 计算时间
        high_resolution_clock::time_point timestamp;
    };

    struct RiskAlert {
        std::string alert_id;
        RiskType risk_type;
        RiskLevel risk_level;
        std::string description;
        std::string affected_assets;
        double estimated_impact;
        double probability;
        json risk_factors;
        std::vector<std::string> suggested_actions;
        bool requires_immediate_action;
        high_resolution_clock::time_point alert_time;
        high_resolution_clock::time_point expiry_time;
    };

    UltraRealTimeRiskEngine();
    ~UltraRealTimeRiskEngine();

    bool initialize();
    void shutdown();

    // 实时风险计算（纳秒级）
    RiskMetrics calculateInstantRisk() const;
    RiskMetrics calculatePredictiveRisk(milliseconds time_horizon) const;
    
    // 多维风险分析
    std::unordered_map<RiskType, double> analyzeRiskByType() const;
    std::unordered_map<std::string, double> analyzeRiskByAsset() const;
    std::unordered_map<std::string, double> analyzeRiskByStrategy() const;
    std::unordered_map<std::string, double> analyzeRiskByTimeframe() const;

    // 极限风险场景
    double calculateTailRisk(double confidence_level = 0.999) const;
    double calculateBlackSwanProbability() const;
    json simulateExtremeScenarios() const;
    double calculateSystemicRiskContribution() const;

    // 量子风险计算
    double calculateQuantumRisk() const;
    json quantumMonteCarloSimulation(int iterations = 1000000) const;
    double quantumEntanglementRiskAnalysis() const;

    // 风险预警系统
    std::vector<RiskAlert> generateRiskAlerts() const;
    void setRiskThresholds(const json& thresholds);
    bool checkCriticalRiskBreach() const;
    void enableEarlyWarningSystem(bool enable = true);

    // 自动风险响应
    void setAutoRiskResponse(bool enable = true);
    json executeEmergencyRiskProtocol();
    void activateRiskCircuitBreaker();
    void enableAdaptiveRiskLimits(bool enable = true);

private:
    struct RiskCalculationEngine* engine_;
    mutable std::mutex risk_mutex_;
    std::thread risk_calculation_thread_;
    std::atomic<bool> calculation_active_{false};
    
    // 风险阈值
    json risk_thresholds_;
    std::atomic<bool> auto_response_enabled_{false};
    std::atomic<bool> early_warning_enabled_{true};
    
    // 性能监控
    std::atomic<long long> calculations_per_second_{0};
    std::atomic<double> average_calculation_time_ns_{0.0};
    
    void riskCalculationLoop();
    void updateRiskMetrics();
    void processRiskAlerts();
    void optimizeCalculationPerformance();
};

/**
 * 🧠 AI风险预测系统
 */
class AIRiskPredictionSystem {
public:
    enum class ModelType {
        LSTM_DEEP_NEURAL_NET,
        TRANSFORMER_ATTENTION,
        CONVOLUTIONAL_NEURAL_NET,
        GRADIENT_BOOSTING_ENSEMBLE,
        QUANTUM_NEURAL_NETWORK,
        GENERATIVE_ADVERSARIAL_NET,
        REINFORCEMENT_LEARNING_AGENT,
        HYBRID_AI_MODEL
    };

    enum class PredictionHorizon {
        MICROSECONDS,
        MILLISECONDS,
        SECONDS,
        MINUTES,
        HOURS,
        DAYS,
        WEEKS,
        MONTHS
    };

    struct RiskPrediction {
        double predicted_risk_score;
        double confidence_interval_lower;
        double confidence_interval_upper;
        double prediction_accuracy;
        std::vector<std::string> key_risk_factors;
        json feature_importance;
        PredictionHorizon horizon;
        high_resolution_clock::time_point prediction_time;
        high_resolution_clock::time_point target_time;
    };

    struct ModelPerformance {
        double accuracy;
        double precision;
        double recall;
        double f1_score;
        double auc_roc;
        double mean_absolute_error;
        double root_mean_square_error;
        int total_predictions;
        int correct_predictions;
        milliseconds average_inference_time;
    };

    AIRiskPredictionSystem();
    ~AIRiskPredictionSystem();

    bool initialize();
    void shutdown();

    // 模型管理
    bool loadModel(ModelType type, const std::string& model_path);
    void trainModel(ModelType type, const json& training_data);
    void updateModelRealTime(const json& new_data);
    void enableEnsembleModeling(bool enable = true);

    // 风险预测
    RiskPrediction predictRisk(PredictionHorizon horizon) const;
    std::vector<RiskPrediction> predictMultiHorizonRisk() const;
    json predictRiskByAssetClass() const;
    json predictMarketRegimeChange() const;

    // 高级预测功能
    json predictBlackSwanEvents() const;
    double predictVolatilitySpike(milliseconds time_ahead) const;
    json predictLiquidityCrisis() const;
    double predictCorrelationBreakdown() const;

    // 模型解释性
    json explainPrediction(const RiskPrediction& prediction) const;
    std::vector<std::string> getTopRiskFactors(int top_n = 10) const;
    json generateRiskScenarios(int num_scenarios = 100) const;

    // 模型性能监控
    ModelPerformance getModelPerformance(ModelType type) const;
    void benchmarkModels();
    void enableAutomaticModelSelection(bool enable = true);
    void setModelUpdateFrequency(milliseconds frequency);

private:
    std::unordered_map<ModelType, std::unique_ptr<class AIModel>> models_;
    std::unique_ptr<class EnsembleManager> ensemble_manager_;
    std::unique_ptr<class FeatureEngine> feature_engine_;
    
    mutable std::mutex prediction_mutex_;
    std::thread model_update_thread_;
    std::atomic<bool> model_updating_{false};
    
    // 性能监控
    std::unordered_map<ModelType, ModelPerformance> model_performance_;
    
    void modelUpdateLoop();
    void retrainModels();
    void optimizeModelParameters();
    void validateModelAccuracy();
};

/**
 * 🌐 全球风险感知系统
 */
class GlobalRiskSentinelSystem {
public:
    enum class RiskSource {
        MARKET_DATA_FEEDS,
        NEWS_SENTIMENT,
        SOCIAL_MEDIA,
        REGULATORY_ANNOUNCEMENTS,
        ECONOMIC_INDICATORS,
        GEOPOLITICAL_EVENTS,
        WEATHER_CLIMATE,
        CYBER_THREAT_INTEL,
        SUPPLY_CHAIN_MONITORS,
        SATELLITE_IMAGERY,
        ALTERNATIVE_DATA,
        QUANTUM_SENSORS
    };

    struct GlobalRiskEvent {
        std::string event_id;
        RiskSource source;
        std::string description;
        std::string geographic_region;
        std::vector<std::string> affected_markets;
        std::vector<std::string> affected_assets;
        double severity_score;
        double propagation_probability;
        double estimated_market_impact;
        json event_metadata;
        high_resolution_clock::time_point detection_time;
        high_resolution_clock::time_point estimated_impact_time;
    };

    struct SentinelConfig {
        std::vector<RiskSource> monitored_sources;
        std::vector<std::string> monitored_regions;
        std::vector<std::string> monitored_assets;
        double sensitivity_threshold;
        bool enable_realtime_monitoring;
        bool enable_predictive_alerts;
        milliseconds update_frequency;
        bool enable_cross_correlation_analysis;
    };

    GlobalRiskSentinelSystem();
    ~GlobalRiskSentinelSystem();

    bool initialize(const SentinelConfig& config);
    void shutdown();

    // 全球风险监控
    std::vector<GlobalRiskEvent> scanGlobalRisks() const;
    json analyzeRiskPropagation(const std::string& event_id) const;
    double calculateGlobalRiskIndex() const;
    json detectSystemicRiskBuildup() const;

    // 区域风险分析
    json analyzeRegionalRisk(const std::string& region) const;
    std::vector<std::string> identifyRiskCorridors() const;
    json trackRiskMigration() const;

    // 实时风险感知
    void enableRealTimeSentinel(bool enable = true);
    json getInstantGlobalRiskSnapshot() const;
    std::vector<GlobalRiskEvent> getEmergingRisks() const;
    void setCustomRiskFilters(const json& filters);

    // 风险情报融合
    void addRiskDataSource(RiskSource source, const json& config);
    void enableCrowdsourcedRiskData(bool enable = true);
    json fuseMultiSourceIntelligence() const;

    // 预警系统
    void setGlobalAlertThresholds(const json& thresholds);
    std::vector<GlobalRiskEvent> generateGlobalAlerts() const;
    void enableCascadeRiskDetection(bool enable = true);

private:
    SentinelConfig config_;
    std::vector<std::unique_ptr<class RiskDataCollector>> data_collectors_;
    std::unique_ptr<class RiskCorrelationEngine> correlation_engine_;
    std::unique_ptr<class GeospatialRiskAnalyzer> geospatial_analyzer_;
    
    mutable std::mutex sentinel_mutex_;
    std::thread monitoring_thread_;
    std::atomic<bool> monitoring_active_{false};
    
    // 风险事件存储
    std::queue<GlobalRiskEvent> risk_events_;
    json global_risk_index_;
    
    void monitoringLoop();
    void collectRiskData();
    void analyzeRiskCorrelations();
    void updateGlobalRiskIndex();
    void processAlerts();
};

/**
 * 🎯 精准风险定位系统
 */
class PrecisionRiskTargetingSystem {
public:
    enum class TargetingMode {
        ASSET_LEVEL,
        STRATEGY_LEVEL,
        PORTFOLIO_LEVEL,
        COUNTERPARTY_LEVEL,
        SECTOR_LEVEL,
        GEOGRAPHIC_LEVEL,
        TIME_BASED_LEVEL,
        QUANTUM_LEVEL
    };

    struct RiskTarget {
        std::string target_id;
        TargetingMode mode;
        std::string description;
        double risk_score;
        double risk_contribution;
        double marginal_var;
        double component_var;
        std::vector<std::string> risk_drivers;
        json risk_decomposition;
        json mitigation_strategies;
    };

    struct RiskAttribution {
        std::string attribution_id;
        std::unordered_map<std::string, double> factor_contributions;
        std::unordered_map<std::string, double> asset_contributions;
        std::unordered_map<std::string, double> strategy_contributions;
        double unexplained_risk;
        double model_uncertainty;
        high_resolution_clock::time_point calculation_time;
    };

    PrecisionRiskTargetingSystem();
    ~PrecisionRiskTargetingSystem();

    bool initialize();
    void shutdown();

    // 精准风险定位
    std::vector<RiskTarget> identifyHighestRiskTargets(int top_n = 20) const;
    RiskTarget analyzeSpecificTarget(const std::string& target_id, TargetingMode mode) const;
    json decomposeRiskHierarchy() const;

    // 风险归因分析
    RiskAttribution performRiskAttribution() const;
    json analyzeRiskFactorExposure() const;
    std::unordered_map<std::string, double> calculateMarginalRiskContributions() const;
    
    // 动态风险追踪
    void enableDynamicRiskTracking(bool enable = true);
    json trackRiskMigration() const;
    std::vector<RiskTarget> identifyEmergingRiskTargets() const;

    // 风险优化建议
    json generateRiskOptimizationSuggestions() const;
    json calculateOptimalHedgingStrategy() const;
    std::vector<std::string> recommendRiskMitigationActions() const;

    // 量子精准定位
    json quantumRiskLocalization() const;
    double calculateQuantumRiskEntanglement(const std::string& target1, const std::string& target2) const;
    json performQuantumRiskTomography() const;

private:
    std::unique_ptr<class RiskDecompositionEngine> decomposition_engine_;
    std::unique_ptr<class AttributionCalculator> attribution_calculator_;
    std::unique_ptr<class RiskOptimizer> risk_optimizer_;
    
    mutable std::mutex targeting_mutex_;
    std::thread tracking_thread_;
    std::atomic<bool> tracking_active_{false};
    
    // 风险目标缓存
    std::unordered_map<std::string, RiskTarget> risk_targets_;
    
    void riskTrackingLoop();
    void updateRiskTargets();
    void recalculateAttributions();
    void optimizeRiskAllocation();
};

/**
 * 🚨 多维度预警系统
 */
class MultiDimensionalAlertSystem {
public:
    enum class AlertType {
        THRESHOLD_BREACH,
        TREND_REVERSAL,
        ANOMALY_DETECTION,
        CORRELATION_BREAKDOWN,
        VOLATILITY_SPIKE,
        LIQUIDITY_DROUGHT,
        CONCENTRATION_RISK,
        MODEL_BREAKDOWN,
        REGULATORY_VIOLATION,
        OPERATIONAL_FAILURE,
        CYBER_ATTACK,
        MARKET_MANIPULATION,
        FLASH_CRASH,
        QUANTUM_INTERFERENCE
    };

    enum class AlertPriority {
        INFO = 0,
        LOW = 1,
        MEDIUM = 2,
        HIGH = 3,
        CRITICAL = 4,
        EMERGENCY = 5
    };

    struct Alert {
        std::string alert_id;
        AlertType type;
        AlertPriority priority;
        std::string title;
        std::string description;
        std::string affected_entity;
        double severity_score;
        double confidence_level;
        json alert_data;
        std::vector<std::string> recommended_actions;
        high_resolution_clock::time_point trigger_time;
        high_resolution_clock::time_point acknowledgment_time;
        high_resolution_clock::time_point resolution_time;
        bool acknowledged;
        bool resolved;
        std::string assigned_to;
    };

    struct AlertRule {
        std::string rule_id;
        std::string rule_name;
        AlertType type;
        AlertPriority priority;
        json trigger_conditions;
        json alert_template;
        bool enabled;
        milliseconds cooldown_period;
        high_resolution_clock::time_point last_triggered;
    };

    MultiDimensionalAlertSystem();
    ~MultiDimensionalAlertSystem();

    bool initialize();
    void shutdown();

    // 预警规则管理
    void addAlertRule(const AlertRule& rule);
    void updateAlertRule(const std::string& rule_id, const AlertRule& rule);
    void removeAlertRule(const std::string& rule_id);
    void enableAlertRule(const std::string& rule_id, bool enable = true);

    // 实时预警监控
    std::vector<Alert> checkAllAlerts() const;
    Alert evaluateSpecificRule(const std::string& rule_id) const;
    void setAlertThresholds(const json& thresholds);

    // 异常检测
    std::vector<Alert> detectAnomalies() const;
    json performOutlierAnalysis() const;
    std::vector<Alert> detectPatternBreaks() const;

    // 预警处理
    void acknowledgeAlert(const std::string& alert_id, const std::string& user_id);
    void resolveAlert(const std::string& alert_id, const std::string& resolution_notes);
    void escalateAlert(const std::string& alert_id, AlertPriority new_priority);

    // 预警优化
    void enableAdaptiveAlerts(bool enable = true);
    void calibrateAlertSensitivity();
    json analyzeAlertEffectiveness() const;
    void enableMachineLearningAlerts(bool enable = true);

    // 通知系统
    void enableEmailNotifications(bool enable = true);
    void enableSMSNotifications(bool enable = true);
    void enableSlackNotifications(bool enable = true);
    void enablePushNotifications(bool enable = true);

private:
    std::unordered_map<std::string, AlertRule> alert_rules_;
    std::queue<Alert> active_alerts_;
    std::unique_ptr<class AnomalyDetector> anomaly_detector_;
    std::unique_ptr<class NotificationManager> notification_manager_;
    
    mutable std::mutex alert_mutex_;
    std::thread alert_monitoring_thread_;
    std::atomic<bool> monitoring_active_{false};
    
    // 预警统计
    std::atomic<int> total_alerts_generated_{0};
    std::atomic<int> false_positive_count_{0};
    std::atomic<double> average_resolution_time_ms_{0.0};
    
    void alertMonitoringLoop();
    void evaluateAlertRules();
    void processNewAlerts();
    void sendNotifications(const Alert& alert);
    void updateAlertStatistics();
};

/**
 * 🔒 智能风险隔离系统
 */
class IntelligentRiskIsolationSystem {
public:
    enum class IsolationMethod {
        POSITION_QUARANTINE,
        STRATEGY_SUSPENSION,
        COUNTERPARTY_BLACKLIST,
        ASSET_RESTRICTION,
        MARKET_DISCONNECTION,
        SYSTEM_SHUTDOWN,
        QUANTUM_ISOLATION
    };

    enum class IsolationTrigger {
        MANUAL_TRIGGER,
        AUTOMATIC_THRESHOLD,
        AI_PREDICTION,
        REGULATORY_REQUIREMENT,
        SYSTEMIC_RISK_ALERT,
        EMERGENCY_PROTOCOL
    };

    struct IsolationAction {
        std::string action_id;
        IsolationMethod method;
        IsolationTrigger trigger;
        std::string target_entity;
        std::string reason;
        double severity_score;
        json isolation_parameters;
        high_resolution_clock::time_point activation_time;
        high_resolution_clock::time_point planned_release_time;
        bool active;
        std::string authorized_by;
    };

    IntelligentRiskIsolationSystem();
    ~IntelligentRiskIsolationSystem();

    bool initialize();
    void shutdown();

    // 风险隔离操作
    std::string activateIsolation(IsolationMethod method, const std::string& target, const std::string& reason);
    void deactivateIsolation(const std::string& action_id);
    void modifyIsolation(const std::string& action_id, const json& new_parameters);

    // 自动隔离规则
    void setAutoIsolationRules(const json& rules);
    void enableAutoIsolation(bool enable = true);
    json evaluateIsolationNeed() const;

    // 隔离效果监控
    json analyzeIsolationEffectiveness() const;
    double calculateIsolationImpact(const std::string& action_id) const;
    std::vector<IsolationAction> getActiveIsolations() const;

    // 智能解除隔离
    void enableIntelligentRelease(bool enable = true);
    json evaluateReleaseConditions(const std::string& action_id) const;
    void scheduleAutomaticRelease(const std::string& action_id, milliseconds delay);

    // 量子隔离技术
    void enableQuantumIsolation(bool enable = true);
    json performQuantumEntanglementIsolation(const std::string& target) const;
    void activateQuantumFirewall();

private:
    std::unordered_map<std::string, IsolationAction> active_isolations_;
    std::unique_ptr<class IsolationExecutor> isolation_executor_;
    std::unique_ptr<class ReleaseEvaluator> release_evaluator_;
    
    mutable std::mutex isolation_mutex_;
    std::thread isolation_monitoring_thread_;
    std::atomic<bool> monitoring_active_{false};
    
    // 自动隔离设置
    json auto_isolation_rules_;
    std::atomic<bool> auto_isolation_enabled_{false};
    
    void isolationMonitoringLoop();
    void evaluateAutoReleaseConditions();
    void updateIsolationStatus();
    void logIsolationActions();
};

/**
 * 🏆 世界最强风险控制核心系统
 */
class WorldStrongestRiskControlSystem {
public:
    struct RiskControlConfig {
        // 实时风险引擎配置
        bool enable_ultra_realtime_engine = true;
        milliseconds risk_calculation_frequency{1};  // 1ms频率
        
        // AI预测系统配置
        bool enable_ai_prediction = true;
        std::vector<AIRiskPredictionSystem::ModelType> enabled_models;
        
        // 全球风险感知配置
        GlobalRiskSentinelSystem::SentinelConfig sentinel_config;
        
        // 预警系统配置
        bool enable_multi_dimensional_alerts = true;
        json alert_thresholds;
        
        // 隔离系统配置
        bool enable_intelligent_isolation = true;
        json auto_isolation_rules;
        
        // 高级功能
        bool enable_quantum_risk_features = true;
        bool enable_predictive_risk_prevention = true;
        bool enable_adaptive_risk_limits = true;
        bool enable_cross_system_integration = true;
    };

    WorldStrongestRiskControlSystem();
    ~WorldStrongestRiskControlSystem();

    // 系统管理
    bool initialize(const RiskControlConfig& config);
    void shutdown();
    void update();

    // 子系统访问
    UltraRealTimeRiskEngine* getRealTimeEngine() { return realtime_engine_.get(); }
    AIRiskPredictionSystem* getAIPrediction() { return ai_prediction_.get(); }
    GlobalRiskSentinelSystem* getGlobalSentinel() { return global_sentinel_.get(); }
    PrecisionRiskTargetingSystem* getPrecisionTargeting() { return precision_targeting_.get(); }
    MultiDimensionalAlertSystem* getAlertSystem() { return alert_system_.get(); }
    IntelligentRiskIsolationSystem* getIsolationSystem() { return isolation_system_.get(); }

    // 综合风险分析
    json generateComprehensiveRiskReport() const;
    double calculateOverallRiskScore() const;
    json predictSystemRiskEvolution(milliseconds time_horizon) const;

    // 极限风险管理
    void activateEmergencyRiskProtocol();
    json performStressTestSuite() const;
    void enableCrisisManagementMode(bool enable = true);

    // 自适应风险控制
    void enableAdaptiveRiskManagement(bool enable = true);
    void calibrateRiskSensitivity();
    json optimizeRiskAllocation() const;

    // 量子风险功能
    void enableQuantumRiskFeatures(bool enable = true);
    json performQuantumRiskAnalysis() const;
    void activateQuantumRiskShield();

    // 性能监控
    json getSystemPerformanceMetrics() const;
    void enablePerformanceOptimization(bool enable = true);
    void benchmarkRiskCalculationSpeed();

private:
    RiskControlConfig config_;
    
    // 核心子系统
    std::unique_ptr<UltraRealTimeRiskEngine> realtime_engine_;
    std::unique_ptr<AIRiskPredictionSystem> ai_prediction_;
    std::unique_ptr<GlobalRiskSentinelSystem> global_sentinel_;
    std::unique_ptr<PrecisionRiskTargetingSystem> precision_targeting_;
    std::unique_ptr<MultiDimensionalAlertSystem> alert_system_;
    std::unique_ptr<IntelligentRiskIsolationSystem> isolation_system_;
    
    // 系统状态
    std::atomic<bool> system_running_{false};
    std::thread main_coordination_thread_;
    mutable std::mutex system_mutex_;
    
    // 性能监控
    std::atomic<long long> risk_calculations_per_second_{0};
    std::atomic<double> average_risk_calculation_time_ns_{0.0};
    std::atomic<int> active_alerts_count_{0};
    std::atomic<int> active_isolations_count_{0};
    
    void coordinationLoop();
    void synchronizeSubsystems();
    void optimizeSystemPerformance();
    void handleSystemEvents();
    
    // 初始化子系统
    bool initializeRealTimeEngine();
    bool initializeAIPrediction();
    bool initializeGlobalSentinel();
    bool initializePrecisionTargeting();
    bool initializeAlertSystem();
    bool initializeIsolationSystem();
};

} // namespace hft::risk
