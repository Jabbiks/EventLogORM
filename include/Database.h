#pragma once

#include <string>
#include <sqlite_orm/sqlite_orm.h>

#include "LogSource.h"
#include "LogEntry.h"

inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,
        // Основна таблиця: джерела подій
        make_table(
            "LogSource",
            make_column("id", &LogSource::id, primary_key().autoincrement()),
            make_column("name", &LogSource::name, not_null()),
            make_column("host", &LogSource::host, not_null()),
            make_column("status", &LogSource::status, not_null())
        ),
        // Залежна таблиця: записи журналу подій
        make_table(
            "LogEntry",
            make_column("id", &LogEntry::id, primary_key().autoincrement()),
            make_column("sourceId", &LogEntry::sourceId),
            make_column("timestamp", &LogEntry::timestamp, not_null()),
            make_column("level", &LogEntry::level, not_null()),
            make_column("message", &LogEntry::message, not_null()),
            make_column("durationMs", &LogEntry::durationMs),
            foreign_key(&LogEntry::sourceId).references(&LogSource::id)
        )
    );
}