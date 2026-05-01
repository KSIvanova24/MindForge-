#pragma once
#include "../pch.h"

static int textLen(const char* s)
{
    int n = 0;
    while (s && s[n] != '\0') n++;
    return n;
}

static void clearText(char* s, int max)
{
    for (int i = 0; i < max; i++) s[i] = '\0';
}

static void copyText(char* dst, int dstMax, const char* src)
{
    if (dstMax <= 0) return;
    int i = 0;
    while (i < dstMax - 1 && src && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static const Color LG_BG = { 12,  10,   8, 255 };
static const Color LG_SIDEBAR = { 28,  24,  20, 255 };
static const Color LG_PANEL = { 22,  18,  14, 255 };
static const Color LG_BORDER = { 55,  46,  36, 255 };
static const Color LG_ORANGE = { 234, 108,  15, 255 };
static const Color LG_ORANGE_LT = { 251, 146,  60, 255 };
static const Color LG_ORANGE_GL = { 251, 146,  60,  50 };
static const Color LG_WHITE = { 255, 255, 255, 255 };
static const Color LG_GREY = { 180, 160, 140, 255 };
static const Color LG_DARKGREY = { 100,  85,  70, 255 };
static const Color LG_INPUT_BG = { 32,  27,  21, 255 };
static const Color LG_INPUT_ACT = { 42,  35,  26, 255 };
static const Color LG_ERROR = { 220,  80,  60, 255 };
static const Color LG_SUCCESS = { 80, 200, 120, 255 };

#define MAX_STARS 120
struct Star { float x, y, speed, size, alpha, alphaSpeed; };
static Star  stars[MAX_STARS];
static bool  starsInit = false;
static float loginTime = 0.0f;

static void initStars(int sw, int sh)
{
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = (float)GetRandomValue(0, sw);
        stars[i].y = (float)GetRandomValue(0, sh);
        stars[i].speed = (float)GetRandomValue(10, 40) / 10.0f;
        stars[i].size = (float)GetRandomValue(1, 4);
        stars[i].alpha = (float)GetRandomValue(0, 255);
        stars[i].alphaSpeed = (float)GetRandomValue(30, 80) / 10.0f;
    }
    starsInit = true;
}

static void drawStars(int sw, int sh)
{
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].y += stars[i].speed;
        stars[i].alpha += stars[i].alphaSpeed;
        if (stars[i].alpha > 255 || stars[i].alpha < 0)
            stars[i].alphaSpeed = -stars[i].alphaSpeed;
        if (stars[i].y > sh) { stars[i].y = 0; stars[i].x = (float)GetRandomValue(0, sw); }
        int a = (int)stars[i].alpha;
        if (a < 0) a = 0; if (a > 255) a = 255;
        Color sc = LG_ORANGE_LT; sc.a = (unsigned char)a;
        DrawCircle((int)stars[i].x, (int)stars[i].y, stars[i].size, sc);
    }
}

static void drawRoundedBorder(Rectangle r, float rnd, int segs, Color col, float thick)
{
    Rectangle outer = { r.x - thick, r.y - thick, r.width + thick * 2, r.height + thick * 2 };
    DrawRectangleRounded(outer, rnd, segs, col);
}

