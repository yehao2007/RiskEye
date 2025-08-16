#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <random>

#include "core/SelfOperatingSystem.h"
#include "core/FaultDetector.h"
#include "core/FaultToleranceManager.h"
#include "core/SystemMonitor.h"
#include "core/Logger.h"
#include "core/System.h"
#include "core/AlertManager.h"
#include "network/DistributedCommunicator.h"

using namespace hft;
using namespace hft::core;

// 模拟系统组件，用于演示目的
class MockSystemMonitor : public SystemMonitor {
public:
    bool initialize() override { 
        std::cout << "初始化系统监控器..." << std::endl;
        return true; 
    }
    
    void startMonitoring() override {
        std::cout << "开始系统监控..." << std::endl;
    }
    
    void stopMonitoring() override {
        std::cout << "停止系统监控..." << std::endl;
    }
    
    SystemState getSystemState() const override {
        SystemState state;
        
        // 生成随机系统状态
        static std::mt19937 gen(std::random_device{}());
        static std::uniform_real_distribution<> cpu_dist(30.0, 95.0);
        static std::uniform_real_distribution<> mem_dist(50.0, 90.0);
        static std::uniform_real_distribution<> latency_dist(100.0, 2000.0);
        
        // CPU状态
        state.cpu.total_usage = cpu_dist(gen);
        state.cpu.core_usage = {cpu_dist(gen), cpu_dist(gen), cpu_dist(gen), cpu_dist(gen)};
        state.cpu.temperature = 65.0 + (cpu_dist(gen) - 50.0) / 10.0;
        state.cpu.context_switches = 50000 + static_cast<uint64_t>(cpu_dist(gen) * 1000);
        
        // 内存状态
        state.memory.total_bytes = 16 * 1024 * 1024 * 1024ULL; // 16 GB
        state.memory.used_bytes = static_cast<size_t>(mem_dist(gen) / 100.0 * state.memory.total_bytes);
        state.memory.cached_bytes = 2 * 1024 * 1024 * 1024ULL; // 2 GB
        state.memory.page_faults = 100 + static_cast<size_t>(mem_dist(gen) * 5);
        
        // 网络状态
        state.network.ingress_mbps = 100.0 + latency_dist(gen) / 100.0;
        state.network.egress_mbps = 80.0 + latency_dist(gen) / 120.0;
        state.network.latency_us = latency_dist(gen);
        state.network.connection_count = 200 + static_cast<int>(cpu_dist(gen));
        
        // 应用状态
        state.application.orders_per_second = 5000 + static_cast<uint64_t>(cpu_dist(gen) * 100);
        state.application.market_data_per_second = 20000 + static_cast<uint64_t>(cpu_dist(gen) * 200);
        state.application.order_latency_us = latency_dist(gen) / 2.0;
        state.application.active_strategies = 10 + static_cast<int>(mem_dist(gen) / 20.0);
        
        return state;
    }
    
    double getMetric(MetricType type) const override {
        switch (type) {
            case MetricType::CPU_USAGE: return 75.0;
            case MetricType::MEMORY_USAGE: return 8 * 1024 * 1024 * 1024ULL;
            case MetricType::NETWORK_LATENCY: return 800.0;
            default: return 0.0;
        }
    }
};

class MockFaultDetector : public FaultDetector {
public:
    bool initialize() override { 
        std::cout << "初始化故障检测器..." << std::endl;
        return true; 
    }
    
    void startDetection() override {
        std::cout << "开始故障检测..." << std::endl;
    }
    
    void stopDetection() override {
        std::cout << "停止故障检测..." << std::endl;
    }
    
    void registerFaultCallback(FaultCallback callback) override {
        callback_ = callback;
    }
    
