#include "VisualizationAndMonitoring.h"
#include "../core/Logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

namespace hft {
namespace diagnostics {

// ChartJSVisualizer 实现
ChartJSVisualizer::ChartJSVisualizer(const VisualizationConfig& config)
    : config_(config) {
    Logger::info("ChartJS visualizer initialized with theme: {}", config.theme);
}

std::string ChartJSVisualizer::generateTimeSeriesChart(
    const std::vector<double>& data,
    const std::vector<std::chrono::system_clock::time_point>& timestamps,
    const std::string& title,
    const std::string& y_label) {
    
    json chart_config;
    chart_config["type"] = "line";
    
    // 数据配置
    json datasets = json::array();
    json dataset;
    dataset["label"] = y_label.empty() ? "Value" : y_label;
    dataset["borderColor"] = getColorScheme();
    dataset["backgroundColor"] = "rgba(54, 162, 235, 0.1)";
    dataset["fill"] = false;
    
    json data_points = json::array();
    for (size_t i = 0; i < data.size() && i < timestamps.size(); ++i) {
        json point;
        point["x"] = formatTimestamp(timestamps[i]);
        point["y"] = data[i];
        data_points.push_back(point);
    }
    dataset["data"] = data_points;
    datasets.push_back(dataset);
    
    chart_config["data"]["datasets"] = datasets;
    
    // 选项配置
    chart_config["options"]["responsive"] = true;
    chart_config["options"]["plugins"]["title"]["display"] = !title.empty();
    chart_config["options"]["plugins"]["title"]["text"] = title;
    chart_config["options"]["scales"]["x"]["type"] = "time";
    chart_config["options"]["scales"]["x"]["time"]["displayFormats"]["hour"] = "HH:mm";
    chart_config["options"]["scales"]["y"]["beginAtZero"] = false;
    chart_config["options"]["animation"]["duration"] = config_.enable_animation ? 1000 : 0;
    
    return generateChartJS(chart_config);
}

std::string ChartJSVisualizer::generateHistogram(
    const std::vector<double>& data,
    const std::string& title,
    int bins) {
    
    if (data.empty()) return "";
    
    // 计算直方图数据
    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());
    double bin_width = (max_val - min_val) / bins;
    
    std::vector<int> histogram(bins, 0);
    std::vector<std::string> labels;
    
    for (double value : data) {
        int bin_index = static_cast<int>((value - min_val) / bin_width);
        bin_index = std::min(bin_index, bins - 1);
        histogram[bin_index]++;
    }
    
    for (int i = 0; i < bins; ++i) {
        double bin_start = min_val + i * bin_width;
        double bin_end = bin_start + bin_width;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << bin_start << "-" << bin_end;
        labels.push_back(oss.str());
    }
    
    json chart_config;
    chart_config["type"] = "bar";
    
    // 数据配置
    chart_config["data"]["labels"] = labels;
    json datasets = json::array();
    json dataset;
    dataset["label"] = "Frequency";
    dataset["backgroundColor"] = getColorScheme();
    dataset["data"] = histogram;
    datasets.push_back(dataset);
    chart_config["data"]["datasets"] = datasets;
    
    // 选项配置
    chart_config["options"]["responsive"] = true;
    chart_config["options"]["plugins"]["title"]["display"] = !title.empty();
    chart_config["options"]["plugins"]["title"]["text"] = title;
    chart_config["options"]["scales"]["y"]["beginAtZero"] = true;
    
