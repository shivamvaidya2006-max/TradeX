# TradeX Technical Defense Guide — Person 2
## Exchange Matching Engine & High-Frequency Order Architect

**Candidate Role:** Person 2  
**Presentation Time:** 5 to 7 Minutes  
**Core Domain:** Continuous Double-Auction Matching Engine, Binary Heaps (Priority Queues), Order Books, Price-Time Priority FIFO Queues, Binary Search Tree (BST) Price Indexing, Order Pipeline.

---

### Assigned Files
- `backend/include/dsa/MatchingEngine.hpp`
- `backend/include/dsa/OrderBook.hpp`
- `backend/include/dsa/OrderQueue.hpp`
- `backend/include/dsa/BST.hpp`
- `backend/include/services/OrderService.hpp`

---

### 1. Executive Summary & Architectural Ownership
Person 2 owns the institutional core of TradeX: the Continuous Double-Auction matching engine and order execution pipeline. Orders placed on the frontend are validated, placed into dual binary priority queues (Max-Heap for buyers, Min-Heap for sellers), matched at resting order prices in $O(\log N)$ time, and surfaced on Level-2 Market Depth ladders.

---

### 2. File-by-File Technical Deep Dive

#### `backend/include/dsa/OrderBook.hpp` (Dual-Heap Order Book)
- Implements `LimitOrder` struct (id, userId, symbol, side, category, quantity, price, timestamp).
- **Max-Heap (Bids):** Comparator prioritizes highest price first; uses timestamp as tie-breaker.
- **Min-Heap (Asks):** Comparator prioritizes lowest price first; uses timestamp as tie-breaker.
- `getSnapshot(5)`: Extracts top 5 price levels, aggregated quantities, and order counts for market depth ladders.

#### `backend/include/dsa/MatchingEngine.hpp` (Continuous Double-Auction Matching)
- Algorithmic loop executing while `hasBuyOrders() && hasSellOrders()`.
- Matching condition: `bestBuy->price >= bestSell->price`.
- Trade quantity: `matchedQty = min(bestBuy->quantity, bestSell->quantity)`.
- Price determination: Follows the **Maker-Taker principle** where execution price equals the timestamp-earlier resting order price.
- Partial fill handling: Decrements remaining quantities and evicts zero-quantity orders.

#### `backend/include/dsa/OrderQueue.hpp` (Price-Time Priority Queue)
- Implements a FIFO queue (`std::deque<std::shared_ptr<LimitOrder>>`) for orders at the exact same limit price.
- Guarantees $O(1)$ push and pop operations while maintaining fair execution sequence.

#### `backend/include/dsa/BST.hpp` (Binary Search Tree Price Ladder)
- Node structure storing `price`, `symbol`, `volume`, and child pointers.
- Average $O(\log N)$ insertion and search.
- Ascending **Inorder traversal** producing sorted price sequences.
- $O(1)$ pointer-based `findMin()` and `findMax()` operations.

#### `backend/include/services/OrderService.hpp` (Order Pipeline)
- Pre-trade margin validation, order creation, and status tracking (`PENDING`, `EXECUTED`, `CANCELLED`, `REJECTED`).
- Wraps matching outcomes in database transactions to adjust holdings and wallet balances.
- Supports atomic cancellation of resting limit orders.

---

### 3. Data Structures & Complexity Matrix

| Data Structure / Algorithm | Time Complexity | Space Complexity | Practical Exchange Role |
|---|---|---|---|
| **Binary Max-Heap (Bids)** | Push: O(log N), Top: O(1), Pop: O(log N) | O(N) | Prioritizes highest buying bid. |
| **Binary Min-Heap (Asks)** | Push: O(log N), Top: O(1), Pop: O(log N) | O(N) | Prioritizes lowest selling ask. |
| **Continuous Double-Auction** | O(M log N) where M = matched trades | O(1) Auxiliary | Clears orders whenever Bid >= Ask at resting price. |
| **FIFO Order Queue** | Enqueue: O(1), Dequeue: O(1) | O(K) per price | Preserves timestamp priority at identical price points. |
| **Binary Search Tree (BST)** | Search/Insert: Avg O(log N) | O(N) | Price ladder indexing and ascending inorder walks. |

---

### 4. Connection to Frontend
1. **Order Execution:** `frontend/src/components/OrderModal.tsx` submits orders via `POST /api/v1/orders`. `OrderService` inserts into dual heaps and triggers matching.
2. **Level-2 Quotes:** `frontend/src/components/OrderBook.tsx` polls `GET /api/v1/dsa/orderbook/:symbol` to render live 5-depth bid/ask ladders.
3. **DSA Lab Tabs 2 & 3:** Drives the Priority Queue visualizer and BST visualizer on `frontend/src/pages/DsaLab.tsx`.

---

### 5. B.Tech Defense & Viva Voce Q&A
- **Q1: Why use Priority Queues instead of sorted arrays?**  
  *Answer:* Sorted arrays require $O(N)$ insertion time due to element shifting. Binary heaps provide $O(\log N)$ push/pop and $O(1)$ top peek in contiguous cache-friendly memory.
- **Q2: How is execution price determined in Continuous Double-Auctions?**  
  *Answer:* By the resting (maker) order that was placed earlier, providing price improvement to the aggressive incoming taker.
- **Q3: How are partial fills handled?**  
  *Answer:* We match `min(buyQty, sellQty)`, decrement both orders, pop whichever hits zero, and leave the remaining volume resting at the top of the heap.
