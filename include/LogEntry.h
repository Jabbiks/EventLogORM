#pragma once

#include <string>

// Залежна сутність (N): Запис журналу подій
struct LogEntry
{
    int id = 0;
    int sourceId = 0;            // Зовнішній ключ -> LogSource::id
    std::string timestamp;       // Часова мітка у форматі YYYY-MM-DD HH:MM:SS
    std::string level;           // Рівень важливості ("INFO", "WARN", "ERROR")
    std::string message;         // Текст повідомлення події
    double durationMs = 0.0;     // Тривалість обробки події в мс (поле для агрегації)
};