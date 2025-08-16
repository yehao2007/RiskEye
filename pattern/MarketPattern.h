#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace hft {
namespace pattern {

// 市场模式类型枚举
enum class PatternType {
    NONE,
    TREND_UP,
    TREND_DOWN,
    RANGE_BOUND,
    BREAKOUT_UP,
    BREAKOUT_DOWN,
    REVERSAL_UP,
    REVERSAL_DOWN,
    CONSOLIDATION
};

// 市场模式类
class MarketPattern {
public:
    MarketPattern(PatternType type, double confidence, uint64_t timestamp);
    ~MarketPattern();

    // 获取模式类型
    PatternType getType() const { return m_type; }
    // 获取置信度
    double getConfidence() const { return m_confidence; }
    // 获取时间戳
    uint64_t getTimestamp() const { return m_timestamp; }
    // 获取模式类型字符串
    std::string getTypeString() const;

    // 设置额外特征
    void addFeature(const std::string& name, double value);
    // 获取额外特征
    double getFeature(const std::string& name) const;
    // 检查是否存在特征
    bool hasFeature(const std::string& name) const;

private:
    PatternType m_type;
    double m_confidence;
    uint64_t m_timestamp;
    std::unordered_map<std::string, double> m_features;
};

} // namespace pattern
} // namespace hft