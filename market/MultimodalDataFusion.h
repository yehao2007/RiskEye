#ifndef MULTIMODAL_DATA_FUSION_H
#define MULTIMODAL_DATA_FUSION_H

#include "MarketData.h"
#include <vector>
#include <string>
#include <memory>

namespace hft {
namespace market {

// 新闻数据结构体
struct NewsData {
    std::string headline;
    std::string content;
    float sentiment_score;
    uint64_t timestamp;
};

// 社交媒体数据结构体
struct SocialMediaData {
    std::string text;
    int likes;
    int shares;
    float sentiment_score;
    uint64_t timestamp;
};

class MultimodalDataFusion {
public:
    MultimodalDataFusion();

    // 初始化多模态融合模型
    bool initialize(const std::string& model_path);

    // 融合市场数据、新闻和社交媒体数据
    MarketData fuseData(const MarketData& market_data,
                        const std::vector<NewsData>& news_data,
                        const std::vector<SocialMediaData>& social_data);

    // 获取融合后的数据
    MarketData getFusedData() const;

    // 更新融合模型
    void updateModel(const std::vector<float>& new_weights);

private:
    MarketData fused_data_;
    std::vector<float> fusion_weights_;
    bool is_initialized_;

    // 计算数据融合权重
    void calculateFusionWeights();
};

} // namespace market
} // namespace hft

#endif // MULTIMODAL_DATA_FUSION_H