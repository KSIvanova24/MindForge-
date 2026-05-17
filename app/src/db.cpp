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
        i = i + 1;
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
        "    id              INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    username        TEXT    NOT NULL,"
        "    title           TEXT    NOT NULL,"
        "    priority        INTEGER NOT NULL DEFAULT 1,"
        "    deadline        TEXT    NOT NULL DEFAULT '',"
        "    duration        INTEGER NOT NULL DEFAULT 0,"
        "    completed       INTEGER NOT NULL DEFAULT 0,"
        "    description     TEXT    NOT NULL DEFAULT '',"
        "    category_name   TEXT    NOT NULL DEFAULT '',"
        "    repeat_type     INTEGER NOT NULL DEFAULT 0,"
        "    repeat_interval INTEGER NOT NULL DEFAULT 0"
        ");";
    sqlite3_exec(db, createTasksTable, nullptr, nullptr, nullptr);

    sqlite3_exec(db, "ALTER TABLE tasks ADD COLUMN description TEXT NOT NULL DEFAULT '';",     nullptr, nullptr, nullptr);
    sqlite3_exec(db, "ALTER TABLE tasks ADD COLUMN category_name TEXT NOT NULL DEFAULT '';",   nullptr, nullptr, nullptr);
    sqlite3_exec(db, "ALTER TABLE tasks ADD COLUMN repeat_type INTEGER NOT NULL DEFAULT 0;",   nullptr, nullptr, nullptr);
    sqlite3_exec(db, "ALTER TABLE tasks ADD COLUMN repeat_interval INTEGER NOT NULL DEFAULT 0;", nullptr, nullptr, nullptr);

    const char* createCategoriesTable =
        "CREATE TABLE IF NOT EXISTS categories ("
        "    id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    username TEXT    NOT NULL,"
        "    name     TEXT    NOT NULL"
        ");";
    sqlite3_exec(db, createCategoriesTable, nullptr, nullptr, nullptr);

    sqlite3_exec(db, "ALTER TABLE categories ADD COLUMN color TEXT NOT NULL DEFAULT '#E46C0F';", nullptr, nullptr, nullptr);

    const char* createSubtasksTable =
        "CREATE TABLE IF NOT EXISTS subtasks ("
        "    id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    task_id   INTEGER NOT NULL,"
        "    title     TEXT    NOT NULL,"
        "    completed INTEGER NOT NULL DEFAULT 0"
        ");";
    sqlite3_exec(db, createSubtasksTable, nullptr, nullptr, nullptr);

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
        "SELECT id, title, priority, deadline, duration, completed, "
        "description, category_name, repeat_type, repeat_interval "
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

        task.id        = sqlite3_column_int(statement, 0);
        copyTextColumn(task.title,        sizeof(task.title),
                       sqlite3_column_text(statement, 1));
        task.priority  = sqlite3_column_int(statement, 2);
        copyTextColumn(task.deadline,     sizeof(task.deadline),
                       sqlite3_column_text(statement, 3));
        task.duration  = sqlite3_column_int(statement, 4);
        task.completed = (sqlite3_column_int(statement, 5) != 0);
        copyTextColumn(task.description,  sizeof(task.description),
                       sqlite3_column_text(statement, 6));
        copyTextColumn(task.categoryName, sizeof(task.categoryName),
                       sqlite3_column_text(statement, 7));
        task.repeatType     = sqlite3_column_int(statement, 8);
        task.repeatInterval = sqlite3_column_int(statement, 9);

        addTask(task);
        rowsLoaded = rowsLoaded + 1;
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
        "INSERT INTO tasks "
        "(username, title, priority, deadline, duration, completed, "
        " description, category_name, repeat_type, repeat_interval) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_text(statement, 1, username,          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, task.title,        -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 3, task.priority);
    sqlite3_bind_text(statement, 4, task.deadline,     -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 5, task.duration);
    sqlite3_bind_int (statement, 6, task.completed ? 1 : 0);
    sqlite3_bind_text(statement, 7, task.description,  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 8, task.categoryName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 9, task.repeatType);
    sqlite3_bind_int (statement, 10, task.repeatInterval);

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

    const char* sql = "UPDATE tasks SET completed = ? WHERE id = ?;";

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

    const char* sql = "UPDATE tasks SET deadline = ? WHERE id = ?;";

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

bool updateTaskDescription(int taskId, const char* description)
{
    if (!description)
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "UPDATE tasks SET description = ? WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, description, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 2, taskId);

    sqlite3_step(statement);
    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}

