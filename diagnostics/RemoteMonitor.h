#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <functional>
#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "nlohmann/json.hpp"

namespace hft {
namespace diagnostics {

using json = nlohmann::json;
using websocketpp::connection_hdl;
using websocket_server = websocketpp::server<websocketpp::config::asio>;

// 远程监控服务器配置
struct RemoteMonitorConfig {
    uint16_t port{8080};                    // 监听端口
    std::string host{"0.0.0.0"};            // 监听地址
    std::string auth_token;                 // 认证令牌
    bool enable_ssl{false};                 // 是否启用SSL
    std::string ssl_cert;                   // SSL证书路径
    std::string ssl_key;                    // SSL密钥路径
    int max_connections{100};               // 最大连接数
    bool enable_compression{true};          // 是否启用压缩
};

// 远程监控服务器状态
struct RemoteMonitorStats {
    uint32_t active_connections{0};         // 活动连接数
    uint64_t messages_sent{0};             // 发送消息数
    uint64_t messages_received{0};          // 接收消息数
    double avg_message_size{0.0};          // 平均消息大小
    std::chrono::system_clock::time_point start_time;  // 启动时间
};

// 远程监控消息类型
enum class MessageType {
    SystemState,           // 系统状态
    DiagnosticReport,     // 诊断报告
    Alert,                // 告警消息
    Command,              // 控制命令
    Response              // 响应消息
};

// 远程监控消息
struct RemoteMessage {
    MessageType type;                      // 消息类型
    json payload;                          // 消息内容
    std::string source;                    // 消息源
    std::chrono::system_clock::time_point timestamp;  // 时间戳
};

// 远程监控客户端接口
class IRemoteClient {
public:
    virtual ~IRemoteClient() = default;
    virtual void onMessage(const RemoteMessage& msg) = 0;
    virtual void onConnect() = 0;
    virtual void onDisconnect() = 0;
    virtual void onError(const std::string& error) = 0;
};

// 远程监控服务器
class RemoteMonitorServer {
public:
    explicit RemoteMonitorServer(const RemoteMonitorConfig& config);
    ~RemoteMonitorServer();

    // 服务器控制
    void start();
    void stop();
    bool isRunning() const { return running_; }

    // 消息广播
    void broadcast(const RemoteMessage& msg);
    void broadcastToAuthenticated(const RemoteMessage& msg);

    // 客户端管理
    void addClient(connection_hdl hdl, std::shared_ptr<IRemoteClient> client);
    void removeClient(connection_hdl hdl);
    
    // 状态查询
    RemoteMonitorStats getStats() const { return stats_; }
    
private:
    // WebSocket事件处理
    void onOpen(connection_hdl hdl);
    void onClose(connection_hdl hdl);
    void onMessage(connection_hdl hdl, websocket_server::message_ptr msg);
    void onError(connection_hdl hdl);

    // 认证处理
    bool authenticate(const std::string& token);

    RemoteMonitorConfig config_;
    RemoteMonitorStats stats_;
    std::unique_ptr<websocket_server> server_;
    std::map<connection_hdl, std::shared_ptr<IRemoteClient>, 
             std::owner_less<connection_hdl>> clients_;
    std::map<connection_hdl, bool, 
             std::owner_less<connection_hdl>> authenticated_clients_;
    bool running_{false};
};

// 数据可视化接口
class IDataVisualizer {
public:
    virtual ~IDataVisualizer() = default;
    
    // 图表生成
    virtual std::string generateTimeSeriesChart(
        const std::vector<double>& data,
        const std::vector<std::chrono::system_clock::time_point>& timestamps,
        const std::string& title = "",
        const std::string& yAxisLabel = "") = 0;
        
    virtual std::string generateHistogram(
        const std::vector<double>& data,
        const std::string& title = "",
        int bins = 10) = 0;
        
    virtual std::string generateHeatMap(
        const std::vector<std::vector<double>>& data,
        const std::string& title = "") = 0;
        
    // 仪表盘生成
    virtual std::string generateGauge(
        double value,
        double min,
        double max,
        const std::string& label = "") = 0;
        
    // 多图表布局
    virtual std::string generateDashboard(
        const std::vector<std::string>& charts,
        int columns = 2) = 0;
};

// 基于D3.js的可视化实现
class D3Visualizer : public IDataVisualizer {
public:
    std::string generateTimeSeriesChart(
        const std::vector<double>& data,
        const std::vector<std::chrono::system_clock::time_point>& timestamps,
        const std::string& title = "",
        const std::string& yAxisLabel = "") override;
        
    std::string generateHistogram(
        const std::vector<double>& data,
        const std::string& title = "",
        int bins = 10) override;
        
    std::string generateHeatMap(
        const std::vector<std::vector<double>>& data,
        const std::string& title = "") override;
        
    std::string generateGauge(
        double value,
        double min,
        double max,
        const std::string& label = "") override;
        
    std::string generateDashboard(
        const std::vector<std::string>& charts,
        int columns = 2) override;
        
private:
    std::string generateD3Script(const json& data,
                                const std::string& chartType);
};

} // namespace diagnostics
} // namespace hft
