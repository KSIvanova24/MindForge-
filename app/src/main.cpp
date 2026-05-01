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

    int appState = 0;

    AppScreen currentScreen = SCREEN_DASHBOARD;

    while (!WindowShouldClose())
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        BeginDrawing();
        ClearBackground({ 12, 10, 8, 255 });

        if (appState == 0)
        {
            int loginResult = DrawLoginScreen(sw, sh);
            if (loginResult >= 1)
                appState = loginResult;
        }
        else
        {
            int contentX = SIDEBAR_W;
            int contentW = sw - SIDEBAR_W;

            switch (currentScreen)
            {
            case SCREEN_DASHBOARD:  DrawDashboardScreen(contentX, contentW, sh); break;
            case SCREEN_ALL_TASKS:  DrawAllTasksScreen(contentX, contentW, sh); break;
            case SCREEN_STATISTICS: DrawStatisticsScreen(contentX, contentW, sh); break;
            case SCREEN_PROFILE:    DrawProfileScreen(contentX, contentW, sh); break;
            case SCREEN_SETTINGS:   DrawSettingsScreen(contentX, contentW, sh); break;
            default: break;
            }

            AppScreen hoveredItem = (AppScreen)-1;
            DrawSidebar(currentScreen, &hoveredItem, sh);

            if (appState == 2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (int)hoveredItem >= 0)
            {
                if (hoveredItem == SCREEN_LOGOUT)
                {
                    ResetLoginToLanding();
                    appState = 0;
                    currentScreen = SCREEN_DASHBOARD;
                }
                else
                {
                    currentScreen = hoveredItem;
                }
            }

            if (appState == 1)
            {
                int loginResult = DrawLoginScreen(sw, sh);
                if (loginResult == 2)
                    appState = 2;
            }
            else
            {
                if ((int)hoveredItem >= 0)
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                else
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}