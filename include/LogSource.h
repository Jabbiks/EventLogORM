#pragma once

#include <string>

// Основна сутність (1): Джерело подій
struct LogSource
{
    int id = 0;
    std::string name;        // Назва сервісу або модуля (напр. "AuthService")
    std::string host;        // Вузол / сервер (напр. "srv-auth-01")
    std::string status;      // Стан джерела ("active", "paused", "archived")
};