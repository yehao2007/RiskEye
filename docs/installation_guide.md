# RiskEye Installation Guide

<div align="center">
<img src="../resources/installation_logo.png" alt="RiskEye Installation" width="150"/>

**Version: v3.5.0 | Last Updated: 2025-08-16**
</div>

## Table of Contents

- [System Requirements](#system-requirements)
- [Windows Installation](#windows-installation)
- [Linux Installation](#linux-installation)
- [Network Configuration](#network-configuration)
- [Hardware Optimization](#hardware-optimization)
- [Performance Testing](#performance-testing)
- [Multi-node Deployment](#multi-node-deployment)
- [Cloud Deployment](#cloud-deployment)
- [Troubleshooting](#troubleshooting)
- [Upgrade Guide](#upgrade-guide)

## System Requirements

### Basic Environment

| Component | Minimum Requirements | Recommended Configuration | Enterprise Configuration |
|------|---------|---------|-----------|
| Operating System | Windows 10 Pro 64-bit<br>Ubuntu 20.04 LTS | Windows 11 Pro<br>Ubuntu 22.04 LTS | Windows Server 2022<br>RHEL 8.6 |
| CPU | 8 cores (Intel i7/AMD Ryzen 7) | 32 cores (Xeon/EPYC) | 64+ cores (multi-socket Xeon/EPYC) |
| Memory | 32GB DDR4 | 128GB DDR4 ECC | 256GB+ DDR4 ECC |
| Storage | 512GB NVMe SSD | 2TB NVMe SSD (7000MB/s+) | 4TB+ NVMe RAID array |
| Network | 1Gbps Ethernet | 10Gbps Ethernet | 40Gbps+ InfiniBand |
| GPU (optional) | NVIDIA GTX 3080 | NVIDIA A100 | Multiple A100/H100 |
| FPGA (optional) | - | Xilinx Alveo U250 | Multiple Alveo U280 |

### Software Dependencies

- **Runtime Environment**: 
  - Microsoft Visual C++ Redistributable 2022
  - .NET Framework 6.0+
  - Python 3.10+
  - CUDA 12.0+ (GPU acceleration)

- **Library Dependencies**:
  - Boost 1.80+
  - Intel MKL
  - TensorRT 8.5+
  - PyTorch 2.0+
  - libtorch 2.0+
  - OpenSSL 3.0+
  - gRPC 1.50+
  
- **数据库**:
  - InfluxDB 2.6+ (时序数据)
  - PostgreSQL 15+ (关系数据)
  - Redis 7.0+ (缓存)

## Windows系统安装

### 安装步骤

1. **准备环境**

   确保系统已安装最新更新，并安装必要的前置软件:
   
   ```powershell
   # 使用管理员权限打开PowerShell
   # 安装Chocolatey包管理器
   Set-ExecutionPolicy Bypass -Scope Process -Force
   [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
   iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
   
   # 安装依赖
   choco install vcredist-all -y
   choco install dotnet-6.0-sdk -y
   choco install python310 -y
   choco install git -y
   choco install visualstudio2022buildtools -y
   ```

2. **下载安装包**

   从官方网站下载最新的安装包:
   ```
   https://example.com/downloads/HFT_System_Windows_v3.5.0.exe
   ```

3. **运行安装程序**

   以管理员身份运行安装程序:
   ```
   HFT_System_Windows_v3.5.0.exe
   ```

4. **安装选项**

   在安装向导中选择以下选项:
   - 安装位置 (推荐: `C:\Program Files\HFT_System`)
   - 组件选择:
     - 核心系统
     - 风险预测模型
     - 自运营系统
     - 开发工具包 (可选)
     - 示例策略 (推荐首次安装)
     - 文档
   - 启动选项:
     - 创建桌面快捷方式
     - 添加到开始菜单
     - 设置环境变量

5. **完成安装**

   安装完成后，系统将提示重启计算机。重启后，HFT系统将准备就绪。

### 配置性能优化

安装完成后，运行性能优化工具:

```
C:\Program Files\HFT_System\bin\hft_performance_optimizer.exe
```

该工具将自动:
- 调整Windows电源设置为高性能模式
- 禁用不必要的服务和功能
- 优化网络设置
- 设置进程优先级
- 配置内存分配

### 验证安装

运行系统诊断工具:

```
C:\Program Files\HFT_System\bin\hft_system_diagnostic.exe
```

成功安装后，所有检查项应显示为"通过"。

## Linux系统安装

### 安装步骤

1. **准备环境**

   ```bash
   # 更新系统
   sudo apt-get update && sudo apt-get upgrade -y
   
   # 安装依赖
   sudo apt-get install -y build-essential cmake git python3-dev python3-pip libboost-all-dev libssl-dev
   
   # 安装CUDA (如果使用GPU加速)
   wget https://developer.download.nvidia.com/compute/cuda/12.0.0/local_installers/cuda_12.0.0_525.60.13_linux.run
   sudo sh cuda_12.0.0_525.60.13_linux.run
   
   # 安装数据库
   sudo apt-get install -y postgresql redis-server
   wget https://dl.influxdata.com/influxdb/releases/influxdb2-2.6.1-amd64.deb
   sudo dpkg -i influxdb2-2.6.1-amd64.deb
   sudo systemctl enable influxdb
   sudo systemctl start influxdb
   ```

2. **下载安装包**

   ```bash
   wget https://example.com/downloads/hft_system_linux_v3.5.0.tar.gz
   tar -xzf hft_system_linux_v3.5.0.tar.gz
   cd hft_system
   ```

3. **运行安装脚本**

   ```bash
   sudo ./install.sh
   ```

4. **安装选项**

   安装脚本将提示选择:
   - 安装位置 (默认: `/opt/hft_system`)
   - 数据存储位置 (默认: `/var/lib/hft_system`)
   - 日志存储位置 (默认: `/var/log/hft_system`)
   - 配置文件位置 (默认: `/etc/hft_system`)
   - 要安装的组件
   - 系统服务配置

5. **完成安装**

   安装完成后，系统将提示设置初始管理员密码。

### 系统服务配置

安装脚本会创建并启用系统服务:

```bash
# 查看服务状态
sudo systemctl status hft-system

# 启动服务
sudo systemctl start hft-system

# 设置开机自启
sudo systemctl enable hft-system
```

### 验证安装

运行系统诊断工具:

```bash
/opt/hft_system/bin/hft_system_diagnostic
```

成功安装后，所有检查项应显示为"通过"。

## 网络配置

### 基本网络要求

- **延迟**: 交易所连接路径延迟<10ms
- **带宽**: 最低100Mbps，推荐1Gbps+
- **连接**: 冗余连接，自动故障转移
- **安全**: 硬件防火墙，VPN访问

### 专线配置

对于专业交易需求，建议配置:

1. **交易所专线**
   - 直连交易所托管区
   - 延迟<1ms
   - 专用硬件加速器

2. **网络设备配置**
   - 启用巨型帧 (Jumbo frames)
   - 配置QoS优先级
   - 禁用不必要的网络服务

3. **防火墙设置**

   配置防火墙以允许以下端口:
   
   | 服务 | 端口 | 协议 |
   |-----|-----|-----|
   | API服务 | 8080 | TCP |
   | 市场数据 | 9001 | TCP/UDP |
   | 管理界面 | 8443 | HTTPS |
   | 集群通信 | 7000-7010 | TCP |
   | 监控服务 | 9090 | HTTP |

### 配置示例

**Linux网络优化**:

```bash
# 创建网络优化配置文件
sudo nano /etc/sysctl.d/99-trading-network.conf

# 添加以下配置
net.core.rmem_max=16777216
net.core.wmem_max=16777216
net.ipv4.tcp_rmem=4096 87380 16777216
net.ipv4.tcp_wmem=4096 65536 16777216
net.ipv4.tcp_no_metrics_save=1
net.ipv4.tcp_low_latency=1
net.ipv4.tcp_fastopen=3
net.core.netdev_max_backlog=5000

# 应用配置
sudo sysctl -p /etc/sysctl.d/99-trading-network.conf
```

**Windows网络优化**:

使用系统提供的网络优化工具:

```
C:\Program Files\HFT_System\bin\hft_network_optimizer.exe
```

## 硬件优化

### CPU优化

1. **BIOS设置**
   - 禁用CPU节能功能
   - 启用高性能模式
   - 禁用超线程 (对延迟敏感的应用)
   - 禁用不必要的设备和功能

2. **CPU亲和性配置**

   为关键进程分配专用CPU核心:
   
   ```bash
   # Linux系统
   sudo taskset -c 0-3 /opt/hft_system/bin/hft_core
   sudo taskset -c 4-7 /opt/hft_system/bin/hft_risk_model
   sudo taskset -c 8-11 /opt/hft_system/bin/hft_execution
   ```

   ```powershell
   # Windows系统
   # 使用图形界面工具设置CPU亲和性
   C:\Program Files\HFT_System\bin\hft_process_affinity.exe
   ```

### 内存优化

1. **大页面配置**

   ```bash
   # Linux系统
   sudo nano /etc/default/grub
   # 添加或修改: GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=16"
   sudo update-grub
   sudo reboot
   ```

2. **内存锁定**

   防止内存交换到磁盘:
   
   ```bash
   # 修改配置文件
   sudo nano /etc/security/limits.conf
   # 添加以下行
   hft_user soft memlock unlimited
   hft_user hard memlock unlimited
   ```

### 存储优化

1. **磁盘调度器配置**

   ```bash
   # Linux系统
   echo noop > /sys/block/nvme0n1/queue/scheduler
   ```

2. **文件系统优化**

   ```bash
   # 使用noatime挂载选项
   sudo nano /etc/fstab
   # 修改挂载选项,添加noatime
   UUID=xxx /opt/hft_system ext4 defaults,noatime 0 1
   ```

3. **日志存储分离**

   将日志存储到独立的物理磁盘:
   
   ```bash
   # 配置日志路径
   sudo mkdir -p /mnt/logs/hft_system
   sudo chown -R hft_user:hft_group /mnt/logs/hft_system
   
   # 修改配置
   sudo nano /etc/hft_system/logging.json
   # 设置日志路径为 /mnt/logs/hft_system
   ```

## 性能测试

安装完成后，建议运行性能测试以验证系统配置是否满足要求。

### 基准测试工具

```bash
# Linux系统
/opt/hft_system/bin/hft_benchmark

# Windows系统
C:\Program Files\HFT_System\bin\hft_benchmark.exe
```

### 关键性能指标

测试将评估以下关键指标:

1. **订单处理延迟**
   - 目标: <10μs (第99百分位)
   - 测试方法: 高频订单处理基准测试

2. **风险模型性能**
   - 目标: <50μs (第95百分位)
   - 测试方法: 模拟市场数据流的风险评估

3. **数据处理吞吐量**
   - 目标: >1,000,000条/秒
   - 测试方法: 批量市场数据处理

4. **内存分配性能**
   - 目标: <1μs (第99百分位)
   - 测试方法: 内存池分配基准测试

### 性能报告

测试完成后，系统将生成详细的性能报告:

```
/var/log/hft_system/benchmark_results.html
```

或

```
C:\Program Files\HFT_System\logs\benchmark_results.html
```

## 多节点部署

### 集群架构

HFT系统支持多节点部署，典型的集群架构包括:

1. **市场数据节点**
   - 负责接收和处理原始市场数据
   - 低延迟网络连接到交易所
   - 运行数据标准化和初步处理

2. **风险分析节点**
   - 运行风险预测模型
   - 高计算能力(GPU/FPGA加速)
   - 分析多维度风险因素

3. **策略执行节点**
   - 运行交易策略和执行逻辑
   - 低延迟连接到交易网关
   - 专注于订单管理和执行

4. **存储和分析节点**
   - 管理历史数据和分析
   - 高存储容量
   - 运行报告和回测服务

### 集群配置

1. **安装主节点**

   按照标准安装步骤完成主节点安装，然后:
   
   ```bash
   # 生成集群配置
   /opt/hft_system/bin/hft_cluster_config --generate-master
   
   # 配置主节点角色
   /opt/hft_system/bin/hft_node_config --role master
   ```

2. **安装从节点**

   在每个从节点上完成标准安装，然后:
   
   ```bash
   # 加入集群
   /opt/hft_system/bin/hft_cluster_config --join-cluster master_ip:7000
   
   # 配置节点角色
   /opt/hft_system/bin/hft_node_config --role market_data
   # 或
   /opt/hft_system/bin/hft_node_config --role risk_analysis
   # 或
   /opt/hft_system/bin/hft_node_config --role execution
   # 或
   /opt/hft_system/bin/hft_node_config --role storage
   ```

3. **验证集群状态**

   ```bash
   /opt/hft_system/bin/hft_cluster_status
   ```

### 集群管理

使用集群管理工具进行日常管理:

```bash
# 集群健康状态检查
/opt/hft_system/bin/hft_cluster_health

# 节点管理
/opt/hft_system/bin/hft_node_manager --list
/opt/hft_system/bin/hft_node_manager --add node_ip role
/opt/hft_system/bin/hft_node_manager --remove node_id

# 负载均衡
/opt/hft_system/bin/hft_load_balancer --auto-balance
```

## 云部署

HFT系统支持在云环境中部署，以下是主要云平台的部署指南。

### AWS部署

1. **推荐实例类型**
   - 计算优化: c6gn.16xlarge (交易执行)
   - 加速计算: p4d.24xlarge (风险模型)
   - 内存优化: r6g.16xlarge (数据处理)

2. **网络配置**
   - 启用增强网络 (ENA)
   - 启用弹性结构适配器 (EFA)
   - 使用置放群组(集群策略)
   - 配置专用主机

3. **存储配置**
   - 使用 io2 Block Express 卷
   - 配置 RAID 0 阵列提高吞吐量
   - 考虑使用 Amazon EFS 共享数据

4. **部署脚本**

   使用提供的AWS CloudFormation模板:
   ```
   /opt/hft_system/cloud/aws/cloudformation.yaml
   ```

### Azure部署

1. **推荐虚拟机类型**
   - HBv3-120 (计算优化)
   - NCv4 (GPU加速)
   - FSv2 (内存优化)

2. **网络配置**
   - 启用加速网络
   - 配置近程放置组
   - 使用 ExpressRoute 专线

3. **部署模板**

   使用提供的ARM模板:
   ```
   /opt/hft_system/cloud/azure/arm_template.json
   ```

### 专有云注意事项

对于私有云环境，请考虑:

1. **硬件选择**
   - 低延迟网络互连 (InfiniBand或RDMA)
   - 高时钟频率CPU
   - NVMe存储阵列

2. **虚拟化优化**
   - 使用裸机服务器或轻量级虚拟化
   - CPU核心独占分配
   - 启用直接设备分配 (SR-IOV)

## 故障排除

### 常见安装问题

1. **依赖冲突**

   问题: 安装过程中报告库版本冲突
   
   解决方案:
   ```bash
   # 清理旧依赖
   sudo apt-get autoremove
   # 重新安装依赖
   sudo apt-get install -y --reinstall libboost-all-dev
   ```

2. **权限问题**

   问题: 无法写入配置或日志目录
   
   解决方案:
   ```bash
   # 检查并修复权限
   sudo chown -R hft_user:hft_group /opt/hft_system
   sudo chown -R hft_user:hft_group /var/log/hft_system
   sudo chown -R hft_user:hft_group /etc/hft_system
   ```

3. **网络配置问题**

   问题: 系统无法连接到市场数据或交易API
   
   解决方案:
   ```bash
   # 检查网络连接
   ping exchange.example.com
   # 检查防火墙配置
   sudo iptables -L
   # 测试API连接
   /opt/hft_system/bin/hft_api_test --exchange=example
   ```

### 日志分析

安装问题的详细信息通常记录在日志文件中:

```bash
# 安装日志
cat /var/log/hft_system/install.log

# 系统启动日志
cat /var/log/hft_system/system_startup.log

# 错误日志
cat /var/log/hft_system/error.log
```

### 诊断工具

系统提供多个诊断工具:

```bash
# 系统环境检查
/opt/hft_system/bin/hft_environment_check

# 依赖验证
/opt/hft_system/bin/hft_dependency_check

# 网络诊断
/opt/hft_system/bin/hft_network_diagnostic

# 性能瓶颈分析
/opt/hft_system/bin/hft_performance_analyzer
```

## 升级指南

### 升级准备

1. **备份数据和配置**

   ```bash
   # 备份配置
   sudo tar -czf /backup/hft_config_$(date +%Y%m%d).tar.gz /etc/hft_system
   
   # 备份数据
   sudo tar -czf /backup/hft_data_$(date +%Y%m%d).tar.gz /var/lib/hft_system
   
   # 备份日志
   sudo tar -czf /backup/hft_logs_$(date +%Y%m%d).tar.gz /var/log/hft_system
   ```

2. **检查兼容性**

   使用兼容性检查工具:
   
   ```bash
   /opt/hft_system/bin/hft_upgrade_check --target-version=3.5.0
   ```

### 执行升级

1. **下载升级包**

   ```bash
   wget https://example.com/downloads/hft_system_upgrade_v3.5.0.tar.gz
   tar -xzf hft_system_upgrade_v3.5.0.tar.gz
   cd hft_system_upgrade
   ```

2. **运行升级脚本**

   ```bash
   sudo ./upgrade.sh
   ```

3. **验证升级**

   ```bash
   # 检查版本
   /opt/hft_system/bin/hft_system --version
   
   # 运行诊断
   /opt/hft_system/bin/hft_system_diagnostic
   ```

### 回滚流程

如果升级失败，可以执行回滚:

```bash
# 执行回滚
sudo /opt/hft_system/bin/hft_rollback --to-version=3.4.0

# 验证回滚
/opt/hft_system/bin/hft_system --version
```

---

<div align="center">
<p>© 2025 HFT系统团队 保留所有权利</p>
<p>版本: v3.5.0 | 更新日期: 2025-08-16</p>
</div>
