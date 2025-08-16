#include "PatternRecognizer.h"
#include <iostream>
#include <stdexcept>
#include <chrono>

namespace hft {
namespace pattern {

PatternRecognizer::PatternRecognizer() : m_running(false), m_recognitionIntervalMs(100) {
    m_dnnModel = std::make_shared<DnnModel>();
    m_eventLoop = std::make_shared<core::EventLoop>();
}

PatternRecognizer::~PatternRecognizer() {
    stop();
}

bool PatternRecognizer::initialize(const std::string& configPath) {
    // 模拟从配置文件加载参数
    std::cout << "Initializing pattern recognizer from config: " << configPath << std::endl;

    // 在实际应用中，这里会从配置文件读取参数
    m_recognitionIntervalMs = 100; // 100ms识别一次

    // 加载默认模型
    if (!loadModel("config\\pattern_model.onnx")) {
        std::cerr << "Failed to load default model" << std::endl;
        return false;
    }

    return true;
}

void PatternRecognizer::start() {
    if (m_running) {
        return;
    }

    if (!m_dnnModel->isLoaded()) {
        throw std::runtime_error("No model loaded");
    }

    m_running = true;
    m_eventLoop->start();

    // 安排定期识别任务
    m_eventLoop->scheduleAtFixedRate(
        std::bind(&PatternRecognizer::recognitionTask, this),
        0, // 立即开始
        m_recognitionIntervalMs * 1000); // 转换为微秒

    std::cout << "Pattern recognizer started. Interval: " << m_recognitionIntervalMs << "ms" << std::endl;
}

void PatternRecognizer::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;
    m_eventLoop->stop();
    std::cout << "Pattern recognizer stopped" << std::endl;
}

bool PatternRecognizer::loadModel(const std::string& modelPath) {
    if (!m_dnnModel->loadModel(modelPath)) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        return false;
    }

    std::cout << "Model loaded successfully: " << modelPath << std::endl;
    return true;
}

std::vector<MarketPattern> PatternRecognizer::recognizePattern(const std::vector<double>& marketData) {
    if (!m_dnnModel->isLoaded()) {
        throw std::runtime_error("No model loaded");
    }

    // 调用DNN模型进行预测
    return m_dnnModel->predict(marketData);
}

void PatternRecognizer::registerPatternCallback(std::function<void(const MarketPattern&)> callback) {
    m_callbacks.push_back(callback);
}

void PatternRecognizer::recognitionTask() {
    try {
        // 获取市场数据特征
        std::vector<double> features = getMarketDataFeatures();

        // 识别模式
        std::vector<MarketPattern> patterns = recognizePattern(features);

        // 触发回调
        for (const auto& pattern : patterns) {
            for (const auto& callback : m_callbacks) {
                callback(pattern);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Pattern recognition error: " << e.what() << std::endl;
    }
}

std::vector<double> PatternRecognizer::getMarketDataFeatures() const {
    // 模拟获取市场数据特征
    // 在实际应用中，这里会从市场数据feed获取数据并提取特征
    std::vector<double> features;
    features.reserve(m_dnnModel->getInputFeatureCount());

    // 生成随机特征数据作为示例
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < m_dnnModel->getInputFeatureCount(); ++i) {
        features.push_back(static_cast<double>(rand()) / RAND_MAX);
    }

    return features;
}

} // namespace pattern
} // namespace hft