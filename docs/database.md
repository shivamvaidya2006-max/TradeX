# TradeX Database Architecture & ER Specification

## 1. Overview
The TradeX persistence layer is implemented in **MySQL 8.0** using a strictly normalized **Third Normal Form (3NF)** relational schema across 12 distinct tables. All financial values (`price`, `balance`, `amount`, `pnl`) are modeled as `DECIMAL(15, 2)` or `DECIMAL(12, 2)` to eliminate IEEE-754 floating-point rounding errors in monetary operations.

---

## 2. Entity-Relationship (ER) Diagram

```mermaid
erDiagram
    USERS ||--o{ ORDERS : places
    USERS ||--o{ HOLDINGS : owns
    USERS ||--o{ POSITIONS : maintains
    USERS ||--|| WALLET : possesses
    USERS ||--o{ TRANSACTIONS : logs
    USERS ||--o{ WATCHLISTS : creates
    USERS ||--o{ NOTIFICATIONS : receives
    USERS ||--o| ADMIN_USERS : role_elevation

    STOCKS ||--o{ ORDERS : references
    STOCKS ||--o{ HOLDINGS : references
    STOCKS ||--o{ POSITIONS : references
    STOCKS ||--o{ WATCHLIST_ITEMS : categorized_in
    STOCKS ||--o{ PRICE_HISTORY : historical_data

    WATCHLISTS ||--o{ WATCHLIST_ITEMS : contains
    ORDERS ||--o{ TRANSACTIONS : settles

    USERS {
        BIGINT id PK
        VARCHAR username UK
        VARCHAR full_name
        VARCHAR email UK
        VARCHAR phone
        VARCHAR password_hash
        ENUM role
        ENUM status
        TIMESTAMP created_at
        TIMESTAMP updated_at
    }

    STOCKS {
        BIGINT id PK
        VARCHAR symbol UK
        VARCHAR company_name
        VARCHAR exchange
        VARCHAR sector
        DECIMAL current_price
        DECIMAL previous_close
        DECIMAL open_price
        DECIMAL day_high
        DECIMAL day_low
        BIGINT volume
        DECIMAL market_cap
        TIMESTAMP updated_at
    }

    WALLET {
        BIGINT id PK
        BIGINT user_id FK,UK
        DECIMAL available_balance
        DECIMAL invested_amount
        DECIMAL withdrawable_amount
        TIMESTAMP updated_at
    }

    ORDERS {
        BIGINT id PK
        BIGINT user_id FK
        BIGINT stock_id FK
        ENUM order_type
        ENUM transaction_type
        INT quantity
        DECIMAL price
        DECIMAL trigger_price
        ENUM status
        TIMESTAMP created_at
        TIMESTAMP executed_at
    }

    HOLDINGS {
        BIGINT id PK
        BIGINT user_id FK
        BIGINT stock_id FK
        INT quantity
        DECIMAL average_price
        TIMESTAMP updated_at
    }

    POSITIONS {
        BIGINT id PK
        BIGINT user_id FK
        BIGINT stock_id FK
        INT quantity
        DECIMAL average_price
        DECIMAL realized_pnl
        DECIMAL unrealized_pnl
        TIMESTAMP updated_at
    }

    TRANSACTIONS {
        BIGINT id PK
        BIGINT user_id FK
        BIGINT order_id FK
        ENUM type
        DECIMAL amount
        VARCHAR description
        TIMESTAMP created_at
    }

    PRICE_HISTORY {
        BIGINT id PK
        BIGINT stock_id FK
        TIMESTAMP timestamp
        DECIMAL open_price
        DECIMAL high_price
        DECIMAL low_price
        DECIMAL close_price
        BIGINT volume
    }

    WATCHLISTS {
        BIGINT id PK
        BIGINT user_id FK
        VARCHAR name
        TIMESTAMP created_at
    }

    WATCHLIST_ITEMS {
        BIGINT id PK
        BIGINT watchlist_id FK
        BIGINT stock_id FK
    }

    NOTIFICATIONS {
        BIGINT id PK
        BIGINT user_id FK
        VARCHAR title
        TEXT message
        VARCHAR type
        BOOLEAN is_read
        TIMESTAMP created_at
    }

    ADMIN_USERS {
        BIGINT id PK
        BIGINT user_id FK,UK
        VARCHAR role
        TIMESTAMP created_at
    }
```

---

## 3. Table Definitions & Constraints

### 3.1 `users`
Primary identity and authentication store.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `username`: VARCHAR(50) NOT NULL UNIQUE
- `full_name`: VARCHAR(100) NOT NULL
- `email`: VARCHAR(100) NOT NULL UNIQUE
- `phone`: VARCHAR(20)
- `password_hash`: VARCHAR(255) NOT NULL (SHA-256 salted hash)
- `role`: ENUM('USER', 'ADMIN', 'SUPER_ADMIN') DEFAULT 'USER'
- `status`: ENUM('ACTIVE', 'SUSPENDED', 'PENDING') DEFAULT 'ACTIVE'
- `created_at`, `updated_at`: TIMESTAMP

