#pragma once

#include <iostream>
#include "raylib.h"

using namespace std;

typedef enum {
    SCREEN_DASHBOARD = 0,
    SCREEN_ALL_TASKS,
    SCREEN_STATISTICS,
    SCREEN_PROFILE,
    SCREEN_SETTINGS
} AppScreen;