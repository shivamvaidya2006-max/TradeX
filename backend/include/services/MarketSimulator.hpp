#pragma once

#include "services/StockService.hpp"
#include <thread>
#include <atomic>
#include <random>
#include <functional>
#include <chrono>
#include <iostream>

namespace tradex::services {

struct MarketTick {
    std::string symbol;
    double price{0.0};
    double change{0.0};
    double changePercent{0.0};
    double dayHigh{0.0};
    double dayLow{0.0};
    int64_t volume{0};
    uint64_t timestamp{0};
};

class MarketSimulator {
public:
    using TickCallback = std::function<void(const MarketTick&)>;

    static MarketSimulator& getInstance() {
        static MarketSimulator instance;
        return instance;
    }

    void setTickCallback(TickCallback cb) {
        tickCallback_ = std::move(cb);
    }

    void setSimulationParams(double volatility, int intervalMs, bool marketOpenOverride) {
        volatility_ = volatility;
        intervalMs_ = intervalMs;
        marketOpenOverride_ = marketOpenOverride;
    }

    bool isMarketOpen() const {
        if (marketOpenOverride_) return true;

        // Check simulated Indian market hours 09:15 to 15:30
        auto now = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(now);
        tm localTm{};
#ifdef _WIN32
        localtime_s(&localTm, &tt);
#else
        localtime_r(&tt, &localTm);
#endif
        int currentMinute = localTm.tm_hour * 60 + localTm.tm_min;
        int openMinute = 9 * 60 + 15;
        int closeMinute = 15 * 60 + 30;

        return (currentMinute >= openMinute && currentMinute <= closeMinute);
    }

    void start() {
        if (running_) return;
        running_ = true;
        simThread_ = std::thread(&MarketSimulator::runLoop, this);
        std::cout << "[MarketSimulator] Simulator thread started." << std::endl;
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        if (simThread_.joinable()) {
            simThread_.join();
        }
        std::cout << "[MarketSimulator] Simulator thread stopped." << std::endl;
    }

    ~MarketSimulator() {
        stop();
    }

private:
    MarketSimulator() = default;

    std::atomic<bool> running_{false};
    std::atomic<bool> marketOpenOverride_{true}; // default open for educational demo
    std::atomic<double> volatility_{0.004};       // 0.4% typical step fluctuation
    std::atomic<int> intervalMs_{2000};          // 2 seconds per tick cycle
    std::thread simThread_;
    TickCallback tickCallback_;

    void runLoop() {
        std::mt19937_64 rng(std::random_device{}());
        std::normal_distribution<double> dist(0.0001, volatility_.load());

        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs_.load()));
            if (!running_) break;

            if (!isMarketOpen()) {
                continue;
            }

            auto stocks = StockService::getInstance().getAllStocks();
            if (stocks.empty()) continue;

            // Pick 3 to 6 stocks at random each cycle for realistic market activity
            size_t count = std::min<size_t>(stocks.size(), 4);
            std::shuffle(stocks.begin(), stocks.end(), rng);

            for (size_t i = 0; i < count; ++i) {
                const auto& s = stocks[i];
                double step = dist(rng);
                // Clamp max movement to +/- 2%
                step = std::max(-0.02, std::min(0.02, step));

                double newPrice = std::round(s.currentPrice * (1.0 + step) * 100.0) / 100.0;
                if (newPrice <= 1.0) newPrice = 1.0; // Floor

                int addVol = static_cast<int>(std::abs(step) * 100000) + 500;
                StockService::getInstance().updateStockPrice(s.symbol, newPrice, addVol);

                MarketTick tick;
                tick.symbol = s.symbol;
                tick.price = newPrice;
                tick.change = newPrice - s.previousClose;
                tick.changePercent = (s.previousClose > 0) ? (tick.change / s.previousClose) * 100.0 : 0.0;
                tick.dayHigh = std::max(s.dayHigh, newPrice);
                tick.dayLow = (s.dayLow > 0) ? std::min(s.dayLow, newPrice) : newPrice;
                tick.volume = s.volume + addVol;
                tick.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();

                if (tickCallback_) {
                    tickCallback_(tick);
                }
            }
        }
    }
};

} // namespace tradex::services
