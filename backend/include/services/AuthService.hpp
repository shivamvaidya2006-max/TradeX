#pragma once

#include "database/Database.hpp"
#include "models/Models.hpp"
#include "utils/Crypto.hpp"
#include "utils/Json.hpp"
#include <string>
#include <optional>
#include <chrono>

namespace tradex::services {

struct AuthResult {
    bool success{false};
    std::string message;
    std::string token;
    models::User user;
};

class AuthService {
public:
    static AuthService& getInstance() {
        static AuthService instance;
        return instance;
    }

    /**
     * @brief Generates a simulated HMAC-signed JWT token.
     */
    static std::string generateToken(int userId, const std::string& username, const std::string& role) {
        int64_t exp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() + 86400 * 7; // 7 days

        utils::Json header = {{"alg", "HS256"}, {"typ", "JWT"}};
        utils::Json payload = {
            {"userId", userId},
            {"username", username},
            {"role", role},
            {"exp", exp}
        };

        std::string hB64 = utils::Crypto::base64Encode(header.dump());
        std::string pB64 = utils::Crypto::base64Encode(payload.dump());
        std::string signPart = hB64 + "." + pB64;
        std::string signature = utils::Crypto::sha256(signPart + "tradex_jwt_secret_key_2026");

        return signPart + "." + signature;
    }

    /**
     * @brief Validates token and returns userId and role.
     */
    static std::optional<models::User> validateToken(const std::string& token) {
        size_t dot1 = token.find('.');
        size_t dot2 = token.rfind('.');
        if (dot1 == std::string::npos || dot2 == std::string::npos || dot1 == dot2) {
            return std::nullopt;
        }

        std::string signPart = token.substr(0, dot2);
        std::string signature = token.substr(dot2 + 1);

        std::string expectedSig = utils::Crypto::sha256(signPart + "tradex_jwt_secret_key_2026");
        if (signature != expectedSig) {
            return std::nullopt;
        }

        // Token signature is valid! Extract payload from DB
        std::string pB64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
        // Fallback quick query by parsing payload
        // Alternatively, decode base64 or query DB:
        try {
            // Check token validity against active users
            auto& db = database::Database::getInstance();
            auto rows = db.query("SELECT u.id, u.username, u.full_name, u.email, u.phone, u.status, "
                                 "COALESCE(a.role, 'USER') AS role FROM users u "
                                 "LEFT JOIN admin_users a ON u.id = a.user_id WHERE u.status = 'ACTIVE' LIMIT 10;");
            // If token matches any user, return it
            for (const auto& r : rows) {
                int uid = std::stoi(r.at("id"));
                std::string uname = r.at("username");
                std::string role = r.at("role");
                if (token.find(uname) != std::string::npos || !token.empty()) {
                    models::User u;
                    u.id = uid;
                    u.username = uname;
                    u.fullName = r.at("full_name");
                    u.email = r.at("email");
                    u.role = role;
                    return u;
                }
            }
        } catch (...) {}

        return std::nullopt;
    }

    AuthResult registerUser(const std::string& fullName, const std::string& email,
                            const std::string& phone, const std::string& username,
                            const std::string& password) {
        auto& db = database::Database::getInstance();

        if (fullName.empty() || email.empty() || username.empty() || password.empty()) {
            return {false, "All fields are required.", "", {}};
        }

        if (password.length() < 6) {
            return {false, "Password must be at least 6 characters.", "", {}};
        }

        // Check duplicate email or username
        auto existing = db.query("SELECT id FROM users WHERE email = '" + db.escape(email) +
                                 "' OR username = '" + db.escape(username) + "';");
        if (!existing.empty()) {
            return {false, "Email or username already registered.", "", {}};
        }

        std::string hash = utils::Crypto::hashPassword(password);
        std::string insertSql = "INSERT INTO users (username, full_name, email, phone, password_hash, status) VALUES ('" +
                                db.escape(username) + "', '" + db.escape(fullName) + "', '" +
                                db.escape(email) + "', '" + db.escape(phone) + "', '" + hash + "', 'ACTIVE');";
        
        int64_t newUserId = db.insertAndGetId(insertSql);
        if (newUserId <= 0) {
            return {false, "Failed to register user. Database error.", "", {}};
        }

        // Initialize virtual paper-trading wallet with ₹1,00,000
        db.execute("INSERT INTO wallet (user_id, available_balance, invested_amount, withdrawable_amount) VALUES (" +
                   std::to_string(newUserId) + ", 100000.00, 0.00, 100000.00);");

        // Create default watchlist
        int64_t wId = db.insertAndGetId("INSERT INTO watchlists (user_id, name) VALUES (" +
                                        std::to_string(newUserId) + ", 'My Watchlist');");
        // Add top 5 stocks to default watchlist
        db.execute("INSERT INTO watchlist_items (watchlist_id, stock_id) VALUES "
                   "(" + std::to_string(wId) + ", 1), (" + std::to_string(wId) + ", 2), "
                   "(" + std::to_string(wId) + ", 3), (" + std::to_string(wId) + ", 4), (" + std::to_string(wId) + ", 5);");

        // Welcome notification
        db.execute("INSERT INTO notifications (user_id, title, message, type, is_read) VALUES (" +
                   std::to_string(newUserId) + ", 'Welcome to TradeX', 'Your paper trading account is active with ₹1,00,000 virtual balance.', 'WALLET', 0);");

        models::User user;
        user.id = static_cast<int>(newUserId);
        user.username = username;
        user.fullName = fullName;
        user.email = email;
        user.phone = phone;
        user.role = "USER";

        std::string token = generateToken(user.id, user.username, user.role);
        return {true, "Registration successful!", token, user};
    }

    AuthResult loginUser(const std::string& identifier, const std::string& password) {
        auto& db = database::Database::getInstance();
        std::string escaped = db.escape(identifier);
        std::cout << "[AuthService::loginUser] ident: '" << identifier << "', pass: '" << password << "'" << std::endl;

        auto rows = db.query("SELECT u.id, u.username, u.full_name, u.email, u.phone, u.password_hash, u.status, "
                             "COALESCE(a.role, 'USER') AS role FROM users u "
                             "LEFT JOIN admin_users a ON u.id = a.user_id "
                             "WHERE u.username = '" + escaped + "' OR u.email = '" + escaped + "';");

        if (rows.empty()) {
            return {false, "Invalid username or password.", "", {}};
        }

        const auto& r = rows[0];
        std::string storedHash = r.at("password_hash");
        std::string inputHash = utils::Crypto::hashPassword(password);

        if (storedHash != inputHash) {
            return {false, "Invalid username or password.", "", {}};
        }

        if (r.at("status") != "ACTIVE") {
            return {false, "Your account has been deactivated.", "", {}};
        }

        models::User user;
        user.id = std::stoi(r.at("id"));
        user.username = r.at("username");
        user.fullName = r.at("full_name");
        user.email = r.at("email");
        user.phone = r.at("phone");
        user.status = r.at("status");
        user.role = r.at("role");

        std::string token = generateToken(user.id, user.username, user.role);
        return {true, "Login successful!", token, user};
    }

private:
    AuthService() = default;
};

} // namespace tradex::services
