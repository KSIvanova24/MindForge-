#pragma once
#include "sqlite3.h"
#include "data.h"

sqlite3* openDatabase();

int  loadTasksForUser(const char* username);
int  saveNewTask(const char* username, const Task& task);
bool updateTaskCompleted(int taskId, bool completed);
bool updateTaskDeadline(int taskId, const char* deadline);
bool updateTaskDescription(int taskId, const char* description);
bool updateTaskRepeat(int taskId, int repeatType, int repeatInterval);
bool updateTaskCategory(int taskId, const char* categoryName);
bool deleteTaskFromDb(int taskId);
bool deleteAllTasksForUser(const char* username);

int  loadSubtasksForTask(int taskId, int* idsOut, char titlesOut[][64], bool* doneOut, int maxCount);
int  saveSubtask(int taskId, const char* title);
bool updateSubtaskCompleted(int subtaskId, bool completed);
bool deleteSubtask(int subtaskId);

int  loadCategoriesForUser(const char* username);
int  saveNewCategory(const char* username, const char* name, const char* color);
bool updateCategoryColor(int categoryId, const char* color);
bool deleteCategoryFromDb(int categoryId);