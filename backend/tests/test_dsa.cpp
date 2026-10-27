#include <iostream>
#include <thread>
#include <mutex>
#include <cassert>
#include "dsa/Trie.hpp"
#include "dsa/OrderBook.hpp"
#include "dsa/MatchingEngine.hpp"
#include "dsa/BST.hpp"
#include "dsa/MarketGraph.hpp"
#include "dsa/SortingAlgorithms.hpp"
#include "dsa/SearchingAlgorithms.hpp"
#include "dsa/MarketAnalyticsService.hpp"
#include "dsa/GreedyPortfolioService.hpp"
#include "dsa/DynamicProgrammingService.hpp"
#include "dsa/LinkedList.hpp"
#include "dsa/Stack.hpp"

int main() {
    using namespace tradex::dsa;
    std::cout << "========================================" << std::endl;
    std::cout << "       TradeX DSA Unit Test Suite       " << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Test Trie
    {
        Trie trie;
        trie.insert("RELIANCE", {"RELIANCE", "Reliance Industries Ltd", "Energy", 2945.50});
        trie.insert("RELINFRA", {"RELINFRA", "Reliance Infrastructure Ltd", "Infrastructure", 210.00});
        trie.insert("TCS", {"TCS", "Tata Consultancy Services Ltd", "Technology", 4210.80});
        trie.insert("TATAMOTORS", {"TATAMOTORS", "Tata Motors Ltd", "Automobile", 975.40});
        trie.insert("TATASTEEL", {"TATASTEEL", "Tata Steel Ltd", "Metals", 158.40});

        assert(trie.search("RELIANCE"));
        assert(!trie.search("NONEXISTENT"));
        assert(trie.startsWith("TA"));

        auto suggestions = trie.suggest("TA");
        assert(suggestions.size() == 2);
        std::cout << "[PASS] Trie Autocomplete & Search (O(L))" << std::endl;
    }

    // 2. Test OrderBook and MatchingEngine
    {
        OrderBook book("TCS");
        // Buyer willing to pay up to 4210 for 10 shares
        auto buyOrder = std::make_shared<LimitOrder>(1, 101, "TCS", OrderSide::BUY, OrderCategory::LIMIT, 10, 4210.0);
        // Seller offering 5 shares at 4205
        auto sellOrder1 = std::make_shared<LimitOrder>(2, 102, "TCS", OrderSide::SELL, OrderCategory::LIMIT, 5, 4205.0);
        // Seller offering 10 shares at 4215 (should not match yet)
        auto sellOrder2 = std::make_shared<LimitOrder>(3, 103, "TCS", OrderSide::SELL, OrderCategory::LIMIT, 10, 4215.0);

        book.addOrder(buyOrder);
        book.addOrder(sellOrder1);
        book.addOrder(sellOrder2);

        MatchingEngine engine;
        auto trades = engine.matchOrders(book);

        assert(trades.size() == 1);
        assert(trades[0].quantity == 5);
        assert(trades[0].executionPrice == 4210.0); // maker price
        assert(book.getBestBuy()->quantity == 5);   // remaining buy quantity
        std::cout << "[PASS] Priority Queue OrderBook & MatchingEngine" << std::endl;
    }

    // 3. Test BST
    {
        BinarySearchTree bst;
        bst.insert(100.0, "STOCK_A");
        bst.insert(50.0, "STOCK_B");
        bst.insert(150.0, "STOCK_C");
        bst.insert(25.0, "STOCK_D");
        bst.insert(75.0, "STOCK_E");

        assert(bst.search(50.0));
        assert(!bst.search(99.0));
        assert(bst.findMin()->price == 25.0);
        assert(bst.findMax()->price == 150.0);

        auto inorder = bst.inorder();
        assert(inorder.size() == 5);
        assert(inorder[0] == 25.0 && inorder[4] == 150.0);

        bst.deletePrice(50.0);
        assert(!bst.search(50.0));
        std::cout << "[PASS] Binary Search Tree Price Indexing & Traversals" << std::endl;
    }

    // 4. Test Graph BFS & DFS
    {
        MarketGraph graph;
        graph.addStock("TCS", "Tata Consultancy Services", "Technology", 4210.8);
        graph.addStock("INFY", "Infosys Ltd", "Technology", 1885.25);
        graph.addStock("HDFCBANK", "HDFC Bank Ltd", "Banking", 1642.4);

        auto bfsOrder = graph.bfs("Technology");
        assert(!bfsOrder.empty());
        auto dfsOrder = graph.dfs("Technology");
        assert(!dfsOrder.empty());
        std::cout << "[PASS] Market Graph BFS & DFS Traversals" << std::endl;
    }

    // 5. Test Sorting & Searching
    {
        std::vector<double> prices = {2945.5, 4210.8, 1885.25, 1642.4, 1215.6, 788.9, 512.4};
        auto bubbleRes = SortingAlgorithms::bubbleSort(prices, false);
        auto quickRes = SortingAlgorithms::quickSort(prices, false);
        auto mergeRes = SortingAlgorithms::mergeSort(prices, false);
        auto heapRes = SortingAlgorithms::heapSort(prices, false);

        assert(quickRes.sortedArray.front() == 512.4);
        assert(quickRes.sortedArray.back() == 4210.8);

        auto bSearchRes = SearchingAlgorithms::binarySearch(quickRes.sortedArray, 1642.4);
        assert(bSearchRes.foundIndex != -1);
        std::cout << "[PASS] Sorting (Quick/Merge/Heap/Bubble) & Binary Search" << std::endl;
    }

    // 6. Test Two Pointer / Sliding Window
    {
        std::vector<double> hist = {100.0, 102.0, 105.0, 108.0, 104.0, 110.0, 115.0};
        auto bestWindow = MarketAnalyticsService::findBestWindow(hist, 3);
        assert(bestWindow.durationDays == 3);

        auto streak = MarketAnalyticsService::findMaxConsecutiveGainPeriod(hist);
        assert(streak.maxConsecutiveDays >= 3);
        std::cout << "[PASS] Two-Pointer & Sliding Window Analytics" << std::endl;
    }

    // 7. Test Greedy Portfolio
    {
        std::vector<StockCandidate> pool = {
            {"RELIANCE", "Reliance", 2945.5, 18.0, 2.0}, // eff = 9.0
            {"TCS", "TCS", 4210.8, 15.0, 1.5},          // eff = 10.0
            {"INFY", "Infosys", 1885.25, 12.0, 1.8}     // eff = 6.67
        };
        auto greedyAlloc = GreedyPortfolioService::optimizeAllocation(100000.0, pool);
        assert(!greedyAlloc.allocations.empty());
        std::cout << "[PASS] Greedy Portfolio Capital Allocation" << std::endl;
    }

    // 8. Test Dynamic Programming Max Profit
    {
        std::vector<double> dailyPrices = {7, 1, 5, 3, 6, 4};
        auto singleTx = DynamicProgrammingService::calculateSingleTransactionMaxProfit(dailyPrices);
        assert(singleTx.maxProfit == 5.0); // Buy at 1, sell at 6

        double multiTx = DynamicProgrammingService::calculateUnlimitedTransactionsMaxProfit(dailyPrices);
        assert(multiTx == 7.0); // (5-1) + (6-3) = 7

        auto kTx = DynamicProgrammingService::calculateKTransactionsMaxProfit(dailyPrices, 2);
        assert(kTx.maxProfit == 7.0);
        std::cout << "[PASS] Dynamic Programming Max Profit (Single & K-Transactions)" << std::endl;
    }

    // 9. Test Doubly Linked List & Stack
    {
        DoublyLinkedList<std::string> auditLog;
        auditLog.pushBack("Order Placed: TCS");
        auditLog.pushBack("Order Executed: TCS");
        assert(auditLog.size() == 2);

        ActionStack<std::string> undoStack;
        undoStack.push("ACTION_1");
        undoStack.push("ACTION_2");
        std::string topAction;
        assert(undoStack.pop(topAction));
        assert(topAction == "ACTION_2");
        std::cout << "[PASS] Doubly Linked List & Action Stack" << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL 9 DSA SUITES PASSED SUCCESSFULLY! " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
