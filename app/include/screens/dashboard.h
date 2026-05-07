#pragma once
#include "raylib.h"

void DrawDashboardScreen(int contentX, int contentWidth, int screenHeight)
{
    DrawRectangle(contentX, 0, contentWidth, screenHeight, RAYWHITE);
}