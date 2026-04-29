#pragma once

#include <iostream>
#include <cstring>
#include <cmath>
#include "raylib.h"

using namespace std;

typedef enum {
    SCREEN_LOGIN = 0,
    SCREEN_DASHBOARD,
    SCREEN_ALL_TASKS,
    SCREEN_STATISTICS,
    SCREEN_PROFILE,
    SCREEN_SETTINGS
} AppScreen;