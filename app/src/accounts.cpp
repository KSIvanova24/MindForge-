#include "../include/accounts.h"
#include "../include/db.h"
#include <cstring>

static char currentUser[64] = {};

bool registerAccount(const char* username, const char* password)
{
    if (!username || !password || username[0] == '\0')
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    sqlite3_stmt* statement = nullptr;
    int prepared = sqlite3_prepare_v2(db,
        "INSERT OR IGNORE INTO accounts (username, password) VALUES (?, ?);",
        -1, &statement, nullptr);

    if (prepared != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, password, -1, SQLITE_TRANSIENT);
    sqlite3_step(statement);

    int rowsInserted = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);

    return rowsInserted > 0;
}

bool accountExists(const char* username)
{
    if (!username || username[0] == '\0')
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    sqlite3_stmt* statement = nullptr;
    int prepared = sqlite3_prepare_v2(db,
        "SELECT 1 FROM accounts WHERE username = ?;",
        -1, &statement, nullptr);

    if (prepared != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);

    bool found = (sqlite3_step(statement) == SQLITE_ROW);

    sqlite3_finalize(statement);
    sqlite3_close(db);

    return found;
}

bool validateCredentials(const char* username, const char* password)
{
    if (!username || !password || username[0] == '\0' || password[0] == '\0')
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    sqlite3_stmt* statement = nullptr;
    int prepared = sqlite3_prepare_v2(db,
        "SELECT 1 FROM accounts WHERE username = ? AND password = ?;",
        -1, &statement, nullptr);

    if (prepared != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, password, -1, SQLITE_TRANSIENT);

    bool valid = (sqlite3_step(statement) == SQLITE_ROW);

    sqlite3_finalize(statement);
    sqlite3_close(db);

    return valid;
}

bool deleteAccount(const char* username)
{
    if (!username || username[0] == '\0')
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    sqlite3_stmt* statement = nullptr;
    int prepared = sqlite3_prepare_v2(db,
        "DELETE FROM accounts WHERE username = ?;",
        -1, &statement, nullptr);

    if (prepared != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_step(statement);

    int rowsDeleted = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);

    return rowsDeleted > 0;
}

void setLoggedInUser(const char* username)
{
    if (!username)
    {
        currentUser[0] = '\0';
        return;
    }

    int i = 0;
    while (i < 63 && username[i] != '\0')
    {
        currentUser[i] = username[i];
        i++;
    }
    currentUser[i] = '\0';
}

void clearLoggedInUser()
{
    currentUser[0] = '\0';
}

const char* getLoggedInUser()
{
    return currentUser;
}
