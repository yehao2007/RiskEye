#include "MultimodalDataFusion.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "../core/Types.h"

namespace hft {
namespace market {

MultimodalDataFusion::MultimodalDataFusion() : is_initialized_(false) {
    // 初始化默认融合权重
    fusion_weights_ = {0.6, 0.2, 0.2}; // 市场数据、新闻、社交媒体的默认权重
}

bool MultimodalDataFusion::initialize(const std::string& model_path) {
    // 在这里加载融合模型
    // 实际应用中，可能需要加载预训练的机器学习模型
    std::cout << "Loading multimodal fusion model from: " << model_path << std::endl;

    // 初始化模型参数
    calculateFusionWeights();
    is_initialized_ = true;
    return true;
}

MarketData MultimodalDataFusion::fuseData(const MarketData& market_data, 
                                         const std::vector<NewsData>& news_data, 
                                         const std::vector<SocialMediaData>& social_data) {
    if (!is_initialized_) {
        std::cerr << "Error: MultimodalDataFusion not initialized!" << std::endl;
        return MarketData();
    }

    // 创建融合后的市场数据
    MarketData result = market_data;

    // 计算新闻数据的平均情感分数
    float avg_news_sentiment = 0.0f;
    if (!news_data.empty()) {
        for (const auto& news : news_data) {
            avg_news_sentiment += news.sentiment_score;
        }
        avg_news_sentiment /= news_data.size();
    }

    // 计算社交媒体数据的平均情感分数
    float avg_social_sentiment = 0.0f;
    if (!social_data.empty()) {
        for (const auto& social : social_data) {
            avg_social_sentiment += social.sentiment_score;
        }
        avg_social_sentiment /= social_data.size();
    }

    // 融合情感数据到市场数据
    // 这里使用一个简单的加权平均策略
    // 在实际应用中，可以使用更复杂的融合算法
    for (auto& bid : result.bids) {
        // 调整价格，考虑情感因素
        double sentiment_factor = 1.0 + (fusion_weights_[1] * avg_news_sentiment + fusion_weights_[2] * avg_social_sentiment) * 0.01;
        bid.price = static_cast<types::Price>(bid.price * sentiment_factor);
    }

    for (auto& ask : result.asks) {
        // 调整价格，考虑情感因素
        double sentiment_factor = 1.0 + (fusion_weights_[1] * avg_news_sentiment + fusion_weights_[2] * avg_social_sentiment) * 0.01;
        ask.price = static_cast<types::Price>(ask.price * sentiment_factor);
    }

    // 保存融合后的数据
    fused_data_ = result;
    return result;
}

MarketData MultimodalDataFusion::getFusedData() const {
    return fused_data_;
}

void MultimodalDataFusion::updateModel(const std::vector<float>& new_weights) {
    if (new_weights.size() != fusion_weights_.size()) {
        std::cerr << "Error: Invalid weight vector size!" << std::endl;
        return;
    }

    // 更新权重
    fusion_weights_ = new_weights;

    // 重新计算融合权重
    calculateFusionWeights();
}

void MultimodalDataFusion::calculateFusionWeights() {
    // 归一化权重
    float sum = 0.0f;
    for (float weight : fusion_weights_) {
        sum += weight;
    }

    if (sum > 0) {
        for (float& weight : fusion_weights_) {
            weight /= sum;
        }
    }
}

} // namespace market
} // namespace hft