#include "../include/logic.h"
#include <cstring>
#include <cctype>

void sortTasksByPriority(Task* tasks, int count, bool ascending)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            bool shouldSwap = false;

            if (ascending)
                shouldSwap = tasks[j].priority > tasks[j + 1].priority;
            else
                shouldSwap = tasks[j].priority < tasks[j + 1].priority;

            if (shouldSwap)
            {
                Task temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }
}

void sortTasksByDeadline(Task* tasks, int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (strcmp(tasks[j].deadline, tasks[j + 1].deadline) > 0)
            {
                Task temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }
}

int linearSearchByTitle(Task* tasks, int count, const char* query, int* results, int maxRes)
{
    int found = 0;

    for (int i = 0; i < count; i++)
    {
        bool match = true;
        int  qLen = strlen(query);

        if (qLen == 0)
        {
            results[found] = i;
            found++;
            continue;
        }

        bool foundInTitle = false;
        int  tLen = strlen(tasks[i].title);

        for (int s = 0; s <= tLen - qLen; s++)
        {
            bool same = true;
            for (int k = 0; k < qLen; k++)
            {
                if (tolower(tasks[i].title[s + k]) != tolower(query[k]))
                {
                    same = false;
                    break;
                }
            }
            if (same)
            {
                foundInTitle = true;
                break;
            }
        }

        if (foundInTitle && found < maxRes)
        {
            results[found] = i;
            found++;
        }
    }

    return found;
}

int binarySearchById(Task* tasks, int count, int id)
{
    int lo = 0;
    int hi = count - 1;

    while (lo <= hi)
    {
        int mid = (lo + hi) / 2;

        if (tasks[mid].id == id)
            return mid;
        else if (tasks[mid].id < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }

    return -1;
}

int calcTotalDurationRecursive(Task* tasks, int index, int count)
{
    if (index >= count)
        return 0;

    int current = 0;
    if (tasks[index].completed == false)
        current = tasks[index].duration;

    return current + calcTotalDurationRecursive(tasks, index + 1, count);
}

int countByPriorityRecursive(Task* tasks, int index, int count, int priority)
{
    if (index >= count)
        return 0;

    int match = 0;
    if (tasks[index].priority == priority)
        match = 1;

    return match + countByPriorityRecursive(tasks, index + 1, count, priority);
}

bool createTask(const char* title, int priority, const char* deadline, int duration)
{
    if (title[0] == '\0')   return false;
    if (priority < 1 || priority > 3) return false;
    if (duration < 0)       return false;

    Task t = {};
    strncpy(t.title, title, 127);
    strncpy(t.deadline, deadline, 15);
    t.priority = priority;
    t.duration = duration;
    t.completed = false;

    return addTask(t);
}

bool deleteTask(int index)
{
    return removeTask(index);
}

bool toggleTaskComplete(int index)
{
    Task* store = getTaskStore();
    int   count = getTaskCount();

    if (index < 0 || index >= count)
        return false;

    if (store[index].completed == true)
        store[index].completed = false;
    else
        store[index].completed = true;

    return true;
}