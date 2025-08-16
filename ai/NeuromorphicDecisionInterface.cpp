#include "NeuromorphicDecisionInterface.h"
#include <Python.h>
#include <iostream>
#include <chrono>
#include <stdexcept>

namespace hft {
namespace ai {

class NeuromorphicDecisionInterface::Impl {
public:
    Impl(const std::string& weights_path)
        : weights_path_(weights_path), last_latency_(0) {
    }

    ~Impl() {
        if (py_module_ != nullptr) {
            Py_DECREF(py_module_);
            py_module_ = nullptr;
        }
        if (py_class_ != nullptr) {
            Py_DECREF(py_class_);
            py_class_ = nullptr;
        }
        if (py_instance_ != nullptr) {
            Py_DECREF(py_instance_);
            py_instance_ = nullptr;
        }
    }

    bool initialize() {
        // 初始化Python解释器
        Py_Initialize();

        // 导入模块
        py_module_ = PyImport_ImportModule("ai.NeuromorphicDecision");
        if (py_module_ == nullptr) {
            PyErr_Print();
            return false;
        }

        // 获取类
        py_class_ = PyObject_GetAttrString(py_module_, "NeuromorphicDecisionModule");
        if (py_class_ == nullptr || !PyCallable_Check(py_class_)) {
            PyErr_Print();
            return false;
        }

        // 创建实例
        PyObject* args = PyTuple_New(1);
        PyTuple_SetItem(args, 0, PyUnicode_FromString(weights_path_.c_str()));
        py_instance_ = PyObject_CallObject(py_class_, args);
        Py_DECREF(args);

        if (py_instance_ == nullptr) {
            PyErr_Print();
            return false;
        }

        return true;
    }

    int makeDecision(const std::vector<float>& market_data) {
        if (py_instance_ == nullptr) {
            throw std::runtime_error("NeuromorphicDecisionModule not initialized");
        }

        // 记录开始时间
        auto start = std::chrono::high_resolution_clock::now();

        // 转换市场数据为Python列表
        PyObject* py_data = PyList_New(market_data.size());
        for (size_t i = 0; i < market_data.size(); ++i) {
            PyList_SetItem(py_data, i, PyFloat_FromDouble(market_data[i]));
        }

        // 调用make_decision方法
        PyObject* result = PyObject_CallMethod(py_instance_, "make_decision", "(O)", py_data);
        Py_DECREF(py_data);

        if (result == nullptr) {
            PyErr_Print();
            return -1;
        }

        // 解析结果
        int action = PyLong_AsLong(result);
        Py_DECREF(result);

        // 计算延迟
        auto end = std::chrono::high_resolution_clock::now();
        last_latency_ = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        return action;
    }

    uint64_t getLastDecisionLatency() const {
        return last_latency_;
    }

private:
    std::string weights_path_;
    PyObject* py_module_ = nullptr;
    PyObject* py_class_ = nullptr;
    PyObject* py_instance_ = nullptr;
    uint64_t last_latency_;
};

NeuromorphicDecisionInterface::NeuromorphicDecisionInterface(const std::string& weights_path)
    : pImpl(std::make_unique<Impl>(weights_path)) {
}

NeuromorphicDecisionInterface::~NeuromorphicDecisionInterface() = default;

bool NeuromorphicDecisionInterface::initialize() {
    return pImpl->initialize();
}

int NeuromorphicDecisionInterface::makeDecision(const std::vector<float>& market_data) {
    return pImpl->makeDecision(market_data);
}

uint64_t NeuromorphicDecisionInterface::getLastDecisionLatency() const {
    return pImpl->getLastDecisionLatency();
}

} // namespace ai
} // namespace hft