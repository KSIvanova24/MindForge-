#include "../include/db.h"
#include "../include/data.h"

static const char* DATABASE_FILE = "mindforge.db";

static void copyTextColumn(char* dst, int dstSize, const unsigned char* src)
{
    if (!src)
    {
        dst[0] = '\0';
        return;
    }
    int i = 0;
    while (i < dstSize - 1 && src[i] != '\0')
    {
        dst[i] = (char)src[i];
        i++;
    }
    dst[i] = '\0';
}

sqlite3* openDatabase()
{
    sqlite3* db = nullptr;

    int openResult = sqlite3_open(DATABASE_FILE, &db);
    if (openResult != SQLITE_OK)
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

    const char* createTasksTable =
        "CREATE TABLE IF NOT EXISTS tasks ("
        "    id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    username  TEXT    NOT NULL,"
        "    title     TEXT    NOT NULL,"
        "    priority  INTEGER NOT NULL DEFAULT 1,"
        "    deadline  TEXT    NOT NULL DEFAULT '',"
        "    duration  INTEGER NOT NULL DEFAULT 0,"
        "    completed INTEGER NOT NULL DEFAULT 0"
        ");";
    sqlite3_exec(db, createTasksTable, nullptr, nullptr, nullptr);

    return db;
}

int loadTasksForUser(const char* username)
{

    initTaskStore();

    if (!username || username[0] == '\0')
        return 0;

    sqlite3* db = openDatabase();
    if (!db)
        return 0;

    const char* sql =
        "SELECT id, title, priority, deadline, duration, completed "
        "FROM tasks "
        "WHERE username = ? "
        "ORDER BY deadline ASC;";

    sqlite3_stmt* statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(db, sql, -1, &statement, nullptr);
    if (prepareResult != SQLITE_OK)
    {
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);

    int rowsLoaded = 0;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        Task task = {};

        task.id       = sqlite3_column_int (statement, 0);
        copyTextColumn(task.title,    sizeof(task.title),
                       sqlite3_column_text(statement, 1));
        task.priority = sqlite3_column_int (statement, 2);
        copyTextColumn(task.deadline, sizeof(task.deadline),
                       sqlite3_column_text(statement, 3));
        task.duration = sqlite3_column_int (statement, 4);
        task.completed = (sqlite3_column_int(statement, 5) != 0);

        addTask(task);
        rowsLoaded++;
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsLoaded;
}

int saveNewTask(const char* username, const Task& task)
{
    if (!username || username[0] == '\0')
        return 0;

    sqlite3* db = openDatabase();
    if (!db)
        return 0;

    const char* sql =
        "INSERT INTO tasks (username, title, priority, deadline, duration, completed) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_text(statement, 1, username,      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, task.title,    -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 3, task.priority);
    sqlite3_bind_text(statement, 4, task.deadline, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 5, task.duration);
    sqlite3_bind_int (statement, 6, task.completed ? 1 : 0);

    sqlite3_step(statement);

    int newId = (int)sqlite3_last_insert_rowid(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return newId;
}

bool updateTaskCompleted(int taskId, bool completed)
{
    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql =
        "UPDATE tasks SET completed = ? WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, completed ? 1 : 0);
    sqlite3_bind_int(statement, 2, taskId);

    sqlite3_step(statement);

    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}

bool updateTaskDeadline(int taskId, const char* deadline)
{
    if (!deadline)
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql =
        "UPDATE tasks SET deadline = ? WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, deadline, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 2, taskId);

    sqlite3_step(statement);
    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}

bool deleteTaskFromDb(int taskId)
{
    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "DELETE FROM tasks WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, taskId);

    sqlite3_step(statement);
    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}

bool deleteAllTasksForUser(const char* username)
{
    if (!username || username[0] == '\0')
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "DELETE FROM tasks WHERE username = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);

    sqlite3_step(statement);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return true;
}
