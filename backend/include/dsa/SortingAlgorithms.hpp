#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

namespace tradex::dsa {

struct SortStep {
    std::vector<double> arrayState;
    int comparedIndex1{-1};
    int comparedIndex2{-1};
    int swappedIndex1{-1};
    int swappedIndex2{-1};
    std::string description;
};

struct SortResult {
    std::string algorithm;
    std::vector<double> sortedArray;
    uint64_t comparisons{0};
    uint64_t swaps{0};
    double durationMicroseconds{0.0};
    std::string timeComplexity;
    std::string spaceComplexity;
    std::vector<SortStep> steps; // for visualizer
};

class SortingAlgorithms {
public:
    static SortResult bubbleSort(std::vector<double> arr, bool recordSteps = true) {
        SortResult res;
        res.algorithm = "Bubble Sort";
        res.timeComplexity = "O(N^2)";
        res.spaceComplexity = "O(1)";

        auto start = std::chrono::high_resolution_clock::now();
        size_t n = arr.size();

        for (size_t i = 0; i < n; ++i) {
            bool swapped = false;
            for (size_t j = 0; j + 1 < n - i; ++j) {
                res.comparisons++;
                if (recordSteps && res.steps.size() < 100) {
                    res.steps.push_back({arr, static_cast<int>(j), static_cast<int>(j + 1), -1, -1, "Comparing"});
                }

                if (arr[j] > arr[j + 1]) {
                    std::swap(arr[j], arr[j + 1]);
                    res.swaps++;
                    swapped = true;
                    if (recordSteps && res.steps.size() < 100) {
                        res.steps.push_back({arr, -1, -1, static_cast<int>(j), static_cast<int>(j + 1), "Swapped"});
                    }
                }
            }
            if (!swapped) break;
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        res.sortedArray = arr;
        return res;
    }

    static SortResult selectionSort(std::vector<double> arr, bool recordSteps = true) {
        SortResult res;
        res.algorithm = "Selection Sort";
        res.timeComplexity = "O(N^2)";
        res.spaceComplexity = "O(1)";

        auto start = std::chrono::high_resolution_clock::now();
        size_t n = arr.size();

        for (size_t i = 0; i < n; ++i) {
            size_t minIdx = i;
            for (size_t j = i + 1; j < n; ++j) {
                res.comparisons++;
                if (arr[j] < arr[minIdx]) {
                    minIdx = j;
                }
            }
            if (minIdx != i) {
                std::swap(arr[i], arr[minIdx]);
                res.swaps++;
                if (recordSteps && res.steps.size() < 100) {
                    res.steps.push_back({arr, -1, -1, static_cast<int>(i), static_cast<int>(minIdx), "Selected Minimum Swapped"});
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        res.sortedArray = arr;
        return res;
    }

    static SortResult insertionSort(std::vector<double> arr, bool recordSteps = true) {
        SortResult res;
        res.algorithm = "Insertion Sort";
        res.timeComplexity = "O(N^2)";
        res.spaceComplexity = "O(1)";

        auto start = std::chrono::high_resolution_clock::now();
        size_t n = arr.size();

        for (size_t i = 1; i < n; ++i) {
            double key = arr[i];
            int j = static_cast<int>(i) - 1;

            while (j >= 0 && (res.comparisons++, arr[j] > key)) {
                arr[j + 1] = arr[j];
                res.swaps++;
                j--;
            }
            arr[j + 1] = key;
            if (recordSteps && res.steps.size() < 100) {
                res.steps.push_back({arr, -1, -1, j + 1, static_cast<int>(i), "Key Inserted"});
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        res.sortedArray = arr;
        return res;
    }

    static SortResult mergeSort(std::vector<double> arr, bool recordSteps = true) {
        SortResult res;
        res.algorithm = "Merge Sort";
        res.timeComplexity = "O(N log N)";
        res.spaceComplexity = "O(N)";

        auto start = std::chrono::high_resolution_clock::now();
        if (!arr.empty()) {
            mergeSortRecursive(arr, 0, arr.size() - 1, res, recordSteps);
        }
        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        res.sortedArray = arr;
        return res;
    }

    static SortResult quickSort(std::vector<double> arr, bool recordSteps = true) {
        SortResult res;
        res.algorithm = "Quick Sort";
        res.timeComplexity = "Average O(N log N), Worst O(N^2)";
        res.spaceComplexity = "O(log N)";

        auto start = std::chrono::high_resolution_clock::now();
        if (!arr.empty()) {
            quickSortRecursive(arr, 0, static_cast<int>(arr.size()) - 1, res, recordSteps);
        }
        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        res.sortedArray = arr;
        return res;
    }

    static SortResult heapSort(std::vector<double> arr, bool recordSteps = true) {
        SortResult res;
        res.algorithm = "Heap Sort";
        res.timeComplexity = "O(N log N)";
        res.spaceComplexity = "O(1)";

        auto start = std::chrono::high_resolution_clock::now();
        size_t n = arr.size();

        for (int i = static_cast<int>(n) / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i, res, recordSteps);
        }

        for (int i = static_cast<int>(n) - 1; i > 0; i--) {
            std::swap(arr[0], arr[i]);
            res.swaps++;
            if (recordSteps && res.steps.size() < 100) {
                res.steps.push_back({arr, -1, -1, 0, i, "Max Root Swapped to End"});
            }
            heapify(arr, i, 0, res, recordSteps);
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.durationMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
        res.sortedArray = arr;
        return res;
    }

private:
    static void merge(std::vector<double>& arr, size_t l, size_t m, size_t r, SortResult& res, bool recordSteps) {
        size_t n1 = m - l + 1;
        size_t n2 = r - m;

        std::vector<double> L(n1), R(n2);
        for (size_t i = 0; i < n1; i++) L[i] = arr[l + i];
        for (size_t j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

        size_t i = 0, j = 0, k = l;
        while (i < n1 && j < n2) {
            res.comparisons++;
            if (L[i] <= R[j]) {
                arr[k] = L[i++];
            } else {
                arr[k] = R[j++];
                res.swaps++;
            }
            k++;
        }

        while (i < n1) arr[k++] = L[i++];
        while (j < n2) arr[k++] = R[j++];

        if (recordSteps && res.steps.size() < 100) {
            res.steps.push_back({arr, static_cast<int>(l), static_cast<int>(r), -1, -1, "Merged Subarray"});
        }
    }

    static void mergeSortRecursive(std::vector<double>& arr, size_t l, size_t r, SortResult& res, bool recordSteps) {
        if (l >= r) return;
        size_t m = l + (r - l) / 2;
        mergeSortRecursive(arr, l, m, res, recordSteps);
        mergeSortRecursive(arr, m + 1, r, res, recordSteps);
        merge(arr, l, m, r, res, recordSteps);
    }

    static void quickSortRecursive(std::vector<double>& arr, int low, int high, SortResult& res, bool recordSteps) {
        if (low < high) {
            double pivot = arr[high];
            int i = low - 1;

            for (int j = low; j < high; j++) {
                res.comparisons++;
                if (arr[j] < pivot) {
                    i++;
                    std::swap(arr[i], arr[j]);
                    res.swaps++;
                }
            }
            std::swap(arr[i + 1], arr[high]);
            res.swaps++;
            int pi = i + 1;

            if (recordSteps && res.steps.size() < 100) {
                res.steps.push_back({arr, -1, -1, pi, high, "Pivot Positioned"});
            }

            quickSortRecursive(arr, low, pi - 1, res, recordSteps);
            quickSortRecursive(arr, pi + 1, high, res, recordSteps);
        }
    }

    static void heapify(std::vector<double>& arr, size_t n, size_t i, SortResult& res, bool recordSteps) {
        size_t largest = i;
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;

        if (left < n) {
            res.comparisons++;
            if (arr[left] > arr[largest]) largest = left;
        }

        if (right < n) {
            res.comparisons++;
            if (arr[right] > arr[largest]) largest = right;
        }

        if (largest != i) {
            std::swap(arr[i], arr[largest]);
            res.swaps++;
            if (recordSteps && res.steps.size() < 100) {
                res.steps.push_back({arr, -1, -1, static_cast<int>(i), static_cast<int>(largest), "Heapified Node"});
            }
            heapify(arr, n, largest, res, recordSteps);
        }
    }
};

} // namespace tradex::dsa
