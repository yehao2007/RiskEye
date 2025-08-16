#include "RemoteMonitor.h"
#include "../core/Logger.h"
#include <sstream>
#include <iomanip>

namespace hft {
namespace diagnostics {

RemoteMonitorServer::RemoteMonitorServer(const RemoteMonitorConfig& config)
    : config_(config)
    , server_(std::make_unique<websocket_server>()) {
    
    // 初始化WebSocket服务器
    server_->init_asio();
    
    // 设置消息处理回调
    server_->set_open_handler(
        std::bind(&RemoteMonitorServer::onOpen, this, std::placeholders::_1));
    server_->set_close_handler(
        std::bind(&RemoteMonitorServer::onClose, this, std::placeholders::_1));
    server_->set_message_handler(
        std::bind(&RemoteMonitorServer::onMessage, this,
                 std::placeholders::_1,
                 std::placeholders::_2));
                 
    // 设置错误处理回调
    server_->set_error_handler(
        std::bind(&RemoteMonitorServer::onError, this, std::placeholders::_1));
        
    // 初始化统计信息
    stats_.start_time = std::chrono::system_clock::now();
}

RemoteMonitorServer::~RemoteMonitorServer() {
    if (running_) {
        stop();
    }
}

void RemoteMonitorServer::start() {
    if (running_) return;
    
    try {
        // 设置监听地址和端口
        server_->listen(config_.host, config_.port);
        
        // 启动服务器
        server_->start_accept();
        
        // 启动异步运行
        server_->run();
        
        running_ = true;
        Logger::info("Remote monitor server started on {}:{}",
                    config_.host, config_.port);
                    
    } catch (const std::exception& e) {
        Logger::error("Failed to start remote monitor server: {}", e.what());
        throw;
    }
}

void RemoteMonitorServer::stop() {
    if (!running_) return;
    
    try {
        // 关闭所有连接
        server_->stop_listening();
        for (auto& [hdl, _] : clients_) {
            server_->close(hdl, websocketpp::close::status::going_away, 
                          "Server shutdown");
        }
        
        clients_.clear();
        authenticated_clients_.clear();
        running_ = false;
        
        Logger::info("Remote monitor server stopped");
        
    } catch (const std::exception& e) {
        Logger::error("Failed to stop remote monitor server: {}", e.what());
        throw;
    }
}

void RemoteMonitorServer::broadcast(const RemoteMessage& msg) {
    if (!running_) return;
    
    try {
        // 序列化消息
        json j = {
            {"type", static_cast<int>(msg.type)},
            {"payload", msg.payload},
            {"source", msg.source},
            {"timestamp", std::chrono::system_clock::to_time_t(msg.timestamp)}
        };
        
        std::string message = j.dump();
        
        // 广播到所有客户端
        for (auto& [hdl, _] : clients_) {
            server_->send(hdl, message, websocketpp::frame::opcode::text);
        }
        
        // 更新统计信息
        stats_.messages_sent += clients_.size();
        stats_.avg_message_size = 
            (stats_.avg_message_size * (stats_.messages_sent - clients_.size()) +
             message.size() * clients_.size()) / stats_.messages_sent;
             
    } catch (const std::exception& e) {
        Logger::error("Failed to broadcast message: {}", e.what());
        throw;
    }
}

void RemoteMonitorServer::broadcastToAuthenticated(const RemoteMessage& msg) {
    if (!running_) return;
    
    try {
        // 序列化消息
        json j = {
            {"type", static_cast<int>(msg.type)},
            {"payload", msg.payload},
            {"source", msg.source},
            {"timestamp", std::chrono::system_clock::to_time_t(msg.timestamp)}
        };
        
        std::string message = j.dump();
        
        // 只广播到已认证的客户端
        for (auto& [hdl, authenticated] : authenticated_clients_) {
            if (authenticated) {
                server_->send(hdl, message, websocketpp::frame::opcode::text);
                stats_.messages_sent++;
            }
        }
        
    } catch (const std::exception& e) {
        Logger::error("Failed to broadcast to authenticated clients: {}", 
                     e.what());
        throw;
    }
}

void RemoteMonitorServer::onOpen(connection_hdl hdl) {
    Logger::info("New client connected");
    stats_.active_connections++;
}

void RemoteMonitorServer::onClose(connection_hdl hdl) {
    removeClient(hdl);
    stats_.active_connections--;
    Logger::info("Client disconnected");
}

void RemoteMonitorServer::onMessage(
    connection_hdl hdl,
    websocket_server::message_ptr msg) {
    
    try {
        // 解析消息
        auto j = json::parse(msg->get_payload());
        RemoteMessage message{
            .type = static_cast<MessageType>(j["type"].get<int>()),
            .payload = j["payload"],
            .source = j["source"].get<std::string>(),
            .timestamp = std::chrono::system_clock::now()
        };
        
        // 处理认证
        if (message.type == MessageType::Command &&
            message.payload["command"] == "authenticate") {
            
            std::string token = message.payload["token"].get<std::string>();
            if (authenticate(token)) {
                authenticated_clients_[hdl] = true;
                Logger::info("Client authenticated successfully");
            } else {
                Logger::warn("Client authentication failed");
                server_->close(hdl, websocketpp::close::status::policy_violation,
                             "Authentication failed");
            }
            return;
        }
        
        // 更新统计信息
        stats_.messages_received++;
        
        // 转发消息到客户端处理器
        auto it = clients_.find(hdl);
        if (it != clients_.end()) {
            it->second->onMessage(message);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Failed to process message: {}", e.what());
    }
}

void RemoteMonitorServer::onError(connection_hdl hdl) {
    Logger::error("WebSocket error occurred");
    auto it = clients_.find(hdl);
    if (it != clients_.end()) {
        it->second->onError("WebSocket error occurred");
    }
}

bool RemoteMonitorServer::authenticate(const std::string& token) {
    return token == config_.auth_token;
}

// D3Visualizer 实现
std::string D3Visualizer::generateTimeSeriesChart(
    const std::vector<double>& data,
    const std::vector<std::chrono::system_clock::time_point>& timestamps,
    const std::string& title,
    const std::string& yAxisLabel) {
    
    // 准备数据
    json chartData;
    chartData["type"] = "timeseries";
    chartData["title"] = title;
    chartData["yAxisLabel"] = yAxisLabel;
    
    json points = json::array();
    for (size_t i = 0; i < data.size(); ++i) {
        points.push_back({
            {"timestamp", std::chrono::system_clock::to_time_t(timestamps[i])},
            {"value", data[i]}
        });
    }
    chartData["data"] = points;
    
    return generateD3Script(chartData, "timeSeriesChart");
}

std::string D3Visualizer::generateHistogram(
    const std::vector<double>& data,
    const std::string& title,
    int bins) {
    
    json chartData;
    chartData["type"] = "histogram";
    chartData["title"] = title;
    chartData["bins"] = bins;
    chartData["data"] = data;
    
    return generateD3Script(chartData, "histogram");
}

std::string D3Visualizer::generateHeatMap(
    const std::vector<std::vector<double>>& data,
    const std::string& title) {
    
    json chartData;
    chartData["type"] = "heatmap";
    chartData["title"] = title;
    chartData["data"] = data;
    
    return generateD3Script(chartData, "heatmap");
}

std::string D3Visualizer::generateGauge(
    double value,
    double min,
    double max,
    const std::string& label) {
    
    json chartData;
    chartData["type"] = "gauge";
    chartData["value"] = value;
    chartData["min"] = min;
    chartData["max"] = max;
    chartData["label"] = label;
    
    return generateD3Script(chartData, "gauge");
}

std::string D3Visualizer::generateDashboard(
    const std::vector<std::string>& charts,
    int columns) {
    
    std::stringstream ss;
    ss << R"(
<!DOCTYPE html>
<html>
<head>
    <title>Diagnostic Dashboard</title>
    <style>
        .dashboard {
            display: grid;
            grid-template-columns: repeat()" << columns << R"(, 1fr);
            gap: 20px;
            padding: 20px;
        }
        .chart {
            border: 1px solid #ccc;
            padding: 10px;
            background: white;
        }
    </style>
    <script src="https://d3js.org/d3.v7.min.js"></script>
</head>
<body>
    <div class="dashboard">
    )";
    
    for (const auto& chart : charts) {
        ss << "<div class='chart'>" << chart << "</div>";
    }
    
    ss << R"(
    </div>
</body>
</html>
    )";
    
    return ss.str();
}

std::string D3Visualizer::generateD3Script(
    const json& data,
    const std::string& chartType) {
    
    // 这里是一个简化的实现，实际应用中需要根据图表类型生成相应的D3.js代码
    std::stringstream ss;
    ss << "<script>";
    
    if (chartType == "timeSeriesChart") {
        // 生成时间序列图表的D3.js代码
        ss << R"(
// 时间序列图表代码
const margin = {top: 20, right: 20, bottom: 30, left: 50};
const width = 600 - margin.left - margin.right;
const height = 400 - margin.top - margin.bottom;

const svg = d3.select(this).append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
  .append("g")
    .attr("transform", `translate(${margin.left},${margin.top})`);
        )";
    }
    // ... 其他图表类型的代码
    
    ss << "</script>";
    return ss.str();
}

} // namespace diagnostics
} // namespace hft
