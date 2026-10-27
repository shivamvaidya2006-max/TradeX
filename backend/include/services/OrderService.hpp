#pragma once

#include "database/Database.hpp"
#include "models/Models.hpp"
#include "dsa/OrderBook.hpp"
#include "dsa/MatchingEngine.hpp"
#include "dsa/LinkedList.hpp"
#include "dsa/Stack.hpp"
#include "dsa/OrderQueue.hpp"
#include "services/StockService.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <iostream>
#include <chrono>

namespace tradex::services {

struct OrderPlacementRequest {
    int userId{0};
    std::string symbol;
    std::string transactionType; // "BUY" or "SELL"
    std::string orderType;       // "MARKET", "LIMIT", "STOP_LOSS"
    int quantity{0};
    double price{0.0};
    double triggerPrice{0.0};
};

struct OrderPlacementResult {
    bool success{false};
    std::string message;
    int orderId{0};
    std::string status;
    double executionPrice{0.0};
    int executedQuantity{0};
    double totalCharges{0.0};
};

class OrderService {
public:
    static OrderService& getInstance() {
        static OrderService instance;
        return instance;
    }

    /**
     * @brief Computes simulated exchange fees, STT, GST, and brokerage.
     */
    static double calculateCharges(double tradeValue) {
        double brokerage = 20.0; // flat ₹20 or 0.03%
        double stt = tradeValue * 0.001; // 0.1% STT
        double exchangeTurnover = tradeValue * 0.0000345;
        double gst = (brokerage + exchangeTurnover) * 0.18;
        return std::round((brokerage + stt + exchangeTurnover + gst) * 100.0) / 100.0;
    }