    return generateChartJS(chart_config);
}

std::string ChartJSVisualizer::generateGaugeChart(
    double value,
    double min_val,
    double max_val,
    const std::string& label) {
    
    // 使用doughnut图表模拟仪表盘
    json chart_config;
    chart_config["type"] = "doughnut";
    
    double percentage = (value - min_val) / (max_val - min_val) * 100;
    
    chart_config["data"]["labels"] = json::array({"Value", "Remaining"});
    json datasets = json::array();
    json dataset;
    dataset["data"] = json::array({percentage, 100 - percentage});
    dataset["backgroundColor"] = json::array({
        percentage > 80 ? "#ff6b6b" : (percentage > 60 ? "#ffd93d" : "#6bcf7f"),
        "#e0e0e0"
    });
    dataset["borderWidth"] = 0;
    datasets.push_back(dataset);
    chart_config["data"]["datasets"] = datasets;
    
    // 选项配置
    chart_config["options"]["responsive"] = true;
    chart_config["options"]["circumference"] = 180;
    chart_config["options"]["rotation"] = 270;
    chart_config["options"]["cutout"] = "80%";
    chart_config["options"]["plugins"]["legend"]["display"] = false;
    chart_config["options"]["plugins"]["title"]["display"] = !label.empty();
    chart_config["options"]["plugins"]["title"]["text"] = 
        label + ": " + std::to_string(value);
    
    return generateChartJS(chart_config);
}

std::string ChartJSVisualizer::generateBarChart(
    const std::vector<std::string>& labels,
    const std::vector<double>& values,
    const std::string& title) {
    
    json chart_config;
    chart_config["type"] = "bar";
    
    chart_config["data"]["labels"] = labels;
    json datasets = json::array();
    json dataset;
    dataset["label"] = "Value";
    dataset["backgroundColor"] = getColorScheme();
    dataset["data"] = values;
    datasets.push_back(dataset);
    chart_config["data"]["datasets"] = datasets;
    
    // 选项配置
    chart_config["options"]["responsive"] = true;
    chart_config["options"]["plugins"]["title"]["display"] = !title.empty();
    chart_config["options"]["plugins"]["title"]["text"] = title;
    chart_config["options"]["scales"]["y"]["beginAtZero"] = true;
    
    return generateChartJS(chart_config);
}

std::string ChartJSVisualizer::generateDashboard(
    const std::vector<std::string>& charts,
    int columns) {
    
    std::ostringstream html;
    
    html << R"(<!DOCTYPE html>
<html>
<head>
    <title>HFT Diagnostic Dashboard</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/date-fns@1.30.1/index.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns@2.0.0/dist/chartjs-adapter-date-fns.bundle.min.js"></script>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: )" << (config_.theme == "dark" ? "#1a1a1a" : "#f5f5f5") << R"(;
            color: )" << (config_.theme == "dark" ? "#ffffff" : "#333333") << R"(;
        }
        .dashboard {
            display: grid;
            grid-template-columns: repeat()" << columns << R"(, 1fr);
            gap: 20px;
            max-width: 1400px;
            margin: 0 auto;
        }
        .chart-container {
            background: )" << (config_.theme == "dark" ? "#2d2d2d" : "#ffffff") << R"(;
            border-radius: 8px;
            padding: 20px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            border: 1px solid )" << (config_.theme == "dark" ? "#404040" : "#e0e0e0") << R"(;
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
            padding: 20px;
            background: )" << (config_.theme == "dark" ? "#2d2d2d" : "#ffffff") << R"(;
            border-radius: 8px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        .status-online { background-color: #4caf50; }
        .status-warning { background-color: #ff9800; }
        .status-error { background-color: #f44336; }
    </style>
</head>
<body>
    <div class="header">
        <h1><span class="status-indicator status-online"></span>HFT System Diagnostic Dashboard</h1>
        <p>Real-time monitoring and analysis - Last updated: <span id="lastUpdate"></span></p>
    </div>
    
    <div class="dashboard">)";
    
    for (size_t i = 0; i < charts.size(); ++i) {
        html << R"(
        <div class="chart-container">
            <canvas id="chart)" << i << R"(" width=")" << config_.chart_width 
             << R"(" height=")" << config_.chart_height << R"("></canvas>
        </div>)";
    }
    
    html << R"(
    </div>
    
    <script>
        // 更新时间戳
        document.getElementById('lastUpdate').textContent = new Date().toLocaleString();
        
        // 自动刷新
        setInterval(() => {
            document.getElementById('lastUpdate').textContent = new Date().toLocaleString();
        }, )" << config_.refresh_interval << R"();
        
        // 图表实例
        const charts = [];
        )";
    
    for (size_t i = 0; i < charts.size(); ++i) {
        html << "\n        // Chart " << i << "\n";
        html << charts[i];
    }
    
    html << R"(
    </script>
</body>
</html>)";
    
    return html.str();
}

