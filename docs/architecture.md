# TradeX System Architecture Specification

## 1. Executive Summary

**TradeX** is an enterprise-grade, full-stack educational paper-trading and stock market simulation platform modeled after modern institutional brokerage architectures. Built to demonstrate high-performance systems engineering, algorithmic trading concepts, and advanced Data Structures and Algorithms (DSA), TradeX pairs a low-latency native **C++17** backend engine with a **MySQL 8.0** relational persistence store and a modern **React 19 / TypeScript / Tailwind CSS** reactive web dashboard.

---

## 2. High-Level Architecture Diagram

```mermaid
graph TD
    Client["Web Client (React 19 + TypeScript + Vite)"]
    
    subgraph Frontend Layer
        UI["Tailwind CSS Financial UI"]
        Context["Market & Auth Context (State Mgmt)"]
        Charts["Recharts Interactive Financial Charts"]
        WSClient["RFC-6455 WebSocket Client"]
        AxiosClient["REST Client (Axios)"]
    end

    subgraph Transport Layer
        HTTP["HTTP/1.1 REST Gateway (:8080)"]
        WS["WebSocket Server (:8080)"]
    end

    subgraph C++ Core Backend Engine
        Router["HttpServer & Dispatcher"]
        AuthSvc["AuthService (JWT + SHA-256)"]
        OrderSvc["OrderService Pipeline"]
        MatchingEng["Double-Auction Matching Engine"]
        PortSvc["Portfolio & PnL Service"]
        WalletSvc["Wallet & Margin Service"]
        Simulator["Brownian Motion Market Simulator"]
        Cache["LRU Cache + Stock Memory Cache"]
    end

    subgraph C++ DSA Subsystems
        Trie["Trie Prefix Tree (Stock Autocomplete)"]
        Heap["Priority Queues (Max-Heap Bids / Min-Heap Asks)"]
        BST["Binary Search Tree (Dynamic Price Index)"]
        Graph["Adjacency Graph (Sector-Stock BFS/DFS)"]
        Sort["6 Sorting Algorithms (Bubble, Selection, Insert, Merge, Quick, Heap)"]
        Search["Linear & Binary Search with Step Tracing"]
        Momentum["Sliding Window & Two-Pointer Momentum Engine"]
        Greedy["Greedy Portfolio Allocator (Sharpe-Heuristic)"]
        DP["Dynamic Programming Stock Profit Maximizer"]
        Audit["Doubly Linked List & Action Stack"]
    end

    subgraph Persistence Layer
        DBPool["Native MySQL Connection Pool (libmysql)"]
        MySQL[("MySQL 8.0 Database (12 Tables)")]
    end

    Client --> UI
    UI --> Context
    Context --> WSClient
    Context --> AxiosClient
    
    AxiosClient -->|HTTP REST| HTTP
    WSClient -->|Persistent WSS/WS| WS

    HTTP --> Router
    WS --> Router

    Router --> AuthSvc
    Router --> OrderSvc
    Router --> PortSvc
    Router --> WalletSvc
    Router --> Trie
    Router --> Heap
    Router --> BST
    Router --> Graph
    Router --> Sort
    Router --> Search
    Router --> Momentum
    Router --> Greedy
    Router --> DP

    OrderSvc --> MatchingEng
    MatchingEng --> PortSvc
    MatchingEng --> WalletSvc
    MatchingEng --> Audit

    Simulator -->|Broadcast Price Ticks| WS
    Simulator -->|Update Quotes| Cache

    OrderSvc --> DBPool
    PortSvc --> DBPool
    WalletSvc --> DBPool
    AuthSvc --> DBPool
    DBPool --> MySQL
```

---

## 3. Layer Breakdown

### 3.1 Presentation Layer (Frontend)
- **Framework:** React 19 with TypeScript, bundled with Vite 8.
- **Styling:** Tailwind CSS 4 with a dark fintech color scheme (slate-950 background, emerald-400 profit, rose-400 loss, blue-500 primary accents).
- **Charting:** Recharts SVG vector graphs with multi-timeframe toggles (1D, 1W, 1M, 6M, 1Y, 5Y), custom tooltips, and linear gradients.
- **Real-Time Data:** Persistent WebSocket connection receiving automated Brownian motion ticks, updating the centralized `MarketContext` and triggering CSS flash animations.

### 3.2 Native C++ Core Engine
- **Language Standard:** C++17 compiled via LLVM-MinGW Clang++ (x86_64).
- **Network I/O:** Multi-threaded Winsock2 socket server with thread pooling and non-blocking socket handling.
- **TCP Chunk Assembly:** Robust loop handling fragmented TCP transfers according to the `Content-Length` HTTP/1.1 standard.
- **WebSocket Protocol:** RFC-6455 compliant handshake with SHA-1 and Base64 Sec-WebSocket-Accept key generation, supporting unmasked server-to-client frame streaming.

### 3.3 Persistence Layer
- **RDBMS:** MySQL 8.0 Enterprise.
- **Database Driver:** Native `libmysql` C API linked dynamically (`libmysql.dll`).
- **Connection Pooling:** Thread-safe connection pool with automatic connection recycling and parameterized SQL statements to eliminate SQL injection.
- **Schema Design:** 12 3NF normalized tables with B-Tree indexes on lookup keys and foreign keys.

---

## 4. Concurrency & Threading Model

```mermaid
sequenceDiagram
    participant Timer as MarketSimulator Thread
    participant Server as HttpServer Thread Pool
    participant Engine as MatchingEngine / OrderService
    participant DB as MySQL Connection Pool
    participant WS as WebSocket Clients

    Timer->>Timer: Step Brownian Motion Prices
    Timer->>WS: Broadcast PRICE_TICK Frame
    
    Server->>Server: Accept TCP Client Connection
    Server->>Server: Read Headers & Body (Content-Length loop)
    Server->>Engine: Validate & Dispatch POST /api/v1/orders
    Engine->>DB: Check Margin in Wallet (Acquire Conn)
    DB-->>Engine: Available Balance Confirmed
    Engine->>Engine: Push Order to Heap (Max-Heap/Min-Heap)
    Engine->>Engine: Match Bids vs Asks (Double-Auction)
    Engine->>DB: Insert Orders, Transactions, Holdings
    Engine-->>Server: HTTP 201 Created (JSON Response)
    Server-->>WS: Broadcast ORDER_EXECUTED Event
```

---

## 5. Security & Isolation Model
1. **Zero Real-World Broker Connectivity:** Educational sandbox with no live exchange or bank APIs.
2. **Password Cryptography:** Secure SHA-256 password hashing with user-specific salts; plain-text passwords are never stored.
3. **Stateless Authorization:** Cryptographic JSON Web Tokens (JWT) using HMAC-SHA256 signature verification on every protected endpoint.
4. **Role-Based Access Control (RBAC):** Strict separation between standard paper-trader accounts (`USER`) and simulation managers (`ADMIN`).
