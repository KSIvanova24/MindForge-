#include "../include/data.h"
#include <cstdio>

static Task s_tasks[MAX_TASKS];
static int  s_count  = 0;
static int  s_nextId = 1;

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
        s_nextId++;
    }
    else if (newTask.id >= s_nextId)
    {
        s_nextId = newTask.id + 1;
    }

    s_tasks[s_count] = newTask;
    s_count++;

    return true;
}

bool removeTask(int index)
{
    if (index < 0 || index >= s_count)
        return false;

    for (int i = index; i < s_count - 1; i++)
        s_tasks[i] = s_tasks[i + 1];

    s_count--;
    return true;
}

bool updateTask(int index, Task updatedTask)
{
    if (index < 0 || index >= s_count)
        return false;

    updatedTask.id  = s_tasks[index].id;
    s_tasks[index]  = updatedTask;
    return true;
}

int loadTasksFromFile(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file)
        return -1;

    initTaskStore();

    Task t;
    int  completedAsInt = 0;

    while (s_count < MAX_TASKS)
    {
        int fieldsRead = fscanf(file, "%d,%127[^,],%d,%15[^,],%d,%d\n",
                                &t.id, t.title, &t.priority,
                                t.deadline, &t.duration, &completedAsInt);
        if (fieldsRead != 6)
            break;

        t.completed     = (completedAsInt == 1);
        s_tasks[s_count] = t;
        s_count++;

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

    for (int i = 0; i < s_count; i++)
    {
        fprintf(file, "%d,%s,%d,%s,%d,%d\n",
                s_tasks[i].id,
                s_tasks[i].title,
                s_tasks[i].priority,
                s_tasks[i].deadline,
                s_tasks[i].duration,
                (int)s_tasks[i].completed);
    }

    fclose(file);
    return true;
}
