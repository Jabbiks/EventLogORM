#include "RawSqliteDemo.h"
#include "LogSource.h"

#include <iostream>
#include <optional>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace
{
    int insertLogSourceRaw(sqlite3* db, const LogSource& source)
    {
        const char* sql = R"(
            INSERT INTO LogSource (name, host, status)
            VALUES (?, ?, ?);
        )";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        rc = sqlite3_bind_text(stmt, 1, source.name.c_str(), -1, SQLITE_TRANSIENT);
        if (rc == SQLITE_OK)
        {
            rc = sqlite3_bind_text(stmt, 2, source.host.c_str(), -1, SQLITE_TRANSIENT);
        }
        if (rc == SQLITE_OK)
        {
            rc = sqlite3_bind_text(stmt, 3, source.status.c_str(), -1, SQLITE_TRANSIENT);
        }

        if (rc != SQLITE_OK)
        {
            std::string err = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error(err);
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            std::string err = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error(err);
        }

        const int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db));
        sqlite3_finalize(stmt);
        return insertedId;
    }

    std::optional<LogSource> findLogSourceByIdRaw(sqlite3* db, int id)
    {
        const char* sql = R"(
            SELECT id, name, host, status
            FROM LogSource
            WHERE id = ?;
        )";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        rc = sqlite3_bind_int(stmt, 1, id);
        if (rc != SQLITE_OK)
        {
            std::string err = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error(err);
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            LogSource source;
            source.id = sqlite3_column_int(stmt, 0);
            source.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            source.host = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            source.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            sqlite3_finalize(stmt);
            return source;
        }

        if (rc != SQLITE_DONE)
        {
            std::string err = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error(err);
        }

        sqlite3_finalize(stmt);
        return std::nullopt;
    }
}

void runRawSqliteApiDemo()
{
    std::cout << "=== 1. RAW SQLITE C API DEMONSTRATION ===\n";
    std::cout << "SQLite library version: " << sqlite3_libversion() << '\n';

    sqlite3* db = nullptr;
    if (sqlite3_open("event_logs_basic.db", &db) != SQLITE_OK)
    {
        std::string err = db ? sqlite3_errmsg(db) : "Unknown open error";
        sqlite3_close(db);
        throw std::runtime_error("Cannot open database: " + err);
    }

    const char* createTableSql = R"(
        CREATE TABLE IF NOT EXISTS LogSource (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            host TEXT NOT NULL,
            status TEXT NOT NULL
        );
    )";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string err = errMsg ? errMsg : "Unknown SQL error";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        throw std::runtime_error("SQL error: " + err);
    }

    try
    {
        LogSource authSource{0, "AuthService", "srv-auth-01", "active"};
        int id = insertLogSourceRaw(db, authSource);
        std::cout << "Inserted LogSource via C API, ID: " << id << '\n';

        auto found = findLogSourceByIdRaw(db, id);
        if (found)
        {
            std::cout << "Found via C API -> ID: " << found->id
                      << " | Name: " << found->name
                      << " | Host: " << found->host
                      << " | Status: " << found->status << '\n';
        }
    }
    catch (...)
    {
        sqlite3_close(db);
        throw;
    }

    sqlite3_close(db);
}