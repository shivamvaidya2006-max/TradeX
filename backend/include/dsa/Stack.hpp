#pragma once

#include <vector>
#include <stdexcept>
#include <string>

namespace tradex::dsa {

/**
 * @brief Stack implementation for Transaction History Undo simulation & operation rollbacks.
 * 
 * Complexity:
 * - push: O(1)
 * - pop: O(1)
 * - top: O(1)
 * - Space: O(N)
 */
template<typename T>
class ActionStack {
public:
    void push(T item) {
        data_.push_back(std::move(item));
    }

    bool pop(T& item) {
        if (data_.empty()) return false;
        item = std::move(data_.back());
        data_.pop_back();
        return true;
    }

    const T& top() const {
        if (data_.empty()) {
            throw std::runtime_error("Stack is empty");
        }
        return data_.back();
    }

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }
    void clear() { data_.clear(); }

    std::vector<T> toVector() const {
        return data_;
    }

private:
    std::vector<T> data_;
};

} // namespace tradex::dsa
