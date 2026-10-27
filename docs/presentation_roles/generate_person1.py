import os
from reportlab.lib import colors
from reportlab.platypus import Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
from pdf_utils import build_pdf
from builder_helpers import (
    title_style, subtitle_style, h1_style, h2_style, body_style, bullet_style,
    code_style, make_callout, make_code_box, make_meta_card, th_style, td_style, td_code_style,
    COLOR_PRIMARY, COLOR_ACCENT, COLOR_BORDER, COLOR_CARD_BG
)

def generate_person_1():
    pdf_path = os.path.join(os.path.dirname(__file__), "Person_1_Infrastructure_Security.pdf")
    md_path = os.path.join(os.path.dirname(__file__), "Person_1_Infrastructure_Security.md")
    
    files = [
        "backend/src/main.cpp",
        "backend/include/api/HttpServer.hpp",
        "backend/include/utils/Crypto.hpp",
        "backend/include/utils/Json.hpp",
        "backend/include/services/AuthService.hpp",
        "backend/include/models/Models.hpp (Shared Contracts)"
    ]
    
    flowables = []
    
    # Title
    flowables.append(Paragraph("TradeX B.Tech Capstone — Technical Defense Guide", title_style))
    flowables.append(Paragraph("Person 1: Systems, Security & Network Infrastructure Architect", subtitle_style))
    flowables.append(HRFlowable(width="100%", thickness=1.5, color=COLOR_ACCENT, spaceAfter=10))
    
    # Meta Card
    flowables.append(make_meta_card(
        role_num=1,
        title="Systems, Security & Network Infrastructure Architect",
        domain="Multi-Threaded Winsock Server, HTTP/1.1 REST Engine, RFC-6455 WebSocket Broadcaster, Salted SHA-256 & JWT Cryptography, Recursive-Descent JSON Parser",
        files_list=files,
        presentation_time="5 to 7 Minutes"
    ))
    flowables.append(Spacer(1, 10))
    
    # 1. Executive Summary
    flowables.append(Paragraph("1. Executive Summary & Architectural Ownership", h1_style))
    flowables.append(Paragraph(
        "Person 1 is responsible for the foundational operating-system and transport layer of TradeX. "
        "Unlike standard web projects that rely on heavy Node.js or Python runtimes, TradeX implements its entire "
        "HTTP and WebSocket server natively in modern C++17 using the Windows Sockets API (Winsock2). "
        "Person 1 owns the socket lifecycle, multi-threaded request dispatching, protocol upgrading, "
        "salted SHA-256 password hashing, RFC-7519 JWT verification, and custom zero-dependency JSON serialization.",
        body_style
    ))
    
    flowables.append(make_callout(
        "<b>Core Defense Pitch:</b> <i>\"I architected the high-throughput network gateway and security subsystem in C++17. "
        "My code receives raw TCP streams on port 8080, reassembles fragmented HTTP frames, performs stateless cryptographic "
        "authentication with HMAC-SHA256 tokens, upgrades persistent client sockets to RFC-6455 WebSockets, and serializes all "
        "financial payloads through a custom recursive-descent JSON engine without any external library dependencies.\"</i>"
    ))
    flowables.append(Spacer(1, 8))
    
    # 2. File-by-File Breakdown
    flowables.append(Paragraph("2. Assigned Files & Detailed Code Responsibilities", h1_style))
    
    f1 = "<b>backend/src/main.cpp</b> (Daemon Entrypoint):<br/>" \
         "• Registers OS interrupt signals (SIGINT, SIGTERM) via <code>signalHandler</code> for clean thread shutdown.<br/>" \
         "• Initializes the MySQL database connection pool (4 pooled connections).<br/>" \
         "• Pre-loads all 32 equities into RAM and initializes the DSA Trie prefix tree.<br/>" \
         "• Launches the <code>MarketSimulator</code> background worker thread and binds <code>HttpServer</code> to port 8080."
    flowables.append(Paragraph(f1, body_style))
    
    f2 = "<b>backend/include/api/HttpServer.hpp</b> (Winsock REST & WebSocket Core):<br/>" \
         "• Socket initialization: <code>WSAStartup</code>, <code>socket()</code>, <code>setsockopt(SO_REUSEADDR)</code>, <code>bind()</code>, and <code>listen()</code>.<br/>" \
         "• Multi-threaded worker model: Spawns <code>std::thread(&HttpServer::handleClient, this, clientSocket)</code> for every accepted connection.<br/>" \
         "• TCP Packet Reassembly: Inspects <code>Content-Length</code> in header buffer and loops <code>recv()</code> until entire payload arrives.<br/>" \
         "• CORS Preflight: Intercepts <code>OPTIONS</code> requests and emits <code>HTTP 204 No Content</code> with permissive cross-origin headers.<br/>" \
         "• RFC-6455 WebSocket Handshake: Detects <code>Upgrade: websocket</code>, extracts <code>Sec-WebSocket-Key</code>, hashes with Magic String <code>258EAFA5-E914-47DA-95CA-C5AB0DC85B11</code>, and sends <code>HTTP 101 Switching Protocols</code>.<br/>" \
         "• Broadcast Engine: Maintains active socket registry (<code>std::vector&lt;SOCKET&gt; wsClients_</code> protected by <code>std::mutex wsMutex_</code>) to broadcast price ticks."
    flowables.append(Paragraph(f2, body_style))
    
    f3 = "<b>backend/include/utils/Crypto.hpp</b> (Cryptographic Security Subsystem):<br/>" \
         "• Salted SHA-256: Implements RFC-6234 bitwise transformations (Ch, Maj, Σ0, Σ1, σ0, σ1) with 32-character random hex salts.<br/>" \
         "• HMAC-SHA256: Keyed-Hash Message Authentication Code using inner (ipad: 0x36) and outer (opad: 0x5C) XOR pads.<br/>" \
         "• JSON Web Tokens (JWT): RFC-7519 compliant Base64URL token generator and validator (Header.Payload.Signature).<br/>" \
         "• WebSocket Accept: Implements SHA-1 digest + Base64 encoding for client handshake validation."
    flowables.append(Paragraph(f3, body_style))
    
    f4 = "<b>backend/include/utils/Json.hpp</b> (Header-Only Native C++ JSON Parser):<br/>" \
         "• Implements a recursive-descent parser and tokenizer without third-party dependencies.<br/>" \
         "• Strongly-typed Variant supporting Object, Array, String, Number, Boolean, and Null.<br/>" \
         "• Method <code>dump()</code> serializes C++ structs into JSON strings in microsecond time."
    flowables.append(Paragraph(f4, body_style))
    
    f5 = "<b>backend/include/services/AuthService.hpp</b> (Identity & RBAC Service):<br/>" \
         "• <code>registerUser()</code>: Validates unique email/username, hashes password with salt, provisions ₹1,00,000 wallet, generates JWT.<br/>" \
         "• <code>loginUser()</code>: Compares salted hashes, validates account status, and issues signed JWT.<br/>" \
         "• <code>validateToken()</code>: Stateless token validation inspecting timestamp expiration and user roles (USER vs ADMIN)."
    flowables.append(Paragraph(f5, body_style))
    flowables.append(Spacer(1, 8))
    
    # 3. DSA Implementation
    flowables.append(Paragraph("3. Data Structures & Algorithms Implemented by Person 1", h1_style))
    
    dsa_table_data = [
        [Paragraph("Data Structure / Algorithm", th_style), Paragraph("Time Complexity", th_style), Paragraph("Space Complexity", th_style), Paragraph("Engineering Role in TradeX", th_style)],
        [Paragraph("<b>Thread-Safe Dynamic Socket Registry</b>", td_style), Paragraph("O(1) Add / O(N) Broadcast", td_style), Paragraph("O(C) where C = Active Clients", td_style), Paragraph("Maintains list of open WebSocket file descriptors with mutex lock guarding push/erase during client disconnects.", td_style)],
        [Paragraph("<b>Recursive-Descent Parser</b> (Json.hpp)", td_style), Paragraph("O(N) where N = Bytes", td_style), Paragraph("O(D) call stack where D = Nesting Depth", td_style), Paragraph("Tokenizes raw TCP string into JSON AST without allocating heavy intermediary objects.", td_style)],
        [Paragraph("<b>HMAC-SHA256 Algorithm</b> (Crypto.hpp)", td_style), Paragraph("O(B) where B = Blocks", td_style), Paragraph("O(1) Auxiliary", td_style), Paragraph("Computes cryptographic integrity tags for session tokens; enables stateless microsecond verification on every HTTP request.", td_style)],
        [Paragraph("<b>Salted Key Derivation</b> (Crypto.hpp)", td_style), Paragraph("O(1) Fixed 64 Rounds", td_style), Paragraph("O(1) Auxiliary (32B Salt)", td_style), Paragraph("Prevents rainbow table and dictionary attacks by salting user credentials before SHA-256 compression.", td_style)],
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
    flowables.append(Paragraph("4. How Person 1's Backend Connects to the Frontend", h1_style))
    flowables.append(Paragraph(
        "• <b>Axios Interceptor Bridge:</b> When a user logs in via <code>Login.tsx</code>, <code>AuthService::loginUser</code> issues a signed JWT. "
        "The React client saves it in <code>localStorage.getItem('tradex_token')</code>. "
        "The Axios interceptor in <code>frontend/src/services/api.ts</code> automatically injects <code>Authorization: Bearer &lt;token&gt;</code> on every subsequent request.<br/>"
        "• <b>WebSocket Handshake Bridge:</b> <code>frontend/src/services/websocket.ts</code> opens <code>new WebSocket('ws://localhost:8080/ws')</code>. "
        "<code>HttpServer::handleWebSocketHandshake</code> receives the HTTP upgrade request, computes <code>Sec-WebSocket-Accept</code>, and responds with status 101. "
        "Thereafter, every price tick generated in the C++ engine streams directly into React's <code>MarketContext.tsx</code>.",
        body_style
    ))
    flowables.append(Spacer(1, 8))
    
    # 5. Viva / Defense Q&A
    flowables.append(Paragraph("5. B.Tech Defense & Viva Voce Q&A (Ready to Speak)", h1_style))
    
    q1 = "<b>Q1: Why write a native C++ HTTP and WebSocket server instead of using Node.js Express or Python FastAPI?</b><br/>" \
         "<b>Answer:</b> <i>\"Real-world high-frequency exchange architectures cannot afford the non-deterministic garbage collection pauses of Node.js V8 or the Global Interpreter Lock (GIL) overhead of Python. By writing the server in C++17 with Winsock, we gain deterministic sub-microsecond latency, contiguous memory layout control, and multi-threading where each worker thread processes socket I/O without runtime virtualization.\"</i>"
    flowables.append(Paragraph(q1, body_style))
    flowables.append(Spacer(1, 4))
    
    q2 = "<b>Q2: How does your server handle TCP fragmentation over Winsock?</b><br/>" \
         "<b>Answer:</b> <i>\"TCP is a byte-stream protocol, meaning large JSON payloads can arrive split across multiple packets. In <code>HttpServer::parseRequest</code>, my code inspects the initial header buffer to parse the <code>Content-Length</code> header. It then executes an accumulation loop calling <code>recv()</code> until the total bytes in memory match <code>header_length + content_length</code> before triggering the JSON parser. This prevents malformed payload errors under heavy network traffic.\"</i>"
    flowables.append(Paragraph(q2, body_style))
    flowables.append(Spacer(1, 4))
    
    q3 = "<b>Q3: Explain the math and steps in your RFC-6455 WebSocket Handshake implementation.</b><br/>" \
         "<b>Answer:</b> <i>\"The browser sends a 24-byte base64-encoded random key in the <code>Sec-WebSocket-Key</code> header. Per RFC-6455 specifications, my server appends the standard UUID Magic String <code>258EAFA5-E914-47DA-95CA-C5AB0DC85B11</code> to this key, calculates a 160-bit SHA-1 hash of the concatenated string, and base64-encodes the hash. We return this in the <code>Sec-WebSocket-Accept</code> header with HTTP 101 Switching Protocols to finalize the connection.\"</i>"
    flowables.append(Paragraph(q3, body_style))
    
    # Build PDF
    build_pdf(pdf_path, "Person 1: Infrastructure & Security", flowables)
    print(f"Generated PDF: {pdf_path}")
    
    # Generate Markdown
    md_content = f"""# TradeX Technical Defense Guide — Person 1
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
"""
    with open(md_path, "w", encoding="utf-8") as f:
        f.write(md_content)
    print(f"Generated Markdown: {md_path}")

if __name__ == "__main__":
    generate_person_1()