    /**
     * @brief Places an order, validates balance/holdings, matches via priority queue, and persists changes.
     */
    OrderPlacementResult placeOrder(const OrderPlacementRequest& req) {
        std::lock_guard<std::mutex> lock(orderMutex_);
        auto& db = database::Database::getInstance();
        auto& stockService = StockService::getInstance();

        // 1. Validation
        if (req.quantity <= 0) {
            return {false, "Quantity must be greater than zero.", 0, "REJECTED"};
        }

        auto stockOpt = stockService.getStockBySymbol(req.symbol);
        if (!stockOpt) {
            return {false, "Invalid stock symbol.", 0, "REJECTED"};
        }
        auto stock = *stockOpt;

        double targetPrice = req.price;
        if (req.orderType == "MARKET" || targetPrice <= 0) {
            targetPrice = stock.currentPrice;
        }

        double tradeValue = req.quantity * targetPrice;
        double charges = calculateCharges(tradeValue);

        // 2. Fund Validation for BUY
        if (req.transactionType == "BUY") {
            auto walletRows = db.query("SELECT available_balance FROM wallet WHERE user_id = " + std::to_string(req.userId) + ";");
            if (walletRows.empty()) {
                return {false, "User wallet not found.", 0, "REJECTED"};
            }
            double balance = std::stod(walletRows[0].at("available_balance"));
            double totalRequired = tradeValue + charges;

            if (balance < totalRequired) {
                return {false, "Insufficient funds. Required: ₹" + std::to_string(totalRequired) + ", Available: ₹" + std::to_string(balance), 0, "REJECTED"};
            }
        } 
        // 3. Holdings Validation for SELL
        else if (req.transactionType == "SELL") {
            auto holdingRows = db.query("SELECT quantity FROM holdings WHERE user_id = " + std::to_string(req.userId) +
                                        " AND stock_id = " + std::to_string(stock.id) + ";");
            int ownedQty = holdingRows.empty() ? 0 : std::stoi(holdingRows[0].at("quantity"));
            if (ownedQty < req.quantity) {
                return {false, "Insufficient holdings. Owned: " + std::to_string(ownedQty) + ", Attempting to sell: " + std::to_string(req.quantity), 0, "REJECTED"};
            }
        } else {
            return {false, "Invalid transaction type (must be BUY or SELL).", 0, "REJECTED"};
        }

        // 4. Insert Order into MySQL
        std::string orderInsertSql = "INSERT INTO orders (user_id, stock_id, order_type, transaction_type, quantity, price, trigger_price, status) VALUES (" +
                                     std::to_string(req.userId) + ", " + std::to_string(stock.id) + ", '" +
                                     db.escape(req.orderType) + "', '" + db.escape(req.transactionType) + "', " +
                                     std::to_string(req.quantity) + ", " + std::to_string(targetPrice) + ", " +
                                     std::to_string(req.triggerPrice) + ", 'PENDING');";
        
        int64_t orderId = db.insertAndGetId(orderInsertSql);
        if (orderId <= 0) {
            return {false, "Failed to create order record.", 0, "REJECTED"};
        }

        // 5. Submit to In-Memory Priority Queue OrderBook
        auto& orderBook = getOrCreateOrderBook(stock.symbol);
        dsa::OrderSide side = (req.transactionType == "BUY") ? dsa::OrderSide::BUY : dsa::OrderSide::SELL;
        dsa::OrderCategory cat = (req.orderType == "MARKET") ? dsa::OrderCategory::MARKET : dsa::OrderCategory::LIMIT;
        
        auto limitOrder = std::make_shared<dsa::LimitOrder>(
            static_cast<int>(orderId), req.userId, stock.symbol, side, cat, req.quantity, targetPrice, req.triggerPrice
        );

        orderBook.addOrder(limitOrder);

        // 6. Execute Matching Engine
        // For MARKET orders or crossing limits, simulate execution against market liquidity
        bool executed = false;
        double execPrice = targetPrice;
        int execQty = req.quantity;

        if (req.orderType == "MARKET" || (req.orderType == "LIMIT" && ((req.transactionType == "BUY" && targetPrice >= stock.currentPrice) || (req.transactionType == "SELL" && targetPrice <= stock.currentPrice)))) {
            executed = true;
            execPrice = stock.currentPrice;
            // Pop the order from book as executed
            if (side == dsa::OrderSide::BUY) orderBook.popBuy();
            else orderBook.popSell();
        } else {
            // Check if matches against existing opposing orders in heap
            auto trades = matchingEngine_.matchOrders(orderBook);
            if (!trades.empty()) {
                executed = true;
                execPrice = trades[0].executionPrice;
                execQty = trades[0].quantity;
            }
        }

        if (executed) {
            // Update order status in MySQL
            db.execute("UPDATE orders SET status = 'EXECUTED', executed_at = CURRENT_TIMESTAMP WHERE id = " + std::to_string(orderId) + ";");

            // Update Wallet, Holdings, Positions, and Transactions
            processExecution(req.userId, stock.id, stock.symbol, req.transactionType, execQty, execPrice, charges, static_cast<int>(orderId));

            // Log in DSA DoublyLinkedList and Stack
            std::string logEntry = (req.transactionType == "BUY" ? "BOUGHT " : "SOLD ") +
                                   std::to_string(execQty) + " " + stock.symbol + " @ ₹" + std::to_string(execPrice);
            auditTrail_.pushBack(logEntry);
            recentActionsStack_.push("ORDER_" + std::to_string(orderId));

            // Create user notification
            db.execute("INSERT INTO notifications (user_id, title, message, type, is_read) VALUES (" +
                       std::to_string(req.userId) + ", 'Order Executed: " + stock.symbol + "', '" + logEntry + "', 'ORDER', 0);");

            return {true, "Order executed successfully!", static_cast<int>(orderId), "EXECUTED", execPrice, execQty, charges};
        }

        // Limit order remains PENDING in heap
        auditTrail_.pushBack("PLACED LIMIT ORDER: " + stock.symbol + " " + std::to_string(req.quantity) + " @ ₹" + std::to_string(targetPrice));
        return {true, "Limit order queued in order book.", static_cast<int>(orderId), "PENDING", targetPrice, 0, charges};
    }

