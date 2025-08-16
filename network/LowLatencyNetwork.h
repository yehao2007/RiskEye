#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include "../core/Configuration.h"
#include "../core/EventLoop.h"
#include "../core/Logger.h"

namespace hft {
namespace network {

// 网络连接状态
enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
};

// 低延迟网络接口
class LowLatencyNetwork {
public:
    LowLatencyNetwork(const core::Configuration& config, std::shared_ptr<core::EventLoop> eventLoop);
    ~LowLatencyNetwork();

    // 初始化网络
    bool initialize();
    // 启动网络服务
    void start();
    // 停止网络服务
    void stop();
    // 网络状态
    bool isRunning() const { return m_running; }

    // 连接到目标服务器
    bool connect(const std::string& host, uint16_t port);
    // 断开连接
    void disconnect(const std::string& host, uint16_t port);
    // 发送数据
    bool send(const std::string& host, uint16_t port, const void* data, size_t size);
    // 注册数据接收回调
    using DataCallback = std::function<void(const std::string& host, uint16_t port, const void* data, size_t size)>;
    void registerDataCallback(const std::string& host, uint16_t port, DataCallback callback);

private:
    struct Connection {
        std::string host;
        uint16_t port;
        ConnectionState state;
        DataCallback callback;
        // 这里可以添加套接字等网络相关成员

        Connection(const std::string& h, uint16_t p)
            : host(h), port(p), state(ConnectionState::DISCONNECTED) {}
    };

    core::Configuration m_config;
    core::Logger m_logger;
    std::atomic<bool> m_running;
    std::shared_ptr<core::EventLoop> m_eventLoop;
    std::vector<std::unique_ptr<Connection>> m_connections;
    std::mutex m_mutex;

    // 查找连接
    Connection* findConnection(const std::string& host, uint16_t port);
    // 连接线程函数
    void connectThreadFunc(Connection* conn);
    // 接收线程函数
    void receiveThreadFunc(Connection* conn);
};

} // namespace network
} // namespace hft