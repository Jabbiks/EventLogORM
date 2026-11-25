#include "SQLiteLogSourceRepository.h"

#include <stdexcept>

SQLiteLogSourceRepository::SQLiteLogSourceRepository(const std::string& databaseFile)
    : storage_(createStorage(databaseFile))
{
    storage_.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign key constraints in SQLiteLogSourceRepository");
        }
    };
    storage_.sync_schema();
}

int SQLiteLogSourceRepository::add(const LogSource& source)
{
    return storage_.insert(source);
}

std::optional<LogSource> SQLiteLogSourceRepository::findById(int id) const
{
    return storage_.get_optional<LogSource>(id);
}

std::vector<LogSource> SQLiteLogSourceRepository::findAll() const
{
    return storage_.get_all<LogSource>();
}

bool SQLiteLogSourceRepository::update(const LogSource& source)
{
    auto existing = storage_.get_optional<LogSource>(source.id);
    if (!existing)
    {
        return false;
    }
    storage_.update(source);
    return true;
}

bool SQLiteLogSourceRepository::remove(int id)
{
    auto existing = storage_.get_optional<LogSource>(id);
    if (!existing)
    {
        return false;
    }
    storage_.remove<LogSource>(id);
    return true;
}