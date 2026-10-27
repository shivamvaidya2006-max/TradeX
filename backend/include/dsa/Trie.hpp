#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace tradex::dsa {

/**
 * @brief Representation of stock metadata indexed in the Trie.
 */
struct TrieStockItem {
    std::string symbol;
    std::string companyName;
    std::string sector;
    double currentPrice{0.0};
};

/**
 * @brief Trie Node for high-performance prefix-based stock search.
 * Supports alphanumeric symbols and company names.
 */
class TrieNode {
public:
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    bool isEndOfWord{false};
    std::shared_ptr<TrieStockItem> stockItem{nullptr};

    TrieNode() = default;
};

/**
 * @brief Prefix Tree (Trie) Data Structure for O(L) stock search and autocomplete.
 * 
 * Complexity:
 * - Insert: O(L) where L is length of string
 * - Search: O(L)
 * - Autocomplete / Suggestions: O(L + K) where K is number of matched descendants
 * - Space Complexity: O(N * L * Sigma) where Sigma is alphabet size
 */
class Trie {
public:
    Trie() : root(std::make_shared<TrieNode>()) {}

    /**
     * @brief Normalizes a string to uppercase for case-insensitive lookup.
     */
    static std::string normalize(const std::string& str) {
        std::string res;
        res.reserve(str.size());
        for (char ch : str) {
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '&' || ch == ' ') {
                res += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            }
        }
        return res;
    }

    /**
     * @brief Inserts a stock item under a key (symbol or name keyword).
     */
    void insert(const std::string& key, const TrieStockItem& item) {
        std::string normKey = normalize(key);
        if (normKey.empty()) return;

        auto current = root;
        for (char ch : normKey) {
            if (current->children.find(ch) == current->children.end()) {
                current->children[ch] = std::make_shared<TrieNode>();
            }
            current = current->children[ch];
        }
        current->isEndOfWord = true;
        current->stockItem = std::make_shared<TrieStockItem>(item);
    }

    /**
     * @brief Checks if exact key exists in Trie.
     */
    bool search(const std::string& key) const {
        std::string normKey = normalize(key);
        auto current = root;
        for (char ch : normKey) {
            auto it = current->children.find(ch);
            if (it == current->children.end()) return false;
            current = it->second;
        }
        return current != nullptr && current->isEndOfWord;
    }

    /**
     * @brief Checks if prefix exists.
     */
    bool startsWith(const std::string& prefix) const {
        std::string normPrefix = normalize(prefix);
        auto current = root;
        for (char ch : normPrefix) {
            auto it = current->children.find(ch);
            if (it == current->children.end()) return false;
            current = it->second;
        }
        return true;
    }

    /**
     * @brief Suggests matching stocks given a prefix query up to maxLimit.
     */
    std::vector<TrieStockItem> suggest(const std::string& prefix, size_t maxLimit = 10) const {
        std::vector<TrieStockItem> results;
        std::string normPrefix = normalize(prefix);
        if (normPrefix.empty()) return results;

        auto current = root;
        for (char ch : normPrefix) {
            auto it = current->children.find(ch);
            if (it == current->children.end()) {
                return results; // No prefix match
            }
            current = it->second;
        }

        // DFS collection of all matches under the matched prefix node
        collect(current, results, maxLimit);
        return results;
    }

    /**
     * @brief Returns a structured visual hierarchy tree for DSA Lab visualization.
     */
    struct VisualNode {
        std::string character;
        bool isTerminal{false};
        std::string symbol;
        std::vector<VisualNode> children;
    };

    VisualNode getVisualTree(const std::string& prefix, int maxDepth = 4) const {
        std::string normPrefix = normalize(prefix);
        auto current = root;
        for (char ch : normPrefix) {
            auto it = current->children.find(ch);
            if (it == current->children.end()) {
                return VisualNode{"ROOT", false, "", {}};
            }
            current = it->second;
        }

        return buildVisualNode(normPrefix.empty() ? "ROOT" : std::string(1, normPrefix.back()), current, 0, maxDepth);
    }

    void clear() {
        root = std::make_shared<TrieNode>();
    }

private:
    std::shared_ptr<TrieNode> root;

    void collect(const std::shared_ptr<TrieNode>& node, std::vector<TrieStockItem>& results, size_t maxLimit) const {
        if (!node || results.size() >= maxLimit) return;

        if (node->isEndOfWord && node->stockItem) {
            // Deduplicate if already added
            bool exists = false;
            for (const auto& r : results) {
                if (r.symbol == node->stockItem->symbol) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                results.push_back(*(node->stockItem));
            }
        }

        for (const auto& pair : node->children) {
            if (results.size() >= maxLimit) break;
            collect(pair.second, results, maxLimit);
        }
    }

    VisualNode buildVisualNode(const std::string& label, const std::shared_ptr<TrieNode>& node, int currentDepth, int maxDepth) const {
        VisualNode vNode;
        vNode.character = label;
        vNode.isTerminal = node->isEndOfWord;
        if (node->stockItem) {
            vNode.symbol = node->stockItem->symbol;
        }

        if (currentDepth < maxDepth) {
            for (const auto& pair : node->children) {
                std::string childChar(1, pair.first);
                vNode.children.push_back(buildVisualNode(childChar, pair.second, currentDepth + 1, maxDepth));
            }
        }
        return vNode;
    }
};

} // namespace tradex::dsa
