#include "../include/data.h"
#include <cstdio>

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

int loadTasksFromFile(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file)
        return -1;

    initTaskStore();

    Task t = {};
    int  completedAsInt = 0;

    while (s_count < MAX_TASKS)
    {
        int fieldsRead = fscanf(file, "%d,%127[^,],%d,%15[^,],%d,%d\n",
                                &t.id, t.title, &t.priority,
                                t.deadline, &t.duration, &completedAsInt);
        if (fieldsRead != 6)
            break;

        t.completed      = (completedAsInt == 1);
        t.description[0] = '\0';
        t.categoryName[0]= '\0';
        t.repeatType     = 0;
        t.repeatInterval = 0;

        s_tasks[s_count] = t;
        s_count = s_count + 1;

        if (t.id >= s_nextId)
            s_nextId = t.id + 1;
    }

    fclose(file);
    return s_count;
}

bool saveTasksToFile(const char* filepath)
{
    FILE* file = fopen(filepath, "w");
    if (!file)
        return false;

    int i = 0;
    while (i < s_count)
    {
        fprintf(file, "%d,%s,%d,%s,%d,%d\n",
                s_tasks[i].id,
                s_tasks[i].title,
                s_tasks[i].priority,
                s_tasks[i].deadline,
                s_tasks[i].duration,
                (int)s_tasks[i].completed);
        i = i + 1;
    }

    fclose(file);
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
