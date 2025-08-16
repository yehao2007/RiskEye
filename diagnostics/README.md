# HFT 诊断系统

一个全面的高频交易系统诊断和监控解决方案，提供实时监控、故障检测、性能分析、可视化仪表板和远程监控功能。

## 🚀 主要功能

### 核心诊断功能
- **实时系统监控** - CPU、内存、磁盘、网络资源监控
- **性能分析** - 延迟、吞吐量、成功率分析
- **故障检测** - 自动异常检测和故障预测
- **日志分析** - 智能日志模式识别和错误分类
- **硬件健康监控** - 温度、频率、健康状态监控

### 高级功能
- **插件系统** - 可扩展的诊断规则和监控插件
- **自定义规则** - 灵活的诊断规则配置
- **自动修复** - 智能故障修复策略
- **趋势预测** - 基于历史数据的性能趋势分析
- **SLA监控** - 服务级别协议合规性检查

### 告警和通知
- **多种告警方式** - 邮件、短信、Webhook、控制台
- **告警聚合** - 智能告警合并和频率控制
- **告警规则** - 灵活的阈值和条件配置
- **冷却期管理** - 防止告警风暴

### 可视化和报告
- **实时仪表板** - 基于Chart.js的交互式图表
- **多种图表类型** - 时间序列、直方图、散点图、热力图、仪表盘
- **主题支持** - 明暗主题和多种配色方案
- **报告导出** - JSON、HTML、CSV、XML格式

### 远程监控
- **WebSocket API** - 实时数据推送
- **客户端认证** - 基于令牌的安全认证
- **数据压缩** - 减少网络带宽占用
- **心跳检测** - 连接状态监控

## 📁 项目结构

```
diagnostics/
├── DiagnosticTool.h              # 基础数据结构定义
├── DiagnosticCore.h/.cpp         # 核心诊断组件
├── DiagnosticPlugin.h            # 插件系统接口
├── PluginManager.cpp             # 插件管理器实现
├── SystemDiagnostics.h/.cpp      # 主诊断系统类
├── VisualizationAndMonitoring.h/.cpp  # 可视化和远程监控
├── EnhancedDiagnostics.h/.cpp    # 增强诊断功能
├── DiagnosticDataSource.h        # 数据源接口
├── HardwareMonitorPlugin.h/.cpp  # 硬件监控插件
├── RemoteMonitor.h/.cpp          # 远程监控服务器
├── example_usage.cpp             # 使用示例
├── CMakeLists.txt               # 构建配置
└── README.md                    # 项目文档
```

## 🛠️ 构建和安装

### 依赖要求

