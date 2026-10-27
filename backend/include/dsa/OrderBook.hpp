#pragma once

#include <queue>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <mutex>
#include <iomanip>
#include <sstream>

namespace tradex::dsa {

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderCategory {
    MARKET,
    LIMIT,
    STOP_LOSS
};

struct LimitOrder {
    int id{0};
    int userId{0};
    std::string symbol;
    OrderSide side{OrderSide::BUY};
    OrderCategory category{OrderCategory::LIMIT};
    int quantity{0};
    int initialQuantity{0};
    double price{0.0};
    double triggerPrice{0.0};
    uint64_t timestamp{0}; // milliseconds epoch for FIFO tie-breaker

    LimitOrder() = default;

    LimitOrder(int id, int userId, std::string sym, OrderSide side, OrderCategory cat, int qty, double pr, double tr = 0.0)
        : id(id), userId(userId), symbol(std::move(sym)), side(side), category(cat),
          quantity(qty), initialQuantity(qty), price(pr), triggerPrice(tr) {
        timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
};

/**
 * @brief Comparator for BUY orders:
 * Max-Heap priority: Highest price first.
 * Tie-breaker: Earlier timestamp first (FIFO).
 */
struct BuyOrderComparator {
    bool operator()(const std::shared_ptr<LimitOrder>& a, const std::shared_ptr<LimitOrder>& b) const {
        if (a->price != b->price) {
            return a->price < b->price; // max heap: top has highest price
        }
        return a->timestamp > b->timestamp; // FIFO: smaller timestamp has higher priority
    }
};

/**
 * @brief Comparator for SELL orders:
 * Min-Heap priority: Lowest price first.
 * Tie-breaker: Earlier timestamp first (FIFO).
 */
struct SellOrderComparator {
    bool operator()(const std::shared_ptr<LimitOrder>& a, const std::shared_ptr<LimitOrder>& b) const {
        if (a->price != b->price) {
            return a->price > b->price; // min heap: top has lowest price
        }
        return a->timestamp > b->timestamp; // FIFO: smaller timestamp has higher priority
    }
};

/**
 * @brief Order Book snapshot item for UI rendering
 */
struct OrderBookLevel {
    double price{0.0};
    int quantity{0};
    int orderCount{0};
};

struct OrderBookSnapshot {
    std::string symbol;
    std::vector<OrderBookLevel> buyLevels;
    std::vector<OrderBookLevel> sellLevels;
    double ltp{0.0};
    double spread{0.0};
};

/**
 * @brief Stock Order Book using Priority Queues (Heaps)
 * 
 * Demonstrates:
 * - std::priority_queue Max-Heap for Buy side
 * - std::priority_queue Min-Heap for Sell side
 * - Thread-safe operations via std::mutex
 * - O(log N) insertion and extraction
 */
class OrderBook {
public:
    explicit OrderBook(std::string symbol) : symbol(std::move(symbol)) {}

    void addOrder(const std::shared_ptr<LimitOrder>& order) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (order->side == OrderSide::BUY) {
            buyQueue_.push(order);
        } else {
            sellQueue_.push(order);
        }
    }

    bool hasBuyOrders() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !buyQueue_.empty();
    }

    bool hasSellOrders() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !sellQueue_.empty();
    }

    std::shared_ptr<LimitOrder> getBestBuy() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buyQueue_.empty()) return nullptr;
        return buyQueue_.top();
    }

    std::shared_ptr<LimitOrder> getBestSell() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (sellQueue_.empty()) return nullptr;
        return sellQueue_.top();
    }

    void popBuy() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!buyQueue_.empty()) buyQueue_.pop();
    }

    void popSell() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!sellQueue_.empty()) sellQueue_.pop();
    }

    size_t buyCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buyQueue_.size();
    }

    size_t sellCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sellQueue_.size();
    }

    /**
     * @brief Generates market depth snapshot (top 5 buy bids and top 5 sell asks)
     */
    OrderBookSnapshot getSnapshot(size_t maxDepth = 5) const {
        std::lock_guard<std::mutex> lock(mutex_);
        OrderBookSnapshot snapshot;
        snapshot.symbol = symbol;

        // Copy heaps for non-destructive snapshot traversal
        auto buyCopy = buyQueue_;
        auto sellCopy = sellQueue_;

        std::vector<std::shared_ptr<LimitOrder>> bids;
        while (!buyCopy.empty() && bids.size() < 20) {
            bids.push_back(buyCopy.top());
            buyCopy.pop();
        }

        std::vector<std::shared_ptr<LimitOrder>> asks;
        while (!sellCopy.empty() && asks.size() < 20) {
            asks.push_back(sellCopy.top());
            sellCopy.pop();
        }

        // Aggregate by price level
        snapshot.buyLevels = aggregateLevels(bids, maxDepth);
        snapshot.sellLevels = aggregateLevels(asks, maxDepth);

        if (!snapshot.buyLevels.empty() && !snapshot.sellLevels.empty()) {
            snapshot.spread = snapshot.sellLevels.front().price - snapshot.buyLevels.front().price;
        }

        return snapshot;
    }

private:
    std::string symbol;
    mutable std::mutex mutex_;
    std::priority_queue<std::shared_ptr<LimitOrder>, std::vector<std::shared_ptr<LimitOrder>>, BuyOrderComparator> buyQueue_;
    std::priority_queue<std::shared_ptr<LimitOrder>, std::vector<std::shared_ptr<LimitOrder>>, SellOrderComparator> sellQueue_;

    static std::vector<OrderBookLevel> aggregateLevels(const std::vector<std::shared_ptr<LimitOrder>>& orders, size_t maxDepth) {
        std::vector<OrderBookLevel> levels;
        for (const auto& ord : orders) {
            bool found = false;
            for (auto& lvl : levels) {
                if (std::abs(lvl.price - ord->price) < 0.001) {
                    lvl.quantity += ord->quantity;
                    lvl.orderCount++;
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (levels.size() < maxDepth) {
                    levels.push_back({ord->price, ord->quantity, 1});
                }
            }
        }
        return levels;
    }
};

} // namespace tradex::dsa
