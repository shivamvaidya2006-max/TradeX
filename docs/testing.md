# TradeX Testing Strategy & Verification Guide

TradeX employs a two-tier testing methodology:
1. **Low-Level Native C++ Unit Tests:** Verifying algorithmic accuracy, data structure invariants, and time complexity bounds.
2. **End-to-End System Acceptance Tests:** Verifying the integrated REST API, JWT authentication, order matching pipeline, database integrity, and DSA endpoints over live network sockets.

---

## 1. C++ DSA Unit Test Suite (`backend/tests/test_dsa.cpp`)

### Running the C++ Unit Tests:
```powershell
# Using LLVM-MinGW Clang++ on Windows:
cd D:\course_projects\DS-1\backend
clang++ -std=c++17 -O2 -Iinclude tests/test_dsa.cpp -o test_dsa.exe
.\test_dsa.exe
```

### Verification Checklist:
- `Trie`: Verifies prefix insertion, exact matches, prefix suggestions, and empty string handling.
- `Priority Queue / Heap Order Book`: Verifies Max-Heap ordering for buy bids and Min-Heap ordering for sell asks with timestamp tie-breaking.
- `Matching Engine`: Verifies price-time cross matching, partial fill deduction, and trade execution callbacks.
- `Binary Search Tree (BST)`: Verifies inorder sorted ordering, preorder, postorder, minimum, and maximum queries.
- `Sorting Algorithms`: Verifies that Bubble, Selection, Insertion, Merge, Quick, and Heap sorts correctly sort randomly permuted price arrays and records non-zero comparisons and swaps.
- `Searching Algorithms`: Verifies Linear Search and Binary Search on sorted sequences, ensuring identical found indices and logarithmic comparison counts.
- `Sliding Window & Two-Pointer`: Verifies linear-time detection of maximum growth windows and consecutive gain streaks.
- `Greedy Capital Allocator`: Verifies budget constraint adherence, integer share allocation, and diversification ceilings.
- `Dynamic Programming`: Verifies single, unlimited, and $K$-transaction optimal profit calculations against known test vectors.

---

## 2. End-to-End Acceptance Test (`tests/acceptance_test.py`)

### Running the Python Integration Test Suite:
```powershell
cd D:\course_projects\DS-1
python tests/acceptance_test.py
```

### Automated Acceptance Scenarios:
1. **JWT Authentication:** Sends `POST /api/v1/auth/login` for demo user `trader`, verifies HTTP 200, checks token formatting, and inspects user profile payload.
2. **Stock Search & Trie:** Queries `GET /api/v1/stocks/search?q=TA`, verifies matched symbols include `TATAMOTORS` and `TCS`.
3. **Double-Auction Order Execution:**
   - Places a `BUY` order for 5 shares of `RELIANCE`.
   - Verifies HTTP 201 response with order ID and `EXECUTED` status.
   - Inspects `wallet` balance to confirm exact debit of `(Qty * Price) + Charges`.
   - Inspects `holdings` to confirm quantity was incremented.
4. **P&L Realization:** Places an offsetting `SELL` order and verifies realized P&L is credited to the wallet.
5. **DSA Lab API Verification:** Sequentially invokes all 10 DSA Lab endpoints (`/dsa/trie/visualize`, `/dsa/orderbook`, `/dsa/bst/tree`, `/dsa/graph/relations`, `/dsa/sort`, `/dsa/search`, `/dsa/analytics/sliding-window`, `/dsa/greedy/allocate`, `/dsa/dp/max-profit`, `/dsa/audit-trail`) and verifies valid JSON payloads.
