#pragma once

/**
 * 🏆 全球顶级风险预测大模型 - GlobalRiskPredictionModel 🏆
 * 
 * 核心特性：
 * - 🧠 多模态深度学习融合 (Transformer + CNN + LSTM + GNN)
 * - 🔮 量子增强风险计算
 * - ⚡ 毫秒级实时预测
 * - 🌐 全球多市场风险感知
 * - 🎯 自适应特征学习
 * - 🛡️ 不确定性量化
 * - 🚨 异常检测与早期预警
 * - 🔄 持续学习与模型进化
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
#include <future>
#include <complex>

#include <torch/torch.h>
#include <nlohmann/json.hpp>
#include <eigen3/Eigen/Dense>
#include <tbb/parallel_for.h>
#include <boost/circular_buffer.hpp>

#include "../core/Types.h"
#include "../core/Logger.h"
#include "../market/MarketData.h"

using json = nlohmann::json;
using namespace std::chrono;

namespace hft {
namespace ai {

/**
 * 🎯 风险类型枚举
 */
enum class RiskType {
    MARKET_RISK,           // 市场风险
    CREDIT_RISK,           // 信用风险
    LIQUIDITY_RISK,        // 流动性风险
    OPERATIONAL_RISK,      // 操作风险
    SYSTEMIC_RISK,         // 系统性风险
    VOLATILITY_RISK,       // 波动率风险
    CORRELATION_RISK,      // 相关性风险
    TAIL_RISK,             // 尾部风险
    BLACK_SWAN_RISK,       // 黑天鹅风险
    REGIME_CHANGE_RISK     // 市场结构变化风险
};

/**
 * 🔮 预测时间范围
 */
enum class PredictionHorizon {
    MICROSECONDS_1,        // 1微秒
    MICROSECONDS_10,       // 10微秒
    MICROSECONDS_100,      // 100微秒
    MILLISECONDS_1,        // 1毫秒
    MILLISECONDS_10,       // 10毫秒
    MILLISECONDS_100,      // 100毫秒
    SECONDS_1,             // 1秒
    MINUTES_1,             // 1分钟
    MINUTES_5,             // 5分钟
    MINUTES_15,            // 15分钟
    HOURS_1,               // 1小时
    HOURS_4,               // 4小时
    DAYS_1,                // 1天
    WEEKS_1,               // 1周
    MONTHS_1               // 1月
};

/**
 * 🧠 AI模型类型
 */
enum class ModelArchitecture {
    MULTI_MODAL_TRANSFORMER,    // 多模态Transformer
    QUANTUM_ENHANCED_LSTM,      // 量子增强LSTM
    ATTENTION_CNN_HYBRID,       // 注意力CNN混合
    GRAPH_NEURAL_NETWORK,       // 图神经网络
    REINFORCEMENT_ENSEMBLE,     // 强化学习集成
    NEUROMORPHIC_PROCESSOR,     // 神经形态处理器
    PHYSICS_INFORMED_NN,        // 物理信息神经网络
    BAYESIAN_DEEP_LEARNING      // 贝叶斯深度学习
};

/**
 * 📊 风险预测结果
 */
struct RiskPrediction {
    RiskType risk_type;
    double risk_score;                           // 风险评分 [0-1]
    double confidence;                           // 置信度 [0-1]
    double probability_of_loss;                  // 损失概率
    double expected_loss;                        // 期望损失
    double value_at_risk_95;                     // 95% VaR
    double value_at_risk_99;                     // 99% VaR
    double conditional_value_at_risk;            // CVaR
    double maximum_drawdown_probability;         // 最大回撤概率
    
    // 不确定性量化
    double epistemic_uncertainty;                // 认知不确定性
    double aleatoric_uncertainty;                // 偶然不确定性
    double total_uncertainty;                    // 总不确定性
    
    // 时间序列预测
    std::vector<double> risk_trajectory;         // 风险轨迹
    std::vector<double> confidence_intervals;    // 置信区间
    
    // 关键风险因子
    std::vector<std::string> key_risk_factors;
    std::vector<double> factor_importance;
    
    // 预测元数据
    PredictionHorizon horizon;
    high_resolution_clock::time_point prediction_time;
    high_resolution_clock::time_point target_time;
    uint64_t computation_time_ns;
    ModelArchitecture used_model;
    
    // SHAP值解释
    json shap_values;
    json lime_explanation;
};

/**
 * 🌐 多模态市场数据
 */
struct MultiModalMarketData {
    // 价格数据
    market::MarketData price_data;
    