### 3.2 `stocks`
Equities traded on simulated NSE and BSE exchanges.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `symbol`: VARCHAR(20) NOT NULL UNIQUE
- `company_name`: VARCHAR(100) NOT NULL
- `exchange`: VARCHAR(10) NOT NULL DEFAULT 'NSE'
- `sector`: VARCHAR(50) NOT NULL
- `current_price`: DECIMAL(12, 2) NOT NULL
- `previous_close`: DECIMAL(12, 2) NOT NULL
- `open_price`: DECIMAL(12, 2) NOT NULL
- `day_high`: DECIMAL(12, 2) NOT NULL
- `day_low`: DECIMAL(12, 2) NOT NULL
- `volume`: BIGINT NOT NULL DEFAULT 0
- `market_cap`: DECIMAL(18, 2) NOT NULL
- `updated_at`: TIMESTAMP

### 3.3 `wallet`
Virtual paper-trading cash balance and margin account.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `user_id`: BIGINT NOT NULL UNIQUE, FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE
- `available_balance`: DECIMAL(15, 2) NOT NULL DEFAULT 100000.00
- `invested_amount`: DECIMAL(15, 2) NOT NULL DEFAULT 0.00
- `withdrawable_amount`: DECIMAL(15, 2) NOT NULL DEFAULT 100000.00
- `updated_at`: TIMESTAMP

### 3.4 `orders`
Double-auction order records.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `user_id`: BIGINT NOT NULL, FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE
- `stock_id`: BIGINT NOT NULL, FOREIGN KEY (`stock_id`) REFERENCES `stocks`(`id`)
- `order_type`: ENUM('MARKET', 'LIMIT', 'STOP_LOSS') NOT NULL
- `transaction_type`: ENUM('BUY', 'SELL') NOT NULL
- `quantity`: INT NOT NULL CHECK (`quantity` > 0)
- `price`: DECIMAL(12, 2) NOT NULL
- `trigger_price`: DECIMAL(12, 2) DEFAULT 0.00
- `status`: ENUM('PENDING', 'EXECUTED', 'CANCELLED', 'REJECTED') DEFAULT 'PENDING'
- `created_at`, `executed_at`: TIMESTAMP

### 3.5 `holdings`
Delivery (CNC) equity portfolio holdings.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `user_id`: BIGINT NOT NULL, FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE
- `stock_id`: BIGINT NOT NULL, FOREIGN KEY (`stock_id`) REFERENCES `stocks`(`id`)
- `quantity`: INT NOT NULL CHECK (`quantity` >= 0)
- `average_price`: DECIMAL(12, 2) NOT NULL
- `updated_at`: TIMESTAMP
- `UNIQUE KEY` (`user_id`, `stock_id`)

### 3.6 `positions`
Intraday (MIS) trading positions with realized and unrealized P&L.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `user_id`: BIGINT NOT NULL, FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE
- `stock_id`: BIGINT NOT NULL, FOREIGN KEY (`stock_id`) REFERENCES `stocks`(`id`)
- `quantity`: INT NOT NULL DEFAULT 0
- `average_price`: DECIMAL(12, 2) NOT NULL DEFAULT 0.00
- `realized_pnl`: DECIMAL(15, 2) NOT NULL DEFAULT 0.00
- `unrealized_pnl`: DECIMAL(15, 2) NOT NULL DEFAULT 0.00
- `updated_at`: TIMESTAMP
- `UNIQUE KEY` (`user_id`, `stock_id`)

### 3.7 `transactions`
Double-entry immutable audit ledger of all cash flows.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `user_id`: BIGINT NOT NULL, FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE
- `order_id`: BIGINT NULL, FOREIGN KEY (`order_id`) REFERENCES `orders`(`id`) ON DELETE SET NULL
- `type`: ENUM('BUY', 'SELL', 'DEPOSIT', 'WITHDRAWAL') NOT NULL
- `amount`: DECIMAL(15, 2) NOT NULL
- `description`: VARCHAR(255) NOT NULL
- `created_at`: TIMESTAMP DEFAULT CURRENT_TIMESTAMP

### 3.8 `price_history`
Granular OHLCV candlestick time series for technical charts.
- `id`: BIGINT AUTO_INCREMENT PRIMARY KEY
- `stock_id`: BIGINT NOT NULL, FOREIGN KEY (`stock_id`) REFERENCES `stocks`(`id`) ON DELETE CASCADE
- `timestamp`: TIMESTAMP NOT NULL
- `open_price`, `high_price`, `low_price`, `close_price`: DECIMAL(12, 2) NOT NULL
- `volume`: BIGINT NOT NULL DEFAULT 0

---

## 4. Indexing Strategy (`indexes.sql`)
To ensure queries execute in sub-millisecond time:
- `idx_stocks_symbol`: Fast symbol resolution for Trie initialization and direct routing.
- `idx_stocks_sector`: Accelerates sector filtering across the Market directory.
- `idx_orders_user_status`: Composite index on `(user_id, status)` for fast retrieval of open pending orders.
- `idx_holdings_user`: Composite index on `(user_id, stock_id)` for $O(1)$ portfolio balance checks.
- `idx_price_history_stock_time`: Composite index on `(stock_id, timestamp DESC)` for fast historical chart window queries.
