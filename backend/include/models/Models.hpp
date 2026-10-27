#pragma once

#include <string>
#include <vector>

namespace tradex::models {

struct User {
    int id{0};
    std::string username;
    std::string fullName;
    std::string email;
    std::string phone;
    std::string passwordHash;
    std::string createdAt;
    std::string status{"ACTIVE"};
    std::string role{"USER"}; // "USER" or "ADMIN"
};

struct Stock {
    int id{0};
    std::string symbol;
    std::string companyName;
    std::string exchange{"NSE"};
    std::string sector;
    double currentPrice{0.0};
    double previousClose{0.0};
    double openPrice{0.0};
    double dayHigh{0.0};
    double dayLow{0.0};
    int64_t volume{0};
    double marketCap{0.0};
    std::string updatedAt;

    double change() const {
        return currentPrice - previousClose;
    }

    double changePercent() const {
        return (previousClose > 0.0) ? ((currentPrice - previousClose) / previousClose) * 100.0 : 0.0;
    }
};

struct Order {
    int id{0};
    int userId{0};
    int stockId{0};
    std::string symbol;
    std::string orderType; // "MARKET", "LIMIT", "STOP_LOSS"
    std::string transactionType; // "BUY", "SELL"
    int quantity{0};
    double price{0.0};
    double triggerPrice{0.0};
    std::string status{"PENDING"}; // "PENDING", "EXECUTED", "CANCELLED", "REJECTED"
    std::string createdAt;
    std::string executedAt;
};

struct Holding {
    int id{0};
    int userId{0};
    int stockId{0};
    std::string symbol;
    std::string companyName;
    int quantity{0};
    double averagePrice{0.0};
    double currentPrice{0.0};
    
    double investedValue() const {
        return quantity * averagePrice;
    }

    double currentValue() const {
        return quantity * currentPrice;
    }

    double pnl() const {
        return currentValue() - investedValue();
    }

    double pnlPercent() const {
        return (investedValue() > 0.0) ? (pnl() / investedValue()) * 100.0 : 0.0;
    }
};

struct Position {
    int id{0};
    int userId{0};
    int stockId{0};
    std::string symbol;
    std::string companyName;
    int quantity{0};
    double averagePrice{0.0};
    double currentPrice{0.0};
    double realizedPnl{0.0};
    
    double unrealizedPnl() const {
        return (currentPrice - averagePrice) * quantity;
    }

    double totalPnl() const {
        return realizedPnl + unrealizedPnl();
    }
};

struct Wallet {
    int id{0};
    int userId{0};
    double availableBalance{100000.00};
    double investedAmount{0.00};
    double withdrawableAmount{100000.00};
    std::string updatedAt;

    double totalAccountValue() const {
        return availableBalance + investedAmount;
    }
};

struct WatchlistItem {
    int id{0};
    int watchlistId{0};
    int stockId{0};
    Stock stock;
};

struct Watchlist {
    int id{0};
    int userId{0};
    std::string name;
    std::string createdAt;
    std::vector<Stock> items;
};

struct Transaction {
    int id{0};
    int userId{0};
    int orderId{0};
    std::string type; // "BUY", "SELL", "DEPOSIT", "WITHDRAWAL"
    double amount{0.0};
    std::string description;
    std::string createdAt;
};

struct Notification {
    int id{0};
    int userId{0};
    std::string title;
    std::string message;
    std::string type{"SYSTEM"}; // "ORDER", "WALLET", "MARKET", "SYSTEM"
    bool isRead{false};
    std::string createdAt;
};

struct PriceHistoryPoint {
    int id{0};
    int stockId{0};
    std::string timestamp;
    double open{0.0};
    double high{0.0};
    double low{0.0};
    double close{0.0};
    int64_t volume{0};
};

} // namespace tradex::models
