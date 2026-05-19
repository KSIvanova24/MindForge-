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

bool changeUsername(const char* oldUsername, const char* newUsername)
{
    if (oldUsername == nullptr || newUsername == nullptr)
    {
        return false;
    }

    if (oldUsername[0] == '\0' || newUsername[0] == '\0')
    {
        return false;
    }

    bool newNameIsAlreadyTaken = accountExists(newUsername);
    if (newNameIsAlreadyTaken == true)
    {
        return false;
    }

    sqlite3* database = openDatabase();
    if (database == nullptr)
    {
        return false;
    }

    sqlite3_stmt* updateAccountCommand = nullptr;
    int prepareResult = sqlite3_prepare_v2(database,
        "UPDATE accounts SET username = ? WHERE username = ?;",
        -1, &updateAccountCommand, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        sqlite3_close(database);
        return false;
    }

    sqlite3_bind_text(updateAccountCommand, 1, newUsername, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(updateAccountCommand, 2, oldUsername, -1, SQLITE_TRANSIENT);

    sqlite3_step(updateAccountCommand);

    int rowsChanged = sqlite3_changes(database);

    sqlite3_finalize(updateAccountCommand);

    if (rowsChanged == 0)
    {
        sqlite3_close(database);
        return false;
    }

    // tasks are stored with the username, so rename them too
    sqlite3_stmt* updateTasksCommand = nullptr;
    sqlite3_prepare_v2(database,
        "UPDATE tasks SET username = ? WHERE username = ?;",
        -1, &updateTasksCommand, nullptr);

    sqlite3_bind_text(updateTasksCommand, 1, newUsername, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(updateTasksCommand, 2, oldUsername, -1, SQLITE_TRANSIENT);

    sqlite3_step(updateTasksCommand);
    sqlite3_finalize(updateTasksCommand);

    sqlite3_close(database);
    return true;
}

bool changePassword(const char* username, const char* currentPassword, const char* newPassword)
{
    if (username == nullptr || currentPassword == nullptr || newPassword == nullptr)
    {
        return false;
    }

    if (username[0] == '\0' || currentPassword[0] == '\0' || newPassword[0] == '\0')
    {
        return false;
    }

    bool currentPasswordIsCorrect = validateCredentials(username, currentPassword);
    if (currentPasswordIsCorrect == false)
    {
        return false;
    }

    sqlite3* database = openDatabase();
    if (database == nullptr)
    {
        return false;
    }

    sqlite3_stmt* updateCommand = nullptr;
    int prepareResult = sqlite3_prepare_v2(database,
        "UPDATE accounts SET password = ? WHERE username = ?;",
        -1, &updateCommand, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        sqlite3_close(database);
        return false;
    }

    sqlite3_bind_text(updateCommand, 1, newPassword, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(updateCommand, 2, username,    -1, SQLITE_TRANSIENT);

    sqlite3_step(updateCommand);

    int rowsChanged = sqlite3_changes(database);

    sqlite3_finalize(updateCommand);
    sqlite3_close(database);

    if (rowsChanged > 0)
    {
        return true;
    }
    return false;
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
