#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <functional>

namespace tradex::dsa {

/**
 * @brief Node for Doubly Linked List
 */
template<typename T>
struct DListNode {
    T data;
    std::shared_ptr<DListNode<T>> next{nullptr};
    std::weak_ptr<DListNode<T>> prev;

    explicit DListNode(T val) : data(std::move(val)) {}
};

/**
 * @brief Custom Doubly Linked List for Audit Logs / Transaction Event Chain.
 * 
 * Complexity:
 * - pushFront: O(1)
 * - pushBack: O(1)
 * - popFront: O(1)
 * - popBack: O(1)
 * - search: O(N)
 * - deleteNode: O(1) given node reference, O(N) by value
 * - traversal: O(N)
 */
template<typename T>
class DoublyLinkedList {
public:
    DoublyLinkedList() = default;

    void pushFront(T val) {
        auto newNode = std::make_shared<DListNode<T>>(std::move(val));
        if (!head_) {
            head_ = tail_ = newNode;
        } else {
            newNode->next = head_;
            head_->prev = newNode;
            head_ = newNode;
        }
        size_++;
    }

    void pushBack(T val) {
        auto newNode = std::make_shared<DListNode<T>>(std::move(val));
        if (!tail_) {
            head_ = tail_ = newNode;
        } else {
            tail_->next = newNode;
            newNode->prev = tail_;
            tail_ = newNode;
        }
        size_++;
    }

    bool popFront(T& outVal) {
        if (!head_) return false;
        outVal = head_->data;
        head_ = head_->next;
        if (head_) {
            head_->prev.reset();
        } else {
            tail_ = nullptr;
        }
        size_--;
        return true;
    }

    bool popBack(T& outVal) {
        if (!tail_) return false;
        outVal = tail_->data;
        auto prevNode = tail_->prev.lock();
        if (prevNode) {
            prevNode->next = nullptr;
            tail_ = prevNode;
        } else {
            head_ = tail_ = nullptr;
        }
        size_--;
        return true;
    }

    std::vector<T> toVector() const {
        std::vector<T> list;
        auto curr = head_;
        while (curr) {
            list.push_back(curr->data);
            curr = curr->next;
        }
        return list;
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void clear() {
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

private:
    std::shared_ptr<DListNode<T>> head_{nullptr};
    std::shared_ptr<DListNode<T>> tail_{nullptr};
    size_t size_{0};
};

} // namespace tradex::dsa
