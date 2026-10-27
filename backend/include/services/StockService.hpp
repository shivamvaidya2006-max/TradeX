#pragma once

#include "database/Database.hpp"
#include "models/Models.hpp"
#include "dsa/Trie.hpp"
#include "dsa/MarketGraph.hpp"
#include "dsa/SortingAlgorithms.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <iostream>

namespace tradex::services {

class StockService {
public:
    static StockService& getInstance() {
        static StockService instance;
        return instance;
    }

    void loadStocksFromDatabase() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& db = database::Database::getInstance();
        try {
            auto rows = db.query("SELECT * FROM stocks ORDER BY symbol ASC;");
            stocksBySymbol_.clear();
            stocksById_.clear();
            stocksList_.clear();
            trie_.clear();

            for (const auto& r : rows) {
                models::Stock s;
                s.id = std::stoi(r.at("id"));
                s.symbol = r.at("symbol");
                s.companyName = r.at("company_name");
                s.exchange = r.at("exchange");
                s.sector = r.at("sector");
                s.currentPrice = std::stod(r.at("current_price"));
                s.previousClose = std::stod(r.at("previous_close"));
                s.openPrice = std::stod(r.at("open_price"));
                s.dayHigh = std::stod(r.at("day_high"));
                s.dayLow = std::stod(r.at("day_low"));
                s.volume = std::stoll(r.at("volume"));
                s.marketCap = std::stod(r.at("market_cap"));
                s.updatedAt = r.at("updated_at");

                stocksBySymbol_[s.symbol] = s;
                stocksById_[s.id] = s;
                stocksList_.push_back(s);

                // Index in Trie for fast autocomplete
                dsa::TrieStockItem tItem{s.symbol, s.companyName, s.sector, s.currentPrice};
                trie_.insert(s.symbol, tItem);
                trie_.insert(s.companyName, tItem);

                // Index in Market Graph
                marketGraph_.addStock(s.symbol, s.companyName, s.sector, s.currentPrice);
            }
            std::cout << "[StockService] Loaded " << stocksList_.size() << " stocks into memory & Trie." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[StockService Error] " << e.what() << std::endl;
        }
    }

    std::vector<models::Stock> getAllStocks() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return stocksList_;
    }

    std::optional<models::Stock> getStockBySymbol(const std::string& symbol) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stocksBySymbol_.find(symbol);
        if (it != stocksBySymbol_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<models::Stock> getStockById(int id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stocksById_.find(id);
        if (it != stocksById_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
     * @brief Autocomplete search using O(L) Trie algorithm!
     */
    std::vector<dsa::TrieStockItem> searchStocks(const std::string& query, size_t limit = 10) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return trie_.suggest(query, limit);
    }

    const dsa::Trie& getTrie() const {
        return trie_;
    }

    const dsa::MarketGraph& getMarketGraph() const {
        return marketGraph_;
    }

    /**
     * @brief Fetches historical prices for a stock from DB.
     */
    std::vector<models::PriceHistoryPoint> getHistory(int stockId, const std::string& range = "1M") const {
        auto& db = database::Database::getInstance();
        int limitDays = 30;
        if (range == "1D") limitDays = 1;
        else if (range == "1W") limitDays = 7;
        else if (range == "1M") limitDays = 30;
        else if (range == "6M") limitDays = 180;
        else if (range == "1Y") limitDays = 365;
        else if (range == "5Y") limitDays = 1825;

        std::string sql = "SELECT * FROM price_history WHERE stock_id = " + std::to_string(stockId) +
                          " ORDER BY timestamp ASC LIMIT " + std::to_string(limitDays) + ";";

        std::vector<models::PriceHistoryPoint> history;
        try {
            auto rows = db.query(sql);
            for (const auto& r : rows) {
                models::PriceHistoryPoint pt;
                pt.id = std::stoi(r.at("id"));
                pt.stockId = std::stoi(r.at("stock_id"));
                pt.timestamp = r.at("timestamp");
                pt.open = std::stod(r.at("open_price"));
                pt.high = std::stod(r.at("high_price"));
                pt.low = std::stod(r.at("low_price"));
                pt.close = std::stod(r.at("close_price"));
                pt.volume = std::stoll(r.at("volume"));
                history.push_back(pt);
            }
        } catch (...) {}
        return history;
    }

    /**
     * @brief Updates stock price in memory and DB (called by matching engine & market simulator).
     */
    void updateStockPrice(const std::string& symbol, double newPrice, int64_t additionalVolume = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stocksBySymbol_.find(symbol);
        if (it != stocksBySymbol_.end()) {
            it->second.currentPrice = newPrice;
            if (newPrice > it->second.dayHigh) it->second.dayHigh = newPrice;
            if (newPrice < it->second.dayLow || it->second.dayLow <= 0) it->second.dayLow = newPrice;
            it->second.volume += additionalVolume;

            stocksById_[it->second.id] = it->second;

            for (auto& s : stocksList_) {
                if (s.symbol == symbol) {
                    s = it->second;
                    break;
                }
            }

            // Asynchronously or quickly write update to MySQL
            try {
                auto& db = database::Database::getInstance();
                std::string sql = "UPDATE stocks SET current_price = " + std::to_string(newPrice) +
                                  ", day_high = " + std::to_string(it->second.dayHigh) +
                                  ", day_low = " + std::to_string(it->second.dayLow) +
                                  ", volume = " + std::to_string(it->second.volume) +
                                  " WHERE id = " + std::to_string(it->second.id) + ";";
                db.execute(sql);
            } catch (...) {}
        }
    }

    /**
     * @brief Computes market overview indices and top movers using Sorting algorithms.
     */
    struct MarketOverview {
        double nifty50{24850.40};
        double niftyChange{142.30};
        double niftyChangePercent{0.58};

        double bankNifty{51230.10};
        double bankNiftyChange{-84.20};
        double bankNiftyChangePercent{-0.16};

        double sensex{81420.75};
        double sensexChange{415.80};
        double sensexChangePercent{0.51};

        std::vector<models::Stock> topGainers;
        std::vector<models::Stock> topLosers;
        std::vector<models::Stock> mostActive;
    };

    MarketOverview getMarketOverview() const {
        std::lock_guard<std::mutex> lock(mutex_);
        MarketOverview ov;
        if (stocksList_.empty()) return ov;

        auto sortedByGain = stocksList_;
        std::sort(sortedByGain.begin(), sortedByGain.end(), [](const models::Stock& a, const models::Stock& b) {
            return a.changePercent() > b.changePercent();
        });

        for (size_t i = 0; i < std::min<size_t>(5, sortedByGain.size()); ++i) {
            ov.topGainers.push_back(sortedByGain[i]);
        }

        for (int i = static_cast<int>(sortedByGain.size()) - 1; i >= std::max<int>(0, static_cast<int>(sortedByGain.size()) - 5); --i) {
            ov.topLosers.push_back(sortedByGain[i]);
        }

        auto sortedByVol = stocksList_;
        std::sort(sortedByVol.begin(), sortedByVol.end(), [](const models::Stock& a, const models::Stock& b) {
            return a.volume > b.volume;
        });

        for (size_t i = 0; i < std::min<size_t>(5, sortedByVol.size()); ++i) {
            ov.mostActive.push_back(sortedByVol[i]);
        }

        return ov;
    }

private:
    StockService() = default;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, models::Stock> stocksBySymbol_;
    std::unordered_map<int, models::Stock> stocksById_;
    std::vector<models::Stock> stocksList_;
    dsa::Trie trie_;
    dsa::MarketGraph marketGraph_;
};

} // namespace tradex::services
