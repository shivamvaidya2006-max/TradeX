# TradeX REST & WebSocket API Specification

All REST endpoints are rooted under `/api/v1`. Protected routes require the standard HTTP header:
```http
Authorization: Bearer <jwt_token>
```

---

## 1. Authentication APIs

### 1.1 User Registration
- **Endpoint:** `POST /api/v1/auth/register`
- **Access:** Public
- **Request Body:**
```json
{
  "fullName": "Rahul Sharma",
  "email": "rahul@example.com",
  "phone": "+919876543210",
  "username": "rahul2026",
  "password": "secretpassword"
}
```
- **Response (`201 Created`):**
```json
{
  "status": "success",
  "message": "User registered successfully",
  "token": "eyJhbGciOi...",
  "user": {
    "id": 3,
    "username": "rahul2026",
    "fullName": "Rahul Sharma",
    "email": "rahul@example.com",
    "role": "USER"
  }
}
```

### 1.2 User Login
- **Endpoint:** `POST /api/v1/auth/login`
- **Access:** Public
- **Request Body:**
```json
{
  "username": "trader",
  "password": "trader123"
}
```
- **Response (`200 OK`):**
```json
{
  "status": "success",
  "token": "eyJhbGciOi...",
  "user": {
    "id": 1,
    "username": "trader",
    "fullName": "Demo Trader",
    "email": "trader@tradex.local",
    "role": "USER"
  }
}
```

---

## 2. Stock & Market APIs

### 2.1 Get All Stocks
- **Endpoint:** `GET /api/v1/stocks`
- **Access:** Public
- **Response:** Array of stock quote objects with current price, day high, day low, volume, and percent changes.

### 2.2 Get Stock by Symbol
- **Endpoint:** `GET /api/v1/stocks/{symbol}`
- **Access:** Public
- **Example:** `GET /api/v1/stocks/RELIANCE`

### 2.3 Prefix Autocomplete (Trie)
- **Endpoint:** `GET /api/v1/stocks/search?q={query}`
- **Access:** Public
- **Example:** `GET /api/v1/stocks/search?q=TA`
- **Response:** Array of matched stocks found in Trie prefix tree.

### 2.4 Get Price History (OHLCV)
- **Endpoint:** `GET /api/v1/stocks/{symbol}/history?range={1D|1W|1M|6M|1Y|5Y}`
- **Access:** Public
- **Response:** Time-series array of `{ timestamp, open, high, low, close, volume }`.

---

## 3. Order Management APIs

### 3.1 Place Buy/Sell Order
- **Endpoint:** `POST /api/v1/orders`
- **Access:** Protected (JWT)
- **Request Body:**
```json
{
  "symbol": "RELIANCE",
  "transactionType": "BUY",
  "orderType": "MARKET",
  "quantity": 10,
  "price": 2840.50,
  "triggerPrice": 0.00
}
```
- **Response (`201 Created`):**
```json
{
  "status": "success",
  "message": "Order executed immediately",
  "data": {
    "id": 42,
    "symbol": "RELIANCE",
    "status": "EXECUTED",
    "quantity": 10,
    "price": 2840.50
  }
}
```

### 3.2 Get User Orders
- **Endpoint:** `GET /api/v1/orders`
- **Access:** Protected (JWT)
- **Response:** List of all user orders across all execution statuses (`EXECUTED`, `PENDING`, `CANCELLED`, `REJECTED`).

### 3.3 Cancel Pending Order
- **Endpoint:** `DELETE /api/v1/orders/{orderId}`
- **Access:** Protected (JWT)

---

## 4. Portfolio & Funds APIs

### 4.1 Get Full Portfolio
- **Endpoint:** `GET /api/v1/portfolio`
- **Access:** Protected (JWT)
- **Response:**
```json
{
  "data": {
    "holdings": [...],
    "positions": [...],
    "wallet": {
      "availableBalance": 85000.00,
      "investedAmount": 15000.00,
      "withdrawableAmount": 85000.00,
      "totalAccountValue": 100000.00
    },
    "summary": {
      "totalInvested": 15000.00,
      "totalCurrentValue": 15650.00,
      "totalPnl": 650.00,
      "totalPnlPercent": 4.33,
      "todayPnl": 210.00,
      "todayPnlPercent": 1.36
    }
  }
}
```

### 4.2 Deposit Simulated Funds
- **Endpoint:** `POST /api/v1/funds/deposit`
- **Access:** Protected (JWT)
- **Request Body:** `{"amount": 50000}`

### 4.3 Withdraw Simulated Funds
- **Endpoint:** `POST /api/v1/funds/withdraw`
- **Access:** Protected (JWT)
- **Request Body:** `{"amount": 10000}`

---

## 5. DSA Laboratory APIs

| Endpoint | Method | Complexity | Description |
|---|---|---|---|
| `/api/v1/dsa/trie/visualize?q={prefix}` | GET | $O(L)$ | Subtree visual structure of Trie node hierarchy |
| `/api/v1/dsa/orderbook/{symbol}` | GET | $O(\log N)$ | Order depth ladder from C++ Max-Heap & Min-Heap |
| `/api/v1/dsa/bst/tree` | GET | $O(N)$ | Inorder, Preorder, Postorder traversals of Price BST |
| `/api/v1/dsa/bst/insert` | POST | $O(\log N)$ | Dynamically inserts price level into C++ BST |
| `/api/v1/dsa/graph/relations` | GET | $O(V + E)$ | Returns Sector-Stock adjacency graph vertices and edges |
| `/api/v1/dsa/graph/bfs?start={node}` | GET | $O(V + E)$ | Executes Breadth-First Search traversal |
| `/api/v1/dsa/graph/dfs?start={node}` | GET | $O(V + E)$ | Executes Depth-First Search traversal |
| `/api/v1/dsa/sort` | POST | Variable | Executes one of 6 C++ sorting algorithms with comparison/swap counters |
| `/api/v1/dsa/search` | POST | $O(N)$ vs $O(\log N)$ | Executes Linear or Binary search with low, mid, high trace |
| `/api/v1/dsa/analytics/sliding-window` | GET | $O(N)$ | Finds best $K$-day growth window and longest positive streak |
| `/api/v1/dsa/greedy/allocate` | POST | $O(N \log N)$ | Sharpe-ratio heuristic greedy capital allocation |
| `/api/v1/dsa/dp/max-profit` | GET | $O(N)$ / $O(KN)$ | Single, unlimited, and 2-transaction Dynamic Programming |
| `/api/v1/dsa/audit-trail` | GET | $O(1)$ | Real-time events from C++ custom `DoublyLinkedList<T>` |

---

## 6. RFC-6455 WebSocket Protocol

- **Endpoint:** `ws://localhost:8080/ws`
- **Payload Format:** Unmasked UTF-8 JSON text frames.

### Event: `PRICE_TICK`
Broadcast periodically by the Brownian Motion market simulator:
```json
{
  "type": "PRICE_TICK",
  "symbol": "RELIANCE",
  "price": 2845.20,
  "change": 12.50,
  "changePercent": 0.44,
  "dayHigh": 2860.00,
  "dayLow": 2820.00,
  "volume": 4210900
}
```
