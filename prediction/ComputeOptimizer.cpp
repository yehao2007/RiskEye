#include "EnhancedPredictor.h"
#include "../core/Logger.h"
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <tbb/parallel_for.h>

namespace hft {
namespace prediction {

void ComputeOptimizer::initialize() {
    // 检查CUDA设备
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    Logger::info("Found {} CUDA devices", deviceCount);
    
    // 选择最佳设备
    int bestDevice = selectBestDevice();
    cudaSetDevice(bestDevice);
    
    // 初始化CUDA流
    const int numStreams = 4;
    streams_.resize(numStreams);
    for (auto& stream : streams_) {
        cudaStreamCreate(&stream);
    }
    
    // 初始化cuBLAS
    cublasCreate(&handle_);
    
    Logger::info("Compute optimizer initialized with {} streams", numStreams);
}

void ComputeOptimizer::optimizeComputation(const std::string& modelType) {
    // 根据模型类型选择优化策略
    if (modelType == "transformer") {
        optimizeTransformerComputation();
    } else if (modelType == "cnn") {
        optimizeCNNComputation();
    } else if (modelType == "rnn") {
        optimizeRNNComputation();
    }
    
    // 监控和调整性能
    auto metrics = monitorPerformance();
    adjustOptimization(metrics);
}

void ComputeOptimizer::optimizeTransformerComputation() {
    // 使用半精度训练
    if (device_.is_cuda() && torch::cuda::is_available()) {
        model_ = model_.to(torch::kCUDA).to(torch::kFloat16);
        useHalfPrecision_ = true;
    }
    
    // 批处理优化
    const int optimalBatchSize = determineOptimalBatchSize();
    
    // 使用CUDA图优化重复计算
    cudaGraph_t graph;
    cudaGraphExec_t instance;
    
    // 捕获计算图
    cudaStreamBeginCapture(streams_[0], cudaStreamCaptureModeGlobal);
    // 执行计算
    cudaStreamEndCapture(streams_[0], &graph);
    
    // 实例化和启动图
    cudaGraphInstantiate(&instance, graph, nullptr, nullptr, 0);
    cudaGraphLaunch(instance, streams_[0]);
}

void ComputeOptimizer::optimizeCNNComputation() {
    // 使用cuDNN优化卷积操作
    cudnnCreate(&cudnnHandle_);
    
    // 设置最佳算法
    cudnnConvolutionFwdAlgo_t algo;
    cudnnGetConvolutionForwardAlgorithm(
        cudnnHandle_,
        inputDesc_,
        filterDesc_,
        convDesc_,
        outputDesc_,
        CUDNN_CONVOLUTION_FWD_PREFER_FASTEST,
        0,
        &algo
    );
    
    // 使用Tensor Cores (如果可用)
    cudnnSetConvolutionMathType(convDesc_, CUDNN_TENSOR_OP_MATH);
}

void ComputeOptimizer::optimizeRNNComputation() {
    // 使用持久化RNN
    cudnnRNNDescriptor_t rnnDesc;
    cudnnSetRNNDescriptor(
        cudnnHandle_,
        rnnDesc,
        hiddenSize_,
        numLayers_,
        dropoutDesc_,
        CUDNN_LINEAR_INPUT,
        CUDNN_UNIDIRECTIONAL,
        CUDNN_LSTM,
        CUDNN_RNN_ALGO_PERSIST_STATIC,
        CUDNN_DATA_FLOAT
    );
}

ComputeMetrics ComputeOptimizer::monitorPerformance() {
    ComputeMetrics metrics;
    
    // 监控GPU利用率
    int device;
    cudaGetDevice(&device);
    
    nvmlDevice_t nvmlDevice;
    nvmlDeviceGetHandleByIndex(device, &nvmlDevice);
    
    // 获取GPU利用率
    nvmlUtilization_t utilization;
    nvmlDeviceGetUtilizationRates(nvmlDevice, &utilization);
    metrics.gpuUtilization = utilization.gpu;
    
    // 获取内存使用情况
    size_t free, total;
    cudaMemGetInfo(&free, &total);
    metrics.memoryUsage = (total - free) / (double)total;
    
    // 获取功耗信息
    unsigned int power;
    nvmlDeviceGetPowerUsage(nvmlDevice, &power);
    metrics.powerEfficiency = power / 1000.0;  // 转换为瓦特
    
    // 计算吞吐量
    metrics.throughput = calculateThroughput();
    
    return metrics;
}

void ComputeOptimizer::adjustOptimization(const ComputeMetrics& metrics) {
    // 根据性能指标调整优化策略
    
    // 调整批大小
    if (metrics.gpuUtilization < 70.0) {
        increaseBatchSize();
    } else if (metrics.gpuUtilization > 95.0) {
        decreaseBatchSize();
    }
    
    // 调整精度
    if (metrics.memoryUsage > 0.9) {
        enableMixedPrecision();
    }
    
    // 调整缓存策略
    if (metrics.throughput < targetThroughput_) {
        optimizeMemoryAccess();
    }
    
    // 功耗优化
    if (metrics.powerEfficiency > powerThreshold_) {
        optimizePowerEfficiency();
    }
}

void ComputeOptimizer::optimizeMemoryAccess() {
    // 实现零拷贝内存访问
    void* pinnedMemory;
    cudaMallocHost(&pinnedMemory, size);
    
    // 使用统一内存
    void* unifiedMemory;
    cudaMallocManaged(&unifiedMemory, size);
    
    // 预取数据
    cudaMemPrefetchAsync(unifiedMemory, size, device_.index());
    
    // 使用纹理内存加速访问
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float>();
    cudaArray_t texArray;
    cudaMallocArray(&texArray, &channelDesc, width, height);
}

int ComputeOptimizer::selectBestDevice() {
    int bestDevice = 0;
    int maxScore = 0;
    
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    
    for (int device = 0; device < deviceCount; ++device) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, device);
        
        // 计算设备得分
        int score = prop.multiProcessorCount * prop.clockRate;
        score += prop.totalGlobalMem / (1024 * 1024);  // 内存大小(MB)
        
        if (prop.computeMode != cudaComputeModeProhibited && score > maxScore) {
            maxScore = score;
            bestDevice = device;
        }
    }
    
    return bestDevice;
}

} // namespace prediction
} // namespace hft
