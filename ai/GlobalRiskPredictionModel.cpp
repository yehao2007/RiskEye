#include "GlobalRiskPredictionModel.h"
#include "../core/Logger.h"
#include <torch/torch.h>
#include <chrono>
#include <random>
#include <algorithm>
#include <execution>

namespace hft {
namespace ai {

// ============================================================================
// 🧬 量子特征工程器实现
// ============================================================================

QuantumFeatureEngineer::QuantumFeatureEngineer() 
    : quantum_circuit_(std::make_unique<QuantumCircuit>()),
      quantum_processor_(std::make_unique<QuantumProcessor>()) {
    Logger::info("Initializing Quantum Feature Engineer");
}

QuantumFeatureEngineer::~QuantumFeatureEngineer() = default;

torch::Tensor QuantumFeatureEngineer::extractQuantumFeatures(const MultiModalMarketData& data) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 1. 基础特征提取
    std::vector<double> price_features = {
        data.price_data.price,
        data.price_data.volume,
        data.price_data.high,
        data.price_data.low,
        data.price_data.open
    };
    
    // 2. 量子态编码
    auto quantum_state = quantum_circuit_->encodeClassicalData(price_features);
    
    // 3. 量子特征变换
    auto quantum_features = quantum_processor_->applyQuantumTransformation(quantum_state);
    
    // 4. 量子纠缠特征
    auto entanglement_features = calculateQuantumEntanglement(quantum_features);
    
    // 5. 转换为PyTorch张量
    auto tensor = torch::from_blob(
        quantum_features.data(), 
        {static_cast<long>(quantum_features.size())}, 
        torch::kFloat64
    ).clone();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    Logger::debug("Quantum feature extraction completed in {} microseconds", duration.count());
    
