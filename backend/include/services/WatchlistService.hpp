#pragma once

#include "database/Database.hpp"
#include "models/Models.hpp"
#include "services/StockService.hpp"
#include <vector>
#include <string>

namespace tradex::services {

class WatchlistService {
public:
    static WatchlistService& getInstance() {
        static WatchlistService instance;
        return instance;
    }

    std::vector<models::Watchlist> getUserWatchlists(int userId) {
        auto& db = database::Database::getInstance();
        auto& stockService = StockService::getInstance();

        std::vector<models::Watchlist> watchlists;
        try {
            auto wRows = db.query("SELECT * FROM watchlists WHERE user_id = " + std::to_string(userId) + " ORDER BY id ASC;");
            for (const auto& wr : wRows) {
                models::Watchlist wl;
                wl.id = std::stoi(wr.at("id"));
                wl.userId = userId;
                wl.name = wr.at("name");
                wl.createdAt = wr.at("created_at");

                auto itemRows = db.query("SELECT s.* FROM watchlist_items wi "
                                         "JOIN stocks s ON wi.stock_id = s.id "
                                         "WHERE wi.watchlist_id = " + std::to_string(wl.id) +
                                         " ORDER BY s.symbol ASC;");
                for (const auto& ir : itemRows) {
                    models::Stock s;
                    s.id = std::stoi(ir.at("id"));
                    s.symbol = ir.at("symbol");
                    s.companyName = ir.at("company_name");
                    s.exchange = ir.at("exchange");
                    s.sector = ir.at("sector");
                    s.currentPrice = std::stod(ir.at("current_price"));
                    s.previousClose = std::stod(ir.at("previous_close"));
                    s.openPrice = std::stod(ir.at("open_price"));
                    s.dayHigh = std::stod(ir.at("day_high"));
                    s.dayLow = std::stod(ir.at("day_low"));
                    s.volume = std::stoll(ir.at("volume"));
                    s.marketCap = std::stod(ir.at("market_cap"));

                    // Update with live in-memory price if available
                    auto sOpt = stockService.getStockBySymbol(s.symbol);
                    if (sOpt) {
                        s.currentPrice = sOpt->currentPrice;
                        s.dayHigh = sOpt->dayHigh;
                        s.dayLow = sOpt->dayLow;
                        s.volume = sOpt->volume;
                    }
                    wl.items.push_back(s);
                }
                watchlists.push_back(wl);
            }
        } catch (...) {}
        return watchlists;
    }

    int createWatchlist(int userId, const std::string& name) {
        auto& db = database::Database::getInstance();
        std::string sql = "INSERT INTO watchlists (user_id, name) VALUES (" +
                          std::to_string(userId) + ", '" + db.escape(name) + "');";
        return static_cast<int>(db.insertAndGetId(sql));
    }

    bool renameWatchlist(int userId, int watchlistId, const std::string& newName) {
        auto& db = database::Database::getInstance();
        std::string sql = "UPDATE watchlists SET name = '" + db.escape(newName) +
                          "' WHERE id = " + std::to_string(watchlistId) + " AND user_id = " + std::to_string(userId) + ";";
        return db.execute(sql) > 0;
    }

    bool deleteWatchlist(int userId, int watchlistId) {
        auto& db = database::Database::getInstance();
        std::string sql = "DELETE FROM watchlists WHERE id = " + std::to_string(watchlistId) +
                          " AND user_id = " + std::to_string(userId) + ";";
        return db.execute(sql) > 0;
    }

    bool addStockToWatchlist(int watchlistId, int stockId) {
        auto& db = database::Database::getInstance();
        try {
            std::string sql = "INSERT IGNORE INTO watchlist_items (watchlist_id, stock_id) VALUES (" +
                              std::to_string(watchlistId) + ", " + std::to_string(stockId) + ");";
            return db.execute(sql) > 0;
        } catch (...) {
            return false;
        }
    }

    bool removeStockFromWatchlist(int watchlistId, int stockId) {
        auto& db = database::Database::getInstance();
        std::string sql = "DELETE FROM watchlist_items WHERE watchlist_id = " + std::to_string(watchlistId) +
                          " AND stock_id = " + std::to_string(stockId) + ";";
        return db.execute(sql) > 0;
    }

private:
    WatchlistService() = default;
};

} // namespace tradex::services
