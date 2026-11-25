#include <exception>
#include <filesystem>
#include <iostream>

#include "RawSqliteDemo.h"
#include "OrmDemos.h"

int main()
{
    try
    {
        // Очищення демонстраційної БД перед запуском для відтворюваності результатів
        std::filesystem::remove("event_logs_basic.db");

        runRawSqliteApiDemo();
        runBasicOrmDemo();
        runRelationshipDemo();
        runRepositoryDemo();
        runQueriesAndAggregatesDemo();
        runTransactionDemos();

        std::cout << "\nAll demonstrations completed successfully.\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "\nFatal application error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}