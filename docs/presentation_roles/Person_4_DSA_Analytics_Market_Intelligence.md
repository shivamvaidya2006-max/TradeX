# TradeX Technical Defense Guide — Person 4
## Algorithmic Trading, Market Intelligence & Advanced DSA Architect

**Candidate Role:** Person 4  
**Presentation Time:** 5 to 7 Minutes  
**Core Domain:** Stock Catalog & LRU Caching, Prefix Trie Autocomplete, Sector-Equity Graph (BFS/DFS), Sorting Suite (6 Algorithms), Searching Suite (Linear vs Binary), Sliding Window & Two-Pointer Momentum, Greedy Capital Allocation, Dynamic Programming Stock Profit Maximizer, Doubly Linked List & Stack Audit Trails.

---

### Assigned Files
- `backend/include/services/StockService.hpp`
- `backend/include/services/CacheService.hpp`
- `backend/include/dsa/Trie.hpp`
- `backend/include/dsa/MarketGraph.hpp`
- `backend/include/dsa/SortingAlgorithms.hpp`
- `backend/include/dsa/SearchingAlgorithms.hpp`
- `backend/include/dsa/MarketAnalyticsService.hpp`
- `backend/include/dsa/GreedyPortfolioService.hpp`
- `backend/include/dsa/DynamicProgrammingService.hpp`
- `backend/include/dsa/LinkedList.hpp`
- `backend/include/dsa/Stack.hpp`
- `backend/tests/test_dsa.cpp`

---

### 1. Executive Summary & Architectural Ownership
Person 4 represents the algorithmic intelligence of TradeX. This engineer designs and maintains the rich C++ Data Structures and Algorithms (DSA) powering stock autocomplete, sector contagion graphs, sorting/searching benchmarks, sliding-window momentum analytics, greedy portfolio allocation, and dynamic programming profit optimization.

---

### 2. File-by-File Technical Deep Dive

#### `backend/include/dsa/Trie.hpp` (Prefix Tree Autocomplete)
- Node structure storing character map (`std::unordered_map<char, std::unique_ptr<TrieNode>>`), `isTerminal` flag, and equity metadata.
- `insert()`: $O(L)$ insertion where $L$ is string length.
- `suggest()`: Navigates to prefix root in $O(L)$, then performs DFS to collect matching suggestions in $O(L + K)$ time.
- `getVisualTree()`: Serializes tree nodes into JSON for interactive SVG tree rendering in the browser.

#### `backend/include/dsa/MarketGraph.hpp` (Sector-Equity Graph)
- Adjacency list representation connecting sectors to equities.
- **Breadth-First Search (BFS):** Explores contagion ripple effects level-by-level using `std::queue` in $O(V + E)$ time.
- **Depth-First Search (DFS):** Traverses deep risk dependency paths using recursion and backtracking in $O(V + E)$ time.

#### `backend/include/dsa/SortingAlgorithms.hpp` (6 Sorting Algorithms)
- Implements Bubble, Selection, Insertion, Merge, Quick, and Heap Sort.
- Collects execution duration in microseconds, counts comparisons and swaps, and snapshots array states (`SortStep`) at each iteration for visual playback.

#### `backend/include/dsa/SearchingAlgorithms.hpp` (Linear & Binary Search)
- Implements Linear Search ($O(N)$) and Binary Search ($O(\log N)$).
- Captures search pointers (`low`, `mid`, `high`) step-by-step for visualization.

#### `backend/include/dsa/MarketAnalyticsService.hpp` (Sliding Window & Two-Pointer)
- **Sliding Window:** Single-pass $O(N)$ scan maintaining a window of size $K$ to discover the highest percentage growth period.
- **Two-Pointer:** Compares adjacent closing prices to detect the longest consecutive sequence of green gain days.

