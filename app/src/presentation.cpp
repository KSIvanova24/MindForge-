#include "../include/presentation.h"
#include "../include/accounts.h"
#include "../include/screens/dashboard.h"
#include "../include/screens/all_tasks.h"
#include "../include/screens/statistics.h"
#include "../include/screens/profile.h"

#include <cstdio>

static bool s_settingsShowDeleteConfirm = false;
static char s_settingsDeleteErr[128] = {};

bool SettingsDeleteModalIsOpen()
{
    return s_settingsShowDeleteConfirm;
}

int SIDEBAR_W = 300;
int BTN_H = 62;
int BTN_GAP = 8;
int BTN_X = 16;
int BTN_W = 268;
int NAV_START = 110;

Color COLOR_SIDEBAR = { 28,  24,  20, 255 };
Color COLOR_HOVER = { 45,  38,  30, 255 };
Color COLOR_ACTIVE = { 234, 108,  15, 255 };
Color COLOR_ACCENT = { 251, 146,  60, 255 };
Color COLOR_DIVIDER = { 55,  46,  36, 255 };
Color COLOR_BRAND_A = { 251, 146,  60, 255 };
Color COLOR_WHITE = { 255, 255, 255, 255 };
Color COLOR_GREY = { 180, 160, 140, 255 };
Color COLOR_DOT_IDLE = { 120, 100,  80, 255 };

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

static bool settings_drawButton(const char* label, int x, int y, int w, int h, Color base, Color hover)
{
    Vector2   mouse = GetMousePosition();
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool      hov = CheckCollisionPointRec(mouse, r);

    if (hov) {
        Rectangle glow = { r.x - 4, r.y - 4, r.width + 8, r.height + 8 };
        DrawRectangleRounded(glow, 0.35f, 8, { 251, 146, 60, 50 });
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    DrawRectangleRounded(r, 0.3f, 8, hov ? hover : base);

    int fs = 20;
    int tw = MeasureText(label, fs);
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs, RAYWHITE);

    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static bool settings_drawOutlineButton(const char* label, int x, int y, int w, int h)
{
    Vector2   mouse = GetMousePosition();
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool      hov = CheckCollisionPointRec(mouse, r);

    Color brd = hov ? Color{ 251, 146, 60, 255 } : Color{ 55, 46, 36, 255 };
    Rectangle outer = { r.x - 2, r.y - 2, r.width + 4, r.height + 4 };
    DrawRectangleRounded(outer, 0.3f, 8, brd);
    DrawRectangleRounded(r, 0.3f, 8, Color{ 22, 18, 14, 255 });

    if (hov) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

    int fs = 18;
    int tw = MeasureText(label, fs);
    Color txt = hov ? Color{ 251, 146, 60, 255 } : Color{ 180, 160, 140, 255 };
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs, txt);

    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void DrawSettingsScreen(int contentX, int contentWidth, int screenHeight, bool* outAccountDeleted)
{
    if (outAccountDeleted)
        *outAccountDeleted = false;

    Color panelBg = { 22, 18, 14, 255 };
    Color orange = { 234, 108, 15, 255 };
    Color orangeLt = { 251, 146, 60, 255 };
    Color grey = { 180, 160, 140, 255 };
    Color errCol = { 220, 80, 60, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, panelBg);

    int pad = 40;
    DrawText("Settings", contentX + pad, pad, 32, RAYWHITE);

    const char* user = getLoggedInUser();
    char signedIn[96];
    if (user && user[0] != '\0')
        snprintf(signedIn, sizeof signedIn, "Signed in as: %s", user);
    else
        snprintf(signedIn, sizeof signedIn, "%s", "Signed in as: —");

    DrawText(signedIn, contentX + pad, pad + 52, 20, grey);

    DrawText("Account", contentX + pad, pad + 110, 22, orangeLt);

    if (s_settingsDeleteErr[0] != '\0')
        DrawText(s_settingsDeleteErr, contentX + pad, pad + 200, 17, errCol);

    int bx = contentX + pad;
    int by = pad + 150;
    int bw = 220;
    int bh = 48;

    if (settings_drawButton("Delete my account", bx, by, bw, bh,
            Color{ 180, 50, 30, 255 }, Color{ 220, 70, 50, 255 }))
    {
        s_settingsDeleteErr[0] = '\0';
        if (!user || user[0] == '\0') {
            snprintf(s_settingsDeleteErr, sizeof s_settingsDeleteErr, "%s", "No active account.");
        }
        else {
            s_settingsShowDeleteConfirm = true;
        }
    }

    if (s_settingsShowDeleteConfirm)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 170 });

        int pw = 460;
        int ph = 240;
        int px = sw / 2 - pw / 2;
        int py = sh / 2 - ph / 2;

        DrawRectangleRounded({ (float)(px - 2), (float)(py - 2), (float)(pw + 4), (float)(ph + 4) }, 0.08f, 8, orange);
        DrawRectangleRounded({ (float)px, (float)py, (float)pw, (float)ph }, 0.08f, 8, Color{ 22, 18, 14, 255 });

        const char* title = "Delete account?";
        const char* msg = "This removes your saved login from accounts.txt.";
        DrawText(title, px + pw / 2 - MeasureText(title, 26) / 2, py + 36, 26, RAYWHITE);
        DrawText(msg, px + pw / 2 - MeasureText(msg, 18) / 2, py + 82, 18, grey);

        int cbw = 170;
        int cbh = 52;
        int cgap = 16;
        int cbx1 = px + pw / 2 - (cbw * 2 + cgap) / 2;
        int cbx2 = cbx1 + cbw + cgap;
        int cby = py + ph - 82;

        if (settings_drawButton("Delete", cbx1, cby, cbw, cbh,
                Color{ 180, 50, 30, 255 }, Color{ 220, 70, 50, 255 }))
        {
            if (user && user[0] != '\0' && deleteAccount(user)) {
                clearLoggedInUser();
                s_settingsShowDeleteConfirm = false;
                s_settingsDeleteErr[0] = '\0';
                if (outAccountDeleted)
                    *outAccountDeleted = true;
            }
            else {
                snprintf(s_settingsDeleteErr, sizeof s_settingsDeleteErr, "%s",
                    "Could not remove account from file.");
                s_settingsShowDeleteConfirm = false;
            }
        }
        if (settings_drawOutlineButton("Cancel", cbx2, cby, cbw, cbh) || IsKeyPressed(KEY_ESCAPE))
        {
            s_settingsShowDeleteConfirm = false;
        }
    }
}