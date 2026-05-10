#pragma once
#include "sqlite3.h"
#include "data.h"

sqlite3* openDatabase();

int loadTasksForUser(const char* username);

int saveNewTask(const char* username, const Task& task);

bool updateTaskCompleted(int taskId, bool completed);

bool updateTaskDeadline(int taskId, const char* deadline);

bool deleteTaskFromDb(int taskId);

bool deleteAllTasksForUser(const char* username);
