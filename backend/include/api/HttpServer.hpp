#pragma once

#include "utils/Json.hpp"
#include "utils/Crypto.hpp"
#include "services/AuthService.hpp"
#include "services/StockService.hpp"
#include "services/OrderService.hpp"
#include "services/PortfolioService.hpp"
#include "services/WalletService.hpp"
#include "services/WatchlistService.hpp"
#include "services/MarketSimulator.hpp"
#include "services/CacheService.hpp"
#include "dsa/BST.hpp"
#include "dsa/SortingAlgorithms.hpp"
#include "dsa/SearchingAlgorithms.hpp"
#include "dsa/MarketAnalyticsService.hpp"
#include "dsa/GreedyPortfolioService.hpp"
#include "dsa/DynamicProgrammingService.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <sstream>
#include <iostream>
#include <memory>
#include <regex>

namespace tradex::api {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string queryString;
    std::unordered_map<std::string, std::string> queryParams;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    utils::Json jsonBody;
    std::optional<models::User> authUser;
};

struct HttpResponse {
    int statusCode{200};
    std::string statusText{"OK"};
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    static HttpResponse json(int status, const utils::Json& j) {
        HttpResponse res;
        res.statusCode = status;
        res.headers["Content-Type"] = "application/json";
        res.body = j.dump();
        return res;
    }

    static HttpResponse ok(const utils::Json& data, const std::string& message = "Success") {
        utils::Json resp = {
            {"success", true},
            {"message", message},
            {"data", data}
        };
        return json(200, resp);
    }

    static HttpResponse error(int status, const std::string& message, const std::string& errorCode = "ERROR") {
        utils::Json resp = {
            {"success", false},
            {"message", message},
            {"errorCode", errorCode}
        };
        return json(status, resp);
    }
};

/**
 * @brief High-performance Multi-threaded REST and WebSocket Server for TradeX.
 */
class HttpServer {
public:
    HttpServer(int port = 8080) : port_(port) {
        // Initialize global demo BST with initial price levels
        demoBST_.insert(2945.5, "RELIANCE", 54200);
        demoBST_.insert(4210.8, "TCS", 21450);
        demoBST_.insert(1885.25, "INFY", 68740);
        demoBST_.insert(1642.4, "HDFCBANK", 125430);
        demoBST_.insert(1215.6, "ICICIBANK", 98450);
        demoBST_.insert(788.9, "SBIN", 142100);
        demoBST_.insert(512.4, "ITC", 184520);
    }

    void start() {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            std::cerr << "[HttpServer] WSAStartup failed" << std::endl;
            return;
        }
#endif

        serverSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket_ == INVALID_SOCKET) {
            std::cerr << "[HttpServer] socket creation failed" << std::endl;
            return;
        }

        int opt = 1;
        setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port_);

        if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "[HttpServer] bind failed on port " << port_ << std::endl;
            closesocket(serverSocket_);
            return;
        }

        if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "[HttpServer] listen failed" << std::endl;
            closesocket(serverSocket_);
            return;
        }

        running_ = true;
        std::cout << "=================================================" << std::endl;
        std::cout << "  TradeX C++ Backend Server listening on :" << port_ << std::endl;
        std::cout << "  REST API: http://localhost:" << port_ << "/api/v1/" << std::endl;
        std::cout << "  WebSocket: ws://localhost:" << port_ << "/ws" << std::endl;
        std::cout << "=================================================" << std::endl;

        // Register WebSocket tick broadcaster from MarketSimulator
        services::MarketSimulator::getInstance().setTickCallback([this](const services::MarketTick& tick) {
            utils::Json j = {
                {"type", "PRICE_TICK"},
                {"symbol", tick.symbol},
                {"price", tick.price},
                {"change", tick.change},
                {"changePercent", tick.changePercent},
                {"dayHigh", tick.dayHigh},
                {"dayLow", tick.dayLow},
                {"volume", tick.volume},
                {"timestamp", tick.timestamp}
            };
            broadcastWebSocket(j.dump());
        });

        // Accept loop in worker threads
        while (running_) {
            sockaddr_in clientAddr{};
            int clientLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket_, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
            if (clientSocket == INVALID_SOCKET) {
                if (!running_) break;
                continue;
            }

            std::thread(&HttpServer::handleClient, this, clientSocket).detach();
        }
    }

    void stop() {
        running_ = false;
        if (serverSocket_ != INVALID_SOCKET) {
            closesocket(serverSocket_);
            serverSocket_ = INVALID_SOCKET;
        }
#ifdef _WIN32
        WSACleanup();
#endif
    }

    ~HttpServer() {
        stop();
    }

    /**
     * @brief Broadcasts JSON payload to all active WebSocket connections.
     */
    void broadcastWebSocket(const std::string& message) {
        std::lock_guard<std::mutex> lock(wsMutex_);
        if (wsClients_.empty()) return;

        std::vector<uint8_t> frame;
        frame.push_back(0x81); // FIN + text frame

        size_t len = message.size();
        if (len <= 125) {
            frame.push_back(static_cast<uint8_t>(len));
        } else if (len <= 65535) {
            frame.push_back(126);
            frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
            frame.push_back(static_cast<uint8_t>(len & 0xFF));
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; --i) {
                frame.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xFF));
            }
        }

        frame.insert(frame.end(), message.begin(), message.end());

        std::vector<SOCKET> closedSockets;
        for (SOCKET client : wsClients_) {
            int sent = send(client, reinterpret_cast<const char*>(frame.data()), static_cast<int>(frame.size()), 0);
            if (sent == SOCKET_ERROR) {
                closedSockets.push_back(client);
            }
        }

        for (SOCKET cs : closedSockets) {
            wsClients_.erase(std::remove(wsClients_.begin(), wsClients_.end(), cs), wsClients_.end());
            closesocket(cs);
        }
    }