    // 订单簿数据
    std::vector<std::pair<double, double>> bid_levels;  // 价格, 数量
    std::vector<std::pair<double, double>> ask_levels;
    
    // 交易流数据
    std::vector<market::Trade> trade_flow;
    
    // 宏观经济数据
    json macroeconomic_indicators;
    
    // 新闻情感数据
    json news_sentiment;
    
    // 社交媒体情感
    json social_sentiment;
    
    // 期权链数据
    json options_chain;
    
    // 资金流向数据
    json money_flow;
    
    // 跨市场相关性
    json cross_market_correlations;
    
    // 地缘政治风险指标
    json geopolitical_risk_indicators;
};

/**
 * 🧬 量子特征工程器
 */
class QuantumFeatureEngineer {
public:
    QuantumFeatureEngineer();
    ~QuantumFeatureEngineer();
    
    // 量子特征提取
    torch::Tensor extractQuantumFeatures(const MultiModalMarketData& data);
    
    // 量子纠缠特征
    torch::Tensor calculateQuantumEntanglement(const torch::Tensor& features);
    
    // 量子傅里叶变换
    torch::Tensor quantumFourierTransform(const torch::Tensor& signal);
    
    // 量子主成分分析
    torch::Tensor quantumPCA(const torch::Tensor& data, int num_components);
    
private:
    std::unique_ptr<class QuantumCircuit> quantum_circuit_;
    std::unique_ptr<class QuantumProcessor> quantum_processor_;
};

/**
 * 🎯 自适应注意力机制
 */
class AdaptiveAttentionMechanism : public torch::nn::Module {
public:
    AdaptiveAttentionMechanism(int input_dim, int num_heads, double dropout_rate = 0.1);
    
    torch::Tensor forward(
        const torch::Tensor& query,
        const torch::Tensor& key,
        const torch::Tensor& value,
        const torch::Tensor& mask = torch::Tensor()
    );
    
private:
    torch::nn::MultiheadAttention attention_;
    torch::nn::LayerNorm layer_norm_;
    torch::nn::Dropout dropout_;
    
    // 自适应权重
    torch::nn::Linear adaptive_weights_;
    torch::nn::Parameter temperature_param_;
};

/**
 * 🧠 多模态融合Transformer
 */
class MultiModalTransformer : public torch::nn::Module {
public:
    MultiModalTransformer(
        int price_dim,
        int orderbook_dim,
        int news_dim,
        int macro_dim,
        int hidden_dim,
        int num_layers,
        int num_heads
    );
    
    torch::Tensor forward(
        const torch::Tensor& price_features,
        const torch::Tensor& orderbook_features,
        const torch::Tensor& news_features,
        const torch::Tensor& macro_features
    );
    
private:
    // 模态特定编码器
    torch::nn::Linear price_encoder_;
    torch::nn::Linear orderbook_encoder_;
    torch::nn::Linear news_encoder_;
    torch::nn::Linear macro_encoder_;
    
    // 位置编码
    torch::nn::Parameter positional_encoding_;
    
    // Transformer层
    torch::nn::ModuleList transformer_layers_;
    
    // 跨模态注意力
    std::unique_ptr<AdaptiveAttentionMechanism> cross_modal_attention_;
    
    // 融合层
    torch::nn::Linear fusion_layer_;
    torch::nn::LayerNorm fusion_norm_;
};

/**
 * 🔄 贝叶斯不确定性量化器
 */
class BayesianUncertaintyQuantifier {
public:
    BayesianUncertaintyQuantifier(int num_samples = 100);
    
    // 计算预测不确定性
    std::pair<double, double> calculateUncertainty(
        const torch::Tensor& predictions,
        const torch::Tensor& features
    );
    
    // 蒙特卡洛dropout
    torch::Tensor monteCarloDropout(
        const torch::nn::Module& model,
        const torch::Tensor& input,
        int num_samples
    );
    
    // 变分推理
    std::pair<torch::Tensor, torch::Tensor> variationalInference(
        const torch::Tensor& predictions
    );
    
private:
    int num_samples_;
    std::random_device rd_;
    std::mt19937 gen_;
};

/**
 * 🏆 全球顶级风险预测大模型
 */
class GlobalRiskPredictionModel {
public:
    GlobalRiskPredictionModel();
    ~GlobalRiskPredictionModel();
    
    // 初始化模型
    bool initialize(const json& config);
    
    // 预测风险
    RiskPrediction predictRisk(
        const MultiModalMarketData& data,
        RiskType risk_type,
        PredictionHorizon horizon
    );
    
