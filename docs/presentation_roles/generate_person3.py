import os
from reportlab.lib import colors
from reportlab.platypus import Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
from pdf_utils import build_pdf
from builder_helpers import (
    title_style, subtitle_style, h1_style, h2_style, body_style, bullet_style,
    code_style, make_callout, make_code_box, make_meta_card, th_style, td_style, td_code_style,
    COLOR_PRIMARY, COLOR_ACCENT, COLOR_BORDER, COLOR_CARD_BG
)

def generate_person_3():
    pdf_path = os.path.join(os.path.dirname(__file__), "Person_3_Database_Ledger_Simulation.pdf")
    md_path = os.path.join(os.path.dirname(__file__), "Person_3_Database_Ledger_Simulation.md")
    
    files = [
        "backend/include/database/Database.hpp",
        "backend/include/services/PortfolioService.hpp",
        "backend/include/services/WalletService.hpp",
        "backend/include/services/PnLService.hpp",
        "backend/include/services/WatchlistService.hpp",
        "backend/include/services/MarketSimulator.hpp",
        "database/schema.sql",
        "database/indexes.sql",
        "database/seed.sql"
    ]
    
    flowables = []
    
    # Title
    flowables.append(Paragraph("TradeX B.Tech Capstone — Technical Defense Guide", title_style))
    flowables.append(Paragraph("Person 3: Persistence, Portfolio Ledger & Quantitative Simulation Architect", subtitle_style))
    flowables.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_ACCENT, spaceAfter=10))
    
    # Meta Card
    flowables.append(make_meta_card(
        role_num=3,
        title="Persistence, Portfolio Ledger & Quantitative Simulation Architect",
        domain="MySQL 8.0 Connection Pooling, Transaction Safety (ACID), Portfolio P&L Math, Virtual Margin Wallet, Autonomous Market Simulator (Geometric Brownian Motion), Statutory Brokerage Calculation",
        files_list=files,
        presentation_time="5 to 7 Minutes"
    ))
    flowables.append(Spacer(1, 10))
    
    # 1. Executive Summary
    flowables.append(Paragraph("1. Executive Summary & Architectural Ownership", h1_style))
    flowables.append(Paragraph(
        "Person 3 manages the financial state, mathematical models, and persistent storage of TradeX. "
        "This engineer is responsible for the thread-safe native MySQL connection pool, relational schema integrity across 12 tables, "
        "ACID transaction safety, the mathematical calculation of realized/unrealized portfolio P&L, virtual margin wallet accounting, "
        "and the background market simulator implementing stochastic Geometric Brownian Motion (GBM) to drive live stock tick feeds.",
        body_style
    ))
    
    flowables.append(make_callout(
        "<b>Core Defense Pitch:</b> <i>\"I engineered the persistence layer, financial accounting models, and quantitative simulation engine. "
        "I built a thread-safe MySQL connection pool using libmysql, ensuring zero connection churn during heavy concurrency. "
        "My portfolio engine dynamically computes mark-to-market unrealized P&L and weighted-average acquisition costs, "
        "while my autonomous market simulation worker uses Geometric Brownian Motion with Gaussian drift to simulate realistic "
        "Indian market volatility and stream live price ticks to the WebSocket transport layer.\"</i>"
    ))
    flowables.append(Spacer(1, 8))
    
    # 2. File-by-File Breakdown
    flowables.append(Paragraph("2. Assigned Files & Detailed Code Responsibilities", h1_style))
    
    f1 = "<b>backend/include/database/Database.hpp</b> (Thread-Safe MySQL Connection Pool):<br/>" \
         "• Implements <code>Database</code> singleton managing a thread-safe queue: <code>std::queue&lt;MYSQL*&gt; connectionPool_</code>.<br/>" \
         "• <code>acquireConnection()</code> &amp; <code>releaseConnection()</code>: Synchronized via <code>std::mutex poolMutex_</code> with auto-ping healthcheck (<code>mysql_ping</code>).<br/>" \
         "• <code>query()</code>: Executes SQL statements and returns strongly-typed row dictionaries (<code>QueryResult</code>).<br/>" \
         "• Transaction Management: Exposes <code>beginTransaction()</code>, <code>commit()</code>, and <code>rollback()</code> to prevent partial trade settlements."
    flowables.append(Paragraph(f1, body_style))
    
    f2 = "<b>backend/include/services/PortfolioService.hpp</b> (Portfolio &amp; P&amp;L Engine):<br/>" \
         "• <code>getCompletePortfolio(userId)</code>: Consolidates delivery holdings (CNC), intraday positions (MIS), wallet balance, and overall summary.<br/>" \
         "• Calculates <b>Weighted Average Price</b> upon adding shares:<br/>" \
         "&nbsp;&nbsp;&nbsp;&nbsp;<i>AvgPrice<sub>new</sub> = (Qty<sub>old</sub> &times; AvgPrice<sub>old</sub> + Qty<sub>new</sub> &times; Price<sub>new</sub>) / Qty<sub>total</sub></i><br/>" \
         "• Dynamically computes <b>Unrealized P&amp;L</b>: <code>(CurrentPrice - AvgPrice) &times; Quantity</code> using live market prices.<br/>" \
         "• Computes overall portfolio gain percentage and today's day gain."
    flowables.append(Paragraph(f2, body_style))
    
    f3 = "<b>backend/include/services/WalletService.hpp</b> (Virtual Margin &amp; Cash Ledger):<br/>" \
         "• Manages virtual capital: <code>availableBalance</code>, <code>investedAmount</code>, and <code>withdrawableAmount</code>.<br/>" \
         "• <code>deposit()</code> &amp; <code>withdraw()</code>: Atomic balance adjustments with overdraft protection.<br/>" \
         "• Financial Audit Ledger: Automatically appends immutable records to the <code>transactions</code> table on every cash event or order settlement."
    flowables.append(Paragraph(f3, body_style))
    
    f4 = "<b>backend/include/services/PnLService.hpp</b> (Statutory Brokerage &amp; Regulatory Fees):<br/>" \
         "• Implements realistic Indian financial statutory charges rules:<br/>" \
         "&nbsp;&nbsp;• <b>Brokerage:</b> Flat ₹20 or 0.03% (whichever is lower)<br/>" \
         "&nbsp;&nbsp;• <b>Securities Transaction Tax (STT):</b> 0.1% on delivery (CNC)<br/>" \
         "&nbsp;&nbsp;• <b>Exchange Transaction Fee:</b> 0.00345% of total turnover<br/>" \
         "&nbsp;&nbsp;• <b>GST:</b> 18% on (Brokerage + Exchange Fee)<br/>" \
         "&nbsp;&nbsp;• <b>Stamp Duty:</b> 0.015% on buy orders only."
    flowables.append(Paragraph(f4, body_style))
    
    f5 = "<b>backend/include/services/MarketSimulator.hpp</b> (Geometric Brownian Motion Engine):<br/>" \
         "• Spawns dedicated background thread <code>runLoop()</code> executing every 1.5 to 2.0 seconds.<br/>" \
         "• Implements stochastic differential equation: <i>&Delta;S = S &times; (&mu;&Delta;t + &sigma;&epsilon;&radic;&Delta;t)</i> using <code>std::normal_distribution</code>.<br/>" \
         "• Evaluates simulated Indian exchange hours (09:15 to 15:30 IST) with 24/7 demo override.<br/>" \
         "• Emits <code>MarketTick</code> structs containing updated LTP, day high, day low, volume, and percentage change."
    flowables.append(Paragraph(f5, body_style))
    
    f6 = "<b>database/schema.sql, indexes.sql, seed.sql</b> (MySQL Relational Architecture):<br/>" \
         "• 12 relational tables with foreign keys and cascade rules.<br/>" \
         "• B-Tree performance indexes on <code>(stock_id, timestamp)</code> and <code>user_id</code>.<br/>" \
         "• Seeds 32 top Indian equities, demo user accounts, starter wallets, and OHLCV history."
    flowables.append(Paragraph(f6, body_style))
    flowables.append(Spacer(1, 8))
    
    # 3. DSA Implementation
    flowables.append(Paragraph("3. Data Structures & Algorithms Implemented by Person 3", h1_style))
    
    dsa_table_data = [
        [Paragraph("Data Structure / Algorithm", th_style), Paragraph("Time Complexity", th_style), Paragraph("Space Complexity", th_style), Paragraph("Engineering Role in TradeX", th_style)],
        [Paragraph("<b>Mutex-Synchronized Queue</b> (Connection Pool)", td_style), Paragraph("Acquire: O(1)<br/>Release: O(1)", td_style), Paragraph("O(P) where P = Pool Size", td_style), Paragraph("Pre-allocates MySQL socket handles in a FIFO queue, eliminating connection handshake latency during concurrent order processing.", td_style)],
        [Paragraph("<b>Geometric Brownian Motion (GBM)</b>", td_style), Paragraph("O(S) per tick cycle (S = 32 stocks)", td_style), Paragraph("O(1) Auxiliary", td_style), Paragraph("Simulates continuous-time stochastic equity price fluctuations using Box-Muller normal distribution random sampling.", td_style)],
        [Paragraph("<b>B-Tree Clustered &amp; Secondary Indexes</b>", td_style), Paragraph("O(log N) lookup", td_style), Paragraph("O(N) index pages", td_style), Paragraph("Accelerates historical OHLCV chart retrieval and user order lookups; prevents database table scans.", td_style)],
        [Paragraph("<b>Weighted-Average Cost Accumulator</b>", td_style), Paragraph("O(1) per execution", td_style), Paragraph("O(1) Auxiliary", td_style), Paragraph("Maintains tax-compliant acquisition costs for equity holdings across fragmented buy orders.", td_style)],
    ]
    t_dsa = Table(dsa_table_data, colWidths=[130, 95, 95, 212])
    t_dsa.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), COLOR_PRIMARY),
        ('GRID', (0,0), (-1,-1), 0.5, COLOR_BORDER),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, COLOR_CARD_BG])
    ]))
    flowables.append(t_dsa)
    flowables.append(Spacer(1, 10))
    
    # 4. Frontend Connection
    flowables.append(Paragraph("4. How Person 3's Backend Connects to the Frontend", h1_style))
    flowables.append(Paragraph(
        "• <b>Portfolio Dashboard &amp; Tables:</b> <code>frontend/src/components/PortfolioSummary.tsx</code>, <code>HoldingsTable.tsx</code>, and <code>PositionsTable.tsx</code> call <code>portfolioApi.getPortfolio()</code>. "
        "<code>PortfolioService</code> computes live valuation and returns JSON consumed by React.<br/>"
        "• <b>Virtual Wallet &amp; Transactions:</b> <code>frontend/src/pages/Funds.tsx</code> and <code>Transactions.tsx</code> interact with <code>walletApi.deposit()</code>, <code>walletApi.withdraw()</code>, and <code>walletApi.getTransactions()</code>.<br/>"
        "• <b>Real-Time Tick Streaming:</b> <code>MarketSimulator::runLoop</code> computes price updates and triggers <code>HttpServer::broadcastWebSocket</code>. "
        "The browser's <code>websocket.ts</code> captures these messages, animating the flashing green/red tickers across <code>MarketTicker.tsx</code> and <code>Markets.tsx</code>.",
        body_style
    ))
    flowables.append(Spacer(1, 8))
    
    # 5. Viva / Defense Q&A
    flowables.append(Paragraph("5. B.Tech Defense & Viva Voce Q&A (Ready to Speak)", h1_style))
    
    q1 = "<b>Q1: Why use a Database Connection Pool instead of opening a connection per HTTP request?</b><br/>" \
         "<b>Answer:</b> <i>\"Establishing a TCP connection to MySQL involves a 3-way handshake, SSL negotiation, and database authentication, which takes 15 to 40 milliseconds per request. In an exchange processing concurrent orders, connection churn would exhaust server file descriptors. My connection pool pre-allocates 4 persistent connection pointers in a mutex-locked queue, allowing worker threads to acquire and release connections in O(1) time with sub-millisecond overhead.\"</i>"
    flowables.append(Paragraph(q1, body_style))
    flowables.append(Spacer(1, 4))
    
    q2 = "<b>Q2: How does your Geometric Brownian Motion (GBM) mathematical model work?</b><br/>" \
         "<b>Answer:</b> <i>\"Geometric Brownian Motion is the standard mathematical model for stock prices under the Black-Scholes framework. It consists of two components: a deterministic drift term representing expected return (&mu;&Delta;t), and a stochastic diffusion term representing market volatility (&sigma;&epsilon;&radic;&Delta;t), where &epsilon; is a standard normal random variable generated via <code>std::normal_distribution</code>. This prevents stock prices from ever turning negative while generating realistic random-walk price paths.\"</i>"
    flowables.append(Paragraph(q2, body_style))
    flowables.append(Spacer(1, 4))
    
    q3 = "<b>Q3: How do you guarantee data consistency and prevent race conditions when two orders execute concurrently?</b><br/>" \
         "<b>Answer:</b> <i>\"Order execution involves multiple state transitions: deducting wallet funds, creating order records, updating holdings, and appending to transaction logs. I wrap these multi-table operations within MySQL ACID transactions (<code>BEGIN ... COMMIT / ROLLBACK</code>) combined with C++ mutex locks. If any step fails—such as a user having insufficient margin—the entire transaction rolls back, guaranteeing zero negative balances and 100% financial consistency.\"</i>"
    flowables.append(Paragraph(q3, body_style))
    
    # Build PDF
    build_pdf(pdf_path, "Person 3: Database, Ledger & Simulation", flowables)
    print(f"Generated PDF: {pdf_path}")
    
    # Generate Markdown
    md_content = """# TradeX Technical Defense Guide — Person 3
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
  $$\text{AvgPrice}_{\text{new}} = \frac{Q_{\text{old}} \cdot P_{\text{old}} + Q_{\text{buy}} \cdot P_{\text{buy}}}{Q_{\text{old}} + Q_{\text{buy}}}$$
- Real-time mark-to-market Unrealized P&L:
  $$\text{Unrealized P&L} = (\text{CurrentPrice} - \text{AvgPrice}) \cdot \text{Quantity}$$

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
"""
    with open(md_path, "w", encoding="utf-8") as f:
        f.write(md_content)
    print(f"Generated Markdown: {md_path}")

if __name__ == "__main__":
    generate_person_3()
