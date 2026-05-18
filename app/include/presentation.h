#pragma once

#include "raylib.h"

typedef enum {
    SCREEN_LOGIN = 0,
    SCREEN_DASHBOARD,
    SCREEN_ALL_TASKS,
    SCREEN_ARCHIVE,
    SCREEN_STATISTICS,
    SCREEN_PROFILE,
    SCREEN_SETTINGS,
    SCREEN_LOGOUT
} AppScreen;

void drawSidebar(AppScreen current, AppScreen* outHovered, int screenHeight);

bool settingsDeleteModalIsOpen();

void drawDashboardScreen(int contentX, int contentWidth, int screenHeight);
void drawAllTasksScreen(int contentX, int contentWidth, int screenHeight);
void drawStatisticsScreen(int contentX, int contentWidth, int screenHeight);
void drawProfileScreen(int contentX, int contentWidth, int screenHeight);
void drawSettingsScreen(int contentX, int contentWidth, int screenHeight, bool* outAccountDeleted);