    // 批量预测
    std::vector<RiskPrediction> predictMultipleRisks(
        const MultiModalMarketData& data,
        const std::vector<RiskType>& risk_types,
        const std::vector<PredictionHorizon>& horizons
    );
    
    // 实时风险监控
    void startRealTimeMonitoring(
        std::function<void(const RiskPrediction&)> callback
    );
    
    void stopRealTimeMonitoring();
    
    // 模型训练
    void trainModel(
        const std::vector<MultiModalMarketData>& training_data,
        const std::vector<RiskPrediction>& target_risks
    );
    
    // 增量学习
    void incrementalLearning(
        const MultiModalMarketData& new_data,
        const RiskPrediction& actual_risk
    );
    
    // 模型评估
    json evaluateModel(
        const std::vector<MultiModalMarketData>& test_data,
        const std::vector<RiskPrediction>& true_risks
    );
    
    // 模型解释
    json explainPrediction(const RiskPrediction& prediction);
    
    // 模型管理
    bool saveModel(const std::string& path);
    bool loadModel(const std::string& path);
    void updateModel();
    
    // 性能监控
    json getPerformanceMetrics();
    void benchmarkModel();
    
    // 高级功能
    std::vector<RiskPrediction> predictRiskScenarios(
        const MultiModalMarketData& data,
        int num_scenarios = 1000
    );
    
    double calculateSystemicRisk(
        const std::vector<MultiModalMarketData>& multi_asset_data
    );
    
    json detectAnomalies(const MultiModalMarketData& data);
    
    std::vector<std::string> generateRiskAlerts(
        const std::vector<RiskPrediction>& predictions
    );

private:
    // 核心AI模型
    std::unique_ptr<MultiModalTransformer> transformer_model_;
    std::unique_ptr<torch::nn::LSTM> lstm_model_;
    std::unique_ptr<torch::nn::Module> cnn_model_;
    std::unique_ptr<torch::nn::Module> gnn_model_;
    
    // 特征工程
    std::unique_ptr<QuantumFeatureEngineer> feature_engineer_;
    
    // 不确定性量化
    std::unique_ptr<BayesianUncertaintyQuantifier> uncertainty_quantifier_;
    
    // 模型集成
    std::vector<std::unique_ptr<torch::nn::Module>> ensemble_models_;
    std::vector<double> ensemble_weights_;
    
    // 数据预处理
    torch::Tensor preprocessData(const MultiModalMarketData& data);
    torch::Tensor extractPriceFeatures(const market::MarketData& price_data);
    torch::Tensor extractOrderbookFeatures(
        const std::vector<std::pair<double, double>>& bid_levels,
        const std::vector<std::pair<double, double>>& ask_levels
    );
    torch::Tensor extractNewsFeatures(const json& news_sentiment);
    torch::Tensor extractMacroFeatures(const json& macro_indicators);
    
    // 后处理
    RiskPrediction postprocessPrediction(
        const torch::Tensor& raw_prediction,
        RiskType risk_type,
        PredictionHorizon horizon,
        const MultiModalMarketData& input_data
    );
    
    // 模型选择
    std::unique_ptr<torch::nn::Module> selectBestModel(
        const MultiModalMarketData& data,
        RiskType risk_type
    );
    
    // 配置
    json config_;
    torch::Device device_;
    
    // 线程管理
    std::atomic<bool> monitoring_active_;
    std::unique_ptr<std::thread> monitoring_thread_;
    std::mutex model_mutex_;
    
    // 性能监控
    struct PerformanceMetrics {
        double accuracy;
        double precision;
        double recall;
        double f1_score;
        double auc_roc;
        double sharpe_ratio;
        double max_drawdown;
        uint64_t average_prediction_time_ns;
        uint64_t total_predictions;
        uint64_t correct_predictions;
    } performance_metrics_;
    
    // 历史数据缓存
    boost::circular_buffer<MultiModalMarketData> data_history_;
    boost::circular_buffer<RiskPrediction> prediction_history_;
    
    // 模型状态
    bool is_initialized_;
    bool is_training_;
    std::string model_version_;
    high_resolution_clock::time_point last_update_time_;
    
    // 辅助函数
    void initializeModels();
    void setupOptimizers();
    void initializeDataPipeline();
    bool validateInput(const MultiModalMarketData& data);
    void logPrediction(const RiskPrediction& prediction);
    void updatePerformanceMetrics(const RiskPrediction& prediction, const RiskPrediction& actual);
};

} // namespace ai
} // namespace hft
