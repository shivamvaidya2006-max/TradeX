# TradeX Technical Defense Guide — Person 1
## Systems, Security & Network Infrastructure Architect

**Candidate Role:** Person 1  
**Presentation Time:** 5 to 7 Minutes  
**Core Domain:** Multi-Threaded Winsock Server, HTTP/1.1 REST Engine, RFC-6455 WebSocket Broadcaster, Salted SHA-256 & JWT Cryptography, Recursive-Descent JSON Parser.

---

### Assigned Files
- `backend/src/main.cpp`
- `backend/include/api/HttpServer.hpp`
- `backend/include/utils/Crypto.hpp`
- `backend/include/utils/Json.hpp`
- `backend/include/services/AuthService.hpp`
- `backend/include/models/Models.hpp`

---

### 1. Executive Summary & Architectural Ownership
Person 1 is responsible for the foundational operating-system and transport layer of TradeX. The entire HTTP and WebSocket server is implemented in native C++17 using the Windows Sockets API (Winsock2). Person 1 owns the socket lifecycle, multi-threaded request dispatching, protocol upgrading, salted SHA-256 password hashing, RFC-7519 JWT verification, and custom zero-dependency JSON serialization.

---

### 2. File-by-File Technical Deep Dive

#### `backend/src/main.cpp` (Daemon Entrypoint)
- Registers OS interrupt signals (`SIGINT`, `SIGTERM`) for graceful server shutdown.
- Initializes the MySQL connection pool (`Database::getInstance().initialize(...)`).
- Pre-loads all 32 equities into RAM and initializes the DSA Trie.
- Starts the `MarketSimulator` background worker thread and binds `HttpServer` to port 8080.

#### `backend/include/api/HttpServer.hpp` (Winsock REST & WebSocket Core)
- **Winsock Initialization:** Calls `WSAStartup()`, creates TCP socket with `SO_REUSEADDR`, binds to port 8080, and calls `listen(serverSocket_, SOMAXCONN)`.
- **Worker Thread Model:** Spawns a dedicated thread `std::thread(&HttpServer::handleClient, this, clientSocket)` for every client.
- **TCP Fragmentation Reassembly:** Reads incoming bytes, parses `Content-Length`, and executes a chunk accumulation loop until the entire body is received.
- **CORS Handling:** Automatically responds to `OPTIONS` preflight requests with `HTTP 204 No Content` and permissive Access-Control headers.
- **RFC-6455 WebSocket Handshake:** Extracts `Sec-WebSocket-Key`, generates accept key, upgrades protocol with `HTTP 101`, and maintains `std::vector<SOCKET> wsClients_` guarded by `std::mutex wsMutex_`.
- **WebSocket Broadcast:** `broadcastWebSocket(msg)` iterates over active client sockets to stream live market ticks.

#### `backend/include/utils/Crypto.hpp` (Cryptographic Subsystem)
- **Salted SHA-256:** Implements RFC-6234 standard SHA-256 transformations with 32-character random salts.
- **HMAC-SHA256:** Keyed-hash message authentication code using standard inner and outer XOR pads.
- **JWT (RFC-7519):** Generates and verifies tokens formatted as `Header.Payload.Signature` with expiration checks.
- **WebSocket Accept Key:** Implements SHA-1 digest + Base64 encoding.

#### `backend/include/utils/Json.hpp` (Native JSON Engine)
- Zero-dependency recursive-descent parser and tokenizer.
- Implements custom `Json` class with Variant support (Objects, Arrays, Strings, Numbers, Booleans).
- Serializes C++ data structures into JSON strings via `dump()`.

#### `backend/include/services/AuthService.hpp` (Authentication Service)
- `registerUser()`: Validates inputs, generates cryptographic salt, hashes password, stores user, provisions ₹1,00,000 wallet, generates JWT.
- `loginUser()`: Compares salted hashes and generates JWT token.
- `validateToken()`: Stateless verification of token signature and expiration.

---

### 3. Data Structures & Complexity Matrix

| Data Structure / Algorithm | Time Complexity | Space Complexity | Engineering Application in TradeX |
|---|---|---|---|
| **Thread-Safe Socket Registry** | O(1) Push / O(N) Broadcast | O(C) where C = Active Clients | Mutex-locked active WebSocket client list. |
| **Recursive-Descent Parser** (`Json.hpp`) | O(N) where N = Bytes | O(D) where D = Nesting Depth | Tokenizes HTTP JSON payloads into variant objects. |
| **HMAC-SHA256 Token Engine** | O(B) where B = Blocks | O(1) Auxiliary | Stateless JWT verification on every HTTP request. |
| **Salted Key Derivation** | O(1) 64 Rounds | O(1) Auxiliary (32B Salt) | Prevents rainbow-table and dictionary attacks. |

---

### 4. Connection to Frontend
1. **Axios REST Bridge:** `frontend/src/services/api.ts` attaches `Authorization: Bearer <token>` to requests; `HttpServer.hpp` validates it via `AuthService`.
2. **WebSocket Real-Time Bridge:** `frontend/src/services/websocket.ts` connects to `ws://localhost:8080/ws`; `HttpServer.hpp` completes handshake and streams price updates to React `MarketContext.tsx`.

---

### 5. B.Tech Defense & Viva Voce Q&A
- **Q1: Why write a native C++ HTTP server instead of Node.js or Python?**  
  *Answer:* To avoid non-deterministic garbage collection pauses and GIL locks, achieving sub-microsecond response times with direct hardware memory layout control.
- **Q2: How is TCP packet fragmentation handled?**  
  *Answer:* By extracting `Content-Length` from the HTTP header and looping `recv()` until `header_length + content_length` bytes are accumulated in RAM before parsing JSON.
- **Q3: How does the RFC-6455 WebSocket handshake work?**  
  *Answer:* It appends the Magic UUID `258EAFA5-E914-47DA-95CA-C5AB0DC85B11` to `Sec-WebSocket-Key`, computes a 160-bit SHA-1 hash, base64 encodes it, and returns it with status 101.