std::string ChartJSVisualizer::generateRealtimeChart(
    const std::string& chart_id,
    const std::string& metric_name) {
    
    std::ostringstream js;
    js << R"(
        const ctx)" << chart_id << R" = document.getElementById(')" << chart_id << R"(').getContext('2d');
        const chart)" << chart_id << R" = new Chart(ctx)" << chart_id << R"(, {
            type: 'line',
            data: {
                datasets: [{
                    label: ')" << metric_name << R"(',
                    data: [],
                    borderColor: ')" << getColorScheme() << R"(',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    fill: false,
                    tension: 0.1
                }]
            },
            options: {
                responsive: true,
                scales: {
                    x: {
                        type: 'realtime',
                        realtime: {
                            duration: 60000,
                            refresh: )" << config_.refresh_interval << R"(,
                            delay: 1000,
                            onRefresh: function(chart) {
                                // 这里可以添加实时数据获取逻辑
                                // fetch('/api/metrics/)" << metric_name << R"(')
                                //     .then(response => response.json())
                                //     .then(data => {
                                //         chart.data.datasets[0].data.push({
                                //             x: Date.now(),
                                //             y: data.value
                                //         });
                                //     });
                            }
                        }
                    },
                    y: {
                        beginAtZero: false
                    }
                },
                plugins: {
                    legend: {
                        display: true
                    }
                }
            }
        });
        charts.push(chart)" << chart_id << R"();
    )";
    
    return js.str();
}

std::string ChartJSVisualizer::generateChartJS(const json& chart_config) {
    std::ostringstream js;
    
    static int chart_counter = 0;
    std::string chart_id = "chart" + std::to_string(chart_counter++);
    
    js << R"(
        const ctx)" << chart_id << R" = document.getElementById(')" << chart_id << R"(').getContext('2d');
        const )" << chart_id << R" = new Chart(ctx)" << chart_id << R"(, )"
       << chart_config.dump(4) << R"();
        charts.push()" << chart_id << R"();
    )";
    
    return js.str();
}

std::string ChartJSVisualizer::getColorScheme() {
    if (config_.color_scheme == "blue") {
        return "rgba(54, 162, 235, 1)";
    } else if (config_.color_scheme == "red") {
        return "rgba(255, 99, 132, 1)";
    } else if (config_.color_scheme == "green") {
        return "rgba(75, 192, 192, 1)";
    } else {
        return "rgba(54, 162, 235, 1)";  // 默认蓝色
    }
}

std::string ChartJSVisualizer::formatTimestamp(
    const std::chrono::system_clock::time_point& tp) {
    
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), config_.date_format.c_str());
    return oss.str();
}

// RemoteMonitorServer 实现
RemoteMonitorServer::RemoteMonitorServer(const RemoteMonitorConfig& config)
    : config_(config) {
    
    server_ = std::make_unique<websocket_server>();
    server_->init_asio();
    
    // 设置事件处理器
    server_->set_open_handler(
        std::bind(&RemoteMonitorServer::onOpen, this, std::placeholders::_1));
    server_->set_close_handler(
        std::bind(&RemoteMonitorServer::onClose, this, std::placeholders::_1));
    server_->set_message_handler(
        std::bind(&RemoteMonitorServer::onMessage, this,
                 std::placeholders::_1, std::placeholders::_2));
    server_->set_error_handler(
        std::bind(&RemoteMonitorServer::onError, this, std::placeholders::_1));
    
    // 初始化统计信息
    stats_.start_time = std::chrono::system_clock::now();
    
    Logger::info("Remote monitor server initialized on {}:{}", config_.host, config_.port);
}

RemoteMonitorServer::~RemoteMonitorServer() {
    if (running_) {
        stop();
    }
}

void RemoteMonitorServer::start() {
    if (running_) return;
    
    try {
        server_->listen(config_.port);
        server_->start_accept();
        
        running_ = true;
        
        // 启动服务器线程
        server_thread_ = std::thread([this]() {
            serverLoop();
        });
        
        // 启动心跳
        startHeartbeat();
        
        Logger::info("Remote monitor server started on port {}", config_.port);
        
    } catch (const std::exception& e) {
        Logger::error("Failed to start remote monitor server: {}", e.what());
        throw;
    }
}

void RemoteMonitorServer::stop() {
    if (!running_) return;
    
    try {
        running_ = false;
        
        // 关闭所有连接
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (auto& [hdl, client] : clients_) {
                server_->close(hdl, websocketpp::close::status::going_away, 
                              "Server shutdown");
            }
            clients_.clear();
            client_handles_.clear();
        }
        
        server_->stop();
        
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        
        Logger::info("Remote monitor server stopped");
        
    } catch (const std::exception& e) {
        Logger::error("Error stopping remote monitor server: {}", e.what());
    }
}

