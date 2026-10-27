import os
from reportlab.lib import colors
from reportlab.platypus import Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
from pdf_utils import build_pdf
from builder_helpers import (
    title_style, subtitle_style, h1_style, h2_style, body_style, bullet_style,
    code_style, make_callout, make_code_box, make_meta_card, th_style, td_style, td_code_style,
    COLOR_PRIMARY, COLOR_ACCENT, COLOR_BORDER, COLOR_CARD_BG
)

def generate_person_2():
    pdf_path = os.path.join(os.path.dirname(__file__), "Person_2_Matching_Engine_Order_Book.pdf")
    md_path = os.path.join(os.path.dirname(__file__), "Person_2_Matching_Engine_Order_Book.md")
    
    files = [
        "backend/include/dsa/MatchingEngine.hpp",
        "backend/include/dsa/OrderBook.hpp",
        "backend/include/dsa/OrderQueue.hpp",
        "backend/include/dsa/BST.hpp",
        "backend/include/services/OrderService.hpp"
    ]
    
    flowables = []
    
    # Title
    flowables.append(Paragraph("TradeX B.Tech Capstone — Technical Defense Guide", title_style))
    flowables.append(Paragraph("Person 2: Exchange Matching Engine & High-Frequency Order Architect", subtitle_style))
    flowables.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_ACCENT, spaceAfter=10))
    
    # Meta Card
    flowables.append(make_meta_card(
        role_num=2,
        title="Exchange Matching Engine & High-Frequency Order Architect",
        domain="Continuous Double-Auction Matching Engine, Binary Heaps (Priority Queues), Order Books, Price-Time Priority FIFO Queues, Binary Search Tree (BST) Price Indexing, Order Pipeline",
        files_list=files,
        presentation_time="5 to 7 Minutes"
    ))
    flowables.append(Spacer(1, 10))
    
    # 1. Executive Summary
    flowables.append(Paragraph("1. Executive Summary & Architectural Ownership", h1_style))
    flowables.append(Paragraph(
        "Person 2 owns the institutional core of TradeX: the Continuous Double-Auction matching engine and order execution pipeline. "
        "Every time a user places an order on the frontend, Person 2's code validates margin, organizes bids and asks in native C++ "
        "binary priority queues (Max-Heap for buyers, Min-Heap for sellers), resolves price-time priority using FIFO queues, matches orders "
        "deterministically at resting order prices, and exposes Level-2 5-depth Market Depth quotes. Person 2 also implements a "
        "Binary Search Tree (BST) for ascending price ladder indexing and traversals.",
        body_style
    ))
    
    flowables.append(make_callout(
        "<b>Core Defense Pitch:</b> <i>\"I implemented the core financial exchange matching engine in modern C++. "
        "My architecture maintains an active order book per equity instrument using two inverted binary heaps: a Max-Heap "
        "ensuring buyers offering the highest prices execute first, and a Min-Heap ensuring sellers offering the lowest prices execute first. "
        "Whenever Best Bid &ge; Best Ask, my Continuous Double-Auction matching engine executes trades at the maker price in O(log N) time, "
        "handling partial fills, timestamp tie-breaking via FIFO queues, and dynamic price indexing with a Binary Search Tree.\"</i>"
    ))
    flowables.append(Spacer(1, 8))
    
    # 2. File-by-File Breakdown
    flowables.append(Paragraph("2. Assigned Files & Detailed Code Responsibilities", h1_style))
    
    f1 = "<b>backend/include/dsa/OrderBook.hpp</b> (Dual-Heap Level-2 Order Book):<br/>" \
         "• Implements <code>LimitOrder</code> struct with <code>id</code>, <code>userId</code>, <code>price</code>, <code>quantity</code>, and <code>timestamp</code>.<br/>" \
         "• <b>Max-Heap for Bids:</b> Comparator <code>BuyComparator</code>: Highest price has top priority. If prices match, older timestamp executes first.<br/>" \
         "• <b>Min-Heap for Asks:</b> Comparator <code>SellComparator</code>: Lowest price has top priority. If prices match, older timestamp executes first.<br/>" \
         "• <code>getSnapshot(int depth = 5)</code>: Aggregates order levels into top 5 price bands, volume sums, and order counts for frontend market depth."
    flowables.append(Paragraph(f1, body_style))
    
    f2 = "<b>backend/include/dsa/MatchingEngine.hpp</b> (Continuous Double-Auction Core):<br/>" \
         "• <code>matchOrders(OrderBook&amp; book)</code>: Core algorithmic loop running while <code>book.hasBuyOrders() &amp;&amp; book.hasSellOrders()</code>.<br/>" \
         "• <b>Execution Condition:</b> Checks <code>bestBuy-&gt;price &gt;= bestSell-&gt;price</code>. If true, matched quantity = <code>min(buyQty, sellQty)</code>.<br/>" \
         "• <b>Execution Price Determination:</b> Follows the <i>Maker-Taker Principle</i>: execution price matches the resting order placed earlier in time.<br/>" \
         "• <b>Quantity Decrements &amp; Heap Eviction:</b> Subtracts matched quantity; pops fully filled orders (<code>popBuy()</code> / <code>popSell()</code>); triggers <code>TradeCallback</code>."
    flowables.append(Paragraph(f2, body_style))
    
    f3 = "<b>backend/include/dsa/OrderQueue.hpp</b> (Price-Time Priority Queue):<br/>" \
         "• Implements a FIFO queue (using <code>std::deque&lt;std::shared_ptr&lt;LimitOrder&gt;&gt;</code>) to guarantee fair execution order among traders bidding identical prices.<br/>" \
         "• Supports O(1) push back, O(1) pop front, and O(N) linear order cancellation."
    flowables.append(Paragraph(f3, body_style))
    
    f4 = "<b>backend/include/dsa/BST.hpp</b> (Binary Search Tree for Dynamic Price Indexing):<br/>" \
         "• Node structure: <code>BSTNode</code> storing <code>price</code>, <code>symbol</code>, <code>volume</code>, <code>left</code>, and <code>right</code> pointers.<br/>" \
         "• Insertion &amp; Search: Average O(log N) binary tree insertion and lookup.<br/>" \
         "• Traversals: Implements <b>Inorder</b> (ascending sorted price ladder), <b>Preorder</b>, and <b>Postorder</b> tree walks.<br/>" \
         "• Extreme price finding: <code>findMin()</code> and <code>findMax()</code> in O(h) time by following leftmost/rightmost pointer chains."
    flowables.append(Paragraph(f4, body_style))
    
    f5 = "<b>backend/include/services/OrderService.hpp</b> (Order Placement Pipeline):<br/>" \
         "• <code>placeOrder()</code>: Validates margin, generates order ID, and wraps execution in a MySQL database transaction.<br/>" \
         "• Handles Product Types: <b>CNC</b> (Cash &amp; Carry Delivery) vs <b>MIS</b> (Intraday Margin).<br/>" \
         "• Dispatches matched trades to <code>PortfolioService</code> and <code>WalletService</code> to adjust holdings, cash balances, and ledger.<br/>" \
         "• <code>cancelOrder()</code>: Removes open resting limit orders from memory books and marks order as <code>CANCELLED</code> in MySQL."
    flowables.append(Paragraph(f5, body_style))
    flowables.append(Spacer(1, 8))
    
    # 3. DSA Implementation
    flowables.append(Paragraph("3. Data Structures & Algorithms Implemented by Person 2", h1_style))
    
    dsa_table_data = [
        [Paragraph("Data Structure / Algorithm", th_style), Paragraph("Time Complexity", th_style), Paragraph("Space Complexity", th_style), Paragraph("Engineering Role in TradeX", th_style)],
        [Paragraph("<b>Binary Max-Heap (Bids)</b>", td_style), Paragraph("Push: O(log N)<br/>Top: O(1)<br/>Pop: O(log N)", td_style), Paragraph("O(N) contiguous array", td_style), Paragraph("Guarantees the highest-paying buyer always has top priority for incoming market orders.", td_style)],
        [Paragraph("<b>Binary Min-Heap (Asks)</b>", td_style), Paragraph("Push: O(log N)<br/>Top: O(1)<br/>Pop: O(log N)", td_style), Paragraph("O(N) contiguous array", td_style), Paragraph("Guarantees the lowest-cost seller always has top priority for incoming buy orders.", td_style)],
        [Paragraph("<b>Continuous Double-Auction Matching</b>", td_style), Paragraph("O(M log N) where M = executed matches", td_style), Paragraph("O(1) Auxiliary", td_style), Paragraph("Greedy matching algorithm that clears orders when Bid &ge; Ask at the resting order's price.", td_style)],
        [Paragraph("<b>FIFO Order Queue</b>", td_style), Paragraph("Enqueue: O(1)<br/>Dequeue: O(1)", td_style), Paragraph("O(K) per price level", td_style), Paragraph("Enforces strict timestamp priority when multiple limit orders arrive at the exact same price.", td_style)],
        [Paragraph("<b>Binary Search Tree (BST)</b>", td_style), Paragraph("Avg: O(log N)<br/>Worst: O(N)", td_style), Paragraph("O(N) pointer nodes", td_style), Paragraph("Dynamic price ladder indexing and ascending Inorder traversals for visual market ladders.", td_style)],
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
    flowables.append(Paragraph("4. How Person 2's Backend Connects to the Frontend", h1_style))
    flowables.append(Paragraph(
        "• <b>Order Placement Modal:</b> When a user clicks \"BUY\" or \"SELL\" on <code>Markets.tsx</code>, <code>frontend/src/components/OrderModal.tsx</code> submits an HTTP POST to <code>/api/v1/orders</code>. "
        "<code>OrderService::placeOrder</code> parses the request, pushes it into Person 2's dual-heap order book, and triggers <code>MatchingEngine::matchOrders</code>.<br/>"
        "• <b>Market Depth (Order Book) Component:</b> <code>frontend/src/components/OrderBook.tsx</code> polls <code>/api/v1/dsa/orderbook/:symbol</code> every 2.5 seconds. "
        "<code>OrderBook::getSnapshot(5)</code> extracts top 5 bids and asks, displaying cumulative quantities and depth bars.<br/>"
        "• <b>DSA Lab Tab 2 &amp; Tab 3:</b> <code>DsaLab.tsx</code> visualizes the live binary heap priority queues and allows interactive BST price insertions via <code>/api/v1/dsa/bst/insert</code>.",
        body_style
    ))
    flowables.append(Spacer(1, 8))
    
    # 5. Viva / Defense Q&A
    flowables.append(Paragraph("5. B.Tech Defense & Viva Voce Q&A (Ready to Speak)", h1_style))
    
    q1 = "<b>Q1: Why are Priority Queues (Binary Heaps) superior to sorted arrays or linked lists for matching engines?</b><br/>" \
         "<b>Answer:</b> <i>\"In a live exchange, orders arrive and cancel at thousands of ticks per second. A sorted array requires O(N) time for every insertion due to shifting elements in memory. A linked list allows O(1) insertion only if the pointer is already known, but finding the sorted position takes O(N). A Binary Heap offers optimal O(log N) insertion and deletion with O(1) peek time for the best bid or ask, stored in contiguous memory for maximum CPU L1/L2 cache locality.\"</i>"
    flowables.append(Paragraph(q1, body_style))
    flowables.append(Spacer(1, 4))
    
    q2 = "<b>Q2: What is the Continuous Double-Auction mechanism and how is the execution price determined?</b><br/>" \
         "<b>Answer:</b> <i>\"A Continuous Double-Auction continuously matches buyers and sellers as orders arrive. The crossing condition is Best Bid &ge; Best Ask. When a trade occurs, the execution price is determined by the resting order (the Maker order that was already sitting in the book), rather than the incoming aggressive Taker order. This eliminates front-running and provides price improvement to the taker.\"</i>"
    flowables.append(Paragraph(q2, body_style))
    flowables.append(Spacer(1, 4))
    
    q3 = "<b>Q3: How do you handle partial fills when order quantities do not match?</b><br/>" \
         "<b>Answer:</b> <i>\"In <code>MatchingEngine::matchOrders</code>, the executed trade quantity is calculated as <code>min(buyOrder-&gt;quantity, sellOrder-&gt;quantity)</code>. We decrement this matched quantity from both orders. If an order's quantity reaches zero, it is popped from the heap. If remaining quantity &gt; 0, it remains at the top of the heap to match against subsequent orders.\"</i>"
    flowables.append(Paragraph(q3, body_style))
    
    # Build PDF
    build_pdf(pdf_path, "Person 2: Matching Engine & Order Book", flowables)
    print(f"Generated PDF: {pdf_path}")
    
    # Generate Markdown
    md_content = f"""# TradeX Technical Defense Guide — Person 2
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
"""
    with open(md_path, "w", encoding="utf-8") as f:
        f.write(md_content)
    print(f"Generated Markdown: {md_path}")

if __name__ == "__main__":
    generate_person_2()
