#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <iostream>

namespace tradex::dsa {

struct GraphNodeInfo {
    std::string id;
    std::string label;
    std::string type; // "SECTOR" or "STOCK"
    double currentPrice{0.0};
};

struct GraphEdgeInfo {
    std::string source;
    std::string target;
    std::string relationship;
};

/**
 * @brief Graph representation of Sector <-> Stock Market Relationships.
 * 
 * Complexity:
 * - BFS: O(V + E)
 * - DFS: O(V + E)
 * - Space: O(V + E)
 */
class MarketGraph {
public:
    MarketGraph() = default;

    void addSector(const std::string& sector) {
        if (nodes_.find(sector) == nodes_.end()) {
            nodes_[sector] = GraphNodeInfo{sector, sector, "SECTOR", 0.0};
            adjacencyList_[sector] = {};
        }
    }

    void addStock(const std::string& symbol, const std::string& companyName, const std::string& sector, double price) {
        addSector(sector);
        nodes_[symbol] = GraphNodeInfo{symbol, companyName, "STOCK", price};
        
        // Undirected edge between sector and stock
        adjacencyList_[sector].push_back(symbol);
        adjacencyList_[symbol].push_back(sector);
        edges_.push_back(GraphEdgeInfo{sector, symbol, "BELONGS_TO"});
    }

    /**
     * @brief Breadth-First Search (BFS) starting from startNode.
     * Explores level-by-level using a FIFO queue.
     */
    std::vector<std::string> bfs(const std::string& startNode) const {
        std::vector<std::string> traversalOrder;
        if (adjacencyList_.find(startNode) == adjacencyList_.end()) return traversalOrder;

        std::unordered_set<std::string> visited;
        std::queue<std::string> q;

        visited.insert(startNode);
        q.push(startNode);

        while (!q.empty()) {
            std::string curr = q.front();
            q.pop();
            traversalOrder.push_back(curr);

            auto it = adjacencyList_.find(curr);
            if (it != adjacencyList_.end()) {
                for (const auto& neighbor : it->second) {
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        q.push(neighbor);
                    }
                }
            }
        }
        return traversalOrder;
    }

    /**
     * @brief Depth-First Search (DFS) starting from startNode.
     * Explores deep along each branch using recursion / LIFO stack.
     */
    std::vector<std::string> dfs(const std::string& startNode) const {
        std::vector<std::string> traversalOrder;
        if (adjacencyList_.find(startNode) == adjacencyList_.end()) return traversalOrder;

        std::unordered_set<std::string> visited;
        dfsHelper(startNode, visited, traversalOrder);
        return traversalOrder;
    }

    std::vector<GraphNodeInfo> getAllNodes() const {
        std::vector<GraphNodeInfo> result;
        for (const auto& pair : nodes_) {
            result.push_back(pair.second);
        }
        return result;
    }

    std::vector<GraphEdgeInfo> getAllEdges() const {
        return edges_;
    }

private:
    std::unordered_map<std::string, GraphNodeInfo> nodes_;
    std::unordered_map<std::string, std::vector<std::string>> adjacencyList_;
    std::vector<GraphEdgeInfo> edges_;

    void dfsHelper(const std::string& node, std::unordered_set<std::string>& visited, std::vector<std::string>& result) const {
        visited.insert(node);
        result.push_back(node);

        auto it = adjacencyList_.find(node);
        if (it != adjacencyList_.end()) {
            for (const auto& neighbor : it->second) {
                if (visited.find(neighbor) == visited.end()) {
                    dfsHelper(neighbor, visited, result);
                }
            }
        }
    }
};

} // namespace tradex::dsa
