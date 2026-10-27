# TradeX — 4-Person Presentation & Technical Defense Master Plan

> **Project Title:** TradeX — Institutional Stock Trading Platform & Simulated Exchange Engine  
> **Team Size:** 4 Members  
> **Presentation Duration:** ~25 to 30 Minutes Total (5–6 mins per speaker + 5 mins Q&A)

---

## 1. Executive Allocation Summary

| Speaker | Engineering Title | Core Domain | Assigned Backend Files | Key DSA / Algorithms |
|---|---|---|---|---|
| **Person 1** | **Systems, Security & Network Infrastructure Architect** | Winsock sockets, HTTP/1.1 REST, RFC-6455 WebSocket, Salted SHA-256, JWT, JSON parser | `main.cpp`<br/>`HttpServer.hpp`<br/>`Crypto.hpp`<br/>`Json.hpp`<br/>`AuthService.hpp` | • Dynamic Thread-Safe Socket Registry<br/>• Recursive-Descent Parser<br/>• HMAC-SHA256 Token Engine |
| **Person 2** | **Exchange Matching Engine & High-Frequency Order Architect** | Continuous Double-Auction matching, Dual Binary Heaps, FIFO order queues, BST price ladder, Order pipeline | `MatchingEngine.hpp`<br/>`OrderBook.hpp`<br/>`OrderQueue.hpp`<br/>`BST.hpp`<br/>`OrderService.hpp` | • **Max-Heap** (Bids) & **Min-Heap** (Asks)<br/>• **Continuous Double-Auction** ($O(\log N)$)<br/>• **FIFO Order Queue** ($O(1)$)<br/>• **Binary Search Tree** ($O(\log N)$) |
| **Person 3** | **Persistence, Portfolio Ledger & Quantitative Simulation Architect** | MySQL connection pooling, ACID transaction safety, Portfolio P&L, Margin wallet, Stochastic market simulation (GBM) | `Database.hpp`<br/>`PortfolioService.hpp`<br/>`WalletService.hpp`<br/>`PnLService.hpp`<br/>`MarketSimulator.hpp`<br/>`schema.sql`, `indexes.sql`, `seed.sql` | • **Connection Pool Queue** ($O(1)$)<br/>• **Geometric Brownian Motion (GBM)**<br/>• **B-Tree Composite Indexes**<br/>• **Weighted-Average Cost Accumulator** |
| **Person 4** | **Algorithmic Trading, Market Intelligence & Advanced DSA Architect** | Prefix Trie autocomplete, Sector contagion Graph (BFS/DFS), 6 Sorting algorithms, Linear/Binary search, Sliding Window, Greedy allocator, Dynamic Programming | `StockService.hpp`<br/>`CacheService.hpp`<br/>`Trie.hpp`, `MarketGraph.hpp`<br/>`SortingAlgorithms.hpp`<br/>`SearchingAlgorithms.hpp`<br/>`MarketAnalyticsService.hpp`<br/>`GreedyPortfolioService.hpp`<br/>`DynamicProgrammingService.hpp`<br/>`LinkedList.hpp`, `Stack.hpp` | • **Prefix Tree (Trie)** ($O(L)$)<br/>• **Graph BFS & DFS** ($O(V+E)$)<br/>• **6 Sorting Algorithms**<br/>• **Sliding Window & Two-Pointer** ($O(N)$)<br/>• **Greedy Sharpe Allocator** ($O(N \log N)$)<br/>• **Dynamic Programming ($K$ Trades)** ($O(K \times N)$) |

---

## 2. Recommended Presentation Order & Smooth Transitions

```text
Speaker 1 (0:00 - 6:00) ───► Gateway & Security
  │  "Once the network request is authorized and parsed, it enters our core trading engine..."
  ▼
Speaker 2 (6:00 - 12:00) ──► Continuous Double-Auction & Heaps
  │  "When an execution occurs, the trade must settle atomically into the database and adjust account balances..."
  ▼
Speaker 3 (12:00 - 18:00) ─► Database, Ledger & Market Simulator
  │  "Beyond transactional matching, our exchange integrates an advanced algorithmic intelligence and DSA suite..."
  ▼
Speaker 4 (18:00 - 24:00) ─► Advanced DSA, Autocomplete & Analytics
  │  "This concludes our technical architecture. We are now open for technical questions."
  ▼
Group Q&A (24:00 - 30:00) ──► Viva Voce Defense
```

---

## 3. Dedicated PDF & Markdown Guides for Each Member

Each team member has a dedicated, multi-page PDF guide formatted specifically for project defense and viva preparation:

1. **Person 1 Guide:**  
   - PDF: [`Person_1_Infrastructure_Security.pdf`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_1_Infrastructure_Security.pdf)  
   - Markdown: [`Person_1_Infrastructure_Security.md`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_1_Infrastructure_Security.md)
2. **Person 2 Guide:**  
   - PDF: [`Person_2_Matching_Engine_Order_Book.pdf`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_2_Matching_Engine_Order_Book.pdf)  
   - Markdown: [`Person_2_Matching_Engine_Order_Book.md`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_2_Matching_Engine_Order_Book.md)
3. **Person 3 Guide:**  
   - PDF: [`Person_3_Database_Ledger_Simulation.pdf`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_3_Database_Ledger_Simulation.pdf)  
   - Markdown: [`Person_3_Database_Ledger_Simulation.md`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_3_Database_Ledger_Simulation.md)
4. **Person 4 Guide:**  
   - PDF: [`Person_4_DSA_Analytics_Market_Intelligence.pdf`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_4_DSA_Analytics_Market_Intelligence.pdf)  
   - Markdown: [`Person_4_DSA_Analytics_Market_Intelligence.md`](file:///d:/course_projects/DS-1/docs/presentation_roles/Person_4_DSA_Analytics_Market_Intelligence.md)
