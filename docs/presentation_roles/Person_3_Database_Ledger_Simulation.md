# TradeX Technical Defense Guide — Person 3
## Persistence, Portfolio Ledger & Quantitative Simulation Architect

**Candidate Role:** Person 3  
**Presentation Time:** 5 to 7 Minutes  
**Core Domain:** MySQL 8.0 Connection Pooling, Transaction Safety (ACID), Portfolio P&L Math, Virtual Margin Wallet, Autonomous Market Simulator (Geometric Brownian Motion), Statutory Brokerage Calculation.

---

### Assigned Files
- `backend/include/database/Database.hpp`
- `backend/include/services/PortfolioService.hpp`
- `backend/include/services/WalletService.hpp`
- `backend/include/services/PnLService.hpp`
- `backend/include/services/WatchlistService.hpp`
- `backend/include/services/MarketSimulator.hpp`
- `database/schema.sql`
- `database/indexes.sql`
- `database/seed.sql`

---

### 1. Executive Summary & Architectural Ownership
Person 3 manages the financial state, mathematical models, and persistent storage of TradeX. This includes the thread-safe MySQL connection pool, relational schema integrity across 12 tables, ACID transactions, portfolio P&L mathematics, virtual margin wallet accounting, and the background market simulator implementing stochastic Geometric Brownian Motion (GBM).

---

### 2. File-by-File Technical Deep Dive

#### `backend/include/database/Database.hpp` (Connection Pool)
- Singleton managing `std::queue<MYSQL*> connectionPool_` protected by `std::mutex poolMutex_`.
- `acquireConnection()` & `releaseConnection()`: Fast $O(1)$ connection checkout with `mysql_ping` health checks.
- SQL helper methods: `query()`, `execute()`, and transaction controls (`beginTransaction`, `commit`, `rollback`).

#### `backend/include/services/PortfolioService.hpp` (Portfolio & P&L Engine)
- Consolidates holdings (CNC) and open positions (MIS).
- Weighted-average purchase price formula:
  $$	ext{AvgPrice}_{	ext{new}} = rac{Q_{	ext{old}} \cdot P_{	ext{old}} + Q_{	ext{buy}} \cdot P_{	ext{buy}}}{Q_{	ext{old}} + Q_{	ext{buy}}}$$
- Real-time mark-to-market Unrealized P&L:
  $$	ext{Unrealized P&L} = (	ext{CurrentPrice} - 	ext{AvgPrice}) \cdot 	ext{Quantity}$$

#### `backend/include/services/WalletService.hpp` (Virtual Margin & Wallet)
- Maintains available balance, invested margin, and withdrawable funds.
- Overdraft protection ensuring withdrawable amounts cannot exceed free capital.
- Appends immutable audit records into the `transactions` table.

#### `backend/include/services/PnLService.hpp` (Regulatory Brokerage Math)
- Brokerage: Flat ₹20 or 0.03% (whichever is lower).
- Securities Transaction Tax (STT): 0.1% on delivery.
- Exchange Transaction Fee: 0.00345%.
- GST: 18% on (Brokerage + Exchange Fee).
- Stamp Duty: 0.015% on buy orders.

#### `backend/include/services/MarketSimulator.hpp` (Geometric Brownian Motion)
- Autonomous worker thread executing tick updates every 1.5–2.0 seconds.
- Continuous-time stochastic model:
  $$\Delta S = S \cdot (\mu \Delta t + \sigma \epsilon \sqrt{\Delta t})$$
  using `std::normal_distribution` and `std::mt19937_64`.
- Validates simulated Indian trading session hours (09:15 to 15:30 IST) with 24/7 override.

#### `database/schema.sql, indexes.sql, seed.sql`
- 12 InnoDB tables with foreign key relationships.
- B-Tree composite indexes accelerating OHLCV candle queries.
- Seeds 32 equities across 8 sectors and initial virtual funds.

---

### 3. Data Structures & Complexity Matrix

| Data Structure / Algorithm | Time Complexity | Space Complexity | Practical Role in TradeX |
|---|---|---|---|
| **Thread-Safe Connection Queue** | O(1) Checkout / Return | O(P) where P = Pool Size | Pre-allocates MySQL sockets to eliminate connection overhead. |
| **Geometric Brownian Motion (GBM)** | O(S) per tick (S = 32) | O(1) Auxiliary | Stochastic differential equation simulating realistic price drift and volatility. |
| **B-Tree Composite Indexes** | O(log N) lookup | O(N) index nodes | Accelerates historical OHLCV chart retrieval. |
| **Weighted-Average Cost Accumulator** | O(1) per execution | O(1) Auxiliary | Dynamically tracks holding acquisition costs. |

---

### 4. Connection to Frontend
1. **Portfolio Hub:** `PortfolioSummary.tsx`, `HoldingsTable.tsx`, `PositionsTable.tsx` fetch data from `GET /api/v1/portfolio`.
2. **Funds Terminal:** `frontend/src/pages/Funds.tsx` and `Transactions.tsx` invoke `walletApi.deposit()`, `walletApi.withdraw()`, and view transaction statements.
3. **Tick Streaming:** `MarketSimulator` emissions broadcast via WebSocket to animate green/red price flashes across `MarketTicker.tsx` and `Markets.tsx`.

---

### 5. B.Tech Defense & Viva Voce Q&A
- **Q1: Why use a Database Connection Pool instead of opening connections per request?**  
  *Answer:* To eliminate the 15–40ms TCP 3-way handshake and SSL authentication overhead on every request, reusing pre-authenticated sockets in $O(1)$ time.
- **Q2: How does the Geometric Brownian Motion model work?**  
  *Answer:* It models stock prices with a deterministic drift term ($\mu \Delta t$) and a stochastic diffusion term ($\sigma \epsilon \sqrt{\Delta t}$), ensuring non-negative prices and realistic market volatility.
- **Q3: How is financial data consistency guaranteed?**  
  *Answer:* Through MySQL ACID transactions (`BEGIN ... COMMIT / ROLLBACK`) combined with C++ mutex locks, preventing race conditions or negative account balances.
