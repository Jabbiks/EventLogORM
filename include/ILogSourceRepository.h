#pragma once

#include <optional>
#include <vector>

#include "LogSource.h"

// Інтерфейс репозиторію для роботи з основною сутністю LogSource
class ILogSourceRepository
{
public:
    virtual ~ILogSourceRepository() = default;

    // Create
    virtual int add(const LogSource& source) = 0;

    // Read
    virtual std::optional<LogSource> findById(int id) const = 0;
    virtual std::vector<LogSource> findAll() const = 0;

    // Update
    virtual bool update(const LogSource& source) = 0;

    // Delete
    virtual bool remove(int id) = 0;
};