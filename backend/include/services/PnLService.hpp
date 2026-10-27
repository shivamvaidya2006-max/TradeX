#pragma once

#include "models/Models.hpp"
#include <vector>
#include <cmath>

namespace tradex::services {

struct PnLSummary {
    double totalInvested{0.0};
    double totalCurrentValue{0.0};
    double totalUnrealizedPnl{0.0};
    double totalRealizedPnl{0.0};
    double totalPnl{0.0};
    double totalPnlPercent{0.0};
    double todayPnl{0.0};
    double todayPnlPercent{0.0};
};

class PnLService {
public:
    /**
     * @brief Computes unrealized P&L for a single holding: (currentPrice - avgBuyPrice) * qty.
     */
    static double calculateUnrealizedPnL(double currentPrice, double avgBuyPrice, int quantity) {
        return (currentPrice - avgBuyPrice) * quantity;
    }

    /**
     * @brief Computes portfolio-level P&L metrics across all holdings and positions.
     */
    static PnLSummary computePortfolioSummary(const std::vector<models::Holding>& holdings,
                                              const std::vector<models::Position>& positions) {
        PnLSummary summary;

        for (const auto& h : holdings) {
            summary.totalInvested += h.investedValue();
            summary.totalCurrentValue += h.currentValue();
            summary.totalUnrealizedPnl += h.pnl();
            // Estimate day's P&L based on 0.5% simulated average daily price delta
            summary.todayPnl += (h.currentValue() * 0.005);
        }

        for (const auto& p : positions) {
            summary.totalRealizedPnl += p.realizedPnl;
        }

        summary.totalPnl = summary.totalUnrealizedPnl + summary.totalRealizedPnl;
        if (summary.totalInvested > 0.0) {
            summary.totalPnlPercent = (summary.totalPnl / summary.totalInvested) * 100.0;
            summary.todayPnlPercent = (summary.todayPnl / summary.totalInvested) * 100.0;
        }

        return summary;
    }
};

} // namespace tradex::services
