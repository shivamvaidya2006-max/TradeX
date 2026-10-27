#pragma once

#include "OrderBook.hpp"
#include <vector>
#include <functional>
#include <memory>
#include <chrono>

namespace tradex::dsa {

struct TradeExecution {
    int buyOrderId{0};
    int sellOrderId{0};
    int buyerId{0};
    int sellerId{0};
    std::string symbol;
    int quantity{0};
    double executionPrice{0.0};
    uint64_t executedAt{0};
};

/**
 * @brief Trade matching engine implementing continuous double auction.
 * Matches highest priority buy orders with lowest priority sell orders.
 */
class MatchingEngine {
public:
    using TradeCallback = std::function<void(const TradeExecution&)>;

    MatchingEngine() = default;

    void setTradeCallback(TradeCallback cb) {
        tradeCallback_ = std::move(cb);
    }

    /**
     * @brief Attempts to match orders in the given order book until no more matches are possible.
     * @return List of executed trades
     */
    std::vector<TradeExecution> matchOrders(OrderBook& orderBook) {
        std::vector<TradeExecution> executedTrades;

        while (orderBook.hasBuyOrders() && orderBook.hasSellOrders()) {
            auto bestBuy = orderBook.getBestBuy();
            auto bestSell = orderBook.getBestSell();

            if (!bestBuy || !bestSell) break;

            // Matching condition: Bid price >= Ask price
            if (bestBuy->price >= bestSell->price) {
                int matchedQty = std::min(bestBuy->quantity, bestSell->quantity);
                
                // Execution price: Maker price (the order that was placed first)
                double executionPrice = (bestBuy->timestamp <= bestSell->timestamp) 
                                      ? bestBuy->price 
                                      : bestSell->price;

                uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();

                TradeExecution trade;
                trade.buyOrderId = bestBuy->id;
                trade.sellOrderId = bestSell->id;
                trade.buyerId = bestBuy->userId;
                trade.sellerId = bestSell->userId;
                trade.symbol = bestBuy->symbol;
                trade.quantity = matchedQty;
                trade.executionPrice = executionPrice;
                trade.executedAt = now;

                executedTrades.push_back(trade);

                if (tradeCallback_) {
                    tradeCallback_(trade);
                }

                // Update remaining quantities
                bestBuy->quantity -= matchedQty;
                bestSell->quantity -= matchedQty;

                if (bestBuy->quantity == 0) {
                    orderBook.popBuy();
                }
                if (bestSell->quantity == 0) {
                    orderBook.popSell();
                }
            } else {
                // Highest bid is lower than lowest ask - no match possible
                break;
            }
        }

        return executedTrades;
    }

private:
    TradeCallback tradeCallback_;
};

} // namespace tradex::dsa
