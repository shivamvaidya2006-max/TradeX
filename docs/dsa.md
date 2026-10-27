# TradeX Data Structures & Algorithms (DSA) Specification

TradeX is purposefully architected as an educational showcase of how classical computer science algorithms and data structures power financial exchanges and brokerage infrastructure.

---

## 1. Summary of DSA Implementations

| # | Data Structure / Algorithm | C++ Class / File | Time Complexity | Space Complexity | Practical Exchange Use Case |
|---|---|---|---|---|---|
| **1** | **Prefix Tree (Trie)** | `Trie.hpp` | Search: $O(L)$, Insert: $O(L)$ | $O(\Sigma \cdot L \cdot N)$ | Sub-millisecond stock search without full DB scans |
| **2** | **Priority Queue (Binary Heaps)** | `OrderBook.hpp` | Push: $O(\log N)$, Top: $O(1)$, Pop: $O(\log N)$ | $O(N)$ | Continuous Double-Auction matching (highest bid, lowest ask) |
| **3** | **Binary Search Tree (BST)** | `BST.hpp` | Search/Insert: $O(\log N)$ avg | $O(N)$ | Dynamic price ladder indexing and ordered range traversals |
| **4** | **Adjacency List Graph (BFS/DFS)** | `MarketGraph.hpp` | $O(V + E)$ | $O(V + E)$ | Sector-equity contagion analysis and cluster exploration |
| **5** | **Sorting Suite (6 Algorithms)** | `SortingAlgorithms.hpp` | $O(N \log N)$ to $O(N^2)$ | $O(1)$ to $O(N)$ | Benchmarking order execution queues and trade tape sorting |
| **6** | **Search Suite (Linear & Binary)** | `SearchingAlgorithms.hpp` | $O(N)$ vs $O(\log N)$ | $O(1)$ | Comparative benchmark of price lookup in sorted arrays |
| **7** | **Sliding Window & Two-Pointer** | `MarketAnalyticsService.hpp` | $O(N)$ | $O(1)$ | Momentum analysis: longest streak and best growth period |
| **8** | **Greedy Capital Allocator** | `GreedyPortfolioService.hpp` | $O(N \log N)$ | $O(N)$ | Sharpe-ratio heuristic fractional/integer capital distribution |
| **9** | **Dynamic Programming (DP)** | `DynamicProgrammingService.hpp` | $O(N)$ to $O(K \times N)$ | $O(K \times N)$ | Optimal buy-sell execution with 1, unlimited, and $K$ trades |
| **10** | **Doubly Linked List & Stack** | `LinkedList.hpp`, `Stack.hpp` | Insert/Delete: $O(1)$ | $O(N)$ | Zero-allocation audit logging and action undo simulation |

---

## 2. In-Depth Module Analysis

### 2.1 Prefix Tree (Trie)
- **Path:** `backend/include/dsa/Trie.hpp`
- **Design:** Each `TrieNode` contains an `std::unordered_map<char, std::shared_ptr<TrieNode>>` of child edges, an `isTerminal` flag, and the full stock symbol.
- **Why in Fintech:** When a trader types `"TA"`, the system traverses 2 edge pointers (`T` $\rightarrow$ `A`) and performs a DFS subtree collection to return `"TATAMOTORS"`, `"TATASTEEL"`, and `"TCS"` in microseconds, bypassing expensive SQL `LIKE '%TA%'` full table scans.

### 2.2 Priority Queue / Double Heap Order Book
- **Path:** `backend/include/dsa/OrderBook.hpp`
- **Design:**
  - **Bids (Buy Book):** Max-Heap using `std::priority_queue` with a comparator prioritizing higher price, and earlier timestamp for tie-breaking.
  - **Asks (Sell Book):** Min-Heap using `std::priority_queue` with a comparator prioritizing lower price, and earlier timestamp for tie-breaking.
- **Double Auction Invariant:** A trade executes whenever $\text{best\_buy\_price} \ge \text{best\_sell\_price}$. The execution price is determined by the resting order's limit price.

### 2.3 Binary Search Tree (BST)
- **Path:** `backend/include/dsa/BST.hpp`
- **Features:** Supports node insertion, search, minimum and maximum queries, and Inorder, Preorder, and Postorder traversals. Inorder traversal produces an ascending, sorted price ladder in $O(N)$ time.

### 2.4 Sector-Stock Adjacency Graph
- **Path:** `backend/include/dsa/MarketGraph.hpp`
- **Design:** Implements an adjacency list `std::unordered_map<std::string, std::vector<std::string>>`.
- **BFS (Breadth-First Search):** Uses `std::queue` to discover peer stocks in the same sector tier.
- **DFS (Depth-First Search):** Uses recursion and visited hash sets to trace systemic supply-chain shock propagation.

### 2.5 Dynamic Programming (DP) Stock Trading
- **Path:** `backend/include/dsa/DynamicProgrammingService.hpp`
- **1 Transaction ($O(N)$):** Kadane-like algorithm tracking `min_price_so_far` and `max_profit_so_far`.
- **Unlimited Transactions ($O(N)$):** Accumulates all positive daily price differentials $\sum \max(0, P_i - P_{i-1})$.
- **$K$ Transactions ($O(K \times N)$):** Maintains 2D DP state table where $DP[t][d]$ represents the maximum profit on day $d$ with at most $t$ transactions:
  $$DP[t][d] = \max(DP[t][d-1], \max_{m \in [0, d-1]} (P_d - P_m + DP[t-1][m]))$$
