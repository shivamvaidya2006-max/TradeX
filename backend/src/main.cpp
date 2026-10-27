#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <iostream>
#include <csignal>
#include "database/Database.hpp"
#include "services/StockService.hpp"
#include "services/MarketSimulator.hpp"
#include "api/HttpServer.hpp"

std::unique_ptr<tradex::api::HttpServer> g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[TradeX] Caught interrupt signal (" << signum << "). Shutting down gracefully..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    tradex::services::MarketSimulator::getInstance().stop();
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << R"(
  _______             _   __   __
 |__   __|           | |  \ \ / /
    | |_ __ __ _   __| | __\ V / 
    | | '__/ _` | / _` |/ _ \> <  
    | | | | (_| || (_| |  __//. \ 
    |_|_|  \__,_| \__,_|\___/_/ \_\
    
    Smart Trading. Simulated. Data Driven.
    ======================================
    )" << std::endl;

    std::string dbHost = "127.0.0.1";
    std::string dbUser = "root";
    std::string dbPass = "Shivam@20";
    std::string dbName = "tradex_db";
    unsigned int dbPort = 3306;
    int httpPort = 8080;

    std::cout << "[Init] Connecting to MySQL (" << dbHost << ":" << dbPort << ")..." << std::endl;
    try {
        tradex::database::Database::getInstance().initialize(dbHost, dbUser, dbPass, dbName, dbPort, 4);
    } catch (const std::exception& e) {
        std::cerr << "[Init Error] Database initialization failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[Init] Initializing StockService and populating DSA Trie..." << std::endl;
    tradex::services::StockService::getInstance().loadStocksFromDatabase();

    std::cout << "[Init] Starting Market Simulation Engine..." << std::endl;
    tradex::services::MarketSimulator::getInstance().start();

    std::cout << "[Init] Launching HTTP and WebSocket Server on port " << httpPort << "..." << std::endl;
    g_server = std::make_unique<tradex::api::HttpServer>(httpPort);
    g_server->start();

    return 0;
}