    /**
     * @brief Cancels an open pending order.
     */
    bool cancelOrder(int userId, int orderId) {
        std::lock_guard<std::mutex> lock(orderMutex_);
        auto& db = database::Database::getInstance();
        auto rows = db.query("SELECT * FROM orders WHERE id = " + std::to_string(orderId) +
                             " AND user_id = " + std::to_string(userId) + " AND status = 'PENDING';");
        if (rows.empty()) return false;

        db.execute("UPDATE orders SET status = 'CANCELLED' WHERE id = " + std::to_string(orderId) + ";");
        db.execute("INSERT INTO notifications (user_id, title, message, type, is_read) VALUES (" +
                   std::to_string(userId) + ", 'Order Cancelled', 'Your pending order #" + std::to_string(orderId) + " was cancelled.', 'ORDER', 0);");
        return true;
    }

    /**
     * @brief Fetches order book snapshot for UI / DSA Lab.
     */
    dsa::OrderBookSnapshot getOrderBookSnapshot(const std::string& symbol) {
        std::lock_guard<std::mutex> lock(orderMutex_);
        auto& book = getOrCreateOrderBook(symbol);
        auto snap = book.getSnapshot(5);
        auto stockOpt = StockService::getInstance().getStockBySymbol(symbol);
        if (stockOpt) {
            snap.ltp = stockOpt->currentPrice;
        }
        return snap;
    }

    const dsa::DoublyLinkedList<std::string>& getAuditTrail() const {
        return auditTrail_;
    }

private:
    OrderService() {
        matchingEngine_.setTradeCallback([this](const dsa::TradeExecution& trade) {
            std::cout << "[Matching Engine] Trade executed: " << trade.quantity 
                      << " of " << trade.symbol << " @ ₹" << trade.executionPrice << std::endl;
        });
    }

    mutable std::mutex orderMutex_;
    std::unordered_map<std::string, std::shared_ptr<dsa::OrderBook>> orderBooks_;
    dsa::MatchingEngine matchingEngine_;
    dsa::DoublyLinkedList<std::string> auditTrail_;
    dsa::ActionStack<std::string> recentActionsStack_;

    dsa::OrderBook& getOrCreateOrderBook(const std::string& symbol) {
        auto it = orderBooks_.find(symbol);
        if (it == orderBooks_.end()) {
            auto book = std::make_shared<dsa::OrderBook>(symbol);
            orderBooks_[symbol] = book;
            return *book;
        }
        return *(it->second);
    }

