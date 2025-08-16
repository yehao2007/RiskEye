#pragma once

#include "DiagnosticCore.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

namespace hft {
namespace diagnostics {

using json = nlohmann::json;
using websocket_server = websocketpp::server<websocketpp::config::asio>;
using connection_hdl = websocketpp::connection_hdl;

// 可视化配置
struct VisualizationConfig {
    std::string theme{"light"};           // 主题 (light/dark)
    std::string color_scheme{"default"};  // 配色方案
    int refresh_interval{5000};           // 刷新间隔(ms)
    bool enable_animation{true};          // 启用动画
    std::string date_format{"%Y-%m-%d %H:%M:%S"};  // 日期格式
    int chart_width{800};                 // 图表宽度
    int chart_height{400};                // 图表高度
    bool enable_realtime{true};           // 启用实时更新
};

// 远程监控配置
struct RemoteMonitorConfig {
    std::string host{"0.0.0.0"};         // 监听地址
    uint16_t port{8080};                  // 监听端口
    std::string auth_token;               // 认证令牌
    bool enable_ssl{false};               // 启用SSL
    std::string ssl_cert;                 // SSL证书路径
    std::string ssl_key;                  // SSL私钥路径
    int max_connections{100};             // 最大连接数
    bool enable_compression{true};        // 启用压缩
    std::chrono::seconds heartbeat_interval{30};  // 心跳间隔
};

// 监控统计
struct RemoteMonitorStats {
    uint32_t active_connections{0};       // 活动连接数
    uint64_t messages_sent{0};           // 发送消息数
    uint64_t messages_received{0};       // 接收消息数
    double avg_message_size{0.0};        // 平均消息大小
    std::chrono::system_clock::time_point start_time;  // 启动时间
    std::map<std::string, uint32_t> client_stats;  // 客户端统计
};

// 消息类型
enum class MessageType {
    SystemState = 1,      // 系统状态
    DiagnosticReport,     // 诊断报告
    Alert,                // 告警消息
    Command,              // 控制命令
    Response,             // 响应消息
    Heartbeat,            // 心跳消息
    PerformanceTrend,     // 性能趋势
    ChartData             // 图表数据
};

// 远程监控消息
struct RemoteMessage {
    MessageType type;                     // 消息类型
    json payload;                         // 消息内容
    std::string source;                   // 消息源
    std::string target;                   // 目标客户端
    std::chrono::system_clock::time_point timestamp;  // 时间戳
    std::string correlation_id;           // 关联ID
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
        const std::string& y_label = "") = 0;
        
    virtual std::string generateHistogram(
        const std::vector<double>& data,
        const std::string& title = "",
        int bins = 20) = 0;
        
    virtual std::string generateScatterPlot(
        const std::vector<double>& x_data,
        const std::vector<double>& y_data,
        const std::string& title = "") = 0;
        
    virtual std::string generateHeatMap(
        const std::vector<std::vector<double>>& data,
        const std::string& title = "") = 0;
        
    virtual std::string generateGaugeChart(
        double value,
        double min_val,
        double max_val,
        const std::string& label = "") = 0;
        
    virtual std::string generateBarChart(
        const std::vector<std::string>& labels,
        const std::vector<double>& values,
        const std::string& title = "") = 0;
        
    // 仪表板生成
    virtual std::string generateDashboard(
        const std::vector<std::string>& charts,
        int columns = 2) = 0;
        
    // 实时图表
    virtual std::string generateRealtimeChart(
        const std::string& chart_id,
        const std::string& metric_name) = 0;
};

// 基于Chart.js的可视化实现
class ChartJSVisualizer : public IDataVisualizer {
public:
    explicit ChartJSVisualizer(const VisualizationConfig& config);
    
    std::string generateTimeSeriesChart(
        const std::vector<double>& data,
        const std::vector<std::chrono::system_clock::time_point>& timestamps,
        const std::string& title = "",
        const std::string& y_label = "") override;
        
    std::string generateHistogram(
        const std::vector<double>& data,
        const std::string& title = "",
        int bins = 20) override;
        
    std::string generateScatterPlot(
        const std::vector<double>& x_data,
        const std::vector<double>& y_data,
        const std::string& title = "") override;
        