static bool drawInputBox(const char* label, char* buf, int bufMax,
    int x, int y, int w, int h,
    bool focused, bool isPassword,
    const char* placeholder)
{
    Vector2 mouse = GetMousePosition();
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool clicked = CheckCollisionPointRec(mouse, r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    DrawText(label, x, y - 26, 18, LG_GREY);

    Color bgCol = focused ? LG_INPUT_ACT : LG_INPUT_BG;
    Color brdCol = focused ? LG_ORANGE : LG_BORDER;
    drawRoundedBorder(r, 0.22f, 8, brdCol, 2.0f);
    DrawRectangleRounded(r, 0.22f, 8, bgCol);

    int len = textLen(buf);
    char display[256] = {};
    if (isPassword) {
        for (int i = 0; i < len && i < 255; i++) display[i] = '*';
    }
    else {
        copyText(display, 256, buf);
    }

    int textSize = 20;
    if (len == 0 && !focused) {
        DrawText(placeholder, x + 14, y + h / 2 - textSize / 2, textSize, LG_DARKGREY);
    }
    else {
        DrawText(display, x + 14, y + h / 2 - textSize / 2, textSize, LG_WHITE);
    }

    if (focused) {
        int cursorX = x + 14 + MeasureText(display, textSize);
        float t = (float)GetTime();
        if ((int)(t * 2) % 2 == 0)
            DrawRectangle(cursorX + 2, y + 12, 2, h - 24, LG_ORANGE_LT);
    }

    if (focused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && len < bufMax - 1) {
                buf[len] = (char)key;
                buf[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
            buf[len - 1] = '\0';
    }

    return clicked;
}

static bool drawButton(const char* label, int x, int y, int w, int h, bool danger = false)
{
    Vector2   mouse = GetMousePosition();
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool      hov = CheckCollisionPointRec(mouse, r);

    Color base = danger ? Color{ 180,50,30,255 } : LG_ORANGE;
    Color hlight = danger ? Color{ 220,70,50,255 } : LG_ORANGE_LT;

    if (hov) {
        Rectangle glow = { r.x - 4, r.y - 4, r.width + 8, r.height + 8 };
        DrawRectangleRounded(glow, 0.35f, 8, LG_ORANGE_GL);
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    DrawRectangleRounded(r, 0.3f, 8, hov ? hlight : base);

    int fs = 22;
    int tw = MeasureText(label, fs);
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs, LG_WHITE);

    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static bool drawOutlineButton(const char* label, int x, int y, int w, int h)
{
    Vector2   mouse = GetMousePosition();
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool      hov = CheckCollisionPointRec(mouse, r);

    drawRoundedBorder(r, 0.3f, 8, hov ? LG_ORANGE_LT : LG_BORDER, 2.0f);
    DrawRectangleRounded(r, 0.3f, 8, LG_PANEL);

    if (hov) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

    int fs = 20;
    int tw = MeasureText(label, fs);
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs, hov ? LG_ORANGE_LT : LG_GREY);

    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

enum ModalMode { MODAL_NONE, MODAL_LOGIN, MODAL_REGISTER };

static ModalMode  s_modal = MODAL_NONE;
static char       s_inputUser[64] = {};
static char       s_inputPass[64] = {};
static char       s_inputUser2[64] = {};
static char       s_inputPass2[64] = {};
static char       s_inputPass3[64] = {};
static int        s_focusField = 0;
static char       s_errorMsg[128] = {};
static bool       s_registerOk = false;
static bool       s_showWelcome = false;

inline void ResetLoginToLanding()
{
    s_modal = MODAL_NONE;
    s_showWelcome = false;
    s_registerOk = false;
    s_errorMsg[0] = '\0';
    clearText(s_inputUser, 64);
    clearText(s_inputPass, 64);
    clearText(s_inputUser2, 64);
    clearText(s_inputPass2, 64);
    clearText(s_inputPass3, 64);
    s_focusField = 0;
}

static void drawOverlay(int sw, int sh)
{
    DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 160 });
}

static Rectangle centredPanel(int sw, int sh, int pw, int ph)
{
    float px = (float)(sw / 2 - pw / 2);
    float py = (float)(sh / 2 - ph / 2);
    DrawRectangleRounded({ px - 2, py - 2, (float)(pw + 4), (float)(ph + 4) }, 0.06f, 8, LG_ORANGE);
    DrawRectangleRounded({ px, py, (float)pw, (float)ph }, 0.06f, 8, LG_PANEL);
    return { px, py, (float)pw, (float)ph };
}

static bool drawLoginModal(int sw, int sh)
{
    drawOverlay(sw, sh);

    if (drawOutlineButton("< Back", 24, 24, 120, 42)) {
        ResetLoginToLanding();
        return false;
    }

    int pw = 480, ph = 520;
    Rectangle panel = centredPanel(sw, sh, pw, ph);
    int px = (int)panel.x, py = (int)panel.y;

    const char* t1 = "Welcome back to ";
    const char* t2 = "TaskDash";
    int ts = 26;
    int tx = px + pw / 2 - (MeasureText(t1, ts) + MeasureText(t2, ts)) / 2;
    DrawText(t1, tx, py + 30, ts, LG_WHITE);
    DrawText(t2, tx + MeasureText(t1, ts), py + 30, ts, LG_ORANGE_LT);

    if (s_registerOk) {
        DrawRectangleRounded({ (float)(px + 20), (float)(py + 68), (float)(pw - 40), 30 }, 0.3f, 8, { 40,100,60,200 });
        const char* okMsg = "Account created! Log in below.";
        DrawText(okMsg, px + pw / 2 - MeasureText(okMsg, 17) / 2, py + 76, 17, LG_SUCCESS);
    }

    int fieldY = py + (s_registerOk ? 116 : 96);
    int fieldW = pw - 60;
    int fieldH = 52;
    int fieldX = px + 30;

    if (drawInputBox("Username", s_inputUser, 64, fieldX, fieldY, fieldW, fieldH,
        s_focusField == 0, false, "Enter your username"))
        s_focusField = 0;

    if (drawInputBox("Password", s_inputPass, 64, fieldX, fieldY + 100, fieldW, fieldH,
        s_focusField == 1, true, "Enter your password"))
        s_focusField = 1;

    if (IsKeyPressed(KEY_TAB))   s_focusField = (s_focusField + 1) % 2;

    if (s_errorMsg[0]) {
        int em = 17;
        DrawText(s_errorMsg, px + pw / 2 - MeasureText(s_errorMsg, em) / 2,
            fieldY + 214, em, LG_ERROR);
    }

    bool loggedIn = false;
    if (drawButton("Log In", fieldX, fieldY + 240, fieldW, 52) ||
        (IsKeyPressed(KEY_ENTER) && s_focusField >= 0))
    {
        if (textLen(s_inputUser) == 0 || textLen(s_inputPass) == 0) {
            copyText(s_errorMsg, 128, "Please fill in all fields.");
        }
        else {
            s_modal = MODAL_NONE;
            s_showWelcome = true;
            s_errorMsg[0] = '\0';
            loggedIn = true;
            clearText(s_inputPass, 64);
        }
    }

    const char* linkTxt = "Don't have an account?  Create one";
    int lts = 18;
    int lx = px + pw / 2 - MeasureText(linkTxt, lts) / 2;
    int ly = (fieldY + 240 + 52) + 18;
    Vector2 mouse = GetMousePosition();
    bool lhov = mouse.x >= lx && mouse.x <= lx + MeasureText(linkTxt, lts) &&
        mouse.y >= ly && mouse.y <= ly + lts;
    DrawText(linkTxt, lx, ly, lts, lhov ? LG_ORANGE_LT : LG_GREY);
    if (lhov) {
        DrawLine(lx, ly + lts + 1, lx + MeasureText(linkTxt, lts), ly + lts + 1, LG_ORANGE_LT);
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            s_modal = MODAL_REGISTER;
            s_errorMsg[0] = '\0';
            s_registerOk = false;
            clearText(s_inputUser2, 64);
            clearText(s_inputPass2, 64);
            clearText(s_inputPass3, 64);
            s_focusField = 0;
        }
    }

    return loggedIn;
}