#### `backend/include/dsa/GreedyPortfolioService.hpp` (Greedy Capital Allocator)
- Computes Sharpe-ratio heuristic: $	ext{Score} = 	ext{ExpectedReturn} / 	ext{RiskScore}$.
- Sorts candidates in $O(N \log N)$ time and greedily distributes capital into highest-ranked assets.

#### `backend/include/dsa/DynamicProgrammingService.hpp` (DP Stock Profit)
- **1 Transaction:** $O(N)$ time, $O(1)$ space tracking the running minimum price.
- **Unlimited Transactions:** $O(N)$ time summing all upward price jumps.
- **$K$ Transactions:** $O(K 	imes N)$ 2D Dynamic Programming table calculation:
  $$DP[i][j] = \max(DP[i][j-1], 	ext{Prices}[j] + 	ext{max\_diff})$$
  $$	ext{max\_diff} = \max(	ext{max\_diff}, DP[i-1][j] - 	ext{Prices}[j])$$

#### `backend/include/dsa/LinkedList.hpp` & `Stack.hpp` (Audit Logging)
- Doubly linked list with $O(1)$ append-tail and bidirectional node traversal.
- LIFO Action Stack supporting audit rollback and order undo simulation.

---

### 3. Data Structures & Complexity Matrix

| Data Structure / Algorithm | Time Complexity | Space Complexity | Practical Role in TradeX |
|---|---|---|---|
| **Prefix Tree (Trie)** | Search: O(L), Insert: O(L) | O(N * L * Alphabet) | Instant symbol autocomplete without SQL scans. |
| **Market Graph (BFS/DFS)** | O(V + E) | O(V + E) | Sector-equity contagion analysis and cluster exploration. |
| **Sorting Suite (6 Algos)** | O(N log N) to O(N^2) | O(1) to O(N) | Algorithmic trade tape and order queue benchmarking. |
| **Sliding Window Analytics** | O(N) | O(1) Auxiliary | Optimal K-day growth window and streak detection. |
| **Greedy Capital Allocator** | O(N log N) | O(N) | Sharpe-ratio heuristic capital distribution. |
| **Dynamic Programming (K Trades)** | O(K * N) | O(K * N) | Optimal buy-sell execution with transaction limits. |
| **Doubly Linked List & Stack** | O(1) Append / Pop | O(N) | Zero-allocation trade execution audit trail. |

---

### 4. Connection to Frontend
1. **Search Bar Autocomplete:** Keystrokes in `frontend/src/components/Navbar.tsx` call `GET /api/v1/stocks/search?q=...`, querying the Trie in under 2ms.
2. **DSA Lab Page:** Powers all 10 interactive visualization modules on `frontend/src/pages/DsaLab.tsx`.
3. **Price Charts:** Feeds historical candle data into `frontend/src/components/PriceChart.tsx` backed by the LRU Cache.

---

### 5. B.Tech Defense & Viva Voce Q&A
- **Q1: Why is a Trie superior to a SQL `LIKE '%query%'` query?**  
  *Answer:* SQL `LIKE '%query%'` requires a full table scan of complexity $O(N \cdot M)$, bottlenecking the database on every keystroke. A Trie locates the prefix root in $O(L)$ time and extracts $K$ suggestions via DFS in $O(L + K)$ microseconds directly from RAM.
- **Q2: Explain the DP recurrence relation for K transactions.**  
  *Answer:* $DP[i][j] = \max(DP[i][j-1], 	ext{Prices}[j] + 	ext{max\_diff})$, where $	ext{max\_diff} = \max(	ext{max\_diff}, DP[i-1][j] - 	ext{Prices}[j])$. Maintaining `max_diff` reduces complexity from $O(K \cdot N^2)$ to $O(K \cdot N)$.
- **Q3: How does the Market Graph use BFS to detect systemic sector risk?**  
  *Answer:* It models sectors and equities as an Adjacency List Graph. When a sector experiences shock, BFS traverses constituent equities (Level 1) and correlated cross-holding industries (Level 2) in $O(V + E)$ time to measure contagion spread.
