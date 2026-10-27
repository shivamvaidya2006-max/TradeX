#pragma once

#include "database/Database.hpp"
#include "models/Models.hpp"
#include "services/StockService.hpp"
#include "services/PnLService.hpp"
#include <vector>

namespace tradex::services {

struct PortfolioData {
    std::vector<models::Holding> holdings;
    std::vector<models::Position> positions;
    PnLSummary summary;
    models::Wallet wallet;
};

class PortfolioService {
public:
    static PortfolioService& getInstance() {
        static PortfolioService instance;
        return instance;
    }

    std::vector<models::Holding> getUserHoldings(int userId) {
        auto& db = database::Database::getInstance();
        auto& stockService = StockService::getInstance();

        std::string sql = "SELECT h.id, h.user_id, h.stock_id, h.quantity, h.average_price, "
                          "s.symbol, s.company_name, s.current_price FROM holdings h "
                          "JOIN stocks s ON h.stock_id = s.id WHERE h.user_id = " + std::to_string(userId) +
                          " AND h.quantity > 0 ORDER BY s.symbol ASC;";

        std::vector<models::Holding> holdings;
        try {
            auto rows = db.query(sql);
            for (const auto& r : rows) {
                models::Holding h;
                h.id = std::stoi(r.at("id"));
                h.userId = std::stoi(r.at("user_id"));
                h.stockId = std::stoi(r.at("stock_id"));
                h.quantity = std::stoi(r.at("quantity"));
                h.averagePrice = std::stod(r.at("average_price"));
                h.symbol = r.at("symbol");
                h.companyName = r.at("company_name");

                // Get live memory price from StockService
                auto sOpt = stockService.getStockBySymbol(h.symbol);
                h.currentPrice = sOpt ? sOpt->currentPrice : std::stod(r.at("current_price"));

                holdings.push_back(h);
            }
        } catch (...) {}
        return holdings;
    }

    std::vector<models::Position> getUserPositions(int userId) {
        auto& db = database::Database::getInstance();
        auto& stockService = StockService::getInstance();

        std::string sql = "SELECT p.id, p.user_id, p.stock_id, p.quantity, p.average_price, p.realized_pnl, "
                          "s.symbol, s.company_name, s.current_price FROM positions p "
                          "JOIN stocks s ON p.stock_id = s.id WHERE p.user_id = " + std::to_string(userId) +
                          " ORDER BY s.symbol ASC;";

        std::vector<models::Position> positions;
        try {
            auto rows = db.query(sql);
            for (const auto& r : rows) {
                models::Position p;
                p.id = std::stoi(r.at("id"));
                p.userId = std::stoi(r.at("user_id"));
                p.stockId = std::stoi(r.at("stock_id"));
                p.quantity = std::stoi(r.at("quantity"));
                p.averagePrice = std::stod(r.at("average_price"));
                p.realizedPnl = std::stod(r.at("realized_pnl"));
                p.symbol = r.at("symbol");
                p.companyName = r.at("company_name");

                auto sOpt = stockService.getStockBySymbol(p.symbol);
                p.currentPrice = sOpt ? sOpt->currentPrice : std::stod(r.at("current_price"));

                positions.push_back(p);
            }
        } catch (...) {}
        return positions;
    }

    PortfolioData getCompletePortfolio(int userId) {
        PortfolioData data;
        data.holdings = getUserHoldings(userId);
        data.positions = getUserPositions(userId);
        data.summary = PnLService::computePortfolioSummary(data.holdings, data.positions);

        auto& db = database::Database::getInstance();
        try {
            auto wRows = db.query("SELECT * FROM wallet WHERE user_id = " + std::to_string(userId) + ";");
            if (!wRows.empty()) {
                data.wallet.id = std::stoi(wRows[0].at("id"));
                data.wallet.userId = userId;
                data.wallet.availableBalance = std::stod(wRows[0].at("available_balance"));
                data.wallet.investedAmount = data.summary.totalInvested;
                data.wallet.withdrawableAmount = std::stod(wRows[0].at("withdrawable_amount"));
            }
        } catch (...) {}

        return data;
    }

private:
    PortfolioService() = default;
};

} // namespace tradex::services
