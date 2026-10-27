#pragma once

#include <vector>
#include <string>
#include <algorithm>

namespace tradex::dsa {

struct StockCandidate {
    std::string symbol;
    std::string companyName;
    double currentPrice{0.0};
    double expectedReturnPercent{0.0}; // e.g. 15.5%
    double riskScore{1.0};             // e.g. 1 (low) to 5 (high)
    double efficiencyRatio{0.0};       // return / risk
};

struct AllocationItem {
    std::string symbol;
    std::string companyName;
    double currentPrice{0.0};
    int allocatedShares{0};
    double investedAmount{0.0};
    double portfolioWeightPercent{0.0};
    double expectedReturnPercent{0.0};
    double riskScore{1.0};
};

struct GreedyPortfolioResult {
    double totalCapital{0.0};
    double capitalAllocated{0.0};
    double remainingCash{0.0};
    double weightedExpectedReturn{0.0};
    double averageRiskScore{0.0};
    std::vector<AllocationItem> allocations;
    std::string complexity{"Time: O(N log N) due to sorting, Space: O(N)"};
    std::string disclaimer{"EDUCATIONAL SIMULATION ONLY: This is an algorithmic demonstration and NOT financial advice."};
};

class GreedyPortfolioService {
public:
    /**
     * @brief Allocates capital greedily based on the Return-to-Risk efficiency ratio.
     * Complexity: O(N log N)
     */
    static GreedyPortfolioResult optimizeAllocation(double totalCapital, std::vector<StockCandidate> candidates) {
        GreedyPortfolioResult res;
        res.totalCapital = totalCapital;

        if (totalCapital <= 0 || candidates.empty()) {
            res.remainingCash = totalCapital;
            return res;
        }

        // Calculate efficiency ratio for each candidate
        for (auto& c : candidates) {
            c.efficiencyRatio = (c.riskScore > 0) ? (c.expectedReturnPercent / c.riskScore) : 0.0;
        }

        // Greedily sort candidates by efficiency ratio in descending order
        std::sort(candidates.begin(), candidates.end(), [](const StockCandidate& a, const StockCandidate& b) {
            return a.efficiencyRatio > b.efficiencyRatio;
        });

        double currentAvailable = totalCapital;
        double maxCapPerStock = totalCapital * 0.35; // Diversification constraint: max 35% in any single stock

        for (const auto& c : candidates) {
            if (currentAvailable < c.currentPrice || c.currentPrice <= 0) continue;

            double maxToInvest = std::min(currentAvailable, maxCapPerStock);
            int shares = static_cast<int>(maxToInvest / c.currentPrice);
            if (shares <= 0) continue;

            double invested = shares * c.currentPrice;
            currentAvailable -= invested;

            AllocationItem item;
            item.symbol = c.symbol;
            item.companyName = c.companyName;
            item.currentPrice = c.currentPrice;
            item.allocatedShares = shares;
            item.investedAmount = invested;
            item.expectedReturnPercent = c.expectedReturnPercent;
            item.riskScore = c.riskScore;

            res.allocations.push_back(item);
        }

        res.capitalAllocated = totalCapital - currentAvailable;
        res.remainingCash = currentAvailable;

        // Calculate weighted metrics
        if (res.capitalAllocated > 0) {
            for (auto& item : res.allocations) {
                item.portfolioWeightPercent = (item.investedAmount / res.capitalAllocated) * 100.0;
                res.weightedExpectedReturn += (item.expectedReturnPercent * (item.investedAmount / res.capitalAllocated));
                res.averageRiskScore += (item.riskScore * (item.investedAmount / res.capitalAllocated));
            }
        }

        return res;
    }
};

} // namespace tradex::dsa