void RemoteMonitorServer::broadcast(const RemoteMessage& message) {
    if (!running_) return;
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    std::string serialized = serializeMessage(message);
    
    for (auto& [hdl, client] : clients_) {
        try {
            server_->send(hdl, serialized, websocketpp::frame::opcode::text);
            stats_.messages_sent++;
        } catch (const std::exception& e) {
            Logger::error("Failed to send message to client: {}", e.what());
        }
    }
    
    // 更新平均消息大小
    stats_.avg_message_size = 
        (stats_.avg_message_size * (stats_.messages_sent - clients_.size()) +
         serialized.size() * clients_.size()) / stats_.messages_sent;
}

void RemoteMonitorServer::sendToClient(const std::string& client_id, 
                                      const RemoteMessage& message) {
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = client_handles_.find(client_id);
    if (it != client_handles_.end()) {
        try {
            std::string serialized = serializeMessage(message);
            server_->send(it->second, serialized, websocketpp::frame::opcode::text);
            stats_.messages_sent++;
        } catch (const std::exception& e) {
            Logger::error("Failed to send message to client {}: {}", client_id, e.what());
        }
    }
}

void RemoteMonitorServer::onOpen(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    stats_.active_connections++;
    Logger::info("New client connected. Total connections: {}", stats_.active_connections);
}

void RemoteMonitorServer::onClose(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(hdl);
    if (it != clients_.end()) {
        std::string client_id = it->second->getClientId();
        clients_.erase(it);
        client_handles_.erase(client_id);
        stats_.active_connections--;
        Logger::info("Client {} disconnected. Total connections: {}", 
                    client_id, stats_.active_connections);
    }
}

void RemoteMonitorServer::onMessage(connection_hdl hdl, 
                                   websocket_server::message_ptr msg) {
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    try {
        RemoteMessage message = deserializeMessage(msg->get_payload());
        stats_.messages_received++;
        
        auto it = clients_.find(hdl);
        if (it != clients_.end()) {
            it->second->onMessage(message);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Failed to process message: {}", e.what());
    }
}

void RemoteMonitorServer::onError(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(hdl);
    if (it != clients_.end()) {
        it->second->onError("WebSocket error occurred");
    }
}

std::string RemoteMonitorServer::serializeMessage(const RemoteMessage& message) {
    json j;
    j["type"] = static_cast<int>(message.type);
    j["payload"] = message.payload;
    j["source"] = message.source;
    j["target"] = message.target;
    j["timestamp"] = std::chrono::system_clock::to_time_t(message.timestamp);
    j["correlation_id"] = message.correlation_id;
    
    return j.dump();
}

RemoteMessage RemoteMonitorServer::deserializeMessage(const std::string& data) {
    json j = json::parse(data);
    
    RemoteMessage message;
    message.type = static_cast<MessageType>(j["type"].get<int>());
    message.payload = j["payload"];
    message.source = j["source"].get<std::string>();
    message.target = j.value("target", "");
    message.correlation_id = j.value("correlation_id", "");
    
    auto timestamp = j["timestamp"].get<std::time_t>();
    message.timestamp = std::chrono::system_clock::from_time_t(timestamp);
    
    return message;
}

void RemoteMonitorServer::serverLoop() {
    try {
        server_->run();
    } catch (const std::exception& e) {
        Logger::error("Server loop error: {}", e.what());
    }
}

void RemoteMonitorServer::startHeartbeat() {
    std::thread([this]() {
        while (running_) {
            std::this_thread::sleep_for(config_.heartbeat_interval);
            if (running_) {
                sendHeartbeat();
            }
        }
    }).detach();
}

void RemoteMonitorServer::sendHeartbeat() {
    RemoteMessage heartbeat;
    heartbeat.type = MessageType::Heartbeat;
    heartbeat.source = "server";
    heartbeat.timestamp = std::chrono::system_clock::now();
    heartbeat.payload["message"] = "heartbeat";
    
    broadcast(heartbeat);
}

RemoteMonitorStats RemoteMonitorServer::getStats() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return stats_;
}

} // namespace diagnostics
} // namespace hft
