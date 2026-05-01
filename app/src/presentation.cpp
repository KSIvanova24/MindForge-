#include "../include/presentation.h"
#include "../include/screens/dashboard.h"
#include "../include/screens/all_tasks.h"
#include "../include/screens/statistics.h"
#include "../include/screens/profile.h"
#include "../include/screens/settings.h"

int SIDEBAR_W = 300;
int BTN_H = 62;
int BTN_GAP = 8;
int BTN_X = 16;
int BTN_W = 268;
int NAV_START = 110;

Color COLOR_SIDEBAR = { 28,  24,  20, 255 };  // very dark warm brown
Color COLOR_HOVER = { 45,  38,  30, 255 };  // warm brown hover
Color COLOR_ACTIVE = { 234, 108,  15, 255 };  // orange
Color COLOR_ACCENT = { 251, 146,  60, 255 };  // light orange accent bar
Color COLOR_DIVIDER = { 55,  46,  36, 255 };  // subtle warm divider
Color COLOR_BRAND_A = { 251, 146,  60, 255 };  // orange for "Task"
Color COLOR_WHITE = { 255, 255, 255, 255 };
Color COLOR_GREY = { 180, 160, 140, 255 };  // warm grey for idle text
Color COLOR_DOT_IDLE = { 120, 100,  80, 255 };  // warm dark dot

void drawOneButton(const char* label, int x, int y, int w, int h, bool isActive, bool isHovered)
{
    if (isActive)
    {
        DrawRectangle(x, y + 10, 4, h - 20, COLOR_ACCENT);
        DrawRectangleRounded({ (float)(x + 4), (float)y, (float)(w - 4), (float)h }, 0.28f, 8, COLOR_ACTIVE);
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_WHITE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_WHITE);
    }
    else if (isHovered)
    {
        DrawRectangleRounded({ (float)x, (float)y, (float)w, (float)h }, 0.28f, 8, COLOR_HOVER);
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_DOT_IDLE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_GREY);
    }
    else
    {
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_DOT_IDLE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_GREY);
    }
}

void DrawSidebar(AppScreen current, AppScreen* outHovered, int screenHeight)
{
    DrawRectangle(0, 0, SIDEBAR_W, screenHeight, COLOR_SIDEBAR);
    DrawRectangleGradientH(SIDEBAR_W - 10, 0, 10, screenHeight, { 0,0,0,0 }, { 0,0,0,60 });

    DrawText("Task", 28, 34, 36, COLOR_BRAND_A);
    DrawText("Dash", 28 + MeasureText("Task", 36) + 8, 34, 36, COLOR_WHITE);

    DrawRectangle(20, 88, SIDEBAR_W - 40, 1, COLOR_DIVIDER);

    const char* mainLabels[] = { "Dashboard", "All Tasks", "Statistics" };
    AppScreen   mainScreens[] = { SCREEN_DASHBOARD, SCREEN_ALL_TASKS, SCREEN_STATISTICS };

    *outHovered = (AppScreen)-1;

    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < 3; i++)
    {
        int x = BTN_X;
        int y = NAV_START + i * (BTN_H + BTN_GAP);
        int w = BTN_W;
        int h = BTN_H;

        bool isActive = (current == mainScreens[i]);
        bool isHovered = CheckCollisionPointRec(mouse, { (float)x, (float)y, (float)w, (float)h });

        if (isHovered)
            *outHovered = mainScreens[i];

        drawOneButton(mainLabels[i], x, y, w, h, isActive, isHovered);
    }

    DrawRectangle(20, screenHeight - 180, SIDEBAR_W - 40, 1, COLOR_DIVIDER);

    const char* bottomLabels[] = { "My Profile", "Settings", "Logout" };
    AppScreen   bottomScreens[] = { SCREEN_PROFILE, SCREEN_SETTINGS, SCREEN_LOGOUT };

    int bottomStart = screenHeight - 160 - (BTN_H + BTN_GAP);

    for (int i = 0; i < 3; i++)
    {
        int x = BTN_X;
        int y = bottomStart + i * (BTN_H + BTN_GAP);
        int w = BTN_W;
        int h = BTN_H;

        bool isActive = (bottomScreens[i] != SCREEN_LOGOUT) && (current == bottomScreens[i]);
        bool isHovered = CheckCollisionPointRec(mouse, { (float)x, (float)y, (float)w, (float)h });

        if (isHovered)
            *outHovered = bottomScreens[i];

        drawOneButton(bottomLabels[i], x, y, w, h, isActive, isHovered);
    }
}