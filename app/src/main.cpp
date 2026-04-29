#include "../include/pch.h"
#include "../include/presentation.h"
#include "../include/screens/login.h"

extern int SIDEBAR_W;
extern int BTN_H;
extern int BTN_GAP;
extern int BTN_X;
extern int NAV_START;

int WINDOW_W = 1600;
int WINDOW_H = 950;

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_W, WINDOW_H, "MindForge");
    SetTargetFPS(60);

    AppScreen currentScreen = SCREEN_LOGIN;

    while (!WindowShouldClose())
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        BeginDrawing();
        ClearBackground({ 12, 10, 8, 255 });

        if (currentScreen == SCREEN_LOGIN)
        {
            bool loggedIn = DrawLoginScreen(sw, sh);
            if (loggedIn)
                currentScreen = SCREEN_DASHBOARD;
        }
        else
        {
            int contentX = SIDEBAR_W;
            int contentW = sw - SIDEBAR_W;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                Vector2 mouse = GetMousePosition();

                AppScreen mainScreens[] = { SCREEN_DASHBOARD, SCREEN_ALL_TASKS, SCREEN_STATISTICS };

                for (int i = 0; i < 3; i++)
                {
                    int y = NAV_START + i * (BTN_H + BTN_GAP);
                    Rectangle r = { (float)BTN_X, (float)y, (float)(SIDEBAR_W - 24), (float)BTN_H };

                    if (CheckCollisionPointRec(mouse, r))
                        currentScreen = mainScreens[i];
                }

                AppScreen bottomScreens[] = { SCREEN_PROFILE, SCREEN_SETTINGS };
                int bottomStart = sh - 160;

                for (int i = 0; i < 2; i++)
                {
                    int y = bottomStart + i * (BTN_H + BTN_GAP);
                    Rectangle r = { (float)BTN_X, (float)y, (float)(SIDEBAR_W - 24), (float)BTN_H };

                    if (CheckCollisionPointRec(mouse, r))
                        currentScreen = bottomScreens[i];
                }
            }

            if (currentScreen == SCREEN_DASHBOARD)
                DrawDashboardScreen(contentX, contentW, sh);
            else if (currentScreen == SCREEN_ALL_TASKS)
                DrawAllTasksScreen(contentX, contentW, sh);
            else if (currentScreen == SCREEN_STATISTICS)
                DrawStatisticsScreen(contentX, contentW, sh);
            else if (currentScreen == SCREEN_PROFILE)
                DrawProfileScreen(contentX, contentW, sh);
            else if (currentScreen == SCREEN_SETTINGS)
                DrawSettingsScreen(contentX, contentW, sh);

            AppScreen hoveredItem;
            DrawSidebar(currentScreen, &hoveredItem, sh);

            if ((int)hoveredItem >= 0)
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            else
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}