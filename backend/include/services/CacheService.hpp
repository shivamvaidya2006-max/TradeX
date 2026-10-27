#pragma once

#include <string>
#include <unordered_map>
#include <list>
#include <chrono>
#include <mutex>
#include <optional>

namespace tradex::services {

struct CacheEntry {
    std::string value;
    std::chrono::steady_clock::time_point expiresAt;
};

/**
 * @brief Thread-safe LRU Cache with TTL expiry for market quotes and search results.
 * Can bridge to Redis or operate as a high-speed L1 cache.
 */
class CacheService {
public:
    static CacheService& getInstance() {
        static CacheService instance;
        return instance;
    }

    void set(const std::string& key, const std::string& value, int ttlSeconds = 60) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto exp = std::chrono::steady_clock::now() + std::chrono::seconds(ttlSeconds);

        if (map_.find(key) != map_.end()) {
            lruList_.erase(map_[key].second);
        } else if (map_.size() >= maxCapacity_) {
            // Evict least recently used item
            std::string oldestKey = lruList_.back();
            lruList_.pop_back();
            map_.erase(oldestKey);
        }

        lruList_.push_front(key);
        map_[key] = {CacheEntry{value, exp}, lruList_.begin()};
    }

    std::optional<std::string> get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it == map_.end()) return std::nullopt;

        // Check TTL
        if (std::chrono::steady_clock::now() > it->second.first.expiresAt) {
            lruList_.erase(it->second.second);
            map_.erase(it);
            return std::nullopt;
        }

        // Move to front of LRU list
        lruList_.erase(it->second.second);
        lruList_.push_front(key);
        it->second.second = lruList_.begin();

        return it->second.first.value;
    }

    void remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            lruList_.erase(it->second.second);
            map_.erase(it);
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.clear();
        lruList_.clear();
    }

private:
    CacheService() = default;

    size_t maxCapacity_{1000};
    mutable std::mutex mutex_;
    std::list<std::string> lruList_;
    std::unordered_map<std::string, std::pair<CacheEntry, std::list<std::string>::iterator>> map_;
};

} // namespace tradex::services
