#pragma once
#include "../pch.h"

#define MAX_STARS 120

struct Star {
    float x;
    float y;
    float speed;
    float size;
    float alpha;
    float alphaSpeed;
};

static Star  stars[MAX_STARS];
static bool  starsInit = false;
static float loginTime = 0.0f;

static void initStars(int sw, int sh)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].x = (float)GetRandomValue(0, sw);
        stars[i].y = (float)GetRandomValue(0, sh);
        stars[i].speed = (float)GetRandomValue(10, 40) / 10.0f;
        stars[i].size = (float)GetRandomValue(1, 4);
        stars[i].alpha = (float)GetRandomValue(0, 255);
        stars[i].alphaSpeed = (float)GetRandomValue(30, 80) / 10.0f;
    }
    starsInit = true;
}

inline bool DrawLoginScreen(int sw, int sh)
{
    loginTime += GetFrameTime();

    if (!starsInit)
        initStars(sw, sh);

    Color bgColor = { 12,  10,  8,   255 };
    Color starColor = { 251, 146, 60,  255 };
    Color titleOrange = { 251, 146, 60,  255 };
    Color titleWhite = { 255, 255, 255, 255 };
    Color subColor = { 180, 160, 140, 255 };
    Color btnNormal = { 234, 108, 15,  255 };
    Color btnHover = { 251, 146, 60,  255 };
    Color btnGlow = { 251, 146, 60,  60 };

    DrawRectangle(0, 0, sw, sh, bgColor);

    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].y += stars[i].speed;
        stars[i].alpha += stars[i].alphaSpeed;

        if (stars[i].alpha > 255 || stars[i].alpha < 0)
            stars[i].alphaSpeed = -stars[i].alphaSpeed;

        if (stars[i].y > sh)
        {
            stars[i].y = 0;
            stars[i].x = (float)GetRandomValue(0, sw);
        }

        int a = (int)stars[i].alpha;
        if (a < 0)   a = 0;
        if (a > 255) a = 255;

        starColor.a = (unsigned char)a;
        DrawCircle((int)stars[i].x, (int)stars[i].y, stars[i].size, starColor);
    }

    float pulse = 1.0f + 0.015f * sinf(loginTime * 2.0f);
    int titleSize = (int)(72 * pulse);

    const char* word1 = "Task";
    const char* word2 = "Dash";

    int w1 = MeasureText(word1, titleSize);
    int w2 = MeasureText(word2, titleSize);
    int gap = 18;
    int totalW = w1 + gap + w2;
    int titleX = sw / 2 - totalW / 2;
    int titleY = sh / 2 - titleSize - 40;

    DrawText(word1, titleX, titleY, titleSize, titleOrange);
    DrawText(word2, titleX + w1 + gap, titleY, titleSize, titleWhite);

    const char* sub = "Your personal task manager";
    int subSize = 22;
    int subW = MeasureText(sub, subSize);
    DrawText(sub, sw / 2 - subW / 2, titleY + titleSize + 14, subSize, subColor);

    int btnW = 220;
    int btnH = 58;
    int btnX = sw / 2 - btnW / 2;
    int btnY = sh / 2 + 40;

    Vector2 mouse = GetMousePosition();
    Rectangle btnRect = { (float)btnX, (float)btnY, (float)btnW, (float)btnH };
    bool hovered = CheckCollisionPointRec(mouse, btnRect);

    DrawRectangleRounded(btnRect, 0.35f, 8, hovered ? btnHover : btnNormal);

    if (hovered)
    {
        Rectangle glowRect = { (float)(btnX - 3), (float)(btnY - 3), (float)(btnW + 6), (float)(btnH + 6) };
        DrawRectangleRounded(glowRect, 0.35f, 8, btnGlow);
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }

    const char* btnLabel = "Log In";
    int lblSize = 26;
    int lblW = MeasureText(btnLabel, lblSize);
    DrawText(btnLabel, btnX + btnW / 2 - lblW / 2, btnY + btnH / 2 - lblSize / 2, lblSize, WHITE);

    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return true;

    return false;
}