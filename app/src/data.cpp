#include "../include/data.h"

static Task s_tasks[MAX_TASKS];
static int  s_count = 0;
static int  s_nextId = 1;

void initTaskStore()
{
    s_count = 0;
    s_nextId = 1;
    for (int i = 0; i < MAX_TASKS; i++)
    {
        s_tasks[i] = Task{};
    }
}

int getTaskCount()
{
    return s_count;
}

Task* getTaskStore()
{
    return s_tasks;
}

bool addTask(Task t)
{
    if (s_count >= MAX_TASKS)
        return false;

    t.id = s_nextId;
    s_nextId++;
    s_tasks[s_count] = t;
    s_count++;
    return true;
}

bool removeTask(int index)
{
    if (index < 0 || index >= s_count)
        return false;

    for (int i = index; i < s_count - 1; i++)
    {
        s_tasks[i] = s_tasks[i + 1];
    }

    s_count--;
    return true;
}

bool updateTask(int index, Task t)
{
    if (index < 0 || index >= s_count)
        return false;

    int savedId = s_tasks[index].id;
    s_tasks[index] = t;
    s_tasks[index].id = savedId;
    return true;
}

int loadTasksFromFile(const char* filepath)
{
    FILE* f = fopen(filepath, "r");
    if (f == NULL)
        return -1;

    initTaskStore();
    int loaded = 0;
    Task t;

    while (fscanf(f, "%d,%127[^,],%d,%15[^,],%d,%d\n",
        &t.id, t.title, &t.priority,
        t.deadline, &t.duration,
        (int*)&t.completed) == 6)
    {
        if (s_count < MAX_TASKS)
        {
            s_tasks[s_count] = t;
            s_count++;

            if (t.id >= s_nextId)
                s_nextId = t.id + 1;

            loaded++;
        }
    }

    fclose(f);
    return loaded;
}

bool saveTasksToFile(const char* filepath)
{
    FILE* f = fopen(filepath, "w");
    if (f == NULL)
        return false;

    for (int i = 0; i < s_count; i++)
    {
        Task t = s_tasks[i];
        fprintf(f, "%d,%s,%d,%s,%d,%d\n",
            t.id, t.title, t.priority,
            t.deadline, t.duration,
            (int)t.completed);
    }

    fclose(f);
    return true;
}