static void drawRegisterModal(int sw, int sh)
{
    drawOverlay(sw, sh);

    if (drawOutlineButton("< Back", 24, 24, 120, 42)) {
        ResetLoginToLanding();
        return;
    }

    int pw = 480, ph = 520;
    Rectangle panel = centredPanel(sw, sh, pw, ph);
    int px = (int)panel.x, py = (int)panel.y;

    const char* t1 = "Create a ";
    const char* t2 = "TaskDash";
    const char* t3 = " Account";
    int ts = 26;
    int totW = MeasureText(t1, ts) + MeasureText(t2, ts) + MeasureText(t3, ts);
    int tx = px + pw / 2 - totW / 2;
    DrawText(t1, tx, py + 30, ts, LG_WHITE);
    DrawText(t2, tx + MeasureText(t1, ts), py + 30, ts, LG_ORANGE_LT);
    DrawText(t3, tx + MeasureText(t1, ts) + MeasureText(t2, ts), py + 30, ts, LG_WHITE);

    int fieldX = px + 30;
    int fieldW = pw - 60;
    int fieldH = 52;
    int fieldY = py + 96;

    if (drawInputBox("Username", s_inputUser2, 64, fieldX, fieldY, fieldW, fieldH,
        s_focusField == 0, false, "Choose a username"))
        s_focusField = 0;

    if (drawInputBox("Password", s_inputPass2, 64, fieldX, fieldY + 100, fieldW, fieldH,
        s_focusField == 1, true, "Choose a password"))
        s_focusField = 1;

    if (drawInputBox("Confirm Password", s_inputPass3, 64, fieldX, fieldY + 200, fieldW, fieldH,
        s_focusField == 2, true, "Repeat your password"))
        s_focusField = 2;

    if (IsKeyPressed(KEY_TAB)) s_focusField = (s_focusField + 1) % 3;

    if (s_errorMsg[0]) {
        int em = 17;
        DrawText(s_errorMsg, px + pw / 2 - MeasureText(s_errorMsg, em) / 2,
            fieldY + 262, em, LG_ERROR);
    }

    if (drawButton("Create Account", fieldX, fieldY + 290, fieldW, 52) ||
        (IsKeyPressed(KEY_ENTER) && s_focusField >= 0))
    {
        if (textLen(s_inputUser2) == 0 || textLen(s_inputPass2) == 0 || textLen(s_inputPass3) == 0) {
            copyText(s_errorMsg, 128, "Please fill in all fields.");
        }
        else if (strcmp(s_inputPass2, s_inputPass3) != 0) {
            copyText(s_errorMsg, 128, "Passwords do not match.");
        }
        else if (textLen(s_inputPass2) < 4) {
            copyText(s_errorMsg, 128, "Password must be at least 4 characters.");
        }
        else {
            copyText(s_inputUser, 64, s_inputUser2);
            clearText(s_inputPass, 64);
            s_registerOk = true;
            s_errorMsg[0] = '\0';
            s_modal = MODAL_LOGIN;
            s_focusField = 1;
            clearText(s_inputPass2, 64);
            clearText(s_inputPass3, 64);
        }
    }

    const char* linkTxt = "Already have an account?  Log in";
    int lts = 18;
    int lx = px + pw / 2 - MeasureText(linkTxt, lts) / 2;
    int ly = (fieldY + 290 + 52) + 18;
    Vector2 mouse = GetMousePosition();
    bool lhov = mouse.x >= lx && mouse.x <= lx + MeasureText(linkTxt, lts) &&
        mouse.y >= ly && mouse.y <= ly + lts;
    DrawText(linkTxt, lx, ly, lts, lhov ? LG_ORANGE_LT : LG_GREY);
    if (lhov) {
        DrawLine(lx, ly + lts + 1, lx + MeasureText(linkTxt, lts), ly + lts + 1, LG_ORANGE_LT);
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            s_modal = MODAL_LOGIN;
            s_errorMsg[0] = '\0';
            s_focusField = 0;
        }
    }
}