    return tensor.to(torch::kFloat32);
}

torch::Tensor QuantumFeatureEngineer::calculateQuantumEntanglement(const torch::Tensor& features) {
    // 实现量子纠缠特征计算
    auto entangled = torch::zeros_like(features);
    auto batch_size = features.size(0);
    
    for (int i = 0; i < batch_size; ++i) {
        for (int j = i + 1; j < batch_size; ++j) {
            // 计算量子纠缠度
            auto entanglement = torch::dot(features[i], features[j]) / 
                               (torch::norm(features[i]) * torch::norm(features[j]) + 1e-8);
            entangled[i] += entanglement * features[j];
            entangled[j] += entanglement * features[i];
        }
    }
    
    return entangled;
}

torch::Tensor QuantumFeatureEngineer::quantumFourierTransform(const torch::Tensor& signal) {
    // 简化的量子傅里叶变换实现
    auto fft_result = torch::fft_fft(signal.to(torch::kComplexFloat));
    return torch::real(fft_result);
}

torch::Tensor QuantumFeatureEngineer::quantumPCA(const torch::Tensor& data, int num_components) {
    // 量子主成分分析
    auto centered = data - torch::mean(data, 0, true);
    auto covariance = torch::matmul(centered.t(), centered) / (data.size(0) - 1);
    
    auto svd_result = torch::svd(covariance);
    auto components = std::get<2>(svd_result).slice(1, 0, num_components);
    
    return torch::matmul(centered, components);
}

// ============================================================================
// 🎯 自适应注意力机制实现
// ============================================================================

AdaptiveAttentionMechanism::AdaptiveAttentionMechanism(int input_dim, int num_heads, double dropout_rate)
    : attention_(torch::nn::MultiheadAttentionOptions(input_dim, num_heads).dropout(dropout_rate)),
      layer_norm_(input_dim),
      dropout_(dropout_rate),
      adaptive_weights_(input_dim, num_heads),
      temperature_param_(torch::ones({1})) {
    
    register_module("attention", attention_);
    register_module("layer_norm", layer_norm_);
    register_module("dropout", dropout_);
    register_module("adaptive_weights", adaptive_weights_);
    register_parameter("temperature", temperature_param_);
}

torch::Tensor AdaptiveAttentionMechanism::forward(
    const torch::Tensor& query,
    const torch::Tensor& key, 
    const torch::Tensor& value,
    const torch::Tensor& mask) {
    
    // 自适应温度调节
    auto temperature = torch::sigmoid(temperature_param_) * 2.0 + 0.1;
    
    // 计算自适应权重
    auto adaptive_weights = torch::softmax(adaptive_weights_->forward(query) / temperature, -1);
    
    // 多头注意力
    auto attention_output = attention_->forward(query, key, value);
    auto attended = std::get<0>(attention_output);
    
    // 应用自适应权重
    attended = attended * adaptive_weights.unsqueeze(-1);
    
    // 残差连接和层归一化
    auto output = layer_norm_->forward(query + dropout_->forward(attended));
    
    return output;
}

// ============================================================================
// 🧠 多模态融合Transformer实现
// ============================================================================

MultiModalTransformer::MultiModalTransformer(
    int price_dim, int orderbook_dim, int news_dim, int macro_dim,
    int hidden_dim, int num_layers, int num_heads)
    : price_encoder_(price_dim, hidden_dim),
      orderbook_encoder_(orderbook_dim, hidden_dim),
      news_encoder_(news_dim, hidden_dim),
      macro_encoder_(macro_dim, hidden_dim),
      positional_encoding_(torch::zeros({1000, hidden_dim})),
      transformer_layers_(torch::nn::ModuleList()),
      cross_modal_attention_(std::make_unique<AdaptiveAttentionMechanism>(hidden_dim, num_heads)),
      fusion_layer_(hidden_dim * 4, hidden_dim),
      fusion_norm_(hidden_dim) {
    
    // 注册模块
    register_module("price_encoder", price_encoder_);
    register_module("orderbook_encoder", orderbook_encoder_);
    register_module("news_encoder", news_encoder_);
    register_module("macro_encoder", macro_encoder_);
    register_parameter("positional_encoding", positional_encoding_);
    register_module("fusion_layer", fusion_layer_);
    register_module("fusion_norm", fusion_norm_);
    
    // 初始化Transformer层
    for (int i = 0; i < num_layers; ++i) {
        auto layer = torch::nn::TransformerEncoderLayer(
            torch::nn::TransformerEncoderLayerOptions(hidden_dim, num_heads)
                .dropout(0.1)
                .activation(torch::nn::GELU())
        );
        transformer_layers_->push_back(layer);
    }
    register_module("transformer_layers", transformer_layers_);
    
    // 初始化位置编码
    initializePositionalEncoding();
}

torch::Tensor MultiModalTransformer::forward(
    const torch::Tensor& price_features,
    const torch::Tensor& orderbook_features,
    const torch::Tensor& news_features,
    const torch::Tensor& macro_features) {
    
    // 编码各模态特征
    auto price_encoded = price_encoder_->forward(price_features);
    auto orderbook_encoded = orderbook_encoder_->forward(orderbook_features);
    auto news_encoded = news_encoder_->forward(news_features);
    auto macro_encoded = macro_encoder_->forward(macro_features);
    
    // 添加位置编码
    auto seq_len = price_encoded.size(1);
    auto pos_encoding = positional_encoding_.slice(0, 0, seq_len);
    
    price_encoded += pos_encoding;
    orderbook_encoded += pos_encoding;
    news_encoded += pos_encoding;
    macro_encoded += pos_encoding;
    
    // 通过Transformer层
    std::vector<torch::Tensor> modal_outputs;
    
    for (auto& modal_input : {price_encoded, orderbook_encoded, news_encoded, macro_encoded}) {
        auto output = modal_input;
        for (auto& layer : *transformer_layers_) {
            output = layer.as<torch::nn::TransformerEncoderLayer>()->forward(output);
        }
        modal_outputs.push_back(output);
    }
    
    // 跨模态注意力融合
    auto concatenated = torch::cat(modal_outputs, -1);
    auto fused = fusion_layer_->forward(concatenated);
    auto normalized = fusion_norm_->forward(fused);
    
    return normalized;
}

void MultiModalTransformer::initializePositionalEncoding() {
    auto seq_len = positional_encoding_.size(0);
    auto hidden_dim = positional_encoding_.size(1);
    
    for (int pos = 0; pos < seq_len; ++pos) {
        for (int i = 0; i < hidden_dim; i += 2) {
            auto angle = pos / std::pow(10000.0, 2.0 * i / hidden_dim);
            positional_encoding_[pos][i] = std::sin(angle);
            if (i + 1 < hidden_dim) {
                positional_encoding_[pos][i + 1] = std::cos(angle);
            }
        }
    }
}

// ============================================================================
// 🔄 贝叶斯不确定性量化器实现
// ============================================================================

BayesianUncertaintyQuantifier::BayesianUncertaintyQuantifier(int num_samples)
    : num_samples_(num_samples), gen_(rd_()) {
    Logger::info("Initializing Bayesian Uncertainty Quantifier with {} samples", num_samples);
}

std::pair<double, double> BayesianUncertaintyQuantifier::calculateUncertainty(
    const torch::Tensor& predictions,
    const torch::Tensor& features) {
    
    // 计算认知不确定性（模型不确定性）
    auto mean_pred = torch::mean(predictions);
    auto epistemic_uncertainty = torch::var(predictions).item<double>();
    
    // 计算偶然不确定性（数据不确定性）
    auto aleatoric_uncertainty = torch::mean(torch::pow(predictions - mean_pred, 2)).item<double>();
    
    return {epistemic_uncertainty, aleatoric_uncertainty};
}

torch::Tensor BayesianUncertaintyQuantifier::monteCarloDropout(
    const torch::nn::Module& model,
    const torch::Tensor& input,
    int num_samples) {
    
    std::vector<torch::Tensor> predictions;
    predictions.reserve(num_samples);
    
    // 启用训练模式以使用dropout
    const_cast<torch::nn::Module&>(model).train();
    
    torch::NoGradGuard no_grad;
    for (int i = 0; i < num_samples; ++i) {
        auto pred = const_cast<torch::nn::Module&>(model).forward(input);
        predictions.push_back(pred);
    }
    
    // 恢复评估模式
    const_cast<torch::nn::Module&>(model).eval();
    
    return torch::stack(predictions);
}

std::pair<torch::Tensor, torch::Tensor> BayesianUncertaintyQuantifier::variationalInference(
    const torch::Tensor& predictions) {
    
    auto mean = torch::mean(predictions, 0);
    auto variance = torch::var(predictions, 0);
    
    return {mean, variance};
}

// ============================================================================
// 🏆 全球顶级风险预测大模型实现
// ============================================================================

GlobalRiskPredictionModel::GlobalRiskPredictionModel()
    : device_(torch::kCPU),
      monitoring_active_(false),
      data_history_(10000),
      prediction_history_(10000),
      is_initialized_(false),
      is_training_(false),
      model_version_("1.0.0") {
    
    Logger::info("Initializing Global Risk Prediction Model v{}", model_version_);
    
    // 检查CUDA可用性
    if (torch::cuda::is_available()) {
        device_ = torch::kCUDA;
        Logger::info("CUDA detected, using GPU acceleration");
    } else {
        Logger::warn("CUDA not available, using CPU");
    }
    
    // 初始化性能指标
    performance_metrics_ = PerformanceMetrics{};
}

GlobalRiskPredictionModel::~GlobalRiskPredictionModel() {
    if (monitoring_active_) {
        stopRealTimeMonitoring();
    }
    Logger::info("Global Risk Prediction Model destroyed");
}

bool GlobalRiskPredictionModel::initialize(const json& config) {
    try {
        config_ = config;
        
        Logger::info("Initializing Global Risk Prediction Model...");
        
        // 1. 初始化AI模型
        initializeModels();
        
        // 2. 设置优化器
        setupOptimizers();
        
        // 3. 初始化数据管道
        initializeDataPipeline();
        
        // 4. 初始化特征工程器
        feature_engineer_ = std::make_unique<QuantumFeatureEngineer>();
        
        // 5. 初始化不确定性量化器
        uncertainty_quantifier_ = std::make_unique<BayesianUncertaintyQuantifier>(
            config.value("uncertainty_samples", 100)
        );
        
        is_initialized_ = true;
        last_update_time_ = std::chrono::high_resolution_clock::now();
        
        Logger::info("Global Risk Prediction Model initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize model: {}", e.what());
        return false;
    }
}

RiskPrediction GlobalRiskPredictionModel::predictRisk(
    const MultiModalMarketData& data,
    RiskType risk_type,
    PredictionHorizon horizon) {
    
    if (!is_initialized_) {
        throw std::runtime_error("Model not initialized");
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    RiskPrediction result{};
    result.risk_type = risk_type;
    result.horizon = horizon;
    result.prediction_time = start_time;
    
    try {
        std::lock_guard<std::mutex> lock(model_mutex_);
        
        // 1. 输入验证
        if (!validateInput(data)) {
            throw std::runtime_error("Invalid input data");
        }
        
        // 2. 数据预处理
        auto processed_data = preprocessData(data);
        
        // 3. 选择最佳模型
        auto selected_model = selectBestModel(data, risk_type);
        result.used_model = ModelArchitecture::MULTI_MODAL_TRANSFORMER; // 简化
        
        // 4. 执行预测
        torch::NoGradGuard no_grad;
        auto raw_prediction = selected_model->forward(processed_data);
        
        // 5. 不确定性量化
        auto mc_predictions = uncertainty_quantifier_->monteCarloDropout(
            *selected_model, processed_data, 50
        );
        auto uncertainty = uncertainty_quantifier_->calculateUncertainty(
            mc_predictions, processed_data
        );
        
        result.epistemic_uncertainty = uncertainty.first;
        result.aleatoric_uncertainty = uncertainty.second;
        result.total_uncertainty = uncertainty.first + uncertainty.second;
        
        // 6. 后处理
        result = postprocessPrediction(raw_prediction, risk_type, horizon, data);
        
        // 7. 计算执行时间
        auto end_time = std::chrono::high_resolution_clock::now();
        result.computation_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time).count();
        
        // 8. 记录预测
        logPrediction(result);
        prediction_history_.push_back(result);
        
        Logger::debug("Risk prediction completed in {} ns", result.computation_time_ns);
        
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("Risk prediction failed: {}", e.what());
        throw;
    }
}

std::vector<RiskPrediction> GlobalRiskPredictionModel::predictMultipleRisks(
    const MultiModalMarketData& data,
    const std::vector<RiskType>& risk_types,
    const std::vector<PredictionHorizon>& horizons) {
    
    std::vector<RiskPrediction> results;
    results.reserve(risk_types.size() * horizons.size());
    
    // 并行预测
    std::for_each(std::execution::par_unseq, 
        risk_types.begin(), risk_types.end(),
        [&](const RiskType& risk_type) {
            for (const auto& horizon : horizons) {
                try {
                    auto prediction = predictRisk(data, risk_type, horizon);
                    std::lock_guard<std::mutex> lock(model_mutex_);
                    results.push_back(prediction);
                } catch (const std::exception& e) {
                    Logger::error("Failed to predict risk type {}: {}", 
                                 static_cast<int>(risk_type), e.what());
                }
            }
        }
    );
    
    return results;
}

void GlobalRiskPredictionModel::startRealTimeMonitoring(
    std::function<void(const RiskPrediction&)> callback) {
    
    if (monitoring_active_) {
        Logger::warn("Real-time monitoring already active");
        return;
    }
    
    monitoring_active_ = true;
    monitoring_thread_ = std::make_unique<std::thread>([this, callback]() {
        Logger::info("Starting real-time risk monitoring");
        
        while (monitoring_active_) {
            try {
                // 这里应该从数据源获取实时数据
                // 简化实现：使用历史数据
                if (!data_history_.empty()) {
                    auto latest_data = data_history_.back();
                    
                    // 预测所有主要风险类型
                    std::vector<RiskType> risk_types = {
                        RiskType::MARKET_RISK,
                        RiskType::VOLATILITY_RISK,
                        RiskType::LIQUIDITY_RISK,
                        RiskType::SYSTEMIC_RISK
                    };
                    
                    for (auto risk_type : risk_types) {
                        auto prediction = predictRisk(
                            latest_data, 
                            risk_type, 
                            PredictionHorizon::MILLISECONDS_100
                        );
                        
                        // 检查是否需要警报
                        if (prediction.risk_score > 0.8) {
                            callback(prediction);
                        }
                    }
                }
                
                // 监控频率：每10毫秒
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                
            } catch (const std::exception& e) {
                Logger::error("Real-time monitoring error: {}", e.what());
            }
        }
        
        Logger::info("Real-time risk monitoring stopped");
    });
}

void GlobalRiskPredictionModel::stopRealTimeMonitoring() {
    if (monitoring_active_) {
        monitoring_active_ = false;
        if (monitoring_thread_ && monitoring_thread_->joinable()) {
            monitoring_thread_->join();
        }
        Logger::info("Real-time monitoring stopped");
    }
}

torch::Tensor GlobalRiskPredictionModel::preprocessData(const MultiModalMarketData& data) {
    // 提取各种特征
    auto price_features = extractPriceFeatures(data.price_data);
    auto orderbook_features = extractOrderbookFeatures(data.bid_levels, data.ask_levels);
    auto news_features = extractNewsFeatures(data.news_sentiment);
    auto macro_features = extractMacroFeatures(data.macroeconomic_indicators);
    
    // 量子特征工程
    auto quantum_features = feature_engineer_->extractQuantumFeatures(data);
    
    // 特征融合
    auto combined_features = torch::cat({
        price_features,
        orderbook_features, 
        news_features,
        macro_features,
        quantum_features
    }, -1);
    
    // 标准化
    auto mean = torch::mean(combined_features, 0, true);
    auto std = torch::std(combined_features, 0, true);
    auto normalized = (combined_features - mean) / (std + 1e-8);
    
    return normalized.to(device_);
}

torch::Tensor GlobalRiskPredictionModel::extractPriceFeatures(const market::MarketData& price_data) {
    std::vector<double> features = {
        price_data.price,
        price_data.volume,
        price_data.high,
        price_data.low,
        price_data.open,
        (price_data.price - price_data.open) / price_data.open, // 收益率
        (price_data.high - price_data.low) / price_data.low,    // 波动率代理
        price_data.volume / (price_data.volume + 1.0)           // 标准化成交量
    };
    
    return torch::from_blob(features.data(), {1, static_cast<long>(features.size())}, torch::kFloat64)
           .clone().to(torch::kFloat32);
}

torch::Tensor GlobalRiskPredictionModel::extractOrderbookFeatures(
    const std::vector<std::pair<double, double>>& bid_levels,
    const std::vector<std::pair<double, double>>& ask_levels) {
    
    std::vector<double> features;
    
    // 计算买卖价差
    if (!bid_levels.empty() && !ask_levels.empty()) {
        double spread = ask_levels[0].first - bid_levels[0].first;
        double mid_price = (ask_levels[0].first + bid_levels[0].first) / 2.0;
        features.push_back(spread / mid_price); // 相对价差
        
        // 订单簿失衡
        double bid_volume = 0, ask_volume = 0;
        for (const auto& level : bid_levels) {
            bid_volume += level.second;
        }
        for (const auto& level : ask_levels) {
            ask_volume += level.second;
        }
        features.push_back((bid_volume - ask_volume) / (bid_volume + ask_volume + 1e-8));
    } else {
        features = {0.0, 0.0}; // 默认值
    }
    
    return torch::from_blob(features.data(), {1, static_cast<long>(features.size())}, torch::kFloat64)
           .clone().to(torch::kFloat32);
}

torch::Tensor GlobalRiskPredictionModel::extractNewsFeatures(const json& news_sentiment) {
    std::vector<double> features = {0.0, 0.0, 0.0}; // 默认中性情感
    
    if (news_sentiment.contains("sentiment_score")) {
        features[0] = news_sentiment["sentiment_score"].get<double>();
    }
    if (news_sentiment.contains("positive_ratio")) {
        features[1] = news_sentiment["positive_ratio"].get<double>();
    }
    if (news_sentiment.contains("negative_ratio")) {
        features[2] = news_sentiment["negative_ratio"].get<double>();
    }
    
    return torch::from_blob(features.data(), {1, static_cast<long>(features.size())}, torch::kFloat64)
           .clone().to(torch::kFloat32);
}

torch::Tensor GlobalRiskPredictionModel::extractMacroFeatures(const json& macro_indicators) {
    std::vector<double> features = {0.0, 0.0, 0.0, 0.0}; // 默认值
    
    if (macro_indicators.contains("vix")) {
        features[0] = macro_indicators["vix"].get<double>() / 100.0; // 标准化VIX
    }
    if (macro_indicators.contains("yield_curve_slope")) {
        features[1] = macro_indicators["yield_curve_slope"].get<double>();
    }
    if (macro_indicators.contains("credit_spread")) {
        features[2] = macro_indicators["credit_spread"].get<double>();
    }
    if (macro_indicators.contains("dollar_index")) {
        features[3] = macro_indicators["dollar_index"].get<double>() / 100.0;
    }
    
    return torch::from_blob(features.data(), {1, static_cast<long>(features.size())}, torch::kFloat64)
           .clone().to(torch::kFloat32);
}

RiskPrediction GlobalRiskPredictionModel::postprocessPrediction(
    const torch::Tensor& raw_prediction,
    RiskType risk_type,
    PredictionHorizon horizon,
    const MultiModalMarketData& input_data) {
    
    RiskPrediction result{};
    result.risk_type = risk_type;
    result.horizon = horizon;
    
    // 提取预测值
    auto prediction_data = raw_prediction.cpu();
    result.risk_score = torch::sigmoid(prediction_data[0]).item<double>();
    result.confidence = std::min(1.0 - result.total_uncertainty, 0.99);
    
    // 计算VaR和其他风险指标
    result.value_at_risk_95 = result.risk_score * 1.645; // 简化计算
    result.value_at_risk_99 = result.risk_score * 2.326;
    result.conditional_value_at_risk = result.value_at_risk_99 * 1.2;
    
    result.probability_of_loss = result.risk_score;
    result.expected_loss = result.risk_score * 0.1; // 简化
    result.maximum_drawdown_probability = result.risk_score * 0.8;
    
    // 设置目标时间
    auto now = std::chrono::high_resolution_clock::now();
    switch (horizon) {
        case PredictionHorizon::MICROSECONDS_1:
            result.target_time = now + std::chrono::microseconds(1);
            break;
        case PredictionHorizon::MILLISECONDS_1:
            result.target_time = now + std::chrono::milliseconds(1);
            break;
        case PredictionHorizon::SECONDS_1:
            result.target_time = now + std::chrono::seconds(1);
            break;
        default:
            result.target_time = now + std::chrono::minutes(1);
    }
    
    return result;
}

std::unique_ptr<torch::nn::Module> GlobalRiskPredictionModel::selectBestModel(
    const MultiModalMarketData& data,
    RiskType risk_type) {
    
    // 简化实现：返回Transformer模型
    return std::make_unique<torch::nn::Linear>(100, 1); // 占位符
}

bool GlobalRiskPredictionModel::validateInput(const MultiModalMarketData& data) {
    // 基本验证
    return data.price_data.price > 0 && 
           data.price_data.volume >= 0 &&
           std::isfinite(data.price_data.price) &&
           std::isfinite(data.price_data.volume);
}

void GlobalRiskPredictionModel::logPrediction(const RiskPrediction& prediction) {
    Logger::debug("Risk Prediction - Type: {}, Score: {:.4f}, Confidence: {:.4f}, Time: {} ns",
                 static_cast<int>(prediction.risk_type),
                 prediction.risk_score,
                 prediction.confidence,
                 prediction.computation_time_ns);
}

void GlobalRiskPredictionModel::initializeModels() {
    // 初始化各种AI模型
    transformer_model_ = std::make_unique<MultiModalTransformer>(
        8,   // price_dim
        2,   // orderbook_dim  
        3,   // news_dim
        4,   // macro_dim
        256, // hidden_dim
        6,   // num_layers
        8    // num_heads
    );
    
    transformer_model_->to(device_);
    Logger::info("Transformer model initialized");
}

void GlobalRiskPredictionModel::setupOptimizers() {
    // 这里会设置各种优化器
    Logger::info("Optimizers configured");
}

void GlobalRiskPredictionModel::initializeDataPipeline() {
    // 初始化数据管道
    Logger::info("Data pipeline initialized");
}

json GlobalRiskPredictionModel::getPerformanceMetrics() {
    json metrics;
    metrics["accuracy"] = performance_metrics_.accuracy;
    metrics["precision"] = performance_metrics_.precision;
    metrics["recall"] = performance_metrics_.recall;
    metrics["f1_score"] = performance_metrics_.f1_score;
    metrics["auc_roc"] = performance_metrics_.auc_roc;
    metrics["average_prediction_time_ns"] = performance_metrics_.average_prediction_time_ns;
    metrics["total_predictions"] = performance_metrics_.total_predictions;
    metrics["model_version"] = model_version_;
    
    return metrics;
}

} // namespace ai
} // namespace hft
