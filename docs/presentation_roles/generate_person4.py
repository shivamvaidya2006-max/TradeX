import os
from reportlab.lib import colors
from reportlab.platypus import Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
from pdf_utils import build_pdf
from builder_helpers import (
    title_style, subtitle_style, h1_style, h2_style, body_style, bullet_style,
    code_style, make_callout, make_code_box, make_meta_card, th_style, td_style, td_code_style,
    COLOR_PRIMARY, COLOR_ACCENT, COLOR_BORDER, COLOR_CARD_BG
)

def generate_person_4():
    pdf_path = os.path.join(os.path.dirname(__file__), "Person_4_DSA_Analytics_Market_Intelligence.pdf")
    md_path = os.path.join(os.path.dirname(__file__), "Person_4_DSA_Analytics_Market_Intelligence.md")
    
    files = [
        "backend/include/services/StockService.hpp",
        "backend/include/services/CacheService.hpp",
        "backend/include/dsa/Trie.hpp",
        "backend/include/dsa/MarketGraph.hpp",
        "backend/include/dsa/SortingAlgorithms.hpp",
        "backend/include/dsa/SearchingAlgorithms.hpp",
        "backend/include/dsa/MarketAnalyticsService.hpp",
        "backend/include/dsa/GreedyPortfolioService.hpp",
        "backend/include/dsa/DynamicProgrammingService.hpp",
        "backend/include/dsa/LinkedList.hpp",
        "backend/include/dsa/Stack.hpp",
        "backend/tests/test_dsa.cpp"
    ]
    
    flowables = []
    
    # Title
    flowables.append(Paragraph("TradeX B.Tech Capstone — Technical Defense Guide", title_style))
    flowables.append(Paragraph("Person 4: Algorithmic Trading, Market Intelligence & Advanced DSA Architect", subtitle_style))
    flowables.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_ACCENT, spaceAfter=10))
    
    # Meta Card
    flowables.append(make_meta_card(
        role_num=4,
        title="Algorithmic Trading, Market Intelligence & Advanced DSA Architect",
        domain="Stock Catalog & LRU Caching, Prefix Trie Autocomplete, Sector-Equity Graph (BFS/DFS), Sorting Suite (6 Algorithms), Searching Suite (Linear vs Binary), Sliding Window & Two-Pointer Momentum, Greedy Capital Allocation, Dynamic Programming Stock Profit Maximizer, Doubly Linked List & Stack Audit Trails",
        files_list=files,
        presentation_time="5 to 7 Minutes"
    ))
    flowables.append(Spacer(1, 10))
    
    # 1. Executive Summary
    flowables.append(Paragraph("1. Executive Summary & Architectural Ownership", h1_style))
    flowables.append(Paragraph(
        "Person 4 represents the pure algorithmic intelligence of the TradeX platform. "
        "This engineer designs and maintains the rich Data Structures and Algorithms (DSA) embedded natively in C++. "
        "Person 4 replaces slow database full-table scans with an in-memory Prefix Tree (Trie) for O(L) stock search, "
        "models market contagion with an Adjacency List Graph using BFS and DFS, benchmarks 6 native sorting algorithms, "
        "implements optimal Sliding Window and Two-Pointer momentum analytics, allocates virtual capital with a Greedy "
        "Sharpe-ratio heuristic, and solves multi-trade profit maximization using Dynamic Programming (DP).",
        body_style
    ))
    
    flowables.append(make_callout(
        "<b>Core Defense Pitch:</b> <i>\"I engineered the algorithmic intelligence and advanced DSA subsystem of TradeX. "
        "Instead of scanning database tables on every keystroke, my Prefix Trie retrieves stock autocomplete suggestions in O(L) microsecond time. "
        "I built an Adjacency List Graph to explore sector-equity systemic risk via BFS and DFS traversals, a Sliding Window engine for momentum analytics, "
        "a Greedy allocator optimizing portfolio risk-adjusted returns, and a multi-transaction Dynamic Programming optimizer that calculates theoretical "
        "maximum profit bounds in O(K &times; N) time. All 10 algorithms run live in C++ and are visually demonstrable on our DSA Lab dashboard.\"</i>"
    ))
    flowables.append(Spacer(1, 8))
    
    # 2. File-by-File Breakdown
    flowables.append(Paragraph("2. Assigned Files & Detailed Code Responsibilities", h1_style))
    
    f1 = "<b>backend/include/dsa/Trie.hpp</b> (Prefix Tree for Autocomplete):<br/>" \
         "• Node structure: <code>TrieNode</code> with <code>std::unordered_map&lt;char, std::unique_ptr&lt;TrieNode&gt;&gt; children</code>, <code>isEndOfWord</code> flag, and equity metadata pointer.<br/>" \
         "• <code>insert(symbol, data)</code>: O(L) character-by-character tree insertion.<br/>" \
         "• <code>suggest(prefix, limit)</code>: Navigates to prefix root in O(L), then executes Depth-First Search (DFS) to collect K matching suggestions.<br/>" \
         "• <code>getVisualTree()</code>: Serializes the in-memory tree nodes for real-time hierarchical SVG rendering on the frontend."
    flowables.append(Paragraph(f1, body_style))
    
    f2 = "<b>backend/include/dsa/MarketGraph.hpp</b> (Adjacency List Graph for Sector Contagion):<br/>" \
         "• Graph model: Bipartite graph representing Sectors (IT, Banking, Energy) and Equities with directed/undirected edges.<br/>" \
         "• <b>Breadth-First Search (BFS):</b> Uses <code>std::queue</code> to traverse equity contagion level-by-level in O(V + E) time.<br/>" \
         "• <b>Depth-First Search (DFS):</b> Uses recursion / <code>std::stack</code> to explore deep risk propagation paths across financial clusters."
    flowables.append(Paragraph(f2, body_style))
    
    f3 = "<b>backend/include/dsa/SortingAlgorithms.hpp</b> (Sorting Benchmark Suite):<br/>" \
         "• Implements 6 distinct algorithms: <b>Bubble, Selection, Insertion, Merge, Quick, and Heap Sort</b>.<br/>" \
         "• Instrumentation: Records comparisons count, swaps count, execution duration in microseconds, and captures snapshots of the array at every step (<code>SortStep</code>) for UI animation."
    flowables.append(Paragraph(f3, body_style))
    
    f4 = "<b>backend/include/dsa/SearchingAlgorithms.hpp</b> (Search Pointer Tracing):<br/>" \
         "• Implements <b>Linear Search</b> (O(N)) and <b>Binary Search</b> (O(log N)).<br/>" \
         "• Captures pointer states (<code>low</code>, <code>mid</code>, <code>high</code>) at each iteration to demonstrate logarithmic search reduction."
    flowables.append(Paragraph(f4, body_style))
    
    f5 = "<b>backend/include/dsa/MarketAnalyticsService.hpp</b> (Sliding Window &amp; Two-Pointer):<br/>" \
         "• <b>Sliding Window:</b> Maintains an active window of size K over closing prices in O(N) single-pass time to locate the period of maximum percentage growth.<br/>" \
         "• <b>Two-Pointer:</b> Traverses price series with <code>slow</code> and <code>fast</code> pointers to identify the longest consecutive streak of green trading days."
    flowables.append(Paragraph(f5, body_style))
    
    f6 = "<b>backend/include/dsa/GreedyPortfolioService.hpp</b> (Sharpe-Ratio Capital Allocator):<br/>" \
         "• Evaluates candidate equities based on a Sharpe-ratio heuristic: <i>Score = ExpectedReturn / RiskScore</i>.<br/>" \
         "• Sorts candidates in O(N log N) time and greedily allocates available capital into highest-ranked assets."
    flowables.append(Paragraph(f6, body_style))
    
    f7 = "<b>backend/include/dsa/DynamicProgrammingService.hpp</b> (DP Stock Profit Maximizer):<br/>" \
         "• <b>1 Transaction:</b> O(N) time, O(1) space tracking running minimum price.<br/>" \
         "• <b>Unlimited Transactions:</b> O(N) time greedy peak-valley aggregator summing all positive price increments.<br/>" \
         "• <b>K Transactions:</b> O(K &times; N) 2D Dynamic Programming table optimization calculating maximum profit under trade count limits."
    flowables.append(Paragraph(f7, body_style))
    
    f8 = "<b>backend/include/dsa/LinkedList.hpp &amp; Stack.hpp</b> (Zero-Allocation Audit Trail):<br/>" \
         "• Doubly Linked List with O(1) append-tail and bidirectional traversal.<br/>" \
         "• Action Stack supporting LIFO trade undo simulation and audit rollback."
    flowables.append(Paragraph(f8, body_style))
    flowables.append(Spacer(1, 8))
    
    # 3. DSA Implementation
    flowables.append(Paragraph("3. Data Structures & Algorithms Complexity Matrix (Person 4)", h1_style))
    
    dsa_table_data = [
        [Paragraph("Data Structure / Algorithm", th_style), Paragraph("Time Complexity", th_style), Paragraph("Space Complexity", th_style), Paragraph("Financial Engineering Role", th_style)],
        [Paragraph("<b>Prefix Tree (Trie)</b>", td_style), Paragraph("Search: O(L)<br/>Insert: O(L)", td_style), Paragraph("O(N &times; L &times; &Sigma;)", td_style), Paragraph("Microsecond stock symbol autocomplete without database queries.", td_style)],
        [Paragraph("<b>Graph BFS &amp; DFS</b>", td_style), Paragraph("O(V + E)", td_style), Paragraph("O(V + E)", td_style), Paragraph("Sector-equity cluster contagion analysis and risk modeling.", td_style)],
        [Paragraph("<b>Sorting Suite (6 Algos)</b>", td_style), Paragraph("O(N log N) to O(N<sup>2</sup>)", td_style), Paragraph("O(1) to O(N)", td_style), Paragraph("Order execution queue sorting and algorithmic trade tape benchmarking.", td_style)],
        [Paragraph("<b>Sliding Window Analytics</b>", td_style), Paragraph("O(N) single-pass", td_style), Paragraph("O(1) Auxiliary", td_style), Paragraph("Locates best K-day holding period and longest consecutive gain run.", td_style)],
        [Paragraph("<b>Greedy Capital Allocator</b>", td_style), Paragraph("O(N log N)", td_style), Paragraph("O(N) allocations", td_style), Paragraph("Sharpe-ratio heuristic fractional capital distribution.", td_style)],
        [Paragraph("<b>Dynamic Programming (K Trades)</b>", td_style), Paragraph("O(K &times; N)", td_style), Paragraph("O(K &times; N)", td_style), Paragraph("Calculates theoretical maximum profit bounds across historical timeframes.", td_style)],
        [Paragraph("<b>Doubly Linked List &amp; Stack</b>", td_style), Paragraph("O(1) Insert / Delete", td_style), Paragraph("O(N)", td_style), Paragraph("Zero-allocation in-memory immutable execution audit trail.", td_style)],
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
    flowables.append(Paragraph("4. How Person 4's Backend Connects to the Frontend", h1_style))
    flowables.append(Paragraph(
        "• <b>Trie Autocomplete in Navbar:</b> As the user types in <code>frontend/src/components/Navbar.tsx</code>, a debounced query calls <code>GET /api/v1/stocks/search?q=...</code>. "
        "<code>StockService::searchStocks</code> delegates to Person 4's Trie, returning matching equities in under 2 milliseconds.<br/>"
        "• <b>Interactive DSA Lab Dashboard:</b> <code>frontend/src/pages/DsaLab.tsx</code> features 10 visual tabs connecting directly to Person 4's C++ endpoints: "
        "<code>/dsa/trie/visualize</code>, <code>/dsa/graph/relations</code>, <code>/dsa/sort</code>, <code>/dsa/search</code>, <code>/dsa/analytics/sliding-window</code>, <code>/dsa/greedy/allocate</code>, <code>/dsa/dp/max-profit</code>, and <code>/dsa/audit-trail</code>.<br/>"
        "• <b>Multi-Timeframe Vector Charts:</b> <code>frontend/src/components/PriceChart.tsx</code> fetches historical OHLCV data accelerated by Person 4's in-memory LRU Cache.",
        body_style
    ))
    flowables.append(Spacer(1, 8))
    
    # 5. Viva / Defense Q&A
    flowables.append(Paragraph("5. B.Tech Defense & Viva Voce Q&A (Ready to Speak)", h1_style))
    
    q1 = "<b>Q1: Why is a Trie superior to a SQL `LIKE '%query%'` for stock autocomplete search?</b><br/>" \
         "<b>Answer:</b> <i>\"A SQL <code>LIKE '%query%'</code> cannot utilize standard B-Tree index prefixes because of the leading wildcard, forcing the database engine to perform a full table scan of complexity O(N &times; M), where N is the row count and M is string length. On every keystroke from hundreds of concurrent traders, this creates severe database connection bottlenecks. In contrast, our in-memory Prefix Trie navigates directly to the prefix root node in O(L) time (where L is prefix length) and retrieves top K suggestions via Depth-First Search in O(L + K) time directly from RAM in microseconds, completely bypassing the database.\"</i>"
    flowables.append(Paragraph(q1, body_style))
    flowables.append(Spacer(1, 4))
    
    q2 = "<b>Q2: Explain the Dynamic Programming recurrence relation for maximizing stock profit with K transactions.</b><br/>" \
         "<b>Answer:</b> <i>\"Let <code>DP[i][j]</code> be the maximum profit on day <code>j</code> with at most <code>i</code> transactions. On day <code>j</code>, we have two choices: either do not trade on day <code>j</code> (profit = <code>DP[i][j-1]</code>), or sell a stock bought on some earlier day <code>m &isin; [0, j-1]</code> (profit = <code>Prices[j] - Prices[m] + DP[i-1][m]</code>). The recurrence is: <code>DP[i][j] = max(DP[i][j-1], Prices[j] + max_diff)</code>, where <code>max_diff = max(max_diff, DP[i-1][j] - Prices[j])</code>. By maintaining running <code>max_diff</code>, we reduce the time complexity from O(K &times; N<sup>2</sup>) to an optimal O(K &times; N).\"</i>"
    flowables.append(Paragraph(q2, body_style))
    flowables.append(Spacer(1, 4))
    
    q3 = "<b>Q3: How does your Market Graph use BFS to detect systemic sector risk?</b><br/>" \
         "<b>Answer:</b> <i>\"In <code>MarketGraph.hpp</code>, equities and market sectors are modeled as an Adjacency List Graph. When a major sector node (such as Banking) experiences extreme volatility, my Breadth-First Search (BFS) traverses the graph level-by-level using a FIFO queue in O(V + E) time. Level 1 identifies direct constituent equities (HDFC, ICICI, SBI), while Level 2 identifies correlated supplier sectors and cross-holding conglomerates (e.g. Reliance, L&amp;T). This provides institutional-grade contagion impact analysis.\"</i>"
    flowables.append(Paragraph(q3, body_style))
    
    # Build PDF
    build_pdf(pdf_path, "Person 4: Advanced DSA & Analytics", flowables)
    print(f"Generated PDF: {pdf_path}")
    
    # Generate Markdown
    md_content = """# TradeX Technical Defense Guide — Person 4
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
- Computes Sharpe-ratio heuristic: $\text{Score} = \text{ExpectedReturn} / \text{RiskScore}$.
- Sorts candidates in $O(N \log N)$ time and greedily distributes capital into highest-ranked assets.

#### `backend/include/dsa/DynamicProgrammingService.hpp` (DP Stock Profit)
- **1 Transaction:** $O(N)$ time, $O(1)$ space tracking the running minimum price.
- **Unlimited Transactions:** $O(N)$ time summing all upward price jumps.
- **$K$ Transactions:** $O(K \times N)$ 2D Dynamic Programming table calculation:
  $$DP[i][j] = \max(DP[i][j-1], \text{Prices}[j] + \text{max\_diff})$$
  $$\text{max\_diff} = \max(\text{max\_diff}, DP[i-1][j] - \text{Prices}[j])$$

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
  *Answer:* $DP[i][j] = \max(DP[i][j-1], \text{Prices}[j] + \text{max\_diff})$, where $\text{max\_diff} = \max(\text{max\_diff}, DP[i-1][j] - \text{Prices}[j])$. Maintaining `max_diff` reduces complexity from $O(K \cdot N^2)$ to $O(K \cdot N)$.
- **Q3: How does the Market Graph use BFS to detect systemic sector risk?**  
  *Answer:* It models sectors and equities as an Adjacency List Graph. When a sector experiences shock, BFS traverses constituent equities (Level 1) and correlated cross-holding industries (Level 2) in $O(V + E)$ time to measure contagion spread.
"""
    with open(md_path, "w", encoding="utf-8") as f:
        f.write(md_content)
    print(f"Generated Markdown: {md_path}")

if __name__ == "__main__":
    generate_person_4()
