#pragma once

#include <vector>
#include <string>
#include <algorithm>

namespace tradex::dsa {

struct SingleTransactionProfit {
    double maxProfit{0.0};
    int buyDayIndex{-1};
    int sellDayIndex{-1};
    double buyPrice{0.0};
    double sellPrice{0.0};
    std::string timeComplexity{"O(N)"};
    std::string spaceComplexity{"O(1)"};
};

struct KTransactionsProfit {
    double maxProfit{0.0};
    int kTransactionsAllowed{2};
    std::vector<std::string> tradeSequence;
    std::string timeComplexity{"O(K * N)"};
    std::string spaceComplexity{"O(K * N)"};
};

/**
 * @brief Dynamic Programming algorithms for stock trading profit maximization.
 */
class DynamicProgrammingService {
public:
    /**
     * @brief Calculates maximum profit achievable with a single BUY and SELL transaction.
     * State transition:
     * minPrice = min(minPrice, prices[i])
     * maxProfit = max(maxProfit, prices[i] - minPrice)
     * 
     * Complexity:
     * - Time: O(N)
     * - Space: O(1)
     */
    static SingleTransactionProfit calculateSingleTransactionMaxProfit(const std::vector<double>& prices) {
        SingleTransactionProfit res;
        if (prices.size() < 2) return res;

        double minPrice = prices[0];
        int minDay = 0;
        double maxProfit = 0.0;
        int bestBuy = 0;
        int bestSell = 0;

        for (size_t i = 1; i < prices.size(); ++i) {
            double currentProfit = prices[i] - minPrice;
            if (currentProfit > maxProfit) {
                maxProfit = currentProfit;
                bestBuy = minDay;
                bestSell = static_cast<int>(i);
            }
            if (prices[i] < minPrice) {
                minPrice = prices[i];
                minDay = static_cast<int>(i);
            }
        }

        res.maxProfit = maxProfit;
        res.buyDayIndex = bestBuy;
        res.sellDayIndex = bestSell;
        res.buyPrice = prices[bestBuy];
        res.sellPrice = prices[bestSell];
        return res;
    }

    /**
     * @brief Calculates maximum profit achievable with unlimited transactions.
     * Greedy / DP: Sum of all positive price differences.
     * Complexity: Time O(N), Space O(1).
     */
    static double calculateUnlimitedTransactionsMaxProfit(const std::vector<double>& prices) {
        if (prices.size() < 2) return 0.0;
        double totalProfit = 0.0;
        for (size_t i = 1; i < prices.size(); ++i) {
            if (prices[i] > prices[i - 1]) {
                totalProfit += (prices[i] - prices[i - 1]);
            }
        }
        return totalProfit;
    }

    /**
     * @brief Calculates maximum profit achievable with at most K transactions using DP.
     * 
     * DP Table: dp[t][d] = max profit on day d with at most t transactions
     * dp[t][d] = max(dp[t][d-1], max(prices[d] - prices[m] + dp[t-1][m])) for m in [0, d-1]
     * Optimized: maxDiff = max(maxDiff, dp[t-1][d-1] - prices[d-1])
     *            dp[t][d] = max(dp[t][d-1], prices[d] + maxDiff)
     * 
     * Complexity:
     * - Time: O(K * N)
     * - Space: O(K * N)
     */
    static KTransactionsProfit calculateKTransactionsMaxProfit(const std::vector<double>& prices, int k) {
        KTransactionsProfit res;
        res.kTransactionsAllowed = k;
        int n = static_cast<int>(prices.size());
        if (n <= 1 || k <= 0) return res;

        // If k >= n / 2, equivalent to unlimited transactions
        if (k >= n / 2) {
            res.maxProfit = calculateUnlimitedTransactionsMaxProfit(prices);
            res.tradeSequence.push_back("K exceeds N/2: unlimited transactions applied.");
            return res;
        }

        std::vector<std::vector<double>> dp(k + 1, std::vector<double>(n, 0.0));

        for (int t = 1; t <= k; ++t) {
            double maxDiff = -prices[0];
            for (int d = 1; d < n; ++d) {
                dp[t][d] = std::max(dp[t][d - 1], prices[d] + maxDiff);
                maxDiff = std::max(maxDiff, dp[t - 1][d] - prices[d]);
            }
        }

        res.maxProfit = dp[k][n - 1];
        return res;
    }
};

} // namespace tradex::dsa
