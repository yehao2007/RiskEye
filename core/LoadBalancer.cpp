#include "LoadBalancer.h"
#include "../core/Logger.h"
#include "../ai/MLModels.h"
#include <algorithm>

namespace hft {
namespace core {

void LoadBalancer::initialize() {
    // 初始化ML模型
    initializeMLModel();
    
    // 设置默认参数
    params_.check_interval = std::chrono::milliseconds(100);   // 100ms检查间隔
    params_.rebalance_threshold = 0.2;                         // 20%负载不均衡阈值
    params_.max_queue_size = 1000;                            // 最大队列长度
    params_.predict_window = std::chrono::seconds(1);         // 1秒预测窗口
}

void LoadBalancer::startBalancing() {
    is_running_ = true;
    
    // 启动负载均衡线程
    balance_thread_ = std::thread([this] {
        while (is_running_) {
            try {
                // 1. 收集负载指标
                auto metrics = collectLoadMetrics();
                
                // 2. 分析负载分布
                analyzeLoadDistribution(metrics);
                
                // 3. 预测未来负载
                predictFutureLoad(metrics);
                
                // 4. 执行负载均衡
                balanceLoad();
                
                // 等待下一个检查周期
                std::this_thread::sleep_for(params_.check_interval);
                
            } catch (const std::exception& e) {
                Logger::error("Load balancing error: {}", e.what());
            }
        }
    });
}

void LoadBalancer::stopBalancing() {
    is_running_ = false;
    if (balance_thread_.joinable()) {
        balance_thread_.join();
    }
}

void LoadBalancer::registerWorker(
    WorkerId worker_id,
    WorkerCapacity capacity) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 注册新工作节点
    workers_[worker_id] = WorkerInfo{
        .capacity = capacity,
        .current_load = 0,
        .status = WorkerStatus::AVAILABLE,
        .performance_metrics = {}
    };
    
    // 更新负载分布
    updateLoadDistribution();
}

void LoadBalancer::unregisterWorker(WorkerId worker_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 安全下线工作节点
    gracefulShutdown(worker_id);
    
    // 移除节点
    workers_.erase(worker_id);
    
    // 更新负载分布
    updateLoadDistribution();
}

WorkerId LoadBalancer::assignTask(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // 1. 预测任务负载
        auto predicted_load = predictTaskLoad(task);
        
        // 2. 选择最佳工作节点
        auto worker_id = selectBestWorker(task, predicted_load);
        
        // 3. 分配任务
        assignTaskToWorker(task, worker_id);
        
        return worker_id;
        
    } catch (const std::exception& e) {
        Logger::error("Task assignment failed: {}", e.what());
        throw;
    }
}

LoadMetrics LoadBalancer::collectLoadMetrics() {
    LoadMetrics metrics;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 收集每个工作节点的指标
    for (const auto& [id, worker] : workers_) {
        // CPU使用率
        metrics.cpu_usage[id] = measureCPUUsage(id);
        
        // 内存使用率
        metrics.memory_usage[id] = measureMemoryUsage(id);
        
        // 队列长度
        metrics.queue_length[id] = getQueueLength(id);
        
        // 处理延迟
        metrics.processing_latency[id] = measureProcessingLatency(id);
    }
    
    return metrics;
}

void LoadBalancer::analyzeLoadDistribution(
    const LoadMetrics& metrics) {
    
    // 1. 计算负载分布统计
    auto stats = calculateLoadStats(metrics);
    
    // 2. 检测负载不均衡
    if (detectLoadImbalance(stats)) {
        // 触发重新平衡
        scheduleRebalancing();
    }
    
    // 3. 更新负载历史
    updateLoadHistory(metrics);
    
    // 4. 检查过载情况
    checkOverloadConditions(metrics);
}

void LoadBalancer::predictFutureLoad(
    const LoadMetrics& current_metrics) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 使用ML模型预测未来负载
    predicted_load_ = ml_model_->predict(
        current_metrics, 
        params_.predict_window);
        
    // 更新预测统计
    updatePredictionStats();
}

void LoadBalancer::balanceLoad() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 1. 计算目标负载分布
    auto target_distribution = calculateTargetDistribution();
    
    // 2. 生成迁移计划
    auto migration_plan = generateMigrationPlan(target_distribution);
    
    // 3. 执行任务迁移
    executeMigrationPlan(migration_plan);
}

WorkerId LoadBalancer::selectBestWorker(
    const Task& task,
    double predicted_load) {
    
    // 1. 过滤可用工作节点
    auto available_workers = filterAvailableWorkers();
    
    // 2. 计算每个节点的适合度分数
    std::vector<std::pair<WorkerId, double>> scores;
    for (const auto& worker_id : available_workers) {
        double score = calculateWorkerScore(
            worker_id, task, predicted_load);
        scores.emplace_back(worker_id, score);
    }
    
    // 3. 选择最高分数的节点
    auto best_worker = std::max_element(
        scores.begin(), scores.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
        
    return best_worker->first;
}

double LoadBalancer::calculateWorkerScore(
    WorkerId worker_id,
    const Task& task,
    double predicted_load) {
    
    const auto& worker = workers_[worker_id];
    
    // 1. 容量得分
    double capacity_score = (worker.capacity.max_load - 
        worker.current_load) / worker.capacity.max_load;
    
    // 2. 性能得分
    double perf_score = calculatePerformanceScore(
        worker.performance_metrics);
    
    // 3. 亲和性得分
    double affinity_score = calculateAffinityScore(
        worker_id, task);
    
    // 4. 负载预测得分
    double prediction_score = calculatePredictionScore(
        worker_id, predicted_load);
    
    // 综合评分
    return (capacity_score * 0.4 + 
            perf_score * 0.3 + 
            affinity_score * 0.2 + 
            prediction_score * 0.1);
}

void LoadBalancer::executeMigrationPlan(
    const MigrationPlan& plan) {
    
    for (const auto& migration : plan.migrations) {
        try {
            // 1. 准备迁移
            prepareMigration(migration);
            
            // 2. 执行迁移
            performMigration(migration);
            
            // 3. 验证迁移
            verifyMigration(migration);
            
            // 4. 更新状态
            updateAfterMigration(migration);
            
        } catch (const std::exception& e) {
            Logger::error("Migration failed: {}", e.what());
            handleMigrationFailure(migration);
        }
    }
}

LoadBalancerStats LoadBalancer::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    LoadBalancerStats stats;
    
    // 计算负载均衡指标
    stats.balance_score = calculateBalanceScore();
    
    // 计算性能指标
    stats.performance_stats = calculatePerformanceStats();
    
    // 计算预测准确度
    stats.prediction_accuracy = calculatePredictionAccuracy();
    
    // 收集迁移统计
    stats.migration_stats = calculateMigrationStats();
    
    return stats;
}

bool LoadBalancer::isBalanced() const {
    auto stats = getStats();
    return stats.balance_score >= params_.balance_threshold;
}

} // namespace core
} // namespace hft