    void processExecution(int userId, int stockId, const std::string& symbol,
                          const std::string& side, int qty, double price, double charges, int orderId) {
        auto& db = database::Database::getInstance();
        double tradeValue = qty * price;

        if (side == "BUY") {
            double totalCost = tradeValue + charges;

            // 1. Deduct from wallet
            db.execute("UPDATE wallet SET available_balance = available_balance - " + std::to_string(totalCost) +
                       ", invested_amount = invested_amount + " + std::to_string(tradeValue) +
                       ", withdrawable_amount = available_balance - " + std::to_string(totalCost) +
                       " WHERE user_id = " + std::to_string(userId) + ";");

            // 2. Update Holdings
            auto hRows = db.query("SELECT id, quantity, average_price FROM holdings WHERE user_id = " +
                                  std::to_string(userId) + " AND stock_id = " + std::to_string(stockId) + ";");
            if (hRows.empty()) {
                db.execute("INSERT INTO holdings (user_id, stock_id, quantity, average_price) VALUES (" +
                           std::to_string(userId) + ", " + std::to_string(stockId) + ", " +
                           std::to_string(qty) + ", " + std::to_string(price) + ");");
            } else {
                int oldQty = std::stoi(hRows[0].at("quantity"));
                double oldAvg = std::stod(hRows[0].at("average_price"));
                int newQty = oldQty + qty;
                double newAvg = ((oldQty * oldAvg) + (qty * price)) / newQty;
                db.execute("UPDATE holdings SET quantity = " + std::to_string(newQty) +
                           ", average_price = " + std::to_string(newAvg) +
                           " WHERE id = " + hRows[0].at("id") + ";");
            }

            // 3. Update Positions
            auto pRows = db.query("SELECT id, quantity, average_price FROM positions WHERE user_id = " +
                                  std::to_string(userId) + " AND stock_id = " + std::to_string(stockId) + ";");
            if (pRows.empty()) {
                db.execute("INSERT INTO positions (user_id, stock_id, quantity, average_price, realized_pnl, unrealized_pnl) VALUES (" +
                           std::to_string(userId) + ", " + std::to_string(stockId) + ", " +
                           std::to_string(qty) + ", " + std::to_string(price) + ", 0.0, 0.0);");
            } else {
                int oldQty = std::stoi(pRows[0].at("quantity"));
                double oldAvg = std::stod(pRows[0].at("average_price"));
                int newQty = oldQty + qty;
                double newAvg = ((oldQty * oldAvg) + (qty * price)) / newQty;
                db.execute("UPDATE positions SET quantity = " + std::to_string(newQty) +
                           ", average_price = " + std::to_string(newAvg) +
                           " WHERE id = " + pRows[0].at("id") + ";");
            }

            // 4. Record Transaction
            db.execute("INSERT INTO transactions (user_id, order_id, type, amount, description) VALUES (" +
                       std::to_string(userId) + ", " + std::to_string(orderId) + ", 'BUY', " +
                       std::to_string(totalCost) + ", 'Bought " + std::to_string(qty) + " " + symbol + " @ ₹" + std::to_string(price) + "');");
        } 
        else if (side == "SELL") {
            double netProceeds = tradeValue - charges;

            // 1. Credit to wallet
            db.execute("UPDATE wallet SET available_balance = available_balance + " + std::to_string(netProceeds) +
                       ", withdrawable_amount = available_balance + " + std::to_string(netProceeds) +
                       " WHERE user_id = " + std::to_string(userId) + ";");

            // 2. Reduce holdings & calculate realized P&L
            auto hRows = db.query("SELECT id, quantity, average_price FROM holdings WHERE user_id = " +
                                  std::to_string(userId) + " AND stock_id = " + std::to_string(stockId) + ";");
            if (!hRows.empty()) {
                int oldQty = std::stoi(hRows[0].at("quantity"));
                double avgBuyPrice = std::stod(hRows[0].at("average_price"));
                int remainingQty = oldQty - qty;
                double realizedProfit = (price - avgBuyPrice) * qty - charges;

                if (remainingQty <= 0) {
                    db.execute("DELETE FROM holdings WHERE id = " + hRows[0].at("id") + ";");
                } else {
                    db.execute("UPDATE holdings SET quantity = " + std::to_string(remainingQty) +
                               " WHERE id = " + hRows[0].at("id") + ";");
                }

                // Update positions with realized P&L
                db.execute("UPDATE positions SET realized_pnl = realized_pnl + " + std::to_string(realizedProfit) +
                           ", quantity = GREATEST(0, quantity - " + std::to_string(qty) + ") " +
                           " WHERE user_id = " + std::to_string(userId) + " AND stock_id = " + std::to_string(stockId) + ";");

                // Adjust invested amount in wallet
                double investedReduction = qty * avgBuyPrice;
                db.execute("UPDATE wallet SET invested_amount = GREATEST(0, invested_amount - " + std::to_string(investedReduction) + ") " +
                           "WHERE user_id = " + std::to_string(userId) + ";");
            }

            // 3. Record Transaction
            db.execute("INSERT INTO transactions (user_id, order_id, type, amount, description) VALUES (" +
                       std::to_string(userId) + ", " + std::to_string(orderId) + ", 'SELL', " +
                       std::to_string(netProceeds) + ", 'Sold " + std::to_string(qty) + " " + symbol + " @ ₹" + std::to_string(price) + "');");
        }
    }
};

} // namespace tradex::services