bool updateTaskRepeat(int taskId, int repeatType, int repeatInterval)
{
    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "UPDATE tasks SET repeat_type = ?, repeat_interval = ? WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, repeatType);
    sqlite3_bind_int(statement, 2, repeatInterval);
    sqlite3_bind_int(statement, 3, taskId);

    sqlite3_step(statement);
    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}

bool updateTaskCategory(int taskId, const char* categoryName)
{
    if (!categoryName)
        return false;

    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "UPDATE tasks SET category_name = ? WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(statement, 1, categoryName, -1, SQLITE_TRANSIENT);
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

int loadCategoriesForUser(const char* username)
{
    initCategoryStore();

    if (!username || username[0] == '\0')
        return 0;

    sqlite3* db = openDatabase();
    if (!db)
        return 0;

    const char* sql =
        "SELECT id, name, color FROM categories WHERE username = ? ORDER BY name ASC;";

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
        Category cat = {};

        cat.id = sqlite3_column_int(statement, 0);
        copyTextColumn(cat.name, sizeof(cat.name),
                       sqlite3_column_text(statement, 1));
        copyTextColumn(cat.color, sizeof(cat.color),
                       sqlite3_column_text(statement, 2));

        addCategoryToStore(cat);
        rowsLoaded = rowsLoaded + 1;
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsLoaded;
}

int saveNewCategory(const char* username, const char* name, const char* color)
{
    if (!username || username[0] == '\0')
        return 0;
    if (!name || name[0] == '\0')
        return 0;

    sqlite3* db = openDatabase();
    if (!db)
        return 0;

    const char* sql = "INSERT INTO categories (username, name, color) VALUES (?, ?, ?);";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_text(statement, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, name,     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, color,    -1, SQLITE_TRANSIENT);

    sqlite3_step(statement);

    int newId = (int)sqlite3_last_insert_rowid(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return newId;
}

bool updateCategoryColor(int categoryId, const char* color)
{
    sqlite3* database = openDatabase();

    bool databaseOpened = (database != nullptr);
    if (databaseOpened == false)
    {
        return false;
    }

    const char* sql = "UPDATE categories SET color = ? WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database, sql, -1, &statement, nullptr);

    bool prepareSucceeded = (prepareResult == SQLITE_OK);
    if (prepareSucceeded == false)
    {
        sqlite3_close(database);
        return false;
    }

    sqlite3_bind_text(statement, 1, color,      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (statement, 2, categoryId);

    sqlite3_step(statement);

    int numberOfRowsChanged = sqlite3_changes(database);

    sqlite3_finalize(statement);
    sqlite3_close(database);

    bool updateWorked = (numberOfRowsChanged > 0);
    return updateWorked;
}

bool deleteCategoryFromDb(int categoryId)
{
    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "DELETE FROM categories WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, categoryId);

    sqlite3_step(statement);
    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}

int loadSubtasksForTask(int taskId, int* idsOut, char titlesOut[][64], bool* doneOut, int maxCount)
{
    sqlite3* db = openDatabase();
    if (!db)
        return 0;

    const char* sql =
        "SELECT id, title, completed FROM subtasks WHERE task_id = ? ORDER BY id ASC;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_int(statement, 1, taskId);

    int count = 0;
    while (sqlite3_step(statement) == SQLITE_ROW && count < maxCount)
    {
        idsOut[count] = sqlite3_column_int(statement, 0);
        copyTextColumn(titlesOut[count], 64, sqlite3_column_text(statement, 1));
        doneOut[count] = (sqlite3_column_int(statement, 2) != 0);
        count = count + 1;
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return count;
}

int saveSubtask(int taskId, const char* title)
{
    if (!title || title[0] == '\0')
        return 0;

    sqlite3* db = openDatabase();
    if (!db)
        return 0;

    const char* sql = "INSERT INTO subtasks (task_id, title, completed) VALUES (?, ?, 0);";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_int (statement, 1, taskId);
    sqlite3_bind_text(statement, 2, title, -1, SQLITE_TRANSIENT);

    sqlite3_step(statement);

    int newId = (int)sqlite3_last_insert_rowid(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return newId;
}

bool updateSubtaskCompleted(int subtaskId, bool completed)
{
    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "UPDATE subtasks SET completed = ? WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, completed ? 1 : 0);
    sqlite3_bind_int(statement, 2, subtaskId);

    sqlite3_step(statement);

    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}

bool deleteSubtask(int subtaskId)
{
    sqlite3* db = openDatabase();
    if (!db)
        return false;

    const char* sql = "DELETE FROM subtasks WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, subtaskId);

    sqlite3_step(statement);
    int rowsTouched = sqlite3_changes(db);

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return rowsTouched > 0;
}