#pragma once
#include <string>
#include <vector>
#include <memory>
#include "MarketPattern.h"

namespace hft {
namespace pattern {

class DnnModel {
public:
    DnnModel();
    ~DnnModel();

    // 加载模型
    bool loadModel(const std::string& modelPath);
    // 保存模型
    bool saveModel(const std::string& modelPath) const;
    // 模型是否已加载
    bool isLoaded() const { return m_modelLoaded; }

    // 前向推理
    std::vector<MarketPattern> predict(const std::vector<double>& inputFeatures);
    // 批量推理
    std::vector<std::vector<MarketPattern>> batchPredict(const std::vector<std::vector<double>>& inputFeaturesBatch);

    // 训练模型
    bool train(const std::vector<std::vector<double>>& trainingData,
               const std::vector<PatternType>& labels,
               int epochs, int batchSize, double learningRate);

    // 获取模型输入特征数量
    int getInputFeatureCount() const { return m_inputFeatureCount; }
    // 获取模型输出类别数量
    int getOutputClassCount() const { return m_outputClassCount; }

private:
    bool m_modelLoaded;
    int m_inputFeatureCount;
    int m_outputClassCount;
    // 实际应用中，这里会包含深度学习框架的模型对象
};

} // namespace pattern
} // namespace hft