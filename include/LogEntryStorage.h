#pragma once

#include <string>
#include <vector>

#include "Database.h"
#include "LogEntry.h"

// Окремий компонент доступу до даних залежної сутності LogEntry
class LogEntryStorage
{
private:
    using Storage = decltype(createStorage(""));
    mutable Storage storage_;

public:
    explicit LogEntryStorage(const std::string& databaseFile);

    int addEntry(const LogEntry& entry);
    std::vector<LogEntry> getBySourceId(int sourceId) const;
    std::vector<LogEntry> getAll() const;
};