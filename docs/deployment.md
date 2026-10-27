# TradeX Deployment & Production Guide

This guide describes how to configure, compile, and deploy TradeX in local development and production container environments.

---

## 1. System Requirements

- **C++ Compiler:** C++17 compatible compiler (Clang++ 14+, GCC 10+, or MSVC 2019+).
- **Database:** MySQL 8.0 or newer.
- **Node.js:** v18.0+ and npm v9.0+.
- **Docker & Docker Compose:** (Optional, for containerized deployments).

---

## 2. Local Windows Environment Setup

### 2.1 Database Initialization
Ensure local MySQL 8.0 is running on port 3306:
```powershell
mysql -u root -p < database/schema.sql
mysql -u root -p tradex < database/indexes.sql
mysql -u root -p tradex < database/seed.sql
```

### 2.2 Compile the C++ Backend Daemon
Using LLVM-MinGW Clang++:
```powershell
cd backend
clang++ -std=c++17 -O2 -Iinclude -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -lmysql -lws2_32 src/main.cpp -o tradex_server.exe
.\tradex_server.exe
```

### 2.3 Run the Frontend
```powershell
cd frontend
npm install
npm run dev
```
Navigate to `http://localhost:5173` in any modern web browser.

---

## 3. Containerized Deployment (Docker Compose)

To launch the complete three-tier TradeX architecture (MySQL + C++ Backend + Vite Frontend) with a single command:

```bash
docker-compose up --build -d
```

### Containers Created:
1. `tradex-mysql`: MySQL 8.0 database automatically initialized with schema, indexes, and seed stocks.
2. `tradex-backend`: Linux Ubuntu container compiling and running the native C++ HTTP & WebSocket server on port 8080.
3. `tradex-frontend`: Alpine Nginx container serving the compiled React 19 production bundle on port 80 (or 5173).

---

## 4. Environment Variables Reference

| Variable | Default Value | Description |
|---|---|---|
| `PORT` | `8080` | TCP port for C++ HTTP & WebSocket server |
| `DB_HOST` | `127.0.0.1` | MySQL hostname or IP address |
| `DB_PORT` | `3306` | MySQL port |
| `DB_USER` | `root` | Database username |
| `DB_PASS` | `Shivam@20` | Database password |
| `DB_NAME` | `tradex` | Database schema name |
| `JWT_SECRET` | `tradex_super_secret_jwt_key_2026` | Secret key used for HMAC-SHA256 signature |
| `MARKET_TICK_MS` | `1500` | Frequency of Brownian motion price tick emissions |
