# 自运营系统使用文档

## 🔄 自运营系统概述

自运营系统（Self-Operating System）是HFT交易系统的一个重要组成部分，旨在提供自动检测与修复系统故障和异常的能力，实现系统的自我修复和无人值守运行。

### 核心特性

- 🛠️ **自动检测与修复** - 自动识别系统故障并执行修复操作
- 🧠 **自适应优化** - 根据系统状态自动优化系统配置
- 🔒 **无人值守模式** - 支持完全自动化的无人值守运行
- 📊 **实时健康监控** - 持续监控系统健康状态
- 🔍 **智能根因分析** - 自动分析故障根本原因
- 📈 **系统性能自优化** - 动态调整系统参数以优化性能
- 🔄 **持续自我学习** - 从修复结果中学习并改进修复策略

## 系统架构

自运营系统由以下核心组件组成：

1. **SelfOperatingSystem** - 核心控制器，协调所有自运营功能
2. **AutoRepairManager** - 自动修复管理器，负责执行修复策略
3. **UnmannedModeController** - 无人模式控制器，管理无人值守运行

### 依赖组件

- **FaultDetector** - 故障检测器，负责检测系统故障
- **FaultToleranceManager** - 容错管理器，提供基本容错功能
- **SystemMonitor** - 系统监控器，提供系统状态信息
- **Logger** - 日志系统，记录自运营活动

## 使用指南

### 初始化和配置

```cpp
// 创建自运营系统
SelfOperatingSystem self_op_system(
    fault_detector,    // FaultDetector 实例
    fault_manager,     // FaultToleranceManager 实例
    system_monitor,    // SystemMonitor 实例
    logger             // Logger 实例
);

// 初始化
if (!self_op_system.initialize()) {
    logger->error("自运营系统初始化失败");
    return;
}

// 启动自运营服务
self_op_system.start();

// 启用自动修复
self_op_system.enableAutoRepair(true);

// 创建自动修复管理器
AutoRepairManager repair_manager(&self_op_system);
repair_manager.initialize();

// 创建无人模式控制器
UnmannedModeController unmanned_controller(&self_op_system);
unmanned_controller.initialize();

// 设置紧急联系人
unmanned_controller.setEmergencyContacts({"admin@example.com", "oncall@example.com"});
```

### 运行模式切换

```cpp
// 切换到半自动模式
self_op_system.setOperatingMode(SelfOperatingSystem::OperatingMode::SEMI_AUTOMATED);

// 切换到无人值守模式
if (unmanned_controller.enableUnmannedMode()) {
    logger->info("成功切换到无人值守模式");
} else {
    logger->warning("无法切换到无人值守模式，系统状况不满足要求");
}

// 退出无人值守模式
unmanned_controller.disableUnmannedMode();
```

### 诊断与监控

```cpp
// 手动触发系统检查
self_op_system.triggerSystemCheck();

// 获取诊断报告
auto report = self_op_system.getDiagnosticReport();
if (report.system_healthy) {
    logger->info("系统运行正常，健康得分: " + std::to_string(report.health_score));
} else {
    logger->warning("系统运行异常，健康得分: " + std::to_string(report.health_score));
    logger->warning("活跃异常数量: " + std::to_string(report.active_anomalies.size()));
}

// 查看当前运行模式
auto mode = self_op_system.getOperatingMode();
```

### 停止服务

```cpp
// 停止自运营系统
self_op_system.stop();
```

## 自定义修复策略

自运营系统支持自定义修复策略，以应对特定类型的系统异常：

```cpp
// 注册自定义修复策略
repair_manager.registerRepairStrategy("CPU_HIGH_USAGE", [](const SelfOperatingSystem::SystemAnomaly& anomaly) {
    // 实现CPU高使用率修复逻辑
    return true; // 返回修复是否成功
});
```

## 安全边界配置

无人值守模式下，可以设置系统运行的安全边界：

```cpp
// 设置安全边界
std::unordered_map<std::string, std::pair<double, double>> safety_boundaries;
safety_boundaries["cpu_usage"] = std::make_pair(0.0, 90.0);       // CPU使用率不超过90%
safety_boundaries["memory_usage"] = std::make_pair(0.0, 85.0);     // 内存使用率不超过85%
safety_boundaries["network_latency_us"] = std::make_pair(0.0, 5000.0); // 网络延迟不超过5ms

unmanned_controller.setSafetyBoundaries(safety_boundaries);
```

## 常见问题解答

**Q: 如何确定系统是否适合启用无人值守模式？**

A: 使用 `UnmannedModeController::canOperateAutonomously()` 方法检查系统状态。系统需要满足以下条件：
- 系统健康得分高于0.7
- 所有系统指标在安全边界内
- 自动修复成功率良好

**Q: 如何处理无人值守模式下的紧急情况？**

A: 系统会自动监控安全边界，一旦超出边界，会触发紧急协议：
- 通知预设的紧急联系人
- 自动退出无人值守模式
- 切换到有人监督模式

**Q: 自动修复失败时会发生什么？**

A: 系统会：
- 记录失败原因
- 尝试备选修复策略
- 如果所有自动修复失败且处于无人值守模式，可能触发紧急协议
- 更新修复策略成功率统计

## 演示程序

系统提供了一个完整的演示程序，展示自运营系统的核心功能：

```bash
# 构建和运行演示程序
build_self_operating_demo.bat
```

演示程序展示了以下功能：
1. 系统状态监控
2. 自动修复功能
3. 故障模拟与自动修复
4. 无人值守模式切换
5. 诊断报告生成

## 注意事项

- 首次启用无人值守模式前，建议在有人监督下运行系统一段时间，确保自动修复功能运行良好
- 定期检查诊断报告，了解系统健康状况
- 配置合适的安全边界，避免系统运行超出安全范围
- 始终配置紧急联系人，确保紧急情况能及时通知相关人员

## 后续开发计划

1. 增强AI驱动的根因分析能力
2. 添加更多预定义的修复策略
3. 支持基于历史数据的预测性维护
4. 改进修复策略自动学习能力
5. 提供基于Web的远程监控界面