- **C++20** 兼容编译器 (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake** 3.16+
- **nlohmann/json** - JSON处理库
- **WebSocket++** - WebSocket服务器库
- **Chart.js** - 前端图表库 (通过CDN)

### 构建步骤

```bash
# 创建构建目录
mkdir build && cd build

# 配置CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --parallel

# 安装
cmake --install .
```

### Windows 构建

```powershell
# 使用Visual Studio生成器
cmake .. -G "Visual Studio 16 2019" -A x64

# 编译
cmake --build . --config Release

# 安装
cmake --install . --config Release
```

## 🚀 快速开始

### 基本使用

```cpp
#include "SystemDiagnostics.h"

// 1. 创建诊断系统
auto diagnostic_system = DiagnosticSystemFactory::createSystem();

// 2. 配置告警
AlertConfig alert_config;
AlertConfig::AlertRule cpu_rule;
cpu_rule.metric = "cpu_usage";
cpu_rule.threshold = 80.0;
cpu_rule.severity = Severity::Warning;
cpu_rule.methods = {AlertConfig::AlertMethod::Console};
alert_config.rules.push_back(cpu_rule);

diagnostic_system->configureAlerts(alert_config);

// 3. 启动诊断
diagnostic_system->startDiagnostics();

// 4. 生成报告
auto report = diagnostic_system->generateReport();
std::cout << "Health Score: " << diagnostic_system->getSystemHealthScore() << "%" << std::endl;

// 5. 生成仪表板
std::string dashboard = diagnostic_system->generateDashboard();
```

### 远程监控设置

```cpp
// 配置远程监控服务器
RemoteMonitorConfig config{
    .host = "0.0.0.0",
    .port = 8080,
    .auth_token = "your-secret-token",
    .enable_ssl = false,
    .max_connections = 100
};

diagnostic_system->startRemoteMonitor(config);
```

### 添加自定义插件

```cpp
// 创建自定义插件
class MyCustomPlugin : public IDiagnosticPlugin {
    // 实现插件接口
};

auto plugin = std::make_unique<MyCustomPlugin>();
PluginConfig plugin_config{
    .name = "MyPlugin",
    .version = "1.0.0",
    .parameters = {{"threshold", "100"}}
};

diagnostic_system->loadPlugin(std::move(plugin), plugin_config);
```

## 📊 监控指标

### 系统资源
- CPU使用率、温度、频率
- 内存使用率、温度、带宽
- 磁盘使用率、IO速率、温度
- 网络使用率、带宽、丢包率

### 性能指标
- 交易延迟 (平均、95%、99%)
- 系统吞吐量 (TPS)
- 成功率和错误率
- 响应时间分布

### 硬件健康
- 温度监控
- 硬件错误计数
- 坏扇区检测
- 网络接口状态

## 🔧 配置选项

### 系统配置

```cpp
DiagnosticSystemConfig config;
config.update_interval = std::chrono::milliseconds(1000);
config.max_history_size = 10000;
config.enable_auto_repair = true;
config.enable_predictions = true;

// 阈值配置
config.thresholds.latency_warning = 100.0;    // ms
config.thresholds.latency_critical = 500.0;   // ms
config.thresholds.cpu_warning = 80.0;         // %
config.thresholds.cpu_critical = 95.0;        // %
```

### 可视化配置

```cpp
VisualizationConfig viz_config{
    .theme = "dark",                    // light/dark
    .color_scheme = "blue",             // blue/red/green
    .refresh_interval = 2000,           // ms
    .enable_animation = true,
    .chart_width = 900,
    .chart_height = 450
};
```

## 🌐 Web仪表板

系统提供基于Web的实时监控仪表板：

- **实时图表** - 自动刷新的性能图表
- **系统状态** - 当前系统健康状况
- **告警面板** - 活动告警和历史记录
- **趋势分析** - 性能趋势和预测
- **配置管理** - 在线配置修改

访问地址：`http://localhost:8080` (默认端口)

## 🔌 插件开发

### 创建自定义规则

```cpp
auto custom_rule = std::make_unique<CustomRule>(
    "HighLatencyRule",
    "检测高延迟交易",
    [](const SystemState& state) {
        return state.performance.latency > 50.0;  // 50ms阈值
    }
);

diagnostic_system->addCustomRule(std::move(custom_rule));
```

### 创建数据源

```cpp
class MyDataSource : public IDiagnosticDataSource {
public:
    std::string getName() const override { return "MySource"; }
    json getData() const override {
        json data;
        data["custom_metric"] = getCurrentValue();
        return data;
    }
    // ... 其他接口实现
};
```

## 📈 性能优化

### 内存优化
- 循环缓冲区管理历史数据
- 自动清理过期数据
- 内存池减少分配开销

### CPU优化
- 多线程并行处理
- 异步IO操作
- 智能轮询间隔调整

### 网络优化
- 数据压缩减少带宽
- 批量消息发送
- 连接池管理

## 🛡️ 安全特性

- **认证机制** - 基于令牌的客户端认证
- **访问控制** - 角色基础的权限管理
- **数据加密** - 可选的SSL/TLS加密
- **审计日志** - 完整的操作审计轨迹

## 🧪 测试

```bash
# 运行单元测试
ctest

# 运行特定测试
ctest -R DiagnosticCore

# 详细输出
ctest --verbose
```

## 📦 部署

### Docker部署

```dockerfile
FROM ubuntu:20.04
COPY . /app
WORKDIR /app
RUN apt-get update && apt-get install -y cmake g++ nlohmann-json3-dev
RUN mkdir build && cd build && cmake .. && make
EXPOSE 8080
CMD ["./build/diagnostic_example"]
```

### 系统服务

```bash
# 创建systemd服务文件
sudo cp hft-diagnostic.service /etc/systemd/system/
sudo systemctl enable hft-diagnostic
sudo systemctl start hft-diagnostic
```

## 🤝 贡献指南

1. Fork 项目仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 📞 支持

- **文档**: [https://hft-diagnostic.readthedocs.io](https://hft-diagnostic.readthedocs.io)
- **Issues**: [GitHub Issues](https://github.com/hft-system/diagnostic/issues)
- **讨论**: [GitHub Discussions](https://github.com/hft-system/diagnostic/discussions)
- **邮件**: support@hft-system.com

## 🏗️ 版本历史

### v1.0.0 (当前版本)
- ✅ 完整的诊断系统框架
- ✅ 实时监控和告警
- ✅ 插件系统和自定义规则
- ✅ Web仪表板和可视化
- ✅ 远程监控API
- ✅ 多格式报告导出
- ✅ 硬件健康监控
- ✅ 性能趋势分析

### 路线图
- 🔄 机器学习异常检测
- 🔄 分布式监控支持
- 🔄 Kubernetes集成
- 🔄 更多可视化图表类型
- 🔄 移动端应用

---

**注意**: 这是一个专业级的高频交易诊断系统，在生产环境中使用前请进行充分测试。