    HealthStatus getHealthStatus() const override {
        HealthStatus status;
        status.is_healthy = true;
        status.health_score = 0.85;
        
        // 如果启用模拟故障，添加一些故障
        if (simulate_fault_) {
            status.is_healthy = false;
            status.health_score = 0.65;
            
            FaultInfo fault;
            fault.type = FaultType::MEMORY_LEAK;
            fault.severity = FaultSeverity::MEDIUM;
            fault.component = "MemoryManager";
            fault.description = "检测到内存泄漏: 持续增长的内存分配";
            fault.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            fault.affected_systems = {"MemoryPool", "DataStore"};
            
            status.active_faults.push_back(fault);
            status.warnings.push_back("内存使用率持续增长，建议检查内存分配");
            
            // 如果有回调，调用回调函数
            if (callback_) {
                callback_(fault);
            }
        }
        
        return status;
    }
    
    bool attemptRecovery(const FaultInfo& fault) override {
        std::cout << "尝试恢复故障: " << fault.description << std::endl;
        
        // 模拟故障恢复
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        
        std::cout << "故障恢复" << (fault.severity == FaultSeverity::CRITICAL ? "失败" : "成功") << std::endl;
        
        // 对于严重故障，恢复失败
        return fault.severity != FaultSeverity::CRITICAL;
    }
    
    // 启用/禁用模拟故障
    void setSimulateFault(bool simulate) {
        simulate_fault_ = simulate;
    }
    
private:
    FaultCallback callback_;
    bool simulate_fault_ = false;
};

class MockSystem : public System {
public:
    bool initialize() override { return true; }
    void shutdown() override {}
    void start() override {}
    void stop() override {}
};

class MockAlertManager : public AlertManager {
public:
    void sendAlert(const std::string& title, const std::string& message, AlertLevel level) override {
        std::cout << "告警 [" << static_cast<int>(level) << "]: " << title << " - " << message << std::endl;
    }
};

class MockDistributedCommunicator : public network::DistributedCommunicator {
public:
    bool initialize() override { return true; }
    void shutdown() override {}
    void sendMessage(const std::string& target, const std::string& message) override {}
};

class MockFaultToleranceManager : public FaultToleranceManager {
public:
    MockFaultToleranceManager() : FaultToleranceManager(&system_, &alert_manager_, &communicator_) {}
    
    bool initialize() override { 
        std::cout << "初始化容错管理器..." << std::endl;
        return true; 
    }
    
    void startFaultDetection() override {
        std::cout << "开始容错检测..." << std::endl;
    }
    
    void stopFaultDetection() override {
        std::cout << "停止容错检测..." << std::endl;
    }
    
    void handleFault(const std::string& component_name, const std::string& fault_type) override {
        std::cout << "处理故障: " << component_name << " - " << fault_type << std::endl;
    }
    
    bool recoverComponent(const std::string& component_name) override {
        std::cout << "恢复组件: " << component_name << std::endl;
        return true;
    }
    
    float getSystemHealth() const override {
        return 0.85f;
    }
    
private:
    MockSystem system_;
    MockAlertManager alert_manager_;
    MockDistributedCommunicator communicator_;
};

// 格式化时间字符串
std::string getTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    return ss.str();
}

// 自定义日志器
class ConsoleLogger : public Logger {
public:
    void info(const std::string& message) override {
        std::cout << getTimeString() << " [INFO] " << message << std::endl;
    }
    
    void warning(const std::string& message) override {
        std::cout << getTimeString() << " [WARN] " << message << std::endl;
    }
    
    void error(const std::string& message) override {
        std::cout << getTimeString() << " [ERROR] " << message << std::endl;
    }
    
    void debug(const std::string& message) override {
        std::cout << getTimeString() << " [DEBUG] " << message << std::endl;
    }
};

