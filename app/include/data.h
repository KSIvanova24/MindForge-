#pragma once

#define MAX_TASKS 256

struct Task {
    int  id;
    char title[128];
    int  priority;
    char deadline[16];
    int  duration;
    bool completed;
};

void  initTaskStore();
int   getTaskCount();
Task* getTaskStore();
bool  addTask(Task t);
bool  removeTask(int index);
bool  updateTask(int index, Task t);
int   loadTasksFromFile(const char* filepath);
bool  saveTasksToFile(const char* filepath);
