#include "OrmDemos.h"
#include "Database.h"
#include "ILogSourceRepository.h"
#include "SQLiteLogSourceRepository.h"
#include "LogEntryStorage.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace
{
    void enableForeignKeys(sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable PRAGMA foreign_keys = ON");
        }
    }

    template <typename T>
    double toDoubleValue(const T& value)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            return static_cast<double>(value);
        }
        else
        {
            return value ? static_cast<double>(*value) : 0.0;
        }
    }
}

void runBasicOrmDemo()
{
    std::cout << "\n=== 2. SQLITE_ORM BASIC CRUD DEMONSTRATION ===\n";
    auto storage = createStorage("event_logs_basic.db");
    storage.on_open = enableForeignKeys;
    storage.sync_schema();

    LogSource paymentSource{0, "PaymentGateway", "srv-pay-01", "active"};
    int id = storage.insert(paymentSource);
    std::cout << "Inserted LogSource via sqlite_orm, ID: " << id << '\n';

    auto found = storage.get_optional<LogSource>(id);
    if (found)
    {
        std::cout << "Found via sqlite_orm -> ID: " << found->id
                  << " | Name: " << found->name
                  << " | Host: " << found->host
                  << " | Status: " << found->status << '\n';
    }
}

void runRelationshipDemo()
{
    std::cout << "\n=== 3. ONE-TO-MANY RELATIONSHIP & FOREIGN KEY TEST ===\n";
    auto storage = createStorage("event_logs_basic.db");
    storage.on_open = enableForeignKeys;
    storage.open_forever();
    storage.sync_schema();

    LogSource apiSource{0, "ApiGateway", "srv-edge-01", "active"};
    int sourceId = storage.insert(apiSource);
    std::cout << "Created parent LogSource ID: " << sourceId << '\n';

    LogEntry e1{0, sourceId, "2026-11-25 10:00:05", "INFO", "Gateway started", 14.5};
    LogEntry e2{0, sourceId, "2026-11-25 10:05:12", "WARN", "High latency on /orders", 342.0};
    int id1 = storage.insert(e1);
    int id2 = storage.insert(e2);
    std::cout << "Inserted child LogEntry IDs: " << id1 << ", " << id2 << '\n';

    try
    {
        LogEntry invalidEntry{0, -1, "2026-11-25 10:10:00", "ERROR", "Orphan log entry", 5.0};
        storage.insert(invalidEntry);
        std::cout << "ERROR: Foreign key constraint did not trigger!\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "Foreign key check passed (rejected invalid sourceId=-1): " << ex.what() << '\n';
    }
}

void runRepositoryDemo()
{
    std::cout << "\n=== 4. REPOSITORY PATTERN DEMONSTRATION ===\n";
    SQLiteLogSourceRepository sqliteRepo("event_logs_basic.db");
    ILogSourceRepository& repository = sqliteRepo;
    LogEntryStorage entryStorage("event_logs_basic.db");

    // CREATE
    int newId = repository.add(LogSource{0, "NotificationWorker", "srv-notify-02", "active"});
    std::cout << "[CREATE] Added LogSource ID: " << newId << '\n';

    // READ (get_optional)
    auto found = repository.findById(newId);
    if (found)
    {
        std::cout << "[READ  ] Found: " << found->name << " (" << found->host << "), status=" << found->status << '\n';

        // UPDATE
        found->status = "paused";
        bool updated = repository.update(*found);
        auto afterUpdate = repository.findById(newId);
        std::cout << "[UPDATE] Success=" << std::boolalpha << updated
                  << ", new status=" << (afterUpdate ? afterUpdate->status : "N/A") << '\n';
    }

    // READ ALL
    std::cout << "[READ ALL] Current LogSources in DB:\n";
    for (const auto& src : repository.findAll())
    {
        std::cout << "  #" << src.id << " | " << src.name << " | " << src.host << " | " << src.status << '\n';
    }

    // DELETE
    bool removed = repository.remove(newId);
    auto afterDelete = repository.findById(newId);
    std::cout << "[DELETE] Removed=" << std::boolalpha << removed
              << ", exists after delete=" << afterDelete.has_value() << '\n';
    std::cout << "Total LogEntry records in basic DB: " << entryStorage.getAll().size() << '\n';
}

