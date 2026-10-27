#pragma once

#include <mysql.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <queue>
#include <iostream>
#include <stdexcept>

namespace tradex::database {

using Row = std::unordered_map<std::string, std::string>;
using QueryResult = std::vector<Row>;

/**
 * @brief Thread-safe MySQL Database Connection Pool.
 */
class Database {
public:
    static Database& getInstance() {
        static Database instance;
        return instance;
    }

    void initialize(const std::string& host, const std::string& user,
                    const std::string& password, const std::string& dbName,
                    unsigned int port = 3306, size_t poolSize = 5) {
        std::lock_guard<std::mutex> lock(poolMutex_);
        host_ = host;
        user_ = user;
        password_ = password;
        dbName_ = dbName;
        port_ = port;

        for (size_t i = 0; i < poolSize; ++i) {
            MYSQL* conn = createConnection();
            if (conn) {
                connectionPool_.push(conn);
            }
        }
        std::cout << "[DB] Connection pool initialized with " << connectionPool_.size() << " connections." << std::endl;
    }

    MYSQL* acquireConnection() {
        std::lock_guard<std::mutex> lock(poolMutex_);
        if (connectionPool_.empty()) {
            return createConnection();
        }
        MYSQL* conn = connectionPool_.front();
        connectionPool_.pop();
        if (mysql_ping(conn) != 0) {
            mysql_close(conn);
            conn = createConnection();
        }
        return conn;
    }

    void releaseConnection(MYSQL* conn) {
        if (!conn) return;
        std::lock_guard<std::mutex> lock(poolMutex_);
        connectionPool_.push(conn);
    }

    /**
     * @brief Executes a SELECT query and returns rows as key-value maps.
     */
    QueryResult query(const std::string& sql) {
        MYSQL* conn = acquireConnection();
        if (!conn) {
            throw std::runtime_error("No available DB connection");
        }

        if (mysql_query(conn, sql.c_str()) != 0) {
            std::string err = mysql_error(conn);
            releaseConnection(conn);
            throw std::runtime_error("MySQL query error: " + err + " (SQL: " + sql + ")");
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            releaseConnection(conn);
            return {};
        }

        int numFields = mysql_num_fields(res);
        MYSQL_FIELD* fields = mysql_fetch_fields(res);

        QueryResult results;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            Row r;
            unsigned long* lengths = mysql_fetch_lengths(res);
            for (int i = 0; i < numFields; ++i) {
                std::string fieldName = fields[i].name;
                std::string val = row[i] ? std::string(row[i], lengths[i]) : "";
                r[fieldName] = val;
            }
            results.push_back(r);
        }

        mysql_free_result(res);
        releaseConnection(conn);
        return results;
    }

    /**
     * @brief Executes an INSERT, UPDATE, or DELETE statement.
     * @return Number of affected rows
     */
    int64_t execute(const std::string& sql) {
        MYSQL* conn = acquireConnection();
        if (!conn) {
            throw std::runtime_error("No available DB connection");
        }

        if (mysql_query(conn, sql.c_str()) != 0) {
            std::string err = mysql_error(conn);
            releaseConnection(conn);
            throw std::runtime_error("MySQL execute error: " + err + " (SQL: " + sql + ")");
        }

        int64_t affected = mysql_affected_rows(conn);
        releaseConnection(conn);
        return affected;
    }

    /**
     * @brief Executes an INSERT statement and returns the auto-increment last insert ID.
     */
    int64_t insertAndGetId(const std::string& sql) {
        MYSQL* conn = acquireConnection();
        if (!conn) {
            throw std::runtime_error("No available DB connection");
        }

        if (mysql_query(conn, sql.c_str()) != 0) {
            std::string err = mysql_error(conn);
            releaseConnection(conn);
            throw std::runtime_error("MySQL insert error: " + err + " (SQL: " + sql + ")");
        }

        int64_t insertId = mysql_insert_id(conn);
        releaseConnection(conn);
        return insertId;
    }

    /**
     * @brief Escapes string to prevent SQL injection.
     */
    std::string escape(const std::string& src) {
        MYSQL* conn = acquireConnection();
        if (!conn) return src;
        std::vector<char> buffer(src.size() * 2 + 1);
        mysql_real_escape_string(conn, buffer.data(), src.c_str(), src.size());
        std::string res(buffer.data());
        releaseConnection(conn);
        return res;
    }

    ~Database() {
        std::lock_guard<std::mutex> lock(poolMutex_);
        while (!connectionPool_.empty()) {
            MYSQL* conn = connectionPool_.front();
            connectionPool_.pop();
            mysql_close(conn);
        }
    }

private:
    Database() = default;

    std::string host_{"127.0.0.1"};
    std::string user_{"root"};
    std::string password_{"Shivam@20"};
    std::string dbName_{"tradex_db"};
    unsigned int port_{3306};
    std::mutex poolMutex_;
    std::queue<MYSQL*> connectionPool_;

    MYSQL* createConnection() {
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) return nullptr;

        bool reconnect = true;
        mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

        if (!mysql_real_connect(conn, host_.c_str(), user_.c_str(), password_.c_str(),
                                dbName_.c_str(), port_, nullptr, 0)) {
            std::cerr << "[DB Error] " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return nullptr;
        }
        return conn;
    }
};

} // namespace tradex::database