    std::string generateHeatMap(
        const std::vector<std::vector<double>>& data,
        const std::string& title = "") override;
        
    std::string generateGaugeChart(
        double value,
        double min_val,
        double max_val,
        const std::string& label = "") override;
        
    std::string generateBarChart(
        const std::vector<std::string>& labels,
        const std::vector<double>& values,
        const std::string& title = "") override;
        
    std::string generateDashboard(
        const std::vector<std::string>& charts,
        int columns = 2) override;
        
    std::string generateRealtimeChart(
        const std::string& chart_id,
        const std::string& metric_name) override;

private:
    VisualizationConfig config_;
    
    std::string generateChartJS(const json& chart_config);
    std::string getColorScheme();
    std::string formatTimestamp(const std::chrono::system_clock::time_point& tp);
};

// 远程监控客户端接口
class IRemoteClient {
public:
    virtual ~IRemoteClient() = default;
    virtual void onConnect() = 0;
    virtual void onDisconnect() = 0;
    virtual void onMessage(const RemoteMessage& message) = 0;
    virtual void onError(const std::string& error) = 0;
    virtual std::string getClientId() const = 0;
    virtual bool isAuthenticated() const = 0;
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

    // 消息处理
    void broadcast(const RemoteMessage& message);
    void sendToClient(const std::string& client_id, const RemoteMessage& message);
    void broadcastToAuthenticated(const RemoteMessage& message);

    // 客户端管理
    void registerClient(connection_hdl hdl, std::shared_ptr<IRemoteClient> client);
    void unregisterClient(connection_hdl hdl);
    std::vector<std::string> getConnectedClients() const;
    
    // 统计信息
    RemoteMonitorStats getStats() const;
    void resetStats();

private:
    RemoteMonitorConfig config_;
    RemoteMonitorStats stats_;
    std::unique_ptr<websocket_server> server_;
    std::map<connection_hdl, std::shared_ptr<IRemoteClient>, 
             std::owner_less<connection_hdl>> clients_;
    std::map<std::string, connection_hdl> client_handles_;
    std::atomic<bool> running_{false};
    std::thread server_thread_;
    mutable std::mutex clients_mutex_;

    // WebSocket事件处理
    void onOpen(connection_hdl hdl);
    void onClose(connection_hdl hdl);
    void onMessage(connection_hdl hdl, websocket_server::message_ptr msg);
    void onError(connection_hdl hdl);
    
    // 认证处理
    bool authenticate(const std::string& token);
    
    // 心跳处理
    void startHeartbeat();
    void sendHeartbeat();
    
    // 内部方法
    void serverLoop();
    std::string serializeMessage(const RemoteMessage& message);
    RemoteMessage deserializeMessage(const std::string& data);
};

// 实时数据流管理器
class RealtimeDataStreamer {
public:
    RealtimeDataStreamer(std::shared_ptr<RemoteMonitorServer> server);
    ~RealtimeDataStreamer();

    // 数据流控制
    void startStreaming();
    void stopStreaming();
    bool isStreaming() const { return streaming_; }

    // 数据推送
    void pushSystemState(const SystemState& state);
    void pushPerformanceTrend(const PerformanceTrend& trend);
    void pushDiagnosticReport(const DiagnosticReport& report);
    void pushAlert(const DiagnosticIssue& issue);

    // 订阅管理
    void subscribeClient(const std::string& client_id, const std::string& metric);
    void unsubscribeClient(const std::string& client_id, const std::string& metric);
    std::vector<std::string> getSubscriptions(const std::string& client_id) const;

private:
    std::shared_ptr<RemoteMonitorServer> server_;
    std::atomic<bool> streaming_{false};
    std::thread streaming_thread_;
    std::map<std::string, std::vector<std::string>> client_subscriptions_;
    std::queue<RemoteMessage> message_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // 内部方法
    void streamingLoop();
    void processMessageQueue();
    bool shouldSendToClient(const std::string& client_id, const RemoteMessage& message);
};

// 数据源接口
class IDiagnosticDataSource {
public:
    virtual ~IDiagnosticDataSource() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual json getData() const = 0;
    virtual std::vector<std::string> getMetrics() const = 0;
    virtual void configure(const std::map<std::string, std::string>& params) = 0;
    virtual bool isActive() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

} // namespace diagnostics
} // namespace hft
