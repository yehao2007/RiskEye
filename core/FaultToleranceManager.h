#ifndef FAULT_TOLERANCE_MANAGER_H
#define FAULT_TOLERANCE_MANAGER_H

#include "core/System.h"
#include "core/AlertManager.h"
#include "network/DistributedCommunicator.h"
#include <memory>
#include <vector>
#include <string>

namespace hft {
namespace core {

class FaultToleranceManager {
public:
    FaultToleranceManager(System* system,
                         AlertManager* alert_manager,
                         network::DistributedCommunicator* communicator);

    // 初始化容错系统
    bool initialize();

    // 启动故障检测
    void startFaultDetection();

    // 停止故障检测
    void stopFaultDetection();

    // 处理系统故障
    void handleFault(const std::string& component_name, const std::string& fault_type);

    // 恢复系统组件
    bool recoverComponent(const std::string& component_name);

    // 获取系统健康状态
    float getSystemHealth() const;

private:
    System* system_;
    AlertManager* alert_manager_;
    network::DistributedCommunicator* communicator_;
    std::atomic<bool> is_monitoring_;
    std::thread monitoring_thread_;
    std::vector<std::string> critical_components_;

    // 故障检测循环
    void faultDetectionLoop();

    // 检查组件健康状态
    bool checkComponentHealth(const std::string& component_name);

    // 启动热备份
    void activateHotBackup(const std::string& component_name);
};

} // namespace core
} // namespace hft

#endif // FAULT_TOLERANCE_MANAGER_H