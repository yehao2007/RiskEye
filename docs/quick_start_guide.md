# Quick Start Guide - RiskEye

<div align="center">
<img src="../resources/quick_start_logo.png" alt="RiskEye Quick Start" width="150"/>

**Version: v3.5.0 | Last Updated: 2025-08-16**
</div>

## 🚀 Introduction

Welcome to RiskEye (High-Frequency Trading) system! This guide will help you get started quickly, understand the basic functions and operation process of the system. RiskEye is a high-performance trading platform that integrates global top-tier risk prediction, intelligent trade execution, and self-operating maintenance.

## ✅ System Requirements

### Minimum Configuration
- **Operating System**: Windows 10 64-bit Professional / Linux Ubuntu 20.04+
- **CPU**: Intel i7-10700K or AMD Ryzen 7 5800X (8+ cores)
- **Memory**: 32GB DDR4
- **Storage**: 512GB SSD
- **Network**: 1Gbps Ethernet
- **Display**: 1920x1080 resolution

### Recommended Configuration
- **Operating System**: Windows 11 Professional / Linux Ubuntu 22.04
- **CPU**: Intel Xeon Gold 6338 or AMD EPYC 7763 (32+ cores)
- **Memory**: 128GB+ DDR4-3200 ECC
- **Storage**: 2TB+ NVMe SSD (read/write speeds >7000MB/s)
- **Network**: 10GbE or InfiniBand network adapter
- **GPU**: NVIDIA A100 or RTX 4090 (for AI acceleration)
- **Display**: Dual 4K monitors

## 📥 Installation Steps

### Windows System

