#pragma once

#include "pch.h"

void DrawSidebar(AppScreen current, AppScreen* outHovered, int screenHeight);

void DrawDashboardScreen(int contentX, int contentWidth, int screenHeight);
void DrawAllTasksScreen(int contentX, int contentWidth, int screenHeight);
void DrawStatisticsScreen(int contentX, int contentWidth, int screenHeight);
void DrawProfileScreen(int contentX, int contentWidth, int screenHeight);
void DrawSettingsScreen(int contentX, int contentWidth, int screenHeight);