void runQueriesAndAggregatesDemo()
{
    std::cout << "\n=== 5. QUERIES, AGGREGATION & INNER JOIN DEMONSTRATION ===\n";
    const std::string dbFile = "event_logs_queries.db";
    std::filesystem::remove(dbFile);

    auto storage = createStorage(dbFile);
    storage.on_open = enableForeignKeys;
    storage.open_forever();
    storage.sync_schema();

    std::vector<LogSource> sources = {
        {0, "AuthService",      "srv-auth-01",   "active"},
        {0, "OrderProcessor",   "srv-orders-01", "active"},
        {0, "LegacyImporter",   "srv-batch-02",  "paused"},
        {0, "BillingEngine",    "srv-bill-01",   "active"},
        {0, "ArchiveDaemon",    "srv-store-03",  "archived"}
    };

    std::vector<int> sIds;
    for (const auto& s : sources)
    {
        sIds.push_back(storage.insert(s));
    }

    std::vector<LogEntry> entries = {
        {0, sIds[0], "2026-11-25 08:00:01", "INFO",  "User login succeeded",           12.4},
        {0, sIds[0], "2026-11-25 08:15:40", "WARN",  "Invalid password attempt",       18.2},
        {0, sIds[0], "2026-11-25 08:30:12", "ERROR", "LDAP connection timeout",       510.0},

        {0, sIds[1], "2026-11-25 09:01:10", "INFO",  "Order #1045 validated",          25.0},
        {0, sIds[1], "2026-11-25 09:05:22", "ERROR", "Inventory lock conflict",       420.5},
        {0, sIds[1], "2026-11-25 09:12:00", "INFO",  "Order #1046 dispatched",         31.8},

        {0, sIds[2], "2026-11-25 02:00:00", "INFO",  "Nightly CSV batch started",      85.0},
        {0, sIds[2], "2026-11-25 02:14:33", "WARN",  "Malformed row skipped",         110.4},
        {0, sIds[2], "2026-11-25 02:29:59", "ERROR", "Disk quota threshold reached",  640.0},

        {0, sIds[3], "2026-11-25 11:00:05", "INFO",  "Invoice #881 generated",         44.2},
        {0, sIds[3], "2026-11-25 11:10:18", "INFO",  "Tax calculation completed",      19.5},
        {0, sIds[3], "2026-11-25 11:45:00", "WARN",  "Retry bank webhook",            290.0},

        {0, sIds[4], "2026-11-24 23:00:00", "INFO",  "Log rotation started",           55.0},
        {0, sIds[4], "2026-11-24 23:05:10", "INFO",  "Compressed 120 segments",       180.0},
        {0, sIds[4], "2026-11-24 23:10:00", "INFO",  "Archive verification OK",        40.0}
    };

    for (const auto& e : entries)
    {
        storage.insert(e);
    }

    std::cout << "Seeded database -> LogSources: " << storage.count<LogSource>()
              << ", LogEntries: " << storage.count<LogEntry>() << '\n';

    using namespace sqlite_orm;

    // 1. Фільтрація: активні джерела подій
    std::cout << "\n1) Filter (status == 'active'):\n";
    auto activeSources = storage.get_all<LogSource>(
        where(c(&LogSource::status) == "active")
    );
    for (const auto& s : activeSources)
    {
        std::cout << "  " << s.id << " | " << s.name << " (" << s.host << ")\n";
    }

    // 2. Сортування та обмеження: Top-3 найповільніші події
    std::cout << "\n2) Sort & Limit (Top 3 slowest LogEntries by durationMs DESC):\n";
    auto slowestEntries = storage.get_all<LogEntry>(
        order_by(&LogEntry::durationMs).desc(),
        limit(3)
    );
    for (const auto& e : slowestEntries)
    {
        std::cout << "  Entry #" << e.id << " [" << e.level << "] "
                  << e.message << " -> " << e.durationMs << " ms\n";
    }

    // 3. Агрегація: COUNT, SUM, AVG та GROUP BY
    std::cout << "\n3) Aggregations (COUNT, SUM, AVG & GROUP BY sourceId):\n";
    auto totalCount = storage.count<LogEntry>();
    double totalDuration = toDoubleValue(storage.sum(&LogEntry::durationMs));
    double avgDuration = toDoubleValue(storage.avg(&LogEntry::durationMs));

    std::cout << std::fixed << std::setprecision(2)
              << "  Total entries: " << totalCount
              << " | Total duration: " << totalDuration << " ms"
              << " | Avg duration: " << avgDuration << " ms\n";

    auto groupedStats = storage.select(
        columns(&LogEntry::sourceId, count(&LogEntry::id), sum(&LogEntry::durationMs)),
        group_by(&LogEntry::sourceId)
    );
    for (const auto& [srcId, cnt, sumDur] : groupedStats)
    {
        std::cout << "  Source ID=" << srcId
                  << " | Entries=" << cnt
                  << " | Sum duration=" << toDoubleValue(sumDur) << " ms\n";
    }

    // 4. Об'єднання таблиць (INNER JOIN) із фільтрацією за рівнем ERROR
    std::cout << "\n4) INNER JOIN (LogSource + LogEntry where level == 'ERROR'):\n";
    auto errorDetails = storage.select(
        columns(
            &LogSource::name,
            &LogSource::host,
            &LogEntry::timestamp,
            &LogEntry::message,
            &LogEntry::durationMs
        ),
        inner_join<LogEntry>(on(c(&LogSource::id) == &LogEntry::sourceId)),
        where(c(&LogEntry::level) == "ERROR"),
        order_by(&LogEntry::durationMs).desc()
    );

    for (const auto& [srcName, host, ts, msg, dur] : errorDetails)
    {
        std::cout << "  [" << ts << "] " << srcName << " (" << host << "): "
                  << msg << " (" << dur << " ms)\n";
    }
}

