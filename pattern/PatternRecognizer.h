#pragma once
#include <vector>
#include <memory>
#include "DeepLearningModel.h"
#include "CnnDetector.h"
#include "TransformerModel.h"

namespace hft {
namespace pattern {

class PatternRecognizer {
public:
    PatternRecognizer();
    ~PatternRecognizer();

    // 初始化模式识别器
    bool initialize();
    // 加载模型
    bool loadModels(const std::string& dnnModelPath,
                    const std::string& cnnModelPath,
                    const std::string& transformerModelPath);
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "DnnModel.h"
#include "MarketPattern.h"
#include "core/EventLoop.h"

namespace hft {
namespace pattern {

class PatternRecognizer {
public:
    PatternRecognizer();
    ~PatternRecognizer();

    // 初始化
    bool initialize(const std::string& configPath);
    // 启动识别
    void start();
    // 停止识别
    void stop();
    // 识别器是否运行中
    bool isRunning() const { return m_running; }

    // 加载模型
    bool loadModel(const std::string& modelPath);
    // 识别市场模式
    std::vector<MarketPattern> recognizePattern(const std::vector<double>& marketData);
    // 注册模式识别回调
    void registerPatternCallback(std::function<void(const MarketPattern&)> callback);

    // 设置识别间隔（毫秒）
    void setRecognitionInterval(int intervalMs) { m_recognitionIntervalMs = intervalMs; }
    // 获取识别间隔
    int getRecognitionInterval() const { return m_recognitionIntervalMs; }

private:
    std::shared_ptr<DnnModel> m_dnnModel;
    std::shared_ptr<core::EventLoop> m_eventLoop;
    bool m_running;
    int m_recognitionIntervalMs;
    std::vector<std::function<void(const MarketPattern&)>> m_callbacks;

    // 定期执行识别的任务
    void recognitionTask();
    // 获取市场数据特征
    std::vector<double> getMarketDataFeatures() const;
};

} // namespace pattern
} // namespace hft

    // 使用DNN识别模式
    std::vector<float> recognizeWithDnn(const std::vector<float>& features);
    // 使用CNN识别模式
    std::vector<float> recognizeWithCnn(const std::vector<std::vector<float>>& imageData);
    // 使用Transformer识别模式
    std::vector<float> recognizeWithTransformer(const std::vector<std::vector<float>>& timeSeries);

    // 自适应选择最佳模型
    std::vector<float> adaptiveRecognize(const std::vector<float>& features,
                                         const std::string& context);

private:
    std::unique_ptr<DeepLearningModel> m_dnnModel;
    std::unique_ptr<CnnDetector> m_cnnModel;
    std::unique_ptr<TransformerModel> m_transformerModel;

    // 模型选择器
    int selectBestModel(const std::string& context);
};

} // namespace pattern
} // namespace hft