#pragma once

#include "raylib.h"

typedef enum {
    SCREEN_LOGIN = 0,
    SCREEN_DASHBOARD,
    SCREEN_ALL_TASKS,
    SCREEN_STATISTICS,
    SCREEN_PROFILE,
    SCREEN_SETTINGS,
    SCREEN_LOGOUT
} AppScreen;

void DrawSidebar(AppScreen current, AppScreen* outHovered, int screenHeight);

bool SettingsDeleteModalIsOpen();

void DrawDashboardScreen(int contentX, int contentWidth, int screenHeight);
void DrawAllTasksScreen(int contentX, int contentWidth, int screenHeight);
void DrawStatisticsScreen(int contentX, int contentWidth, int screenHeight);
void DrawProfileScreen(int contentX, int contentWidth, int screenHeight);
void DrawSettingsScreen(int contentX, int contentWidth, int screenHeight, bool* outAccountDeleted);
