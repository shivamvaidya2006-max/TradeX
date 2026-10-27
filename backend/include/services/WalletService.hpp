#pragma once

#include "database/Database.hpp"
#include "models/Models.hpp"
#include <vector>

namespace tradex::services {

struct WalletOperationResult {
    bool success{false};
    std::string message;
    models::Wallet wallet;
};

class WalletService {
public:
    static WalletService& getInstance() {
        static WalletService instance;
        return instance;
    }

    models::Wallet getWallet(int userId) {
        auto& db = database::Database::getInstance();
        models::Wallet w;
        w.userId = userId;

        auto rows = db.query("SELECT * FROM wallet WHERE user_id = " + std::to_string(userId) + ";");
        if (!rows.empty()) {
            w.id = std::stoi(rows[0].at("id"));
            w.availableBalance = std::stod(rows[0].at("available_balance"));
            w.investedAmount = std::stod(rows[0].at("invested_amount"));
            w.withdrawableAmount = std::stod(rows[0].at("withdrawable_amount"));
        } else {
            // Default 100k
            db.execute("INSERT INTO wallet (user_id, available_balance, invested_amount, withdrawable_amount) VALUES (" +
                       std::to_string(userId) + ", 100000.00, 0.00, 100000.00);");
            w.availableBalance = 100000.00;
            w.withdrawableAmount = 100000.00;
        }
        return w;
    }

    WalletOperationResult deposit(int userId, double amount) {
        if (amount <= 0) {
            return {false, "Deposit amount must be greater than zero.", getWallet(userId)};
        }
        auto& db = database::Database::getInstance();
        db.execute("UPDATE wallet SET available_balance = available_balance + " + std::to_string(amount) +
                   ", withdrawable_amount = withdrawable_amount + " + std::to_string(amount) +
                   " WHERE user_id = " + std::to_string(userId) + ";");

        db.execute("INSERT INTO transactions (user_id, type, amount, description) VALUES (" +
                   std::to_string(userId) + ", 'DEPOSIT', " + std::to_string(amount) +
                   ", 'Simulated funds deposit into TradeX wallet');");

        db.execute("INSERT INTO notifications (user_id, title, message, type, is_read) VALUES (" +
                   std::to_string(userId) + ", 'Funds Credited', '₹" + std::to_string(amount) +
                   " successfully credited to your simulated wallet.', 'WALLET', 0);");

        return {true, "Deposit of ₹" + std::to_string(amount) + " successful.", getWallet(userId)};
    }

    WalletOperationResult withdraw(int userId, double amount) {
        if (amount <= 0) {
            return {false, "Withdrawal amount must be greater than zero.", getWallet(userId)};
        }
        auto currentWallet = getWallet(userId);
        if (currentWallet.withdrawableAmount < amount) {
            return {false, "Insufficient withdrawable balance.", currentWallet};
        }

        auto& db = database::Database::getInstance();
        db.execute("UPDATE wallet SET available_balance = available_balance - " + std::to_string(amount) +
                   ", withdrawable_amount = withdrawable_amount - " + std::to_string(amount) +
                   " WHERE user_id = " + std::to_string(userId) + ";");

        db.execute("INSERT INTO transactions (user_id, type, amount, description) VALUES (" +
                   std::to_string(userId) + ", 'WITHDRAWAL', " + std::to_string(amount) +
                   ", 'Simulated funds withdrawal from TradeX wallet');");

        db.execute("INSERT INTO notifications (user_id, title, message, type, is_read) VALUES (" +
                   std::to_string(userId) + ", 'Funds Withdrawn', '₹" + std::to_string(amount) +
                   " successfully debited from your simulated wallet.', 'WALLET', 0);");

        return {true, "Withdrawal of ₹" + std::to_string(amount) + " successful.", getWallet(userId)};
    }

    std::vector<models::Transaction> getTransactions(int userId) {
        auto& db = database::Database::getInstance();
        std::vector<models::Transaction> txs;
        try {
            auto rows = db.query("SELECT * FROM transactions WHERE user_id = " + std::to_string(userId) +
                                 " ORDER BY created_at DESC LIMIT 50;");
            for (const auto& r : rows) {
                models::Transaction t;
                t.id = std::stoi(r.at("id"));
                t.userId = std::stoi(r.at("user_id"));
                t.orderId = r.at("order_id").empty() ? 0 : std::stoi(r.at("order_id"));
                t.type = r.at("type");
                t.amount = std::stod(r.at("amount"));
                t.description = r.at("description");
                t.createdAt = r.at("created_at");
                txs.push_back(t);
            }
        } catch (...) {}
        return txs;
    }

private:
    WalletService() = default;
};

} // namespace tradex::services