1. **Download Installation Package**
   - Visit the [download page](https://example.com/download) to get the latest version
   - Select the Windows x64 installation package

2. **Run the Installer**
   ```
   RiskEye_Installer_v3.5.0.exe
   ```

3. **Follow the Installation Wizard**
   - Choose installation location
   - Select components (full selection recommended)
   - Configure initial settings

4. **Launch the Program after Installation**
   - 桌面快捷方式: HFT系统
   - 或从开始菜单启动

### Linux系统

1. **下载安装包**
   ```bash
   wget https://example.com/downloads/hft_system_v3.5.0.tar.gz
   ```

2. **解压文件**
   ```bash
   tar -xzf hft_system_v3.5.0.tar.gz
   cd hft_system
   ```

3. **运行安装脚本**
   ```bash
   sudo ./install.sh
   ```

4. **启动系统**
   ```bash
   ./bin/hft_system
   ```

## 🔧 初始配置

### 配置向导

首次启动系统时，配置向导将自动运行，帮助您完成基本设置：

1. **账户设置**
   - 设置管理员用户名和密码
   - 配置交易账户信息

2. **网络设置**
   - 配置交易所连接
   - 设置数据源
   - 配置网络参数

3. **性能设置**
   - 选择优化模式
   - 配置资源分配
   - 设置日志级别

4. **风险控制**
   - 设置交易限额
   - 配置风险预警阈值
   - 设置自动熔断规则

### 手动配置

完成初始配置后，您可以在`config`目录下编辑配置文件进行更详细的设置：

- `config/config.json` - 主配置文件
- `config/risk_model_config.json` - 风险模型配置
- `config/exchange_config.json` - 交易所连接配置
- `config/strategy_params.json` - 策略参数配置

## 💻 基本操作

### 启动和关闭

**Windows:**
- 启动: 双击桌面快捷方式或运行`bin\hft_system.exe`
- 关闭: 在系统界面点击"安全退出"或按Ctrl+Q

**Linux:**
- 启动: `./bin/hft_system`
- 关闭: 在系统界面点击"安全退出"或按Ctrl+Q

### 界面概览

HFT系统界面分为几个主要区域：

1. **主控制面板** - 顶部工具栏，包含主要功能按钮
2. **市场数据区** - 左侧面板，显示实时市场数据
3. **交易执行区** - 中央面板，用于策略监控和订单管理
4. **风险监控区** - 右侧面板，显示风险指标和预警
5. **系统状态栏** - 底部状态栏，显示系统健康状态和性能指标

### 快捷键

| 功能 | 快捷键 |
|------|-------|
| 启动交易 | F5 |
| 停止交易 | F6 |
| 紧急熔断 | F12 |
| 系统状态面板 | Ctrl+S |
| 风险预测面板 | Ctrl+R |
| 配置面板 | Ctrl+P |
| 日志查看器 | Ctrl+L |
| 全屏模式 | F11 |
| 退出系统 | Ctrl+Q |

## 🚦 快速入门场景

### 场景1: 启动风险监控

1. 启动系统并登录
2. 点击主控制面板的"风险监控"按钮
3. 在风险监控面板选择要监控的市场和资产
4. 点击"启动监控"按钮
5. 观察风险预测指标和预警

风险监控面板将显示：
- 实时风险评分
- 风险趋势图表
- 预警阈值和当前状态
- 关键风险因子分析

### 场景2: 执行简单交易策略

1. 点击主控制面板的"策略管理"按钮
2. 从策略库中选择"基础动量策略"
3. 设置交易参数:
   - 交易品种
   - 仓位大小
   - 止损水平
   - 风险限制
4. 点击"部署策略"按钮
5. 确认策略参数并点击"启动"

您可以在交易执行区监控策略运行情况:
- 策略状态和性能指标
- 当前持仓和盈亏
- 近期交易历史
- 执行延迟统计

### 场景3: 启用自运营系统

1. 点击主控制面板的"系统管理"按钮
2. 选择"自运营系统"选项卡
3. 点击"初始化"按钮
4. 配置自运营参数:
   - 运行模式(推荐初次使用选择"半自动")
   - 自动修复级别
   - 系统检查频率
5. 点击"启动自运营系统"按钮

系统状态栏将显示自运营系统状态:
- 当前运行模式
- 系统健康分数
- 最近检测到的异常
- 最近执行的修复操作

## 📊 基本数据可视化

HFT系统提供多种数据可视化工具:

1. **实时性能指标**
   - 点击主控制面板的"性能监控"按钮
   - 查看CPU/内存/网络使用情况
   - 监控交易延迟和吞吐量

2. **风险热图**
   - 在风险监控区点击"风险热图"
   - 查看不同资产的风险分布
   - 识别高风险区域

3. **策略性能图表**
   - 在交易执行区点击策略名称
   - 查看策略回报曲线
   - 分析关键性能指标

## 🛠️ 故障排除

### 常见问题

1. **系统启动失败**
   - 检查日志文件: `logs/system.log`
   - 确认配置文件格式正确
   - 验证系统要求是否满足

2. **数据连接问题**
   - 检查网络连接状态
   - 验证交易所API密钥是否有效
   - 查看`logs/network.log`获取详细错误信息

3. **性能低下**
   - 检查系统资源使用情况
   - 减少同时运行的策略数量
   - 调整性能配置参数

### 自动诊断

系统提供自动诊断工具:
1. 点击主控制面板的"系统工具"按钮
2. 选择"系统诊断"选项
3. 点击"开始诊断"
4. 查看诊断报告并按照建议操作

### 联系支持

如需进一步帮助:
- 技术支持邮箱: support@example.com
- 在线帮助: https://example.com/support
- 电话支持: +1-234-567-8910 (工作日9:00-18:00)

## 📚 进阶学习

本快速入门指南仅涵盖基本功能。要深入了解系统功能，请参考:

- [完整用户手册](../docs/user_manual.pdf)
- [高级用户指南](../docs/advanced_user_guide.md)
- [风险预测模型详解](../docs/risk_model_guide.pdf)
- [策略开发文档](../docs/strategy_development.md)
- [API参考](../docs/api_reference/index.html)

## 📝 版本说明

v3.5.0版本新增功能:
- ✨ 自运营系统，支持自动修复和无人值守模式
- 🔄 升级全球风险预测大模型，提高预测精度和速度
- 🛠️ 优化内存管理，减少GC暂停
- 📊 增强可视化分析工具

查看完整[版本历史](../docs/version_history.md)

---

<div align="center">
<p>感谢选择HFT系统，祝您交易顺利！</p>
<p>© 2025 HFT系统团队 保留所有权利</p>
</div>
