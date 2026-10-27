# TradeX Security & Compliance Architecture

## 1. Overview
Financial platforms demand strict defense-in-depth protections across transport, identity, authorization, and data storage. TradeX incorporates industrial cybersecurity principles while ensuring complete sandboxed isolation.

---

## 2. Authentication & Credential Storage

### 2.1 Password Hashing (Salted SHA-256)
- Plaintext passwords are **never** stored in memory or in the database.
- Every user has an unpredictable random salt concatenated before computing the cryptographic SHA-256 hash:
  $$\text{Hash} = \text{SHA256}(\text{Salt} \parallel \text{PlainPassword})$$
- Password matching is computed in constant time to prevent timing side-channel attacks.

### 2.2 JSON Web Tokens (JWT)
- Authenticated sessions issue an RFC-7519 compliant JSON Web Token signed via HMAC-SHA256.
- The JWT payload encapsulates `userId`, `username`, `role`, and expiration timestamp (`exp`).
- Tokens are verified cryptographically in C++ on every incoming protected HTTP and WebSocket request before dispatching to business controllers.

---

## 3. Defense Against Common Vulnerabilities

| Threat | Mitigation in TradeX |
|---|---|
| **SQL Injection (SQLi)** | Native prepared statements and parameterized inputs via `libmysql` C API. User inputs are never concatenated directly into raw SQL strings. |
| **Cross-Origin Resource Sharing (CORS)** | Strict HTTP CORS pre-flight headers (`Access-Control-Allow-Origin`, `Access-Control-Allow-Headers`) allowing only configured trusted origins. |
| **Denial of Service (DoS) Buffer Attacks** | Content-Length bounded reading loops in Winsock2 TCP server preventing unbounded heap allocation exploits. |
| **Cross-Site Scripting (XSS)** | React automatic JSX context escaping and strict input sanitation on all user-submitted names and symbols. |
| **Negative Balances & Race Conditions** | Mutex-synchronized wallet balance updates and database transaction locks (`BEGIN ... COMMIT`) around order placement and fund deduction. |

---

## 4. Role-Based Access Control (RBAC)

TradeX implements strict role separation:
- **`USER`:** Access to personal trading, portfolio, watchlists, order history, and read-only market data. Cannot access admin APIs or mutate global simulation states.
- **`ADMIN`:** Privileged access to user directories, market volatility controls, simulator pause/resume, and system diagnostic counters.
