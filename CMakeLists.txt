cmake_minimum_required(VERSION 3.21)
project(EventLogORM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(EventLogORM
    src/main.cpp
    src/SQLiteLogSourceRepository.cpp
    src/LogEntryStorage.cpp
    src/RawSqliteDemo.cpp
    src/OrmDemos.cpp
)

target_include_directories(EventLogORM
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(EventLogORM
    PRIVATE
        sqlite_orm::sqlite_orm
        unofficial::sqlite3::sqlite3
)