void runTransactionDemos()
{
    std::cout << "\n=== 6. TRANSACTIONS: COMMIT, ROLLBACK & WITHOUT TRANSACTION ===\n";
    const std::string dbFile = "event_logs_queries.db";

    auto storage = createStorage(dbFile);
    storage.on_open = enableForeignKeys;
    storage.open_forever();
    storage.sync_schema();

    // Сценарій 6.1: Успішна транзакція (COMMIT)
    std::cout << "\n--- 6.1. Successful Transaction (COMMIT) ---\n";
    std::cout << "Before COMMIT -> Sources: " << storage.count<LogSource>()
              << ", Entries: " << storage.count<LogEntry>() << '\n';

    bool committed = storage.transaction([&]()
    {
        int newSourceId = storage.insert(LogSource{0, "SearchIndexer", "srv-search-01", "active"});
        storage.insert(LogEntry{0, newSourceId, "2026-11-25 12:00:00", "INFO", "Source registered and initialized", 15.0});
        return true;
    });

    std::cout << "Committed: " << std::boolalpha << committed
              << " | After COMMIT -> Sources: " << storage.count<LogSource>()
              << ", Entries: " << storage.count<LogEntry>() << '\n';

    // Сценарій 6.2: Помилка на другій операції -> автоматичний ROLLBACK
    std::cout << "\n--- 6.2. Failed Transaction (ROLLBACK) ---\n";
    std::cout << "Before ROLLBACK -> Sources: " << storage.count<LogSource>()
              << ", Entries: " << storage.count<LogEntry>() << '\n';

    try
    {
        storage.transaction([&]()
        {
            storage.insert(LogSource{0, "MetricsCollector", "srv-mon-01", "active"});
            storage.insert(LogEntry{0, -1, "2026-11-25 12:05:00", "ERROR", "Invalid FK test", 10.0});
            return true;
        });
    }
    catch (const std::exception& ex)
    {
        std::cout << "Transaction aborted with exception: " << ex.what() << '\n';
    }

    using namespace sqlite_orm;
    auto leaked = storage.get_all<LogSource>(where(c(&LogSource::name) == "MetricsCollector"));
    std::cout << "After ROLLBACK -> Sources: " << storage.count<LogSource>()
              << ", Entries: " << storage.count<LogEntry>()
              << " | 'MetricsCollector' exists: " << std::boolalpha << !leaked.empty() << '\n';

    // Сценарій 6.3: Ті самі операції БЕЗ явної транзакції
    std::cout << "\n--- 6.3. Same Operations WITHOUT Explicit Transaction ---\n";
    const std::string noTxDb = "event_logs_no_tx.db";
    std::filesystem::remove(noTxDb);

    auto noTxStorage = createStorage(noTxDb);
    noTxStorage.on_open = enableForeignKeys;
    noTxStorage.open_forever();
    noTxStorage.sync_schema();

    try
    {
        noTxStorage.insert(LogSource{0, "MetricsCollector", "srv-mon-01", "active"});
        noTxStorage.insert(LogEntry{0, -1, "2026-11-25 12:05:00", "ERROR", "Invalid FK test", 10.0});
    }
    catch (const std::exception& ex)
    {
        std::cout << "Second operation failed: " << ex.what() << '\n';
    }

    std::cout << "After operations WITHOUT transaction -> Sources: " << noTxStorage.count<LogSource>()
              << " (partial save!), Entries: " << noTxStorage.count<LogEntry>() << '\n';
}