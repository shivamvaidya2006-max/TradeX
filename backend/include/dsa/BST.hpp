#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <iostream>

namespace tradex::dsa {

struct BSTNode {
    double price{0.0};
    std::string symbol;
    int volume{0};
    std::shared_ptr<BSTNode> left{nullptr};
    std::shared_ptr<BSTNode> right{nullptr};

    BSTNode(double p, std::string sym, int vol = 0)
        : price(p), symbol(std::move(sym)), volume(vol) {}
};

/**
 * @brief Binary Search Tree (BST) for Stock Price Level Indexing.
 * 
 * Complexity:
 * - Insert: Average O(log N), Worst O(N)
 * - Search: Average O(log N), Worst O(N)
 * - Delete: Average O(log N), Worst O(N)
 * - Traversals: O(N)
 * - Space: O(N)
 */
class BinarySearchTree {
public:
    BinarySearchTree() = default;

    void insert(double price, const std::string& symbol, int volume = 0) {
        root_ = insertRecursive(root_, price, symbol, volume);
    }

    bool search(double price) const {
        return searchRecursive(root_, price) != nullptr;
    }

    std::shared_ptr<BSTNode> findMin() const {
        return findMinNode(root_);
    }

    std::shared_ptr<BSTNode> findMax() const {
        return findMaxNode(root_);
    }

    void deletePrice(double price) {
        root_ = deleteRecursive(root_, price);
    }

    std::vector<double> inorder() const {
        std::vector<double> result;
        inorderRecursive(root_, result);
        return result;
    }

    std::vector<double> preorder() const {
        std::vector<double> result;
        preorderRecursive(root_, result);
        return result;
    }

    std::vector<double> postorder() const {
        std::vector<double> result;
        postorderRecursive(root_, result);
        return result;
    }

    struct VisualTreeNode {
        double price{0.0};
        std::string symbol;
        std::shared_ptr<VisualTreeNode> left{nullptr};
        std::shared_ptr<VisualTreeNode> right{nullptr};
    };

    std::shared_ptr<VisualTreeNode> getVisualTree() const {
        return convertToVisual(root_);
    }

    void clear() {
        root_ = nullptr;
    }

private:
    std::shared_ptr<BSTNode> root_{nullptr};

    std::shared_ptr<BSTNode> insertRecursive(std::shared_ptr<BSTNode> node, double price, const std::string& symbol, int volume) {
        if (!node) {
            return std::make_shared<BSTNode>(price, symbol, volume);
        }
        if (price < node->price) {
            node->left = insertRecursive(node->left, price, symbol, volume);
        } else if (price > node->price) {
            node->right = insertRecursive(node->right, price, symbol, volume);
        } else {
            // Equal price: update volume
            node->volume += volume;
        }
        return node;
    }

    std::shared_ptr<BSTNode> searchRecursive(std::shared_ptr<BSTNode> node, double price) const {
        if (!node || std::abs(node->price - price) < 0.001) {
            return node;
        }
        if (price < node->price) {
            return searchRecursive(node->left, price);
        }
        return searchRecursive(node->right, price);
    }

    std::shared_ptr<BSTNode> findMinNode(std::shared_ptr<BSTNode> node) const {
        auto current = node;
        while (current && current->left) {
            current = current->left;
        }
        return current;
    }

    std::shared_ptr<BSTNode> findMaxNode(std::shared_ptr<BSTNode> node) const {
        auto current = node;
        while (current && current->right) {
            current = current->right;
        }
        return current;
    }

    std::shared_ptr<BSTNode> deleteRecursive(std::shared_ptr<BSTNode> node, double price) {
        if (!node) return nullptr;

        if (price < node->price) {
            node->left = deleteRecursive(node->left, price);
        } else if (price > node->price) {
            node->right = deleteRecursive(node->right, price);
        } else {
            // Node with only one child or no child
            if (!node->left) {
                return node->right;
            } else if (!node->right) {
                return node->left;
            }

            // Node with two children: get inorder successor (smallest in the right subtree)
            auto temp = findMinNode(node->right);
            node->price = temp->price;
            node->symbol = temp->symbol;
            node->volume = temp->volume;
            node->right = deleteRecursive(node->right, temp->price);
        }
        return node;
    }

    void inorderRecursive(std::shared_ptr<BSTNode> node, std::vector<double>& result) const {
        if (!node) return;
        inorderRecursive(node->left, result);
        result.push_back(node->price);
        inorderRecursive(node->right, result);
    }

    void preorderRecursive(std::shared_ptr<BSTNode> node, std::vector<double>& result) const {
        if (!node) return;
        result.push_back(node->price);
        preorderRecursive(node->left, result);
        preorderRecursive(node->right, result);
    }

    void postorderRecursive(std::shared_ptr<BSTNode> node, std::vector<double>& result) const {
        if (!node) return;
        postorderRecursive(node->left, result);
        postorderRecursive(node->right, result);
        result.push_back(node->price);
    }

    std::shared_ptr<VisualTreeNode> convertToVisual(std::shared_ptr<BSTNode> node) const {
        if (!node) return nullptr;
        auto v = std::make_shared<VisualTreeNode>();
        v->price = node->price;
        v->symbol = node->symbol;
        v->left = convertToVisual(node->left);
        v->right = convertToVisual(node->right);
        return v;
    }
};

} // namespace tradex::dsa
