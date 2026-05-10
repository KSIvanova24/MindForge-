#include "../include/accounts.h"
<<<<<<< HEAD
#include "../include/db.h"
#include <cstring>

static char currentUser[64] = {};
=======
#include <fstream>
#include <string>

static const char* ACCOUNTS_FILE = "accounts.txt";
static char current_user[64] = {};
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e

bool registerAccount(const char* username, const char* password)
{
    if (!username || !password || username[0] == '\0')
<<<<<<< HEAD
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
=======
    {
        return false;
    }

    std::ofstream file(ACCOUNTS_FILE, std::ios::app);
    if (!file)
    {
        return false;
    }

    file << username << '\n';
    file << password << '\n';
    return true;
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
}

bool accountExists(const char* username)
{
    if (!username || username[0] == '\0')
<<<<<<< HEAD
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
=======
    {
        return false;
    }

    std::ifstream file(ACCOUNTS_FILE);
    std::string line_username;
    std::string line_password;

    while (std::getline(file, line_username))
    {
        std::getline(file, line_password);

        if (line_username == username)
        {
            return true;
        }
    }

    return false;
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
}

bool validateCredentials(const char* username, const char* password)
{
<<<<<<< HEAD
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
=======
    if (!username || !password || username[0] == '\0')
    {
        return false;
    }

    std::ifstream file(ACCOUNTS_FILE);
    std::string line_username;
    std::string line_password;

    while (std::getline(file, line_username))
    {
        std::getline(file, line_password);

        if (line_username == username && line_password == password)
        {
            return true;
        }
    }

    return false;
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
}

bool deleteAccount(const char* username)
{
    if (!username || username[0] == '\0')
<<<<<<< HEAD
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
=======
    {
        return false;
    }

    std::ifstream input(ACCOUNTS_FILE);
    if (!input)
    {
        return false;
    }

    std::string saved_usernames[256];
    std::string saved_passwords[256];
    int count = 0;

    while (std::getline(input, saved_usernames[count]))
    {
        std::getline(input, saved_passwords[count]);
        count++;
    }
    input.close();

    bool found = false;
    for (int i = 0; i < count; i++)
    {
        if (saved_usernames[i] == username)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        return false;
    }

    std::ofstream output(ACCOUNTS_FILE);
    if (!output)
    {
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        if (saved_usernames[i] != username)
        {
            output << saved_usernames[i] << '\n';
            output << saved_passwords[i] << '\n';
        }
    }

    return true;
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
}

void setLoggedInUser(const char* username)
{
    if (!username)
    {
<<<<<<< HEAD
        currentUser[0] = '\0';
=======
        current_user[0] = '\0';
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
        return;
    }

    int i = 0;
    while (i < 63 && username[i] != '\0')
    {
<<<<<<< HEAD
        currentUser[i] = username[i];
        i++;
    }
    currentUser[i] = '\0';
=======
        current_user[i] = username[i];
        i++;
    }
    current_user[i] = '\0';
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
}

void clearLoggedInUser()
{
<<<<<<< HEAD
    currentUser[0] = '\0';
=======
    current_user[0] = '\0';
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
}

const char* getLoggedInUser()
{
<<<<<<< HEAD
    return currentUser;
=======
    return current_user;
>>>>>>> 5fd3af9016514b25366d6d2b9968150c9c312e9e
}
