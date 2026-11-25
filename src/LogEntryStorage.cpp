#include "LogEntryStorage.h"

#include <stdexcept>

LogEntryStorage::LogEntryStorage(const std::string& databaseFile)
    : storage_(createStorage(databaseFile))
{
    storage_.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign key constraints in LogEntryStorage");
        }
    };
    storage_.sync_schema();
}

int LogEntryStorage::addEntry(const LogEntry& entry)
{
    return storage_.insert(entry);
}

std::vector<LogEntry> LogEntryStorage::getBySourceId(int sourceId) const
{
    using namespace sqlite_orm;
    return storage_.get_all<LogEntry>(
        where(c(&LogEntry::sourceId) == sourceId),
        order_by(&LogEntry::timestamp).asc()
    );
}

std::vector<LogEntry> LogEntryStorage::getAll() const
{
    return storage_.get_all<LogEntry>();
}