// 演示自运营系统功能
void demonstrateSelfOperatingSystem() {
    std::cout << "=========================================================" << std::endl;
    std::cout << "   🤖 自运营系统演示 - 自动修复与无人值守模式 🤖" << std::endl;
    std::cout << "=========================================================" << std::endl;
    
    // 创建模拟组件
    MockSystemMonitor system_monitor;
    MockFaultDetector fault_detector;
    MockFaultToleranceManager fault_manager;
    ConsoleLogger logger;
    
    // 初始化组件
    system_monitor.initialize();
    fault_detector.initialize();
    fault_manager.initialize();
    
    // 创建自运营系统
    SelfOperatingSystem self_op_system(&fault_detector, &fault_manager, &system_monitor, &logger);
    
    // 初始化并启动自运营系统
    if (!self_op_system.initialize()) {
        std::cout << "自运营系统初始化失败！" << std::endl;
        return;
    }
    
    self_op_system.start();
    
    // 创建自动修复管理器
    AutoRepairManager repair_manager(&self_op_system);
    repair_manager.initialize();
    
    // 创建无人模式控制器
    UnmannedModeController unmanned_controller(&self_op_system);
    unmanned_controller.initialize();
    
    // 设置紧急联系人
    unmanned_controller.setEmergencyContacts({"admin@example.com", "oncall@example.com"});
    
    std::cout << "\n✅ 系统初始化完成，进入正常运行状态" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 演示1: 系统状态监控
    std::cout << "\n📊 演示1: 系统状态监控" << std::endl;
    auto state = system_monitor.getSystemState();
    std::cout << "  CPU使用率: " << state.cpu.total_usage << "%" << std::endl;
    std::cout << "  内存使用: " << (state.memory.used_bytes / (1024.0 * 1024.0 * 1024.0)) << " GB / " 
              << (state.memory.total_bytes / (1024.0 * 1024.0 * 1024.0)) << " GB" << std::endl;
    std::cout << "  网络延迟: " << state.network.latency_us << " μs" << std::endl;
    std::cout << "  订单速率: " << state.application.orders_per_second << " 订单/秒" << std::endl;
    
    // 诊断报告
    auto report = self_op_system.getDiagnosticReport();
    std::cout << "  系统健康得分: " << report.health_score << std::endl;
    std::cout << "  系统状态: " << (report.system_healthy ? "健康" : "异常") << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 演示2: 启用自动修复
    std::cout << "\n🛠️ 演示2: 启用自动修复功能" << std::endl;
    self_op_system.enableAutoRepair(true);
    std::cout << "  自动修复功能已启用" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 演示3: 模拟故障并自动修复
    std::cout << "\n🔄 演示3: 模拟故障并进行自动修复" << std::endl;
    std::cout << "  模拟内存泄漏故障..." << std::endl;
    fault_detector.setSimulateFault(true);
    
    // 触发系统检查
    self_op_system.triggerSystemCheck();
    
    // 等待自动修复
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 关闭故障模拟
    fault_detector.setSimulateFault(false);
    
    // 演示4: 切换到无人值守模式
    std::cout << "\n🤖 演示4: 切换到无人值守模式" << std::endl;
    if (unmanned_controller.enableUnmannedMode()) {
        std::cout << "  成功切换到无人值守模式" << std::endl;
        std::cout << "  当前运行模式: " << (self_op_system.getOperatingMode() == SelfOperatingSystem::OperatingMode::FULLY_AUTOMATED ? "完全自动" : "其他模式") << std::endl;
    } else {
        std::cout << "  无法切换到无人值守模式，系统状况不满足要求" << std::endl;
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 演示5: 退出无人值守模式
    std::cout << "\n👨‍💻 演示5: 退出无人值守模式" << std::endl;
    unmanned_controller.disableUnmannedMode();
    std::cout << "  已退出无人值守模式" << std::endl;
    std::cout << "  当前运行模式: " << (self_op_system.getOperatingMode() == SelfOperatingSystem::OperatingMode::SUPERVISED ? "有人监督" : "其他模式") << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 停止自运营系统
    std::cout << "\n⏹️ 停止自运营系统..." << std::endl;
    self_op_system.stop();
    
    std::cout << "\n✅ 演示完成！" << std::endl;
}

int main() {
    demonstrateSelfOperatingSystem();
    return 0;
}
