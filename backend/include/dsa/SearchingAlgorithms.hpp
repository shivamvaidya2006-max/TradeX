#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

namespace tradex::dsa {

struct SearchStep {
    int index{-1};
    int low{-1};
    int mid{-1};
    int high{-1};
    double value{0.0};
    std::string action;
};

struct SearchResult {
    std::string algorithm;
    double target{0.0};
    int foundIndex{-1};
    uint64_t comparisons{0};
    double durationMicroseconds{0.0};
    std::string timeComplexity;
    std::string spaceComplexity;
    std::vector<SearchStep> steps;
};

class SearchingAlgorithms {
public:
    static SearchResult linearSearch(const std::vector<double>& arr, double target) {
        SearchResult res;
        res.algorithm = "Linear Search";
        res.target = target;
        res.timeComplexity = "O(N)";
        res.spaceComplexity = "O(1)";

        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < arr.size(); ++i) {
            res.comparisons++;
            if (res.steps.size() < 100) {
                res.steps.push_back({static_cast<int>(i), -1, -1, -1, arr[i], "Checking element"});
            }
            if (std::abs(arr[i] - target) < 0.001) {
                res.foundIndex = static_cast<int>(i);
                break;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        return res;
    }

    static SearchResult binarySearch(const std::vector<double>& sortedArr, double target) {
        SearchResult res;
        res.algorithm = "Binary Search";
        res.target = target;
        res.timeComplexity = "O(log N)";
        res.spaceComplexity = "O(1)";

        auto start = std::chrono::high_resolution_clock::now();
        int low = 0;
        int high = static_cast<int>(sortedArr.size()) - 1;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            res.comparisons++;

            if (res.steps.size() < 100) {
                res.steps.push_back({-1, low, mid, high, sortedArr[mid], "Checking middle element"});
            }

            if (std::abs(sortedArr[mid] - target) < 0.001) {
                res.foundIndex = mid;
                break;
            } else if (sortedArr[mid] < target) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        return res;
    }
};

} // namespace tradex::dsa
