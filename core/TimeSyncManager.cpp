#include "TimeSyncManager.h"
#include "../core/Logger.h"
#include <chrono>
#include <algorithm>
#include <cmath>

namespace hft {
namespace core {

void TimeSyncManager::initialize() {
    // 初始化时间源
    initializeTimeSources();
    
    // 设置默认参数
    params_.sync_interval = std::chrono::milliseconds(100);  // 100ms同步间隔
    params_.max_drift = std::chrono::microseconds(10);       // 最大允许漂移10us
    params_.kalman_process_noise = 1e-6;                     // 卡尔曼滤波器参数
    params_.kalman_measurement_noise = 1e-4;
}

void TimeSyncManager::startSync() {
    is_running_ = true;
    
    // 启动同步线程
    sync_thread_ = std::thread([this] {
        while (is_running_) {
            try {
                // 1. 收集时间样本
                auto samples = collectTimeSamples();
                
                // 2. 过滤异常值
                filterOutliers(samples);
                
                // 3. 计算时钟偏移
                calculateClockOffset(samples);
                
                // 4. 更新时钟校正
                updateClockCorrection();
                
                // 等待下一个同步周期
                std::this_thread::sleep_for(params_.sync_interval);
                
            } catch (const std::exception& e) {
                Logger::error("Time sync error: {}", e.what());
            }
        }
    });
}

void TimeSyncManager::stopSync() {
    is_running_ = false;
    if (sync_thread_.joinable()) {
        sync_thread_.join();
    }
}

std::chrono::nanoseconds TimeSyncManager::getTime() const {
    // 获取硬件时间戳
    auto hw_time = readHardwareTimestamp();
    
    // 应用校正
    return applyCorrection(hw_time);
}

double TimeSyncManager::getClockOffset() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_offset_;
}

double TimeSyncManager::getClockDrift() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_drift_;
}

void TimeSyncManager::initializeTimeSources() {
    // 初始化硬件时间戳计数器
    initializeHardwareCounter();
    
    // 配置PTP客户端
    configurePTPClient();
    
    // 初始化GPS接收器(如果可用)
    initializeGPSReceiver();
}

std::vector<TimeSample> TimeSyncManager::collectTimeSamples() {
    std::vector<TimeSample> samples;
    
    // 1. 收集PTP样本
    collectPTPSamples(samples);
    
    // 2. 收集GPS样本(如果可用)
    if (gps_available_) {
        collectGPSSamples(samples);
    }
    
    // 3. 收集硬件时间戳样本
    collectHardwareTimestamps(samples);
    
    return samples;
}

void TimeSyncManager::filterOutliers(
    std::vector<TimeSample>& samples) {
    
    // 1. 计算中位数和标准差
    double median = calculateMedian(samples);
    double std_dev = calculateStandardDeviation(samples, median);
    
    // 2. 移除3个标准差以外的样本
    auto it = std::remove_if(samples.begin(), samples.end(),
        [=](const TimeSample& sample) {
            return std::abs(sample.offset - median) > 3 * std_dev;
        });
    
    samples.erase(it, samples.end());
}

void TimeSyncManager::calculateClockOffset(
    const std::vector<TimeSample>& samples) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 1. 更新卡尔曼滤波器状态
    updateKalmanFilter(samples);
    
    // 2. 计算新的时钟偏移
    current_offset_ = kalman_filter_.getState()[0];
    
    // 3. 计算时钟漂移率
    current_drift_ = kalman_filter_.getState()[1];
    
    // 4. 检查是否需要告警
    checkDriftAlerts();
}

void TimeSyncManager::updateClockCorrection() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 1. 计算新的校正参数
    auto new_correction = calculateCorrection();
    
    // 2. 平滑过渡到新的校正值
    smoothCorrection(new_correction);
    
    // 3. 更新校正表
    updateCorrectionTable();
}

std::chrono::nanoseconds TimeSyncManager::applyCorrection(
    std::chrono::nanoseconds raw_time) const {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 1. 应用线性校正
    auto corrected = raw_time + 
        std::chrono::nanoseconds(
            static_cast<int64_t>(current_offset_ * 1e9));
    
    // 2. 应用漂移校正
    corrected += std::chrono::nanoseconds(
        static_cast<int64_t>(current_drift_ * 
            raw_time.count()));
    
    // 3. 查表进行非线性校正
    corrected = applyNonlinearCorrection(corrected);
    
    return corrected;
}

void TimeSyncManager::updateKalmanFilter(
    const std::vector<TimeSample>& samples) {
    
    for (const auto& sample : samples) {
        // 测量向量
        Eigen::Vector2d measurement;
        measurement << sample.offset, sample.drift;
        
        // 更新滤波器
        kalman_filter_.update(
            measurement,
            sample.timestamp.time_since_epoch().count() * 1e-9);
    }
}

void TimeSyncManager::checkDriftAlerts() {
    // 检查漂移是否超过阈值
    if (std::abs(current_drift_) > params_.max_drift.count() * 1e-6) {
        // 发送告警
        Alert alert{
            .type = AlertType::CLOCK_DRIFT,
            .severity = AlertSeverity::HIGH,
            .message = "Clock drift exceeded threshold",
            .value = current_drift_
        };
        
        raiseAlert(alert);
    }
}

TimeStats TimeSyncManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TimeStats stats;
    
    // 计算同步精度
    stats.sync_accuracy = calculateSyncAccuracy();
    
    // 计算稳定性指标
    stats.stability = calculateStability();
    
    // 收集漂移统计
    stats.drift_stats = calculateDriftStats();
    
    // 收集告警统计
    stats.alert_stats = calculateAlertStats();
    
    return stats;
}

bool TimeSyncManager::isSyncAccurate() const {
    auto stats = getStats();
    
    // 检查同步精度
    if (stats.sync_accuracy > params_.max_sync_error) {
        return false;
    }
    
    // 检查稳定性
    if (stats.stability < params_.min_stability) {
        return false;
    }
    
    return true;
}

} // namespace core
} // namespace hft
