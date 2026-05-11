#pragma once

#define MAX_TASKS      256
#define MAX_CATEGORIES  32

struct Task
{
    int  id;
    char title[128];
    int  priority;
    char deadline[16];
    int  duration;
    bool completed;
    char description[512];
    char categoryName[64];
    int  repeatType;
    int  repeatInterval;
};

struct Category
{
    int  id;
    char name[64];
};

void      initTaskStore();
int       getTaskCount();
Task*     getTaskStore();
bool      addTask(Task t);
bool      removeTask(int index);
bool      updateTask(int index, Task t);
int       loadTasksFromFile(const char* filepath);
bool      saveTasksToFile(const char* filepath);

void      initCategoryStore();
int       getCategoryCount();
Category* getCategoryStore();
bool      addCategoryToStore(Category c);
bool      removeCategoryAtIndex(int index);

void        setCurrentCategoryFilter(const char* name);
const char* getCurrentCategoryFilter();
