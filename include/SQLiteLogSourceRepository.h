#pragma once

#include <string>

#include "ILogSourceRepository.h"
#include "Database.h"

class SQLiteLogSourceRepository : public ILogSourceRepository
{
private:
    using Storage = decltype(createStorage(""));
    mutable Storage storage_;

public:
    explicit SQLiteLogSourceRepository(const std::string& databaseFile);

    int add(const LogSource& source) override;
    std::optional<LogSource> findById(int id) const override;
    std::vector<LogSource> findAll() const override;
    bool update(const LogSource& source) override;
    bool remove(int id) override;
};