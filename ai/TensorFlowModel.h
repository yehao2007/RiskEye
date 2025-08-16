#pragma once
#include "Model.h"
#include <tensorflow/c/c_api.h>
#include <string>
#include <vector>

namespace hft {
namespace ai {

class TensorFlowModel : public Model {
public:
    TensorFlowModel();
    ~TensorFlowModel() override;

    bool load(const std::string& model_path) override;
    bool save(const std::string& model_path) override;
    ModelOutput predict(const std::vector<double>& features) override;
    bool train(const std::vector<std::vector<double>>& features, 
               const std::vector<double>& labels) override;
    bool isLoaded() const override;

    // 新增功能
    double evaluate(const std::vector<std::vector<double>>& features, 
                    const std::vector<double>& labels);
    void setLearningRate(double rate);
    void setTrainingSteps(int steps);
    std::vector<double> getWeights();
    void setWeights(const std::vector<double>& weights);
    void enableGPUAcceleration(bool enable);

private:
    bool initialize();
    void release();

    TF_Graph* m_graph;
    TF_Session* m_session;
    TF_Status* m_status;
    std::vector<TF_Output> m_inputs;
    std::vector<TF_Output> m_outputs;
    std::vector<TF_Output> m_training_outputs;
    bool m_is_loaded;
    double m_learning_rate;
    int m_training_steps;
    bool m_use_gpu;
};

} // namespace ai
} // namespace hft