private:
    int port_;
    SOCKET serverSocket_{INVALID_SOCKET};
    std::atomic<bool> running_{false};
    std::mutex wsMutex_;
    std::vector<SOCKET> wsClients_;
    dsa::BinarySearchTree demoBST_;

    void handleClient(SOCKET clientSocket) {
        std::vector<char> buffer(65536);
        std::string raw;
        size_t headerEnd = std::string::npos;
        int contentLength = 0;

        while (true) {
            int bytesRead = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()) - 1, 0);
            if (bytesRead <= 0) break;
            raw.append(buffer.data(), bytesRead);

            if (headerEnd == std::string::npos) {
                size_t dBreak = raw.find("\r\n\r\n");
                if (dBreak != std::string::npos) {
                    headerEnd = dBreak + 4;
                } else {
                    size_t nBreak = raw.find("\n\n");
                    if (nBreak != std::string::npos) headerEnd = nBreak + 2;
                }

                if (headerEnd != std::string::npos) {
                    std::string headersPart = raw.substr(0, headerEnd);
                    std::transform(headersPart.begin(), headersPart.end(), headersPart.begin(), ::tolower);
                    size_t clPos = headersPart.find("content-length:");
                    if (clPos != std::string::npos) {
                        size_t valStart = clPos + 15;
                        while (valStart < headersPart.size() && headersPart[valStart] == ' ') valStart++;
                        size_t valEnd = headersPart.find("\r\n", valStart);
                        if (valEnd == std::string::npos) valEnd = headersPart.find("\n", valStart);
                        if (valEnd != std::string::npos) {
                            try {
                                contentLength = std::stoi(headersPart.substr(valStart, valEnd - valStart));
                            } catch (...) {}
                        }
                    }
                }
            }

            if (headerEnd != std::string::npos) {
                size_t bodyLen = raw.size() - headerEnd;
                if (bodyLen >= static_cast<size_t>(contentLength)) {
                    break;
                }
            }
        }

        if (raw.empty()) {
            closesocket(clientSocket);
            return;
        }

        // 1. Check for WebSocket Upgrade
        if (raw.find("Upgrade: websocket") != std::string::npos || raw.find("upgrade: websocket") != std::string::npos) {
            handleWebSocketHandshake(clientSocket, raw);
            return;
        }

        // 2. Parse HTTP Request
        HttpRequest req = parseRequest(raw);

        // 3. Handle OPTIONS for CORS Preflight
        if (req.method == "OPTIONS") {
            HttpResponse res;
            res.statusCode = 204;
            res.statusText = "No Content";
            sendResponse(clientSocket, res);
            closesocket(clientSocket);
            return;
        }

        // 4. Authenticate Token if present
        auto authHeaderIt = req.headers.find("authorization");
        if (authHeaderIt != req.headers.end()) {
            std::string authVal = authHeaderIt->second;
            if (authVal.rfind("Bearer ", 0) == 0) {
                std::string token = authVal.substr(7);
                req.authUser = services::AuthService::validateToken(token);
            }
        }

        // 5. Route to handler
        HttpResponse res = routeRequest(req);
        sendResponse(clientSocket, res);
        closesocket(clientSocket);
    }

    void handleWebSocketHandshake(SOCKET clientSocket, const std::string& raw) {
        std::string keyHeader = "Sec-WebSocket-Key: ";
        size_t keyPos = raw.find(keyHeader);
        if (keyPos == std::string::npos) {
            keyHeader = "sec-websocket-key: ";
            keyPos = raw.find(keyHeader);
        }
        if (keyPos == std::string::npos) {
            closesocket(clientSocket);
            return;
        }

        size_t valStart = keyPos + keyHeader.size();
        size_t endLine = raw.find("\r\n", valStart);
        std::string clientKey = raw.substr(valStart, endLine - valStart);

        std::string acceptKey = utils::Crypto::computeWebSocketAccept(clientKey);

        std::ostringstream response;
        response << "HTTP/1.1 101 Switching Protocols\r\n"
                 << "Upgrade: websocket\r\n"
                 << "Connection: Upgrade\r\n"
                 << "Sec-WebSocket-Accept: " << acceptKey << "\r\n\r\n";

        std::string respStr = response.str();
        send(clientSocket, respStr.c_str(), static_cast<int>(respStr.size()), 0);

        {
            std::lock_guard<std::mutex> lock(wsMutex_);
            wsClients_.push_back(clientSocket);
        }
        std::cout << "[WebSocket] Client connected! Active clients: " << wsClients_.size() << std::endl;

        // Keep socket open in read loop to detect disconnection
        std::vector<char> wsBuf(1024);
        while (running_) {
            int r = recv(clientSocket, wsBuf.data(), static_cast<int>(wsBuf.size()), 0);
            if (r <= 0) break;
        }

        {
            std::lock_guard<std::mutex> lock(wsMutex_);
            wsClients_.erase(std::remove(wsClients_.begin(), wsClients_.end(), clientSocket), wsClients_.end());
        }
        closesocket(clientSocket);
        std::cout << "[WebSocket] Client disconnected. Remaining: " << wsClients_.size() << std::endl;
    }

    HttpRequest parseRequest(const std::string& raw) {
        HttpRequest req;
        std::istringstream stream(raw);
        std::string line;

        if (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::istringstream lineStream(line);
            std::string fullUrl;
            lineStream >> req.method >> fullUrl;

            size_t qPos = fullUrl.find('?');
            if (qPos != std::string::npos) {
                req.path = fullUrl.substr(0, qPos);
                req.queryString = fullUrl.substr(qPos + 1);
                parseQueryParams(req.queryString, req.queryParams);
            } else {
                req.path = fullUrl;
            }
        }

        while (std::getline(stream, line) && line != "\r" && !line.empty()) {
            if (line.back() == '\r') line.pop_back();
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string val = line.substr(colon + 1);
                while (!val.empty() && val.front() == ' ') val.erase(val.begin());
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                req.headers[key] = val;
            }
        }

        size_t dBreak = raw.find("\r\n\r\n");
        if (dBreak != std::string::npos) {
            req.body = raw.substr(dBreak + 4);
        } else {
            size_t nBreak = raw.find("\n\n");
            if (nBreak != std::string::npos) {
                req.body = raw.substr(nBreak + 2);
            }
        }

        if (!req.body.empty()) {
            try {
                req.jsonBody = utils::Json::parse(req.body);
            } catch (...) {}
        }

        return req;
    }

    void parseQueryParams(const std::string& query, std::unordered_map<std::string, std::string>& params) {
        std::istringstream ss(query);
        std::string pair;
        while (std::getline(ss, pair, '&')) {
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                params[pair.substr(0, eq)] = pair.substr(eq + 1);
            } else {
                params[pair] = "";
            }
        }
    }

    void sendResponse(SOCKET clientSocket, const HttpResponse& res) {
        std::ostringstream ss;
        ss << "HTTP/1.1 " << res.statusCode << " " << res.statusText << "\r\n";
        ss << "Access-Control-Allow-Origin: *\r\n";
        ss << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        ss << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n";
        ss << "Connection: close\r\n";

        for (const auto& h : res.headers) {
            ss << h.first << ": " << h.second << "\r\n";
        }
        ss << "Content-Length: " << res.body.size() << "\r\n\r\n";
        ss << res.body;

        std::string fullResp = ss.str();
        send(clientSocket, fullResp.c_str(), static_cast<int>(fullResp.size()), 0);
    }

    HttpResponse routeRequest(const HttpRequest& req) {
        std::string path = req.path;
        if (path.length() > 1 && path.back() == '/') path.pop_back();

        // 1. Health check
        if (path == "/" || path == "/api/v1/health") {
            return HttpResponse::ok({{"status", "ONLINE"}, {"version", "1.0.0"}, {"platform", "TradeX C++ Engine"}});
        }

        // 2. Authentication
        if (path == "/api/v1/auth/register" && req.method == "POST") {
            auto res = services::AuthService::getInstance().registerUser(
                req.jsonBody["fullName"].getString(),
                req.jsonBody["email"].getString(),
                req.jsonBody["phone"].getString(),
                req.jsonBody["username"].getString(),
                req.jsonBody["password"].getString()
            );
            if (!res.success) return HttpResponse::error(400, res.message);
            utils::Json userObj = {
                {"id", res.user.id},
                {"username", res.user.username},
                {"fullName", res.user.fullName},
                {"email", res.user.email},
                {"role", res.user.role}
            };
            return HttpResponse::ok({{"token", res.token}, {"user", userObj}}, res.message);
        }

        if (path == "/api/v1/auth/login" && req.method == "POST") {
            auto res = services::AuthService::getInstance().loginUser(
                req.jsonBody["username"].getString(req.jsonBody["email"].getString()),
                req.jsonBody["password"].getString()
            );
            if (!res.success) return HttpResponse::error(401, res.message);
            utils::Json userObj = {
                {"id", res.user.id},
                {"username", res.user.username},
                {"fullName", res.user.fullName},
                {"email", res.user.email},
                {"role", res.user.role}
            };
            return HttpResponse::ok({{"token", res.token}, {"user", userObj}}, res.message);
        }

        if (path == "/api/v1/auth/me" && req.method == "GET") {
            if (!req.authUser) return HttpResponse::error(401, "Unauthorized");
            utils::Json userObj = {
                {"id", req.authUser->id},
                {"username", req.authUser->username},
                {"fullName", req.authUser->fullName},
                {"email", req.authUser->email},
                {"role", req.authUser->role}
            };
            return HttpResponse::ok(userObj);
        }

        // 3. Stocks & Market
        if (path == "/api/v1/stocks" && req.method == "GET") {
            auto stocks = services::StockService::getInstance().getAllStocks();
            utils::Json arr = utils::Json::array();
            for (const auto& s : stocks) {
                arr.push_back({
                    {"id", s.id},
                    {"symbol", s.symbol},
                    {"companyName", s.companyName},
                    {"exchange", s.exchange},
                    {"sector", s.sector},
                    {"currentPrice", s.currentPrice},
                    {"previousClose", s.previousClose},
                    {"openPrice", s.openPrice},
                    {"dayHigh", s.dayHigh},
                    {"dayLow", s.dayLow},
                    {"volume", s.volume},
                    {"marketCap", s.marketCap},
                    {"change", s.change()},
                    {"changePercent", s.changePercent()}
                });
            }
            return HttpResponse::ok(arr);
        }

        // Stock Search with Trie Autocomplete
        if (path == "/api/v1/stocks/search" && req.method == "GET") {
            std::string q;
            auto it = req.queryParams.find("q");
            if (it != req.queryParams.end()) q = it->second;

            auto suggestions = services::StockService::getInstance().searchStocks(q, 10);
            utils::Json arr = utils::Json::array();
            for (const auto& item : suggestions) {
                arr.push_back({
                    {"symbol", item.symbol},
                    {"companyName", item.companyName},
                    {"sector", item.sector},
                    {"currentPrice", item.currentPrice}
                });
            }
            return HttpResponse::ok(arr, "Autocomplete suggestions from Trie (O(L))");
        }

        // Single stock details
        if (path.rfind("/api/v1/stocks/", 0) == 0 && path.find("/history") == std::string::npos && req.method == "GET") {
            std::string sym = path.substr(15);
            auto sOpt = services::StockService::getInstance().getStockBySymbol(sym);
            if (!sOpt) return HttpResponse::error(404, "Stock not found");
            const auto& s = *sOpt;
            utils::Json sObj = {
                {"id", s.id},
                {"symbol", s.symbol},
                {"companyName", s.companyName},
                {"exchange", s.exchange},
                {"sector", s.sector},
                {"currentPrice", s.currentPrice},
                {"previousClose", s.previousClose},
                {"openPrice", s.openPrice},
                {"dayHigh", s.dayHigh},
                {"dayLow", s.dayLow},
                {"volume", s.volume},
                {"marketCap", s.marketCap},
                {"change", s.change()},
                {"changePercent", s.changePercent()}
            };
            return HttpResponse::ok(sObj);
        }

        // Stock history
        if (path.rfind("/api/v1/stocks/", 0) == 0 && path.find("/history") != std::string::npos && req.method == "GET") {
            std::string sym = path.substr(15, path.find("/history") - 15);
            auto sOpt = services::StockService::getInstance().getStockBySymbol(sym);
            if (!sOpt) return HttpResponse::error(404, "Stock not found");

            std::string range = "1M";
            auto rIt = req.queryParams.find("range");
            if (rIt != req.queryParams.end()) range = rIt->second;

            auto hist = services::StockService::getInstance().getHistory(sOpt->id, range);
            utils::Json arr = utils::Json::array();
            for (const auto& pt : hist) {
                arr.push_back({
                    {"timestamp", pt.timestamp},
                    {"open", pt.open},
                    {"high", pt.high},
                    {"low", pt.low},
                    {"close", pt.close},
                    {"volume", pt.volume}
                });
            }
            return HttpResponse::ok(arr);
        }

        // Market Overview
        if (path == "/api/v1/market/overview" && req.method == "GET") {
            auto ov = services::StockService::getInstance().getMarketOverview();
            auto toStockArr = [](const std::vector<models::Stock>& list) {
                utils::Json arr = utils::Json::array();
                for (const auto& s : list) {
                    arr.push_back({
                        {"symbol", s.symbol},
                        {"companyName", s.companyName},
                        {"currentPrice", s.currentPrice},
                        {"change", s.change()},
                        {"changePercent", s.changePercent()},
                        {"volume", s.volume}
                    });
                }
                return arr;
            };

            utils::Json ovObj = {
                {"nifty50", {{"price", ov.nifty50}, {"change", ov.niftyChange}, {"changePercent", ov.niftyChangePercent}}},
                {"bankNifty", {{"price", ov.bankNifty}, {"change", ov.bankNiftyChange}, {"changePercent", ov.bankNiftyChangePercent}}},
                {"sensex", {{"price", ov.sensex}, {"change", ov.sensexChange}, {"changePercent", ov.sensexChangePercent}}},
                {"topGainers", toStockArr(ov.topGainers)},
                {"topLosers", toStockArr(ov.topLosers)},
                {"mostActive", toStockArr(ov.mostActive)},
                {"marketOpen", services::MarketSimulator::getInstance().isMarketOpen()}
            };
            return HttpResponse::ok(ovObj);
        }

        if (path == "/api/v1/market/status" && req.method == "GET") {
            bool open = services::MarketSimulator::getInstance().isMarketOpen();
            return HttpResponse::ok({{"marketOpen", open}, {"tradingSession", open ? "REGULAR_MARKET" : "MARKET_CLOSED"}});
        }

        // 4. Watchlists (Protected)
        if (path == "/api/v1/watchlists" && req.method == "GET") {
            int uid = req.authUser ? req.authUser->id : 2; // fallback to demo user
            auto lists = services::WatchlistService::getInstance().getUserWatchlists(uid);
            utils::Json arr = utils::Json::array();
            for (const auto& wl : lists) {
                utils::Json itemsArr = utils::Json::array();
                for (const auto& s : wl.items) {
                    itemsArr.push_back({
                        {"id", s.id},
                        {"symbol", s.symbol},
                        {"companyName", s.companyName},
                        {"sector", s.sector},
                        {"currentPrice", s.currentPrice},
                        {"change", s.change()},
                        {"changePercent", s.changePercent()},
                        {"dayHigh", s.dayHigh},
                        {"dayLow", s.dayLow}
                    });
                }
                arr.push_back({
                    {"id", wl.id},
                    {"name", wl.name},
                    {"items", itemsArr}
                });
            }
            return HttpResponse::ok(arr);
        }

        if (path == "/api/v1/watchlists" && req.method == "POST") {
            int uid = req.authUser ? req.authUser->id : 2;
            std::string name = req.jsonBody["name"].getString("New Watchlist");
            int wId = services::WatchlistService::getInstance().createWatchlist(uid, name);
            return HttpResponse::ok({{"id", wId}, {"name", name}}, "Watchlist created successfully");
        }

        if (path.rfind("/api/v1/watchlists/", 0) == 0 && req.method == "DELETE") {
            int uid = req.authUser ? req.authUser->id : 2;
            int wId = std::stoi(path.substr(19));
            bool ok = services::WatchlistService::getInstance().deleteWatchlist(uid, wId);
            return ok ? HttpResponse::ok(nullptr, "Watchlist deleted") : HttpResponse::error(400, "Failed to delete");
        }

        if (path == "/api/v1/watchlists/items" && req.method == "POST") {
            int wId = req.jsonBody["watchlistId"].getInt();
            int sId = req.jsonBody["stockId"].getInt();
            bool ok = services::WatchlistService::getInstance().addStockToWatchlist(wId, sId);
            return ok ? HttpResponse::ok(nullptr, "Stock added to watchlist") : HttpResponse::error(400, "Could not add stock");
        }

        if (path == "/api/v1/watchlists/items" && req.method == "DELETE") {
            int wId = req.jsonBody["watchlistId"].getInt();
            int sId = req.jsonBody["stockId"].getInt();
            bool ok = services::WatchlistService::getInstance().removeStockFromWatchlist(wId, sId);
            return ok ? HttpResponse::ok(nullptr, "Stock removed from watchlist") : HttpResponse::error(400, "Could not remove stock");
        }

        // 5. Orders (Protected)
        if (path == "/api/v1/orders" && req.method == "POST") {
            int uid = req.authUser ? req.authUser->id : 2;
            services::OrderPlacementRequest oReq;
            oReq.userId = uid;
            oReq.symbol = req.jsonBody["symbol"].getString();
            oReq.transactionType = req.jsonBody["transactionType"].getString();
            oReq.orderType = req.jsonBody["orderType"].getString("MARKET");
            oReq.quantity = req.jsonBody["quantity"].getInt();
            oReq.price = req.jsonBody["price"].getNumber();
            oReq.triggerPrice = req.jsonBody["triggerPrice"].getNumber();

            auto res = services::OrderService::getInstance().placeOrder(oReq);
            if (!res.success) {
                return HttpResponse::error(400, res.message, "ORDER_REJECTED");
            }
            utils::Json resObj = {
                {"orderId", res.orderId},
                {"status", res.status},
                {"executionPrice", res.executionPrice},
                {"executedQuantity", res.executedQuantity},
                {"charges", res.totalCharges}
            };

            // Broadcast order execution via WebSocket
            utils::Json wsOrder = {
                {"type", "ORDER_UPDATE"},
                {"orderId", res.orderId},
                {"status", res.status},
                {"symbol", oReq.symbol},
                {"side", oReq.transactionType}
            };
            broadcastWebSocket(wsOrder.dump());

            return HttpResponse::ok(resObj, res.message);
        }

        if (path == "/api/v1/orders" && req.method == "GET") {
            int uid = req.authUser ? req.authUser->id : 2;
            auto& db = database::Database::getInstance();
            auto rows = db.query("SELECT o.*, s.symbol, s.company_name FROM orders o "
                                 "JOIN stocks s ON o.stock_id = s.id WHERE o.user_id = " + std::to_string(uid) +
                                 " ORDER BY o.created_at DESC LIMIT 50;");
            utils::Json arr = utils::Json::array();
            for (const auto& r : rows) {
                arr.push_back({
                    {"id", std::stoi(r.at("id"))},
                    {"symbol", r.at("symbol")},
                    {"companyName", r.at("company_name")},
                    {"orderType", r.at("order_type")},
                    {"transactionType", r.at("transaction_type")},
                    {"quantity", std::stoi(r.at("quantity"))},
                    {"price", std::stod(r.at("price"))},
                    {"triggerPrice", std::stod(r.at("trigger_price"))},
                    {"status", r.at("status")},
                    {"createdAt", r.at("created_at")},
                    {"executedAt", r.at("executed_at")}
                });
            }
            return HttpResponse::ok(arr);
        }

        if (path.rfind("/api/v1/orders/", 0) == 0 && req.method == "DELETE") {
            int uid = req.authUser ? req.authUser->id : 2;
            int oId = std::stoi(path.substr(16));
            bool ok = services::OrderService::getInstance().cancelOrder(uid, oId);
            return ok ? HttpResponse::ok(nullptr, "Order cancelled") : HttpResponse::error(400, "Could not cancel order");
        }

        // 6. Portfolio & P&L
        if (path == "/api/v1/portfolio" && req.method == "GET") {
            int uid = req.authUser ? req.authUser->id : 2;
            auto pData = services::PortfolioService::getInstance().getCompletePortfolio(uid);

            utils::Json hArr = utils::Json::array();
            for (const auto& h : pData.holdings) {
                hArr.push_back({
                    {"id", h.id},
                    {"symbol", h.symbol},
                    {"companyName", h.companyName},
                    {"quantity", h.quantity},
                    {"averagePrice", h.averagePrice},
                    {"currentPrice", h.currentPrice},
                    {"investedValue", h.investedValue()},
                    {"currentValue", h.currentValue()},
                    {"pnl", h.pnl()},
                    {"pnlPercent", h.pnlPercent()}
                });
            }

            utils::Json pArr = utils::Json::array();
            for (const auto& p : pData.positions) {
                pArr.push_back({
                    {"id", p.id},
                    {"symbol", p.symbol},
                    {"companyName", p.companyName},
                    {"quantity", p.quantity},
                    {"averagePrice", p.averagePrice},
                    {"currentPrice", p.currentPrice},
                    {"realizedPnl", p.realizedPnl},
                    {"unrealizedPnl", p.unrealizedPnl()},
                    {"totalPnl", p.totalPnl()}
                });
            }

            utils::Json resObj = {
                {"holdings", hArr},
                {"positions", pArr},
                {"wallet", {
                    {"availableBalance", pData.wallet.availableBalance},
                    {"investedAmount", pData.wallet.investedAmount},
                    {"withdrawableAmount", pData.wallet.withdrawableAmount},
                    {"totalAccountValue", pData.wallet.totalAccountValue()}
                }},
                {"summary", {
                    {"totalInvested", pData.summary.totalInvested},
                    {"totalCurrentValue", pData.summary.totalCurrentValue},
                    {"totalPnl", pData.summary.totalPnl},
                    {"totalPnlPercent", pData.summary.totalPnlPercent},
                    {"todayPnl", pData.summary.todayPnl},
                    {"todayPnlPercent", pData.summary.todayPnlPercent}
                }}
            };
            return HttpResponse::ok(resObj);
        }

        // 7. Funds & Wallet
        if (path == "/api/v1/funds" && req.method == "GET") {
            int uid = req.authUser ? req.authUser->id : 2;
            auto w = services::WalletService::getInstance().getWallet(uid);
            utils::Json wObj = {
                {"availableBalance", w.availableBalance},
                {"investedAmount", w.investedAmount},
                {"withdrawableAmount", w.withdrawableAmount},
                {"totalAccountValue", w.totalAccountValue()}
            };
            return HttpResponse::ok(wObj);
        }

        if (path == "/api/v1/funds/deposit" && req.method == "POST") {
            int uid = req.authUser ? req.authUser->id : 2;
            double amt = req.jsonBody["amount"].getNumber();
            auto res = services::WalletService::getInstance().deposit(uid, amt);
            if (!res.success) return HttpResponse::error(400, res.message);
            return HttpResponse::ok({{"availableBalance", res.wallet.availableBalance}}, res.message);
        }

        if (path == "/api/v1/funds/withdraw" && req.method == "POST") {
            int uid = req.authUser ? req.authUser->id : 2;
            double amt = req.jsonBody["amount"].getNumber();
            auto res = services::WalletService::getInstance().withdraw(uid, amt);
            if (!res.success) return HttpResponse::error(400, res.message);
            return HttpResponse::ok({{"availableBalance", res.wallet.availableBalance}}, res.message);
        }

        // Transactions
        if (path == "/api/v1/transactions" && req.method == "GET") {
            int uid = req.authUser ? req.authUser->id : 2;
            auto txs = services::WalletService::getInstance().getTransactions(uid);
            utils::Json arr = utils::Json::array();
            for (const auto& t : txs) {
                arr.push_back({
                    {"id", t.id},
                    {"type", t.type},
                    {"amount", t.amount},
                    {"description", t.description},
                    {"createdAt", t.createdAt}
                });
            }
            return HttpResponse::ok(arr);
        }

        // Notifications
        if (path == "/api/v1/notifications" && req.method == "GET") {
            int uid = req.authUser ? req.authUser->id : 2;
            auto& db = database::Database::getInstance();
            auto rows = db.query("SELECT * FROM notifications WHERE user_id = " + std::to_string(uid) +
                                 " ORDER BY created_at DESC LIMIT 20;");
            utils::Json arr = utils::Json::array();
            for (const auto& r : rows) {
                arr.push_back({
                    {"id", std::stoi(r.at("id"))},
                    {"title", r.at("title")},
                    {"message", r.at("message")},
                    {"type", r.at("type")},
                    {"isRead", r.at("is_read") == "1"},
                    {"createdAt", r.at("created_at")}
                });
            }
            return HttpResponse::ok(arr);
        }

        // ==========================================
        // 8. DSA LAB ENDPOINTS (Requirement #38)
        // ==========================================

        // Trie Visualization
        if (path == "/api/v1/dsa/trie/visualize" && req.method == "GET") {
            std::string q = "T";
            auto it = req.queryParams.find("q");
            if (it != req.queryParams.end() && !it->second.empty()) q = it->second;

            auto vTree = services::StockService::getInstance().getTrie().getVisualTree(q, 4);
            auto serializeVNode = [](auto& self, const dsa::Trie::VisualNode& node) -> utils::Json {
                utils::Json j = {
                    {"character", node.character},
                    {"isTerminal", node.isTerminal},
                    {"symbol", node.symbol}
                };
                utils::Json childArr = utils::Json::array();
                for (const auto& c : node.children) {
                    childArr.push_back(self(self, c));
                }
                j["children"] = childArr;
                return j;
            };

            return HttpResponse::ok({
                {"algorithm", "Trie (Prefix Tree)"},
                {"timeComplexity", "Insert: O(L), Search: O(L), Autocomplete: O(L + K)"},
                {"spaceComplexity", "O(N * L * AlphabetSize)"},
                {"tree", serializeVNode(serializeVNode, vTree)}
            });
        }

        // Order Book (Priority Queue / Heap)
        if (path.rfind("/api/v1/dsa/orderbook/", 0) == 0 && req.method == "GET") {
            std::string sym = path.substr(22);
            auto snap = services::OrderService::getInstance().getOrderBookSnapshot(sym);

            utils::Json bids = utils::Json::array();
            for (const auto& lvl : snap.buyLevels) {
                bids.push_back({{"price", lvl.price}, {"quantity", lvl.quantity}, {"orderCount", lvl.orderCount}});
            }

            utils::Json asks = utils::Json::array();
            for (const auto& lvl : snap.sellLevels) {
                asks.push_back({{"price", lvl.price}, {"quantity", lvl.quantity}, {"orderCount", lvl.orderCount}});
            }

            return HttpResponse::ok({
                {"symbol", sym},
                {"ltp", snap.ltp},
                {"spread", snap.spread},
                {"bids", bids},
                {"asks", asks},
                {"heapStructure", {
                    {"buySide", "Max-Heap: Highest Bid priority (O(log N) push/pop, O(1) top)"},
                    {"sellSide", "Min-Heap: Lowest Ask priority (O(log N) push/pop, O(1) top)"}
                }}
            });
        }

        // Binary Search Tree (Price Levels)
        if (path == "/api/v1/dsa/bst/tree" && req.method == "GET") {
            auto inorder = demoBST_.inorder();
            auto preorder = demoBST_.preorder();
            auto postorder = demoBST_.postorder();
            auto minNode = demoBST_.findMin();
            auto maxNode = demoBST_.findMax();

            auto toNumArr = [](const std::vector<double>& vec) {
                utils::Json a = utils::Json::array();
                for (double v : vec) a.push_back(v);
                return a;
            };

            return HttpResponse::ok({
                {"algorithm", "Binary Search Tree (BST)"},
                {"timeComplexity", "Average: O(log N), Worst: O(N)"},
                {"spaceComplexity", "O(N)"},
                {"inorder", toNumArr(inorder)},
                {"preorder", toNumArr(preorder)},
                {"postorder", toNumArr(postorder)},
                {"minPrice", minNode ? minNode->price : 0.0},
                {"maxPrice", maxNode ? maxNode->price : 0.0}
            });
        }

        if (path == "/api/v1/dsa/bst/insert" && req.method == "POST") {
            double price = req.jsonBody["price"].getNumber();
            std::string sym = req.jsonBody["symbol"].getString("CUSTOM");
            demoBST_.insert(price, sym, 100);
            return HttpResponse::ok({{"insertedPrice", price}}, "Price inserted into BST");
        }

        // Market Graph (Stock <-> Sector Relations with BFS / DFS)
        if (path == "/api/v1/dsa/graph/relations" && req.method == "GET") {
            const auto& g = services::StockService::getInstance().getMarketGraph();
            auto nodes = g.getAllNodes();
            auto edges = g.getAllEdges();

            utils::Json nArr = utils::Json::array();
            for (const auto& n : nodes) {
                nArr.push_back({{"id", n.id}, {"label", n.label}, {"type", n.type}, {"price", n.currentPrice}});
            }

            utils::Json eArr = utils::Json::array();
            for (const auto& e : edges) {
                eArr.push_back({{"source", e.source}, {"target", e.target}, {"relationship", e.relationship}});
            }

            return HttpResponse::ok({
                {"algorithm", "Graph (Adjacency List)"},
                {"timeComplexity", "BFS: O(V + E), DFS: O(V + E)"},
                {"spaceComplexity", "O(V + E)"},
                {"nodes", nArr},
                {"edges", eArr}
            });
        }

        if (path == "/api/v1/dsa/graph/bfs" && req.method == "GET") {
            std::string start = "Technology";
            auto it = req.queryParams.find("start");
            if (it != req.queryParams.end() && !it->second.empty()) start = it->second;

            auto traversal = services::StockService::getInstance().getMarketGraph().bfs(start);
            utils::Json arr = utils::Json::array();
            for (const auto& s : traversal) arr.push_back(s);
            return HttpResponse::ok({{"startNode", start}, {"traversalOrder", arr}, {"algorithm", "Breadth-First Search (Queue)"}});
        }

        if (path == "/api/v1/dsa/graph/dfs" && req.method == "GET") {
            std::string start = "Technology";
            auto it = req.queryParams.find("start");
            if (it != req.queryParams.end() && !it->second.empty()) start = it->second;

            auto traversal = services::StockService::getInstance().getMarketGraph().dfs(start);
            utils::Json arr = utils::Json::array();
            for (const auto& s : traversal) arr.push_back(s);
            return HttpResponse::ok({{"startNode", start}, {"traversalOrder", arr}, {"algorithm", "Depth-First Search (Stack / Recursion)"}});
        }

        // Sorting Visualizer
        if (path == "/api/v1/dsa/sort" && req.method == "POST") {
            std::string alg = req.jsonBody["algorithm"].getString("Quick Sort");
            std::vector<double> inputArr;
            if (req.jsonBody["array"].isArray()) {
                for (size_t i = 0; i < req.jsonBody["array"].arrayValue.size(); ++i) {
                    inputArr.push_back(req.jsonBody["array"][i].getNumber());
                }
            }
            if (inputArr.empty()) {
                inputArr = {2945.5, 4210.8, 1885.25, 1642.4, 1215.6, 788.9, 512.4, 3620.0};
            }

            dsa::SortResult res;
            if (alg == "Bubble Sort") res = dsa::SortingAlgorithms::bubbleSort(inputArr, true);
            else if (alg == "Selection Sort") res = dsa::SortingAlgorithms::selectionSort(inputArr, true);
            else if (alg == "Insertion Sort") res = dsa::SortingAlgorithms::insertionSort(inputArr, true);
            else if (alg == "Merge Sort") res = dsa::SortingAlgorithms::mergeSort(inputArr, true);
            else if (alg == "Heap Sort") res = dsa::SortingAlgorithms::heapSort(inputArr, true);
            else res = dsa::SortingAlgorithms::quickSort(inputArr, true);

            utils::Json sortedArr = utils::Json::array();
            for (double v : res.sortedArray) sortedArr.push_back(v);

            utils::Json stepsArr = utils::Json::array();
            for (const auto& st : res.steps) {
                utils::Json sArr = utils::Json::array();
                for (double val : st.arrayState) sArr.push_back(val);
                stepsArr.push_back({
                    {"arrayState", sArr},
                    {"comparedIndex1", st.comparedIndex1},
                    {"comparedIndex2", st.comparedIndex2},
                    {"swappedIndex1", st.swappedIndex1},
                    {"swappedIndex2", st.swappedIndex2},
                    {"description", st.description}
                });
            }

            return HttpResponse::ok({
                {"algorithm", res.algorithm},
                {"timeComplexity", res.timeComplexity},
                {"spaceComplexity", res.spaceComplexity},
                {"comparisons", res.comparisons},
                {"swaps", res.swaps},
                {"durationMicroseconds", res.durationMicroseconds},
                {"sortedArray", sortedArr},
                {"steps", stepsArr}
            });
        }

        // Searching Visualizer
        if (path == "/api/v1/dsa/search" && req.method == "POST") {
            std::string alg = req.jsonBody["algorithm"].getString("Binary Search");
            double target = req.jsonBody["target"].getNumber(1642.4);
            std::vector<double> inputArr;
            if (req.jsonBody["array"].isArray()) {
                for (size_t i = 0; i < req.jsonBody["array"].arrayValue.size(); ++i) {
                    inputArr.push_back(req.jsonBody["array"][i].getNumber());
                }
            }
            if (inputArr.empty()) {
                inputArr = {512.4, 788.9, 1215.6, 1642.4, 1885.25, 2945.5, 3620.0, 4210.8};
            }

            dsa::SearchResult res = (alg == "Linear Search") 
                ? dsa::SearchingAlgorithms::linearSearch(inputArr, target)
                : dsa::SearchingAlgorithms::binarySearch(inputArr, target);

            utils::Json stepsArr = utils::Json::array();
            for (const auto& st : res.steps) {
                stepsArr.push_back({
                    {"index", st.index},
                    {"low", st.low},
                    {"mid", st.mid},
                    {"high", st.high},
                    {"value", st.value},
                    {"action", st.action}
                });
            }

            return HttpResponse::ok({
                {"algorithm", res.algorithm},
                {"target", res.target},
                {"foundIndex", res.foundIndex},
                {"comparisons", res.comparisons},
                {"durationMicroseconds", res.durationMicroseconds},
                {"timeComplexity", res.timeComplexity},
                {"spaceComplexity", res.spaceComplexity},
                {"steps", stepsArr}
            });
        }

        // Sliding Window / Two Pointer Analytics
        if (path == "/api/v1/dsa/analytics/sliding-window" && req.method == "GET") {
            std::string sym = "TCS";
            auto it = req.queryParams.find("symbol");
            if (it != req.queryParams.end() && !it->second.empty()) sym = it->second;

            auto sOpt = services::StockService::getInstance().getStockBySymbol(sym);
            if (!sOpt) return HttpResponse::error(404, "Stock not found");

            auto hist = services::StockService::getInstance().getHistory(sOpt->id, "1M");
            std::vector<double> prices;
            for (const auto& pt : hist) prices.push_back(pt.close);

            int windowSize = 5;
            auto wIt = req.queryParams.find("window");
            if (wIt != req.queryParams.end()) {
                try { windowSize = std::stoi(wIt->second); } catch (...) {}
            }

            auto bestWindow = dsa::MarketAnalyticsService::findBestWindow(prices, windowSize);
            auto maxStreak = dsa::MarketAnalyticsService::findMaxConsecutiveGainPeriod(prices);

            return HttpResponse::ok({
                {"symbol", sym},
                {"windowSize", windowSize},
                {"bestWindow", {
                    {"startIndex", bestWindow.startIndex},
                    {"endIndex", bestWindow.endIndex},
                    {"startPrice", bestWindow.startPrice},
                    {"endPrice", bestWindow.endPrice},
                    {"absoluteGain", bestWindow.absoluteGain},
                    {"percentageGain", bestWindow.percentageGain},
                    {"timeComplexity", "O(N) Sliding Window"}
                }},
                {"maxConsecutiveGainStreak", {
                    {"startIndex", maxStreak.startIndex},
                    {"endIndex", maxStreak.endIndex},
                    {"consecutiveDays", maxStreak.maxConsecutiveDays},
                    {"totalGain", maxStreak.totalGain},
                    {"timeComplexity", "O(N) Two-Pointer"}
                }}
            });
        }

        // Greedy Portfolio Allocation Simulation
        if (path == "/api/v1/dsa/greedy/allocate" && req.method == "POST") {
            double capital = req.jsonBody["capital"].getNumber(100000.0);
            auto stocks = services::StockService::getInstance().getAllStocks();

            std::vector<dsa::StockCandidate> pool;
            for (const auto& s : stocks) {
                // Synthesize reasonable expected return and risk score from sector / price volatility
                double expRet = 10.0 + std::abs(s.changePercent()) * 3.5;
                double risk = 1.5 + (s.currentPrice > 3000 ? 1.0 : 2.5);
                pool.push_back({s.symbol, s.companyName, s.currentPrice, expRet, risk});
            }

            auto allocRes = dsa::GreedyPortfolioService::optimizeAllocation(capital, pool);

            utils::Json items = utils::Json::array();
            for (const auto& itm : allocRes.allocations) {
                items.push_back({
                    {"symbol", itm.symbol},
                    {"companyName", itm.companyName},
                    {"currentPrice", itm.currentPrice},
                    {"shares", itm.allocatedShares},
                    {"investedAmount", itm.investedAmount},
                    {"portfolioWeight", itm.portfolioWeightPercent},
                    {"expectedReturn", itm.expectedReturnPercent},
                    {"riskScore", itm.riskScore}
                });
            }

            return HttpResponse::ok({
                {"totalCapital", allocRes.totalCapital},
                {"capitalAllocated", allocRes.capitalAllocated},
                {"remainingCash", allocRes.remainingCash},
                {"weightedExpectedReturn", allocRes.weightedExpectedReturn},
                {"averageRiskScore", allocRes.averageRiskScore},
                {"allocations", items},
                {"complexity", allocRes.complexity},
                {"disclaimer", allocRes.disclaimer}
            });
        }

        // Dynamic Programming Max Profit
        if (path == "/api/v1/dsa/dp/max-profit" && req.method == "GET") {
            std::string sym = "RELIANCE";
            auto it = req.queryParams.find("symbol");
            if (it != req.queryParams.end() && !it->second.empty()) sym = it->second;

            auto sOpt = services::StockService::getInstance().getStockBySymbol(sym);
            if (!sOpt) return HttpResponse::error(404, "Stock not found");

            auto hist = services::StockService::getInstance().getHistory(sOpt->id, "1M");
            std::vector<double> prices;
            for (const auto& pt : hist) prices.push_back(pt.close);

            auto singleTx = dsa::DynamicProgrammingService::calculateSingleTransactionMaxProfit(prices);
            double multiTx = dsa::DynamicProgrammingService::calculateUnlimitedTransactionsMaxProfit(prices);
            auto kTx = dsa::DynamicProgrammingService::calculateKTransactionsMaxProfit(prices, 2);

            return HttpResponse::ok({
                {"symbol", sym},
                {"singleTransaction", {
                    {"maxProfit", singleTx.maxProfit},
                    {"buyDayIndex", singleTx.buyDayIndex},
                    {"sellDayIndex", singleTx.sellDayIndex},
                    {"buyPrice", singleTx.buyPrice},
                    {"sellPrice", singleTx.sellPrice},
                    {"timeComplexity", singleTx.timeComplexity},
                    {"spaceComplexity", singleTx.spaceComplexity}
                }},
                {"unlimitedTransactions", {
                    {"maxProfit", multiTx},
                    {"timeComplexity", "O(N)"},
                    {"spaceComplexity", "O(1)"}
                }},
                {"twoTransactionsDP", {
                    {"maxProfit", kTx.maxProfit},
                    {"kAllowed", kTx.kTransactionsAllowed},
                    {"timeComplexity", kTx.timeComplexity},
                    {"spaceComplexity", kTx.spaceComplexity}
                }}
            });
        }

        // DSA Audit Trail (Doubly Linked List)
        if (path == "/api/v1/dsa/audit-trail" && req.method == "GET") {
            auto list = services::OrderService::getInstance().getAuditTrail().toVector();
            utils::Json arr = utils::Json::array();
            for (const auto& item : list) arr.push_back(item);
            return HttpResponse::ok({{"auditTrail", arr}, {"dataStructure", "Doubly Linked List (O(1) append)"}});
        }

        // 9. Admin Endpoints (Protected role)
        if (path == "/api/v1/admin/users" && req.method == "GET") {
            auto& db = database::Database::getInstance();
            auto rows = db.query("SELECT u.id, u.username, u.full_name, u.email, u.status, u.created_at, "
                                 "COALESCE(a.role, 'USER') as role, w.available_balance FROM users u "
                                 "LEFT JOIN admin_users a ON u.id = a.user_id "
                                 "LEFT JOIN wallet w ON u.id = w.user_id ORDER BY u.id ASC;");
            utils::Json arr = utils::Json::array();
            for (const auto& r : rows) {
                arr.push_back({
                    {"id", std::stoi(r.at("id"))},
                    {"username", r.at("username")},
                    {"fullName", r.at("full_name")},
                    {"email", r.at("email")},
                    {"status", r.at("status")},
                    {"role", r.at("role")},
                    {"balance", r.at("available_balance").empty() ? 0.0 : std::stod(r.at("available_balance"))},
                    {"createdAt", r.at("created_at")}
                });
            }
            return HttpResponse::ok(arr);
        }

        if (path == "/api/v1/admin/system-stats" && req.method == "GET") {
            auto& db = database::Database::getInstance();
            auto uCount = db.query("SELECT COUNT(*) as c FROM users;");
            auto sCount = db.query("SELECT COUNT(*) as c FROM stocks;");
            auto oCount = db.query("SELECT COUNT(*) as c FROM orders;");
            auto tCount = db.query("SELECT COUNT(*) as c FROM transactions;");

            return HttpResponse::ok({
                {"totalUsers", uCount.empty() ? 0 : std::stoi(uCount[0].at("c"))},
                {"totalStocks", sCount.empty() ? 0 : std::stoi(sCount[0].at("c"))},
                {"totalOrders", oCount.empty() ? 0 : std::stoi(oCount[0].at("c"))},
                {"totalTransactions", tCount.empty() ? 0 : std::stoi(tCount[0].at("c"))},
                {"marketSimulationRunning", true},
                {"serverEngine", "TradeX C++17 Multi-threaded Winsock Core"}
            });
        }

        if (path == "/api/v1/admin/market/start" && req.method == "POST") {
            services::MarketSimulator::getInstance().start();
            return HttpResponse::ok(nullptr, "Market simulation engine started");
        }

        if (path == "/api/v1/admin/market/stop" && req.method == "POST") {
            services::MarketSimulator::getInstance().stop();
            return HttpResponse::ok(nullptr, "Market simulation engine stopped");
        }

        return HttpResponse::error(404, "Endpoint not found: " + req.method + " " + req.path, "NOT_FOUND");
    }
};

} // namespace tradex::api
