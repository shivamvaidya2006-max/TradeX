#pragma once

#include <vector>
#include <string>
#include <algorithm>

namespace tradex::dsa {

struct GrowthWindowResult {
    int startIndex{0};
    int endIndex{0};
    double startPrice{0.0};
    double endPrice{0.0};
    double absoluteGain{0.0};
    double percentageGain{0.0};
    int durationDays{0};
};

struct ConsecutiveGainResult {
    int startIndex{0};
    int endIndex{0};
    int maxConsecutiveDays{0};
    double totalGain{0.0};
};

/**
 * @brief Market Analytics using Two-Pointer and Sliding Window algorithms.
 */
class MarketAnalyticsService {
public:
    /**
     * @brief Finds the best fixed-length sliding window of N days with highest return.
     * Sliding Window Algorithm: O(N) time, O(1) space.
     */
    static GrowthWindowResult findBestWindow(const std::vector<double>& prices, int windowSize) {
        GrowthWindowResult result;
        if (prices.size() < static_cast<size_t>(windowSize) || windowSize <= 1) {
            return result;
        }

        double maxGain = -1e9;
        int bestStart = 0;

        for (size_t i = 0; i + windowSize <= prices.size(); ++i) {
            double startP = prices[i];
            double endP = prices[i + windowSize - 1];
            double gain = endP - startP;
            if (gain > maxGain) {
                maxGain = gain;
                bestStart = static_cast<int>(i);
            }
        }

        result.startIndex = bestStart;
        result.endIndex = bestStart + windowSize - 1;
        result.startPrice = prices[result.startIndex];
        result.endPrice = prices[result.endIndex];
        result.absoluteGain = result.endPrice - result.startPrice;
        result.percentageGain = (result.startPrice > 0) ? (result.absoluteGain / result.startPrice) * 100.0 : 0.0;
        result.durationDays = windowSize;
        return result;
    }

    /**
     * @brief Two-Pointer algorithm to find the longest consecutive streak of daily positive gains.
     * Complexity: O(N) time, O(1) space.
     */
    static ConsecutiveGainResult findMaxConsecutiveGainPeriod(const std::vector<double>& prices) {
        ConsecutiveGainResult result;
        if (prices.size() < 2) return result;

        int left = 0;
        int maxLen = 0;
        int bestLeft = 0;
        int bestRight = 0;

        for (size_t right = 1; right < prices.size(); ++right) {
            if (prices[right] <= prices[right - 1]) {
                // Streak broken; reset left pointer
                left = static_cast<int>(right);
            } else {
                int currentLen = static_cast<int>(right) - left + 1;
                if (currentLen > maxLen) {
                    maxLen = currentLen;
                    bestLeft = left;
                    bestRight = static_cast<int>(right);
                }
            }
        }

        result.startIndex = bestLeft;
        result.endIndex = bestRight;
        result.maxConsecutiveDays = maxLen;
        result.totalGain = (prices[bestRight] - prices[bestLeft]);
        return result;
    }
};

} // namespace tradex::dsa
