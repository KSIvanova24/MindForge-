#include "../include/db.h"

static const char* DATABASE_FILE = "mindforge.db";

sqlite3* openDatabase()
{
    sqlite3* db = nullptr;
    int result = sqlite3_open(DATABASE_FILE, &db);

    if (result != SQLITE_OK)
    {
        sqlite3_close(db);
        return nullptr;
    }

    const char* createAccountsTable =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "    username TEXT PRIMARY KEY,"
        "    password TEXT NOT NULL"
        ");";

    sqlite3_exec(db, createAccountsTable, nullptr, nullptr, nullptr);
    return db;
}
