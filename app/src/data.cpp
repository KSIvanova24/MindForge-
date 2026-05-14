#include "../include/data.h"

static Task s_tasks[MAX_TASKS];
static int  s_count  = 0;
static int  s_nextId = 1;

static Category s_categories[MAX_CATEGORIES];
static int      s_categoryCount = 0;

static char s_currentCategoryFilter[64] = {};

void initTaskStore()
{
    s_count  = 0;
    s_nextId = 1;
}

int getTaskCount()
{
    return s_count;
}

Task* getTaskStore()
{
    return s_tasks;
}

bool addTask(Task newTask)
{
    if (s_count >= MAX_TASKS)
        return false;

    if (newTask.id == 0)
    {
        newTask.id = s_nextId;
        s_nextId = s_nextId + 1;
    }
    else if (newTask.id >= s_nextId)
    {
        s_nextId = newTask.id + 1;
    }

    s_tasks[s_count] = newTask;
    s_count = s_count + 1;

    return true;
}

bool removeTask(int index)
{
    if (index < 0 || index >= s_count)
        return false;

    int i = index;
    while (i < s_count - 1)
    {
        s_tasks[i] = s_tasks[i + 1];
        i = i + 1;
    }

    s_count = s_count - 1;
    return true;
}

bool updateTask(int index, Task updatedTask)
{
    if (index < 0 || index >= s_count)
        return false;

    updatedTask.id = s_tasks[index].id;
    s_tasks[index] = updatedTask;
    return true;
}

void initCategoryStore()
{
    s_categoryCount = 0;
}

int getCategoryCount()
{
    return s_categoryCount;
}

Category* getCategoryStore()
{
    return s_categories;
}

bool addCategoryToStore(Category c)
{
    if (s_categoryCount >= MAX_CATEGORIES)
        return false;

    s_categories[s_categoryCount] = c;
    s_categoryCount = s_categoryCount + 1;
    return true;
}

bool removeCategoryAtIndex(int index)
{
    if (index < 0 || index >= s_categoryCount)
        return false;

    int i = index;
    while (i < s_categoryCount - 1)
    {
        s_categories[i] = s_categories[i + 1];
        i = i + 1;
    }

    s_categoryCount = s_categoryCount - 1;
    return true;
}

void setCurrentCategoryFilter(const char* name)
{
    if (name == nullptr)
    {
        s_currentCategoryFilter[0] = '\0';
        return;
    }

    int i = 0;
    while (i < 63 && name[i] != '\0')
    {
        s_currentCategoryFilter[i] = name[i];
        i = i + 1;
    }
    s_currentCategoryFilter[i] = '\0';
}

const char* getCurrentCategoryFilter()
{
    return s_currentCategoryFilter;
}
