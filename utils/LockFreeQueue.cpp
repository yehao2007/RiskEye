#include "LockFreeQueue.h"
#include <memory>

namespace hft {
namespace utils {

// 实现LockFreeQueue

template<typename T>
LockFreeQueue<T>::LockFreeQueue() {
    // 创建一个哑节点作为头节点和尾节点
    Node* dummy = new Node();
    m_head.store(dummy);
    m_tail.store(dummy);
}

template<typename T>
LockFreeQueue<T>::~LockFreeQueue() {
    // 清空队列
    T dummy;
    while (pop(dummy)) {}
    // 删除哑节点
    delete m_head.load();
}

template<typename T>
void LockFreeQueue<T>::push(const T& data) {
    // 创建新节点
    Node* new_node = new Node(data);

    // 循环直到成功插入
    while (true) {
        // 获取当前尾节点
        Node* tail = m_tail.load(std::memory_order_relaxed);
        // 获取尾节点的下一个节点
        Node* next = tail->next.load(std::memory_order_acquire);

        // 检查尾节点是否一致
        if (tail == m_tail.load(std::memory_order_relaxed)) {
            // 尾节点的下一个节点为空，表示可以插入
            if (next == nullptr) {
                // 尝试将新节点设置为尾节点的下一个节点
                if (tail->next.compare_exchange_weak(next, new_node, std::memory_order_release)) {
                    // 成功插入，更新尾节点
                    m_tail.compare_exchange_strong(tail, new_node, std::memory_order_release);
                    return;
                }
            } else {
                // 尾节点的下一个节点不为空，帮助更新尾节点
                m_tail.compare_exchange_strong(tail, next, std::memory_order_release);
            }
        }
    }
}

template<typename T>
bool LockFreeQueue<T>::pop(T& data) {
    // 循环直到成功弹出或队列为空
    while (true) {
        // 获取当前头节点
        Node* head = m_head.load(std::memory_order_relaxed);
        // 获取尾节点
        Node* tail = m_tail.load(std::memory_order_relaxed);
        // 获取头节点的下一个节点
        Node* next = head->next.load(std::memory_order_acquire);

        // 检查头节点是否一致
        if (head == m_head.load(std::memory_order_relaxed)) {
            // 头节点和尾节点指向同一个节点
            if (head == tail) {
                // 队列为空
                if (next == nullptr) {
                    return false;
                }
                // 帮助更新尾节点
                m_tail.compare_exchange_strong(tail, next, std::memory_order_release);
            } else {
                // 队列不为空，尝试弹出数据
                if (next != nullptr && next->data) {
                    data = *next->data;
                    // 尝试更新头节点
                    if (m_head.compare_exchange_weak(head, next, std::memory_order_release)) {
                        // 释放旧的头节点
                        delete head;
                        return true;
                    }
                }
            }
        }
    }
}

template<typename T>
bool LockFreeQueue<T>::empty() const {
    Node* head = m_head.load(std::memory_order_relaxed);
    Node* tail = m_tail.load(std::memory_order_relaxed);
    return head == tail && head->next.load(std::memory_order_acquire) == nullptr;
}

// 显式实例化常见类型
template class LockFreeQueue<int>;
template class LockFreeQueue<double>;
template class LockFreeQueue<std::string>;

} // namespace utils
} // namespace hft