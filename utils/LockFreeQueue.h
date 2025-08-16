#pragma once
#include <atomic>
#include <memory>

namespace hft {
namespace utils {

// 无锁队列
template<typename T>
class LockFreeQueue {
public:
    LockFreeQueue();
    ~LockFreeQueue();

    // 入队
    void push(const T& data);
    // 出队
    bool pop(T& data);
    // 检查队列是否为空
    bool empty() const;

private:
    // 节点结构
    struct Node {
        std::shared_ptr<T> data; // 数据
        std::atomic<Node*> next; // 下一个节点

        Node() : next(nullptr) {}
        explicit Node(const T& item) : data(std::make_shared<T>(item)), next(nullptr) {}
    };

    std::atomic<Node*> m_head; // 头节点
    std::atomic<Node*> m_tail; // 尾节点
};

} // namespace utils
} // namespace hft