#pragma once

#include "data.h"

void sortTasksByPriority(Task* tasks, int count, bool ascending);
void sortTasksByDeadline(Task* tasks, int count);
int  linearSearchByTitle(Task* tasks, int count, const char* query, int* results, int maxRes);
int  binarySearchById(Task* tasks, int count, int id);
int  calcTotalDurationRecursive(Task* tasks, int index, int count);
int  countByPriorityRecursive(Task* tasks, int index, int count, int priority);
bool createTask(const char* title, int priority, const char* deadline, int duration);
bool deleteTask(int index);
bool toggleTaskComplete(int index);
