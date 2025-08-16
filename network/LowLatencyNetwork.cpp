#include "LowLatencyNetwork.h"
#include <iostream>
#include <chrono>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace hft {
namespace network {

LowLatencyNetwork::LowLatencyNetwork(const core::Configuration& config, std::shared_ptr<core::EventLoop> eventLoop)
    : m_config(config), m_logger("LowLatencyNetwork"), m_running(false), m_eventLoop(eventLoop) {
    // 初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        m_logger.error("WSAStartup failed: " + std::to_string(WSAGetLastError()));
    }
}

LowLatencyNetwork::~LowLatencyNetwork() {
    stop();
    WSACleanup();
}

bool LowLatencyNetwork::initialize() {
    m_logger.info("Initializing low latency network...");

    // 从配置中读取预定义的连接
    std::vector<std::string> connections = m_config.getStringList("network_connections");
    for (const auto& connStr : connections) {
        // 解析连接字符串 (格式: host:port)
        size_t pos = connStr.find(':');
        if (pos == std::string::npos) {
            m_logger.warning("Invalid connection string: " + connStr);
            continue;
        }
        std::string host = connStr.substr(0, pos);
        uint16_t port = static_cast<uint16_t>(std::stoi(connStr.substr(pos + 1)));

        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.push_back(std::make_unique<Connection>(host, port));
        m_logger.info("Added connection: " + host + ":" + std::to_string(port));
    }

    m_logger.info("Low latency network initialized successfully");
    return true;
}

void LowLatencyNetwork::start() {
    if (m_running) {
        m_logger.warning("Low latency network is already running");
        return;
    }

    m_logger.info("Starting low latency network...");
    m_running = true;

    // 启动所有预定义连接
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& conn : m_connections) {
        if (conn->state == ConnectionState::DISCONNECTED) {
            std::thread connectThread(&LowLatencyNetwork::connectThreadFunc, this, conn.get());
            connectThread.detach();
        }
    }

    m_logger.info("Low latency network started successfully");
}

void LowLatencyNetwork::stop() {
    if (!m_running) {
        m_logger.warning("Low latency network is not running");
        return;
    }

    m_logger.info("Stopping low latency network...");
    m_running = false;

    // 断开所有连接
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& conn : m_connections) {
        if (conn->state != ConnectionState::DISCONNECTED) {
            conn->state = ConnectionState::DISCONNECTING;
        }
    }

    // 等待所有连接断开
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    m_logger.info("Low latency network stopped successfully");
}

bool LowLatencyNetwork::connect(const std::string& host, uint16_t port) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Connection* conn = findConnection(host, port);
    if (!conn) {
        // 创建新连接
        m_connections.push_back(std::make_unique<Connection>(host, port));
        conn = m_connections.back().get();
        m_logger.info("Created new connection: " + host + ":" + std::to_string(port));
    }

    if (conn->state == ConnectionState::DISCONNECTED) {
        conn->state = ConnectionState::CONNECTING;
        std::thread connectThread(&LowLatencyNetwork::connectThreadFunc, this, conn);
        connectThread.detach();
        return true;
    }

    return false;
}

void LowLatencyNetwork::disconnect(const std::string& host, uint16_t port) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Connection* conn = findConnection(host, port);
    if (conn && conn->state != ConnectionState::DISCONNECTED && conn->state != ConnectionState::DISCONNECTING) {
        conn->state = ConnectionState::DISCONNECTING;
        m_logger.info("Disconnecting from: " + host + ":" + std::to_string(port));
    }
}

bool LowLatencyNetwork::send(const std::string& host, uint16_t port, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Connection* conn = findConnection(host, port);
    if (!conn || conn->state != ConnectionState::CONNECTED) {
        m_logger.error("Cannot send data to " + host + ":" + std::to_string(port) + ", not connected");
        return false;
    }

    // 实际发送逻辑将在这里实现
    // 注意：这只是一个框架，实际实现需要使用套接字API发送数据
    m_logger.debug("Sending " + std::to_string(size) + " bytes to " + host + ":" + std::to_string(port));

    return true;
}

void LowLatencyNetwork::registerDataCallback(const std::string& host, uint16_t port, DataCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Connection* conn = findConnection(host, port);
    if (conn) {
        conn->callback = callback;
        m_logger.info("Registered data callback for " + host + ":" + std::to_string(port));
    } else {
        m_logger.error("Cannot register callback for " + host + ":" + std::to_string(port) + ", connection not found");
    }
}

LowLatencyNetwork::Connection* LowLatencyNetwork::findConnection(const std::string& host, uint16_t port) {
    for (auto& conn : m_connections) {
        if (conn->host == host && conn->port == port) {
            return conn.get();
        }
    }
    return nullptr;
}

void LowLatencyNetwork::connectThreadFunc(Connection* conn) {
    m_logger.info("Connecting to " + conn->host + ":" + std::to_string(conn->port) + "...");

    // 模拟连接过程
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 实际连接逻辑将在这里实现
    // 注意：这只是一个框架，实际实现需要使用套接字API建立连接

    conn->state = ConnectionState::CONNECTED;
    m_logger.info("Connected to " + conn->host + ":" + std::to_string(conn->port));

    // 启动接收线程
    std::thread receiveThread(&LowLatencyNetwork::receiveThreadFunc, this, conn);
    receiveThread.detach();
}

void LowLatencyNetwork::receiveThreadFunc(Connection* conn) {
    while (m_running && conn->state == ConnectionState::CONNECTED) {
        // 实际接收逻辑将在这里实现
        // 注意：这只是一个框架，实际实现需要使用套接字API接收数据

        // 模拟接收数据
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // 假设我们接收到了一些数据
        // 在实际实现中，这里应该是从套接字读取数据
        char dummyData[] = "Market data update";
        size_t dataSize = sizeof(dummyData) - 1;

        if (conn->callback) {
            // 在事件循环中执行回调
            m_eventLoop->postTask([conn, dummyData, dataSize]() {
                conn->callback(conn->host, conn->port, dummyData, dataSize);
            });
        }
    }

    if (conn->state == ConnectionState::DISCONNECTING) {
        conn->state = ConnectionState::DISCONNECTED;
        m_logger.info("Disconnected from " + conn->host + ":" + std::to_string(conn->port));
    }
}

} // namespace network
} // namespace hft