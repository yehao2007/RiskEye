#pragma once

#include <string>
#include <vector>
#include <memory>
#include <tensorflow/c/c_api.h>

namespace hft {
namespace ai {

struct ModelInput {
    std::vector<double> features;
    uint64_t timestamp;
};

// 模型输出
struct ModelOutput {
    std::vector<double> predictions;
    double confidence;
};

// 模型接口
class Model {
public:
    Model();
    virtual ~Model() = default;

    // 加载模型
    virtual bool load(const std::string& model_path) = 0;

    // 保存模型
    virtual bool save(const std::string& model_path) = 0;

    // 预测
    virtual ModelOutput predict(const std::vector<double>& features) = 0;

    // 训练
    virtual bool train(const std::vector<std::vector<double>>& features, 
                      const std::vector<double>& labels) = 0;

    // 是否已加载
    virtual bool isLoaded() const = 0;
};

using ModelPtr = std::shared_ptr<Model>;

} // namespace ai
} // namespace hft