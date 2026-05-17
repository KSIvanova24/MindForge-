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
    InitWindow(WINDOW_W, WINDOW_H, "MindForge");
    SetTargetFPS(60);

    int appState = 0;
    bool showLogoutConfirm = false;
    bool accountDeletedLogout = false;

    AppScreen currentScreen = SCREEN_DASHBOARD;

    while (!WindowShouldClose())
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        BeginDrawing();
        ClearBackground({ 12, 10, 8, 255 });

        if (appState == 0)
        {
            int loginResult = drawLoginScreen(sw, sh);
            if (loginRequestedExit())
                break;
            if (loginResult >= 1)
                appState = loginResult;
        }
        else
        {
            int contentX = SIDEBAR_W;
            int contentW = sw - SIDEBAR_W;

            switch (currentScreen)
            {
            case SCREEN_DASHBOARD:  drawDashboardScreen(contentX, contentW, sh); break;
            case SCREEN_ALL_TASKS:  drawAllTasksScreen(contentX, contentW, sh); break;
            case SCREEN_STATISTICS: drawStatisticsScreen(contentX, contentW, sh); break;
            case SCREEN_PROFILE:    drawProfileScreen(contentX, contentW, sh); break;
            case SCREEN_SETTINGS:   drawSettingsScreen(contentX, contentW, sh, &accountDeletedLogout); break;
            default: break;
            }

            AppScreen hoveredItem = (AppScreen)-1;
            drawSidebar(currentScreen, &hoveredItem, sh);

            if (!showLogoutConfirm && !settingsDeleteModalIsOpen() && appState == 2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (int)hoveredItem >= 0)
            {
                if (hoveredItem == SCREEN_LOGOUT)
                {
                    showLogoutConfirm = true;
                }
                else
                {
                    currentScreen = hoveredItem;
                }
            }

            if (appState == 1)
            {
                int loginResult = drawLoginScreen(sw, sh);
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

            if (accountDeletedLogout)
            {
                accountDeletedLogout = false;
                resetLoginToLanding();
                appState = 0;
                currentScreen = SCREEN_DASHBOARD;
            }

            if (showLogoutConfirm)
            {
                DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 170 });

                int pw = 460;
                int ph = 240;
                int px = sw / 2 - pw / 2;
                int py = sh / 2 - ph / 2;

                DrawRectangleRounded({ (float)(px - 2), (float)(py - 2), (float)(pw + 4), (float)(ph + 4) }, 0.08f, 8, LG_ORANGE);
                DrawRectangleRounded({ (float)px, (float)py, (float)pw, (float)ph }, 0.08f, 8, LG_PANEL);

                const char* title = "Log out?";
                const char* msg = "Are you sure you want to log out?";
                DrawText(title, px + pw / 2 - MeasureText(title, 28) / 2, py + 40, 28, LG_WHITE);
                DrawText(msg, px + pw / 2 - MeasureText(msg, 18) / 2, py + 86, 18, LG_GREY);

                int bw = 170;
                int bh = 52;
                int gap = 16;
                int bx1 = px + pw / 2 - (bw * 2 + gap) / 2;
                int bx2 = bx1 + bw + gap;
                int by = py + ph - 82;

                if (drawButton("Logout", bx1, by, bw, bh, true))
                {
                    showLogoutConfirm = false;
                    resetLoginToLanding();
                    appState = 0;
                    currentScreen = SCREEN_DASHBOARD;
                }
                if (drawOutlineButton("Cancel", bx2, by, bw, bh) || IsKeyPressed(KEY_ESCAPE))
                {
                    showLogoutConfirm = false;
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}