static bool drawWelcomePopup(int sw, int sh)
{
    drawOverlay(sw, sh);

    int pw = 420, ph = 260;
    Rectangle panel = centredPanel(sw, sh, pw, ph);
    int px = (int)panel.x, py = (int)panel.y;

    DrawText("★", px + pw / 2 - 14, py + 22, 28, LG_ORANGE_LT);

    const char* line1 = "Welcome to TaskDash";
    int l1s = 24;
    DrawText(line1, px + pw / 2 - MeasureText(line1, l1s) / 2, py + 64, l1s, LG_GREY);

    const char* line3 = "You're all set. Let's get things done.";
    int l3s = 17;
    DrawText(line3, px + pw / 2 - MeasureText(line3, l3s) / 2, py + 120, l3s, LG_DARKGREY);

    int bw = 140, bh = 48;
    int bx = px + pw / 2 - bw / 2;
    int by = py + ph - 72;
    if (drawButton("OK", bx, by, bw, bh)) {
        s_showWelcome = false;
        return true;
    }
    return false;
}

inline int DrawLoginScreen(int sw, int sh)
{
    loginTime += GetFrameTime();
    if (!starsInit) initStars(sw, sh);

    if (s_modal == MODAL_NONE && !s_showWelcome)
    {
        DrawRectangle(0, 0, sw, sh, LG_BG);
        drawStars(sw, sh);

        float pulse = 1.0f + 0.015f * sinf(loginTime * 2.0f);
        int   tSize = (int)(72 * pulse);
        const char* w1 = "Task";
        const char* w2 = "Dash";
        int w1w = MeasureText(w1, tSize), w2w = MeasureText(w2, tSize);
        int titleX = sw / 2 - (w1w + 18 + w2w) / 2;
        int titleY = sh / 2 - tSize - 40;
        DrawText(w1, titleX, titleY, tSize, LG_ORANGE_LT);
        DrawText(w2, titleX + w1w + 18, titleY, tSize, LG_WHITE);

        const char* sub = "Your personal task manager";
        int subS = 22;
        DrawText(sub, sw / 2 - MeasureText(sub, subS) / 2, titleY + tSize + 14, subS, LG_GREY);

        int bw = 220, bh = 58;
        int bx = sw / 2 - bw / 2, by = sh / 2 + 40;
        if (drawButton("Log In", bx, by, bw, bh))
        {
            s_modal = MODAL_LOGIN;
            s_focusField = 0;
            s_errorMsg[0] = '\0';
            clearText(s_inputUser, 64);
            clearText(s_inputPass, 64);
            s_registerOk = false;
        }

        return 0;
    }

    if (!s_showWelcome)
    {
        DrawRectangle(0, 0, sw, sh, LG_BG);
        drawStars(sw, sh);

        if (s_modal == MODAL_LOGIN)    drawLoginModal(sw, sh);
        else if (s_modal == MODAL_REGISTER) drawRegisterModal(sw, sh);

        return 0;
    }

    if (s_showWelcome)
    {
        bool dismissed = drawWelcomePopup(sw, sh);
        return dismissed ? 2 : 1;
    }

    return 0;
}