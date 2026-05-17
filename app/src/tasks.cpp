#include "../include/screens/tasks.h"
#include "../include/data.h"
#include "../include/db.h"
#include "../include/accounts.h"
#include "raylib.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "../include/logic.h"

static const Color AT_BG        = {  22,  18,  14, 255 };
static const Color AT_PANEL     = {  35,  28,  20, 255 };
static const Color AT_PANEL_LT  = {  46,  37,  27, 255 };
static const Color AT_BORDER    = {  55,  46,  36, 255 };
static const Color AT_WHITE     = { 255, 255, 255, 255 };
static const Color AT_GREY      = { 180, 160, 140, 255 };
static const Color AT_DARK      = { 100,  85,  70, 255 };
static const Color AT_ORANGE    = { 234, 108,  15, 255 };
static const Color AT_ORANGE_LT = { 251, 146,  60, 255 };
static const Color AT_RED       = { 220,  80,  60, 255 };
static const Color AT_RED_DIM   = { 120,  40,  30, 255 };
static const Color AT_GREEN     = {  80, 200, 100, 255 };
static const Color AT_BLUE      = {  90, 140, 220, 255 };

enum AT_Filter { ATF_ALL = 0, ATF_ACTIVE, ATF_DONE, ATF_OVERDUE, ATF_HIGHPRI, ATF_COUNT };
enum AT_View   { ATV_LIST = 0, ATV_HEATMAP };

static const char* AT_FILTER_LABELS[ATF_COUNT] = {
    "All", "Active", "Done", "Overdue", "High Priority"
};

static bool  atSelected[MAX_TASKS]  = {};
static float atFadeTimer[MAX_TASKS] = {};

static AT_Filter atFilter        = ATF_ALL;
static AT_View   atView          = ATV_LIST;
static char      atSearchBuf[64] = {};
static bool      atSearchFocused = false;
static float     atScroll        = 0.0f;
static int       atStreak        = 0;
static int       atLastCompleteDay = 0;

static bool  atShowAddModal      = false;
static char  atNewTitle[128]     = {};
static char  atNewDeadline[16]   = {};
static int   atNewPriority       = 2;
static char  atNewDuration[8]    = {};
static int   atNewFocusField     = 0;
static char  atNewError[128]     = {};
static int   atNewRepeatType     = 0;
static char  atNewRepeatDays[8]  = {};
static int   atNewCategoryIndex  = -1;
static int   atSortMode          = 0;

static int   atExpandedTaskIndex       = -1;
static bool  atIsEditingDescription    = false;
static char  atDescriptionEditBuffer[512] = {};

static const char* AT_PRIO_LABEL[] = { "?", "Low", "Med", "High" };
static Color       AT_PRIO_COLOR[] = { AT_GREY, AT_GREEN, AT_ORANGE_LT, AT_RED };

static int atParseToDay(const char* s)
{
    int y = 0, m = 0, d = 0;
    if (!s || sscanf(s, "%d-%d-%d", &y, &m, &d) != 3)
        return 0;
    struct tm t  = {};
    t.tm_year    = y - 1900;
    t.tm_mon     = m - 1;
    t.tm_mday    = d;
    t.tm_hour    = 12;
    time_t tt    = mktime(&t);
    return (int)(tt / 86400);
}

static int atTodayDay()
{
    time_t now    = time(NULL);
    struct tm t   = *localtime(&now);
    t.tm_hour     = 12;
    t.tm_min      = 0;
    t.tm_sec      = 0;
    return (int)(mktime(&t) / 86400);
}

static int atDaysFromToday(const char* deadline)
{
    int d = atParseToDay(deadline);
    if (d == 0)
        return 9999;
    return d - atTodayDay();
}

static const char* atTodayStr()
{
    static char buf[16];
    time_t now     = time(NULL);
    struct tm t    = *localtime(&now);
    int todayYear  = t.tm_year + 1900;
    int todayMonth = t.tm_mon + 1;
    int todayDay   = t.tm_mday;
    buildDateText(buf, todayYear, todayMonth, todayDay);
    return buf;
}

static void atAddDaysToDeadline(char* deadline, int days)
{
    int year  = 0;
    int month = 0;
    int day   = 0;
    bool parseSucceeded = parseDateText(deadline, &year, &month, &day);
    if (parseSucceeded == false)
    {
        return;
    }
    struct tm dateInfo = {};
    dateInfo.tm_year   = year - 1900;
    dateInfo.tm_mon    = month - 1;
    dateInfo.tm_mday   = day + days;
    dateInfo.tm_hour   = 12;
    mktime(&dateInfo);
    int newYear  = dateInfo.tm_year + 1900;
    int newMonth = dateInfo.tm_mon + 1;
    int newDay   = dateInfo.tm_mday;
    buildDateText(deadline, newYear, newMonth, newDay);
}

static void atSnoozeOneDay(char* deadline)
{
    atAddDaysToDeadline(deadline, 1);
}

static const char* atDurationLabel(int dur)
{
    if (dur <= 0)  return "";
    if (dur <= 15) return "5-15m";
    if (dur <= 60) return "30m-1h";
    return "1h+";
}

static bool atIsQuick(int dur)      { return dur > 0 && dur <= 15; }
static bool atLowEffort(int dur)    { return dur > 0 && dur <= 30; }
static bool atHighUrgency(int days) { return days <= 3; }

static void atBumpStreakOnComplete()
{
    int today = atTodayDay();
    if (atLastCompleteDay == 0)
    {
        atStreak = 1;
    }
    else if (today == atLastCompleteDay)
    {
    }
    else if (today == atLastCompleteDay + 1)
    {
        atStreak = atStreak + 1;
    }
    else
    {
        atStreak = 1;
    }
    atLastCompleteDay = today;
}

static void atLower(const char* in, char* out, int outMax)
{
    int i = 0;
    while (in[i] && i < outMax - 1)
    {
        char c = in[i];
        if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 'a';
        out[i] = c;
        i = i + 1;
    }
    out[i] = '\0';
}

static bool atMatchesSearch(const Task& t)
{
    if (atSearchBuf[0] == '\0')
        return true;
    char a[128], b[64];
    atLower(t.title, a, 128);
    atLower(atSearchBuf, b, 64);
    return strstr(a, b) != nullptr;
}

static bool atMatchesFilter(const Task& t)
{
    int days = atDaysFromToday(t.deadline);
    if (atFilter == ATF_ALL)
        return true;
    if (atFilter == ATF_ACTIVE)
        return !t.completed;
    if (atFilter == ATF_DONE)
        return t.completed;
    if (atFilter == ATF_OVERDUE)
        return !t.completed && days < 0;
    if (atFilter == ATF_HIGHPRI)
        return !t.completed && t.priority >= 3;
    return true;
}

static bool atMatchesCategory(const Task& t)
{
    const char* filter = getCurrentCategoryFilter();
    if (filter[0] == '\0')
        return true;
    return textsAreEqual(t.categoryName, filter);
}

enum AT_Group { ATG_OVERDUE, ATG_TODAY, ATG_WEEK, ATG_LATER, ATG_DONE, ATG_COUNT };
static const char* AT_GROUP_LABELS[ATG_COUNT] = {
    "Overdue", "Today", "This Week", "Later", "Done"
};

static AT_Group atGroupFor(const Task& t)
{
    if (t.completed)
        return ATG_DONE;
    int d = atDaysFromToday(t.deadline);
    if (d < 0)  return ATG_OVERDUE;
    if (d == 0) return ATG_TODAY;
    if (d <= 7) return ATG_WEEK;
    return ATG_LATER;
}

static bool atClickIn(Rectangle r)
{
    if (atShowAddModal)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        Rectangle modal = { (float)(sw / 2 - 260), (float)(sh / 2 - 290),
                            520.0f, 580.0f };
        if (!CheckCollisionPointRec(GetMousePosition(), modal))
            return false;
    }
    return CheckCollisionPointRec(GetMousePosition(), r) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static bool atHoverIn(Rectangle r)
{
    return CheckCollisionPointRec(GetMousePosition(), r);
}

static bool atDrawPill(const char* label, int x, int y, int h, bool active, int* outW)
{
    int textW     = MeasureText(label, 16);
    int w         = textW + 28;
    Rectangle r   = { (float)x, (float)y, (float)w, (float)h };
    bool hov      = atHoverIn(r);
    Color bg      = active ? AT_ORANGE : (hov ? AT_PANEL_LT : AT_PANEL);
    Color border  = active ? AT_ORANGE_LT : AT_BORDER;
    Color txt     = active ? AT_WHITE : (hov ? AT_WHITE : AT_GREY);
    DrawRectangleRounded(r, 0.5f, 8, bg);
    DrawRectangleRoundedLines(r, 0.5f, 8, 1.5f, border);
    DrawText(label, x + 14, y + h / 2 - 8, 16, txt);
    if (outW)
        *outW = w;
    return atClickIn(r);
}

static bool atDrawSmallBtn(const char* label, int x, int y, int w, int h, bool danger = false)
{
    Rectangle r  = { (float)x, (float)y, (float)w, (float)h };
    bool hov     = atHoverIn(r);
    Color base   = danger ? Color{ 180, 50, 30, 255 } : AT_ORANGE;
    Color hl     = danger ? Color{ 220, 70, 50, 255 } : AT_ORANGE_LT;
    DrawRectangleRounded(r, 0.3f, 8, hov ? hl : base);
    int fs = 16;
    int tw = MeasureText(label, fs);
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs, AT_WHITE);
    return atClickIn(r);
}

static bool atDrawGhostBtn(const char* label, int x, int y, int w, int h)
{
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool hov    = atHoverIn(r);
    DrawRectangleRoundedLines(r, 0.3f, 8, 1.5f, hov ? AT_ORANGE_LT : AT_BORDER);
    int fs = 16;
    int tw = MeasureText(label, fs);
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs,
             hov ? AT_ORANGE_LT : AT_GREY);
    return atClickIn(r);
}

static void atDrawSearchBox(int x, int y, int w, int h)
{
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    if (atClickIn(r))
    {
        atSearchFocused = true;
        atIsEditingDescription = false;
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !atHoverIn(r))
    {
        atSearchFocused = false;
    }

    Color bg = atSearchFocused ? AT_PANEL_LT : AT_PANEL;
    DrawRectangleRounded(r, 0.3f, 8, bg);
    DrawRectangleRoundedLines(r, 0.3f, 8, 1.5f,
        atSearchFocused ? AT_ORANGE : AT_BORDER);

    DrawText("Search", x + 14, y + h / 2 - 8, 16, AT_DARK);

    int len   = textLength(atSearchBuf);
    int textX = x + 14 + MeasureText("Search   ", 16);
    if (len == 0 && !atSearchFocused)
    {
        DrawText("Type to filter tasks...", textX, y + h / 2 - 8, 16, AT_DARK);
    }
    else
    {
        DrawText(atSearchBuf, textX, y + h / 2 - 8, 16, AT_WHITE);
        if (atSearchFocused && ((int)(GetTime() * 2) % 2 == 0))
        {
            int cx = textX + MeasureText(atSearchBuf, 16);
            DrawRectangle(cx + 2, y + 10, 2, h - 20, AT_ORANGE_LT);
        }
    }

    if (atSearchFocused)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key >= 32 && key <= 126 && len < 63)
            {
                atSearchBuf[len] = (char)key;
                len = len + 1;
                atSearchBuf[len] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
            atSearchBuf[len - 1] = '\0';
        if (IsKeyPressed(KEY_ESCAPE))
        {
            atSearchBuf[0]  = '\0';
            atSearchFocused = false;
        }
    }
}

static void atDrawWrappedText(const char* text, int x, int y, int maxWidth, int fontSize, Color color)
{
    char currentLine[256];
    int  currentLineLen   = 0;
    int  currentY         = y;
    int  textIndex        = 0;

    while (text[textIndex] != '\0')
    {
        char nextChar                    = text[textIndex];
        currentLine[currentLineLen]      = nextChar;
        currentLine[currentLineLen + 1]  = '\0';
        currentLineLen                   = currentLineLen + 1;

        int lineWidth = MeasureText(currentLine, fontSize);

        if (lineWidth >= maxWidth && currentLineLen > 1)
        {
            currentLine[currentLineLen - 1] = '\0';
            DrawText(currentLine, x, currentY, fontSize, color);
            currentY               = currentY + fontSize + 4;
            currentLine[0]         = nextChar;
            currentLine[1]         = '\0';
            currentLineLen         = 1;
        }

        textIndex = textIndex + 1;
    }

    if (currentLineLen > 0)
    {
        DrawText(currentLine, x, currentY, fontSize, color);
    }
}

static int atCountWrappedLines(const char* text, int maxWidth, int fontSize)
{
    if (text[0] == '\0')
        return 0;

    char currentLine[256];
    int  currentLineLen = 0;
    int  lines          = 1;
    int  textIndex      = 0;

    while (text[textIndex] != '\0')
    {
        char nextChar               = text[textIndex];
        currentLine[currentLineLen] = nextChar;
        currentLine[currentLineLen + 1] = '\0';
        currentLineLen = currentLineLen + 1;

        int lineWidth = MeasureText(currentLine, fontSize);

        if (lineWidth >= maxWidth && currentLineLen > 1)
        {
            lines = lines + 1;
            currentLine[0] = nextChar;
            currentLine[1] = '\0';
            currentLineLen = 1;
        }

        textIndex = textIndex + 1;
    }

    return lines;
}

static void atSpawnRepeatCopy(const Task& completedTask)
{
    if (completedTask.repeatType <= 0)
        return;

    int intervalDays = completedTask.repeatInterval;
    if (completedTask.repeatType == 1) intervalDays = 1;
    if (completedTask.repeatType == 2) intervalDays = 7;
    if (completedTask.repeatType == 3) intervalDays = 30;
    if (intervalDays <= 0)             intervalDays = 1;

    Task ghostTask            = completedTask;
    ghostTask.id              = 0;
    ghostTask.completed       = false;

    atAddDaysToDeadline(ghostTask.deadline, intervalDays);

    const char* username = getLoggedInUser();
    int newId = saveNewTask(username, ghostTask);
    if (newId > 0)
    {
        ghostTask.id = newId;
        addTask(ghostTask);
    }
}

static int atDrawTaskRow(int taskIdx, int x, int y, int w, float alpha, bool isExpanded)
{
    Task* tasks = getTaskStore();
    Task& t     = tasks[taskIdx];
    int   rowH  = 64;

    int  days    = atDaysFromToday(t.deadline);
    bool overdue = !t.completed && days < 0;

    Rectangle rowRect = { (float)x, (float)y, (float)w, (float)rowH };

    Color bg = overdue ? Color{ 60, 28, 24, 255 } : AT_PANEL;
    bg.a = (unsigned char)(bg.a * alpha);
    DrawRectangleRounded(rowRect, 0.16f, 8, bg);

    int pr = (t.priority >= 1 && t.priority <= 3) ? t.priority : 1;
    Color prCol = AT_PRIO_COLOR[pr];
    prCol.a = (unsigned char)(prCol.a * alpha);
    DrawRectangle(x, y, 4, rowH, prCol);

    int cbSz = 20;
    int cbX  = x + 16;
    int cbY  = y + rowH / 2 - cbSz / 2;
    Rectangle cbR = { (float)cbX, (float)cbY, (float)cbSz, (float)cbSz };
    bool hovCb    = atHoverIn(cbR);
    Color cbBorder = atSelected[taskIdx] ? AT_ORANGE_LT : (hovCb ? AT_ORANGE : AT_DARK);
    cbBorder.a = (unsigned char)(cbBorder.a * alpha);
    DrawRectangleRoundedLines(cbR, 0.25f, 8, 1.5f, cbBorder);
    if (atSelected[taskIdx])
    {
        Color fill = AT_ORANGE;
        fill.a = (unsigned char)(fill.a * alpha);
        DrawRectangleRounded(
            { (float)(cbX + 3), (float)(cbY + 3), (float)(cbSz - 6), (float)(cbSz - 6) },
            0.25f, 8, fill);
    }
    if (atClickIn(cbR))
        atSelected[taskIdx] = !atSelected[taskIdx];

    Color titleCol = t.completed ? AT_DARK : AT_WHITE;
    titleCol.a = (unsigned char)(titleCol.a * alpha);
    int titleX = cbX + cbSz + 14;
    DrawText(t.title, titleX, y + rowH / 2 - 11, 20, titleCol);
    if (t.completed)
    {
        int tw     = MeasureText(t.title, 20);
        Color line = AT_DARK;
        line.a = (unsigned char)(line.a * alpha);
        DrawRectangle(titleX, y + rowH / 2, tw, 1, line);
    }

    if (t.description[0] != '\0')
    {
        Color dotColor = AT_ORANGE_LT;
        dotColor.a = (unsigned char)(dotColor.a * alpha * 0.7f);
        DrawCircle(titleX + MeasureText(t.title, 20) + 10, y + rowH / 2, 4, dotColor);
    }

    if (t.repeatType > 0)
    {
        const char* repeatSymbol = "~";
        Color symColor = AT_BLUE;
        symColor.a = (unsigned char)(symColor.a * alpha);
        DrawText(repeatSymbol, x + w - 420, y + rowH / 2 - 8, 14, symColor);
    }

    const char* durLabel = atDurationLabel(t.duration);
    int durX = x + w - 360;
    if (durLabel[0])
    {
        int dw = MeasureText(durLabel, 14) + 16;
        Color dBg = atIsQuick(t.duration) ? AT_GREEN : AT_BORDER;
        dBg.a = (unsigned char)(dBg.a * 0.45f * alpha);
        Color dTx = atIsQuick(t.duration) ? AT_GREEN : AT_GREY;
        dTx.a = (unsigned char)(dTx.a * alpha);
        DrawRectangleRounded(
            { (float)durX, (float)(y + rowH / 2 - 12), (float)dw, 24 },
            0.5f, 8, dBg);
        DrawText(durLabel, durX + 8, y + rowH / 2 - 8, 14, dTx);
    }

    Color prTxt = AT_PRIO_COLOR[pr];
    prTxt.a = (unsigned char)(prTxt.a * alpha);
    DrawText(AT_PRIO_LABEL[pr], x + w - 260, y + rowH / 2 - 8, 16, prTxt);

    Color dlCol = overdue ? AT_RED : AT_GREY;
    dlCol.a = (unsigned char)(dlCol.a * alpha);
    DrawText(t.deadline, x + w - 200, y + rowH / 2 - 8, 16, dlCol);
    if (overdue)
    {
        char od[32] = {};
        int odPos = 0;
        int daysOverdue = -days;
        odPos = appendNumber(od, odPos, daysOverdue);
        if (daysOverdue == 1)
        {
            odPos = appendText(od, odPos, " day overdue");
        }
        else
        {
            odPos = appendText(od, odPos, " days overdue");
        }
        Color rd = AT_RED;
        rd.a = (unsigned char)(rd.a * alpha);
        DrawText(od, x + w - 200, y + rowH / 2 + 8, 12, rd);
    }

    int doneSz = 26;
    int doneX  = x + w - doneSz - 14;
    int doneY  = y + rowH / 2 - doneSz / 2;
    Rectangle doneR = { (float)doneX, (float)doneY, (float)doneSz, (float)doneSz };
    bool hovDone = atHoverIn(doneR);
    Color border = t.completed ? AT_GREEN : (hovDone ? AT_ORANGE_LT : AT_DARK);
    border.a = (unsigned char)(border.a * alpha);
    DrawCircleLines(doneX + doneSz / 2, doneY + doneSz / 2, doneSz / 2.0f, border);
    if (t.completed)
    {
        Color g = AT_GREEN;
        g.a = (unsigned char)(g.a * alpha);
        DrawCircle(doneX + doneSz / 2, doneY + doneSz / 2, doneSz / 2.0f - 4, g);
    }
    if (atClickIn(doneR) && !t.completed)
    {
        t.completed = true;
        atFadeTimer[taskIdx] = 1.0f;
        atBumpStreakOnComplete();
        updateTaskCompleted(t.id, true);
        atSpawnRepeatCopy(t);
    }
    else if (atClickIn(doneR) && t.completed)
    {
        t.completed = false;
        atFadeTimer[taskIdx] = 0.0f;
        updateTaskCompleted(t.id, false);
    }

    bool clickedInRow       = CheckCollisionPointRec(GetMousePosition(), rowRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool clickedInCheckbox  = CheckCollisionPointRec(GetMousePosition(), cbR);
    bool clickedInComplete  = CheckCollisionPointRec(GetMousePosition(), doneR);

    if (!atShowAddModal && clickedInRow && !clickedInCheckbox && !clickedInComplete)
    {
        if (atExpandedTaskIndex == taskIdx)
        {
            atExpandedTaskIndex    = -1;
            atIsEditingDescription = false;
        }
        else
        {
            atExpandedTaskIndex    = taskIdx;
            atIsEditingDescription = false;
            atSearchFocused        = false;
        }
    }

    int totalHeight = rowH;

    if (isExpanded == true)
    {
        int panelX  = x;
        int panelY  = y + rowH + 2;
        int panelW  = w;
        int descFontSize = 15;
        int textAreaW    = panelW - 32;

        int panelH = 0;

        if (atIsEditingDescription == true && atExpandedTaskIndex == taskIdx)
        {
            panelH = 136;
        }
        else if (t.description[0] != '\0')
        {
            int wrappedLines = atCountWrappedLines(t.description, textAreaW, descFontSize);
            if (wrappedLines < 1) wrappedLines = 1;
            int textBlockH   = wrappedLines * (descFontSize + 4) + 8;
            panelH = textBlockH + 52;
        }
        else
        {
            panelH = 50;
        }

        DrawRectangleRounded(
            { (float)panelX, (float)panelY, (float)panelW, (float)panelH },
            0.12f, 8, { 28, 22, 16, 255 });
        DrawRectangle(panelX, panelY, 4, panelH, { 55, 46, 36, 200 });

        if (atIsEditingDescription == true && atExpandedTaskIndex == taskIdx)
        {
            int editBoxX = panelX + 16;
            int editBoxY = panelY + 12;
            int editBoxW = panelW - 32;
            int editBoxH = 68;

            DrawRectangleRounded(
                { (float)editBoxX, (float)editBoxY, (float)editBoxW, (float)editBoxH },
                0.12f, 8, AT_PANEL_LT);
            DrawRectangleRoundedLines(
                { (float)editBoxX, (float)editBoxY, (float)editBoxW, (float)editBoxH },
                0.12f, 8, 1.5f, AT_ORANGE);

            int editDescLen = 0;
            while (atDescriptionEditBuffer[editDescLen] != '\0')
            {
                editDescLen = editDescLen + 1;
            }

            if (editDescLen == 0)
            {
                DrawText("Type a description...", editBoxX + 10, editBoxY + 10, descFontSize, AT_DARK);
            }
            else
            {
                atDrawWrappedText(atDescriptionEditBuffer, editBoxX + 10, editBoxY + 8,
                                  editBoxW - 20, descFontSize, AT_WHITE);
            }

            if ((int)(GetTime() * 2) % 2 == 0 && !atShowAddModal)
            {
                int cx = editBoxX + 10 + MeasureText(atDescriptionEditBuffer, descFontSize);
                if (cx > editBoxX + editBoxW - 8)
                    cx = editBoxX + editBoxW - 8;
                DrawRectangle(cx + 2, editBoxY + 10, 2, descFontSize, AT_ORANGE_LT);
            }

            if (!atSearchFocused && !atShowAddModal)
            {
                int typedChar = GetCharPressed();
                while (typedChar > 0)
                {
                    if (typedChar >= 32 && typedChar <= 126 && editDescLen < 511)
                    {
                        atDescriptionEditBuffer[editDescLen] = (char)typedChar;
                        editDescLen = editDescLen + 1;
                        atDescriptionEditBuffer[editDescLen] = '\0';
                    }
                    typedChar = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && editDescLen > 0)
                {
                    atDescriptionEditBuffer[editDescLen - 1] = '\0';
                }
            }

            int saveBtnX   = panelX + panelW - 200;
            int cancelBtnX = panelX + panelW - 96;
            int btnsY      = panelY + 88;

            if (atDrawSmallBtn("Save", saveBtnX, btnsY, 96, 34))
            {
                int copyLen = 0;
                while (atDescriptionEditBuffer[copyLen] != '\0')
                {
                    copyLen = copyLen + 1;
                }
                if (copyLen > 511) copyLen = 511;

                int k = 0;
                while (k < copyLen)
                {
                    t.description[k] = atDescriptionEditBuffer[k];
                    k = k + 1;
                }
                t.description[k] = '\0';

                updateTaskDescription(t.id, t.description);
                atIsEditingDescription = false;
            }

            if (atDrawGhostBtn("Cancel", cancelBtnX, btnsY, 88, 34))
            {
                atIsEditingDescription = false;
            }
        }
        else if (t.description[0] != '\0')
        {
            atDrawWrappedText(t.description, panelX + 16, panelY + 10,
                              textAreaW, descFontSize, AT_GREY);

            int editBtnY = panelY + panelH - 42;
            if (atDrawGhostBtn("Edit description", panelX + 16, editBtnY, 160, 32))
            {
                int copyLen = 0;
                while (t.description[copyLen] != '\0')
                {
                    copyLen = copyLen + 1;
                }
                if (copyLen > 511) copyLen = 511;

                int k = 0;
                while (k < copyLen)
                {
                    atDescriptionEditBuffer[k] = t.description[k];
                    k = k + 1;
                }
                atDescriptionEditBuffer[k] = '\0';

                atIsEditingDescription = true;
                atSearchFocused        = false;
            }
        }
        else
        {
            if (atDrawGhostBtn("+ Add a description", panelX + 16, panelY + 10, 180, 32))
            {
                atDescriptionEditBuffer[0] = '\0';
                atIsEditingDescription     = true;
                atSearchFocused            = false;
            }
        }

        totalHeight = rowH + 2 + panelH + 6;
    }

    return totalHeight;
}

static void atDrawStreakFlame(int x, int y)
{
    if (atStreak <= 0)
    {
        DrawText("No streak yet", x, y + 6, 16, AT_DARK);
        return;
    }
    char buf[32] = {};
    int bufPos = 0;
    bufPos = appendNumber(buf, bufPos, atStreak);
    bufPos = appendText(buf, bufPos, "-day streak");
    DrawCircle(x + 12, y + 14, 10, AT_ORANGE);
    DrawCircle(x + 12, y + 10,  8, AT_ORANGE_LT);
    DrawCircle(x + 12, y + 8,   4, AT_WHITE);
    DrawText(buf, x + 30, y + 6, 18, AT_ORANGE_LT);
}

static int atCountSelected()
{
    int n = 0;
    int i = 0;
    while (i < getTaskCount())
    {
        if (atSelected[i]) n = n + 1;
        i = i + 1;
    }
    return n;
}

static void atClearSelection()
{
    int i = 0;
    while (i < MAX_TASKS)
    {
        atSelected[i] = false;
        i = i + 1;
    }
}

static void atBulkMarkDone()
{
    Task* tasks = getTaskStore();
    int i = 0;
    while (i < getTaskCount())
    {
        if (atSelected[i] && !tasks[i].completed)
        {
            tasks[i].completed  = true;
            atFadeTimer[i]      = 1.0f;
            atBumpStreakOnComplete();
            updateTaskCompleted(tasks[i].id, true);
            atSpawnRepeatCopy(tasks[i]);
        }
        i = i + 1;
    }
    atClearSelection();
}

static void atBulkSnooze()
{
    Task* tasks = getTaskStore();
    int i = 0;
    while (i < getTaskCount())
    {
        if (atSelected[i])
        {
            atSnoozeOneDay(tasks[i].deadline);
            updateTaskDeadline(tasks[i].id, tasks[i].deadline);
        }
        i = i + 1;
    }
    atClearSelection();
}

static void atBulkDelete()
{
    Task* tasks = getTaskStore();
    int i = getTaskCount() - 1;
    while (i >= 0)
    {
        if (atSelected[i])
        {
            if (atExpandedTaskIndex == i)
            {
                atExpandedTaskIndex    = -1;
                atIsEditingDescription = false;
            }
            deleteTaskFromDb(tasks[i].id);
            removeTask(i);

            int j = i;
            while (j < MAX_TASKS - 1)
            {
                atFadeTimer[j] = atFadeTimer[j + 1];
                atSelected[j]  = atSelected[j + 1];
                j = j + 1;
            }
            atFadeTimer[MAX_TASKS - 1] = 0.0f;
            atSelected[MAX_TASKS - 1]  = false;
        }
        i = i - 1;
    }
}

static void atDrawHeatCell(int x, int y, int w, int h,
                            const char* title, const char* sub,
                            Color tint, int* taskIdx, int n)
{
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    DrawRectangleRounded(r, 0.06f, 8, AT_PANEL);
    DrawRectangleRoundedLines(r, 0.06f, 8, 1.5f, AT_BORDER);
    DrawRectangle(x, y, 4, h, tint);

    Color stripe = tint;
    stripe.a = 60;
    DrawRectangle(x + 4, y, w - 4, 36, stripe);
    DrawText(title, x + 16, y + 8, 18, AT_WHITE);
    DrawText(sub,   x + 16, y + h - 24, 12, AT_DARK);

    Task* tasks  = getTaskStore();
    int   rowY   = y + 48;
    int   maxRows = (h - 80) / 26;
    int   shown  = 0;
    int   i      = 0;
    while (i < n && shown < maxRows)
    {
        Task& t  = tasks[taskIdx[i]];
        int days = atDaysFromToday(t.deadline);
        int pr   = (t.priority >= 1 && t.priority <= 3) ? t.priority : 1;
        Color dot = AT_PRIO_COLOR[pr];
        DrawCircle(x + 18, rowY + 8, 4, dot);
        char clipped[64] = {};
        int clipIndex = 0;
        while (clipIndex < 40 && t.title[clipIndex] != '\0')
        {
            clipped[clipIndex] = t.title[clipIndex];
            clipIndex = clipIndex + 1;
        }
        clipped[clipIndex] = '\0';
        DrawText(clipped, x + 30, rowY, 14, AT_WHITE);
        char tail[32] = {};
        int tailPos = 0;
        if (days < 0)
        {
            int daysLate = -days;
            tailPos = appendNumber(tail, tailPos, daysLate);
            tailPos = appendText(tail, tailPos, "d late");
        }
        else if (days == 0)
        {
            tailPos = appendText(tail, tailPos, "today");
        }
        else
        {
            tailPos = appendNumber(tail, tailPos, days);
            tailPos = appendText(tail, tailPos, "d");
        }
        DrawText(tail, x + w - 60, rowY, 13, AT_GREY);
        rowY  = rowY + 26;
        shown = shown + 1;
        i     = i + 1;
    }
    if (n == 0)
    {
        DrawText("(empty)", x + 18, y + 56, 14, AT_DARK);
    }
    else if (shown < n)
    {
        char more[32] = {};
        int morePos = 0;
        int moreCount = n - shown;
        morePos = appendText(more, morePos, "+");
        morePos = appendNumber(more, morePos, moreCount);
        morePos = appendText(more, morePos, " more");
        DrawText(more, x + 18, rowY, 13, AT_DARK);
    }
}

static void atOpenAddModal();
static void atDrawAddModal(int sw, int sh);

void drawAllTasksScreen(int contentX, int contentWidth, int screenHeight)
{
    DrawRectangle(contentX, 0, contentWidth, screenHeight, AT_BG);

    int margin = 40;
    int innerX = contentX + margin;
    int innerW = contentWidth - margin * 2;

    const char* categoryFilter = getCurrentCategoryFilter();
    char pageTitle[128];
    if (categoryFilter[0] == '\0')
    {
        copyText(pageTitle, "All Tasks");
    }
    else
    {
        copyText(pageTitle, categoryFilter);
    }

    DrawText(pageTitle, innerX, margin, 32, AT_WHITE);
    atDrawStreakFlame(innerX + innerW - 420, margin + 4);

    int addBtnW = 140, addBtnH = 38;
    int addBtnX = innerX + innerW - addBtnW;
    int addBtnY = margin + 4;
    if (atDrawSmallBtn("+ New Task", addBtnX, addBtnY, addBtnW, addBtnH))
        atOpenAddModal();

    int pillY = margin + 64;
    int pillH = 32;
    int px    = innerX;
    int i     = 0;
    while (i < ATF_COUNT)
    {
        int w = 0;
        if (atDrawPill(AT_FILTER_LABELS[i], px, pillY, pillH, atFilter == i, &w))
            atFilter = (AT_Filter)i;
        px = px + w + 8;
        i  = i + 1;
    }

    int viewBtnW = 96;
    int viewX    = innerX + innerW - viewBtnW * 2 - 8;
    if (atDrawPill("List",    viewX,                pillY, pillH, atView == ATV_LIST,    nullptr))
        atView = ATV_LIST;
    if (atDrawPill("Heatmap", viewX + viewBtnW + 8, pillY, pillH, atView == ATV_HEATMAP, nullptr))
        atView = ATV_HEATMAP;

    int sortPillY = pillY + pillH + 8;
    int sortPillH = 28;
    int sortPillX = innerX;

    const char* sortLabels[4] = { "Deadline", "Priority", "Title", "Duration" };
    int sortIndex = 0;
    while (sortIndex < 4)
    {
        int sortPillW = 0;
        if (atDrawPill(sortLabels[sortIndex], sortPillX, sortPillY, sortPillH, atSortMode == sortIndex, &sortPillW))
            atSortMode = sortIndex;
        sortPillX = sortPillX + sortPillW + 8;
        sortIndex = sortIndex + 1;
    }

    int searchY = sortPillY + sortPillH + 12;
    atDrawSearchBox(innerX, searchY, innerW - 280, 38);

    Task* tasks      = getTaskStore();
    int   taskCount  = getTaskCount();
    int   quickCount = 0;
    i = 0;
    while (i < taskCount)
    {
        if (!tasks[i].completed && atIsQuick(tasks[i].duration))
            quickCount = quickCount + 1;
        i = i + 1;
    }
    int hintX = innerX + innerW - 268;
    int hintY = searchY + 8;
    if (quickCount > 0)
    {
        DrawCircle(hintX + 10, hintY + 10, 6, AT_GREEN);
        char hint[64] = {};
        int hintPos = 0;
        hintPos = appendNumber(hint, hintPos, quickCount);
        if (quickCount == 1)
        {
            hintPos = appendText(hint, hintPos, " quick task right now");
        }
        else
        {
            hintPos = appendText(hint, hintPos, " quick tasks right now");
        }
        DrawText(hint, hintX + 24, hintY + 2, 16, AT_GREEN);
    }
    else
    {
        DrawText("No quick tasks pending", hintX, hintY + 2, 14, AT_DARK);
    }

    int bulkY = searchY + 50;
    int bulkH = 0;
    int sel   = atCountSelected();
    if (sel > 0)
    {
        bulkH = 50;
        Rectangle br = { (float)innerX, (float)bulkY, (float)innerW, (float)bulkH };
        DrawRectangleRounded(br, 0.18f, 8, AT_PANEL_LT);
        DrawRectangleRoundedLines(br, 0.18f, 8, 1.5f, AT_ORANGE);
        char selTxt[32] = {};
        int selTxtPos = 0;
        selTxtPos = appendNumber(selTxt, selTxtPos, sel);
        selTxtPos = appendText(selTxt, selTxtPos, " selected");
        DrawText(selTxt, innerX + 16, bulkY + bulkH / 2 - 9, 18, AT_WHITE);
        int bw = 130, bh = 32;
        int bx = innerX + innerW - bw * 4 - 24;
        int by = bulkY + bulkH / 2 - bh / 2;
        if (atDrawSmallBtn("Mark Done",  bx,                by, bw, bh)) atBulkMarkDone();
        if (atDrawSmallBtn("Snooze +1d", bx + (bw + 8),     by, bw, bh)) atBulkSnooze();
        if (atDrawSmallBtn("Delete",     bx + (bw + 8) * 2, by, bw, bh, true)) atBulkDelete();
        if (atDrawGhostBtn("Cancel",     bx + (bw + 8) * 3, by, bw, bh)) atClearSelection();
    }

    taskCount = getTaskCount();

    int contentTop = bulkY + (bulkH > 0 ? bulkH + 12 : 8);
    int contentH   = screenHeight - contentTop - 24;
    if (contentH < 80) contentH = 80;

    int idxs[MAX_TASKS];
    int nIdx = 0;
    i = 0;
    while (i < taskCount)
    {
        if (!atMatchesFilter(tasks[i]))     { i = i + 1; continue; }
        if (!atMatchesSearch(tasks[i]))     { i = i + 1; continue; }
        if (!atMatchesCategory(tasks[i]))   { i = i + 1; continue; }
        if (tasks[i].completed && atFadeTimer[i] == 0.0f && atFilter == ATF_ACTIVE)
        {
            i = i + 1;
            continue;
        }
        idxs[nIdx] = i;
        nIdx = nIdx + 1;
        i    = i + 1;
    }

    float dt = GetFrameTime();
    i = 0;
    while (i < taskCount)
    {
        if (atFadeTimer[i] > 0.0f)
        {
            atFadeTimer[i] = atFadeTimer[i] - dt;
            if (atFadeTimer[i] < 0.0f)
                atFadeTimer[i] = 0.0f;
        }
        i = i + 1;
    }

    int k = 0;

    if (atView == ATV_LIST)
    {
        int buckets[ATG_COUNT][MAX_TASKS];
        int bn[ATG_COUNT] = {};
        k = 0;
        while (k < nIdx)
        {
            int idx    = idxs[k];
            AT_Group g = atGroupFor(tasks[idx]);
            buckets[g][bn[g]] = idx;
            bn[g] = bn[g] + 1;
            k = k + 1;
        }

        int g = 0;
        while (g < ATG_COUNT)
        {
            int a = 0;
            while (a < bn[g] - 1)
            {
                int b = a + 1;
                while (b < bn[g])
                {
                    int sortDifference = 0;
                    if (atSortMode == 0)
                    {
                        sortDifference = compareTexts(tasks[buckets[g][a]].deadline,
                                                     tasks[buckets[g][b]].deadline);
                    }
                    else if (atSortMode == 1)
                    {
                        sortDifference = tasks[buckets[g][b]].priority - tasks[buckets[g][a]].priority;
                    }
                    else if (atSortMode == 2)
                    {
                        sortDifference = compareTexts(tasks[buckets[g][a]].title,
                                                     tasks[buckets[g][b]].title);
                    }
                    else if (atSortMode == 3)
                    {
                        sortDifference = tasks[buckets[g][a]].duration - tasks[buckets[g][b]].duration;
                    }

                    if (sortDifference > 0)
                    {
                        int tmp       = buckets[g][a];
                        buckets[g][a] = buckets[g][b];
                        buckets[g][b] = tmp;
                    }
                    b = b + 1;
                }
                a = a + 1;
            }
            g = g + 1;
        }

        if (atHoverIn({ (float)innerX, (float)contentTop, (float)innerW, (float)contentH }))
        {
            float wheel = GetMouseWheelMove();
            atScroll = atScroll - wheel * 36;
        }

        BeginScissorMode(innerX, contentTop, innerW, contentH);
        int  curY      = contentTop - (int)atScroll;
        bool empty     = true;

        g = 0;
        while (g < ATG_COUNT)
        {
            if (bn[g] == 0)
            {
                g = g + 1;
                continue;
            }
            empty = false;

            Color hcol = AT_ORANGE_LT;
            if (g == ATG_OVERDUE) hcol = AT_RED;
            if (g == ATG_DONE)    hcol = AT_DARK;
            char hdr[64] = {};
            int hdrPos = 0;
            hdrPos = appendText(hdr, hdrPos, AT_GROUP_LABELS[g]);
            hdrPos = appendText(hdr, hdrPos, "  (");
            hdrPos = appendNumber(hdr, hdrPos, bn[g]);
            hdrPos = appendText(hdr, hdrPos, ")");
            DrawText(hdr, innerX, curY, 18, hcol);
            DrawRectangle(innerX, curY + 28, innerW, 1, AT_BORDER);
            curY = curY + 40;

            k = 0;
            while (k < bn[g])
            {
                int idx    = buckets[g][k];
                float alpha = 1.0f;
                if (tasks[idx].completed)
                {
                    if (atFadeTimer[idx] > 0.0f)
                        alpha = atFadeTimer[idx];
                    else if (atFilter == ATF_ALL)
                        alpha = 0.45f;
                }
                bool isExpanded = (idx == atExpandedTaskIndex);
                int rh = atDrawTaskRow(idx, innerX, curY, innerW, alpha, isExpanded);
                curY = curY + rh + 8;
                k    = k + 1;
            }
            curY = curY + 8;
            g    = g + 1;
        }
        EndScissorMode();

        if (empty)
        {
            const char* msg;
            if (atSearchBuf[0] || atFilter != ATF_ALL)
            {
                msg = "No tasks match your filter.";
            }
            else if (categoryFilter[0] != '\0')
            {
                msg = "No tasks in this list yet. Click \"+ New Task\" to add one.";
            }
            else if (taskCount == 0)
            {
                msg = "No tasks yet. Click \"+ New Task\" to add one.";
            }
            else
            {
                msg = "Nothing here yet.";
            }
            DrawText(msg, innerX, contentTop + 20, 18, AT_DARK);
        }

        int totalH    = (curY + (int)atScroll) - contentTop;
        int maxScroll = totalH - contentH;
        if (maxScroll < 0) maxScroll = 0;
        if (atScroll < 0)          atScroll = 0;
        if (atScroll > maxScroll)  atScroll = (float)maxScroll;
    }
    else
    {
        int q[4][MAX_TASKS];
        int qn[4] = {};
        k = 0;
        while (k < nIdx)
        {
            int idx = idxs[k];
            if (tasks[idx].completed) { k = k + 1; continue; }
            int days = atDaysFromToday(tasks[idx].deadline);
            int u    = atHighUrgency(days) ? 1 : 0;
            int e    = atLowEffort(tasks[idx].duration) ? 0 : 1;
            int qi   = (u == 1 && e == 0) ? 0
                     : (u == 1 && e == 1) ? 1
                     : (u == 0 && e == 0) ? 2
                                          : 3;
            q[qi][qn[qi]] = idx;
            qn[qi] = qn[qi] + 1;
            k = k + 1;
        }

        int gap   = 12;
        int cellW = (innerW - gap) / 2;
        int cellH = (contentH - gap) / 2;

        atDrawHeatCell(innerX,               contentTop,
                       cellW, cellH,
                       "Quick wins",
                       "high urgency · low effort", AT_GREEN, q[0], qn[0]);
        atDrawHeatCell(innerX + cellW + gap, contentTop,
                       cellW, cellH,
                       "Plan & focus",
                       "high urgency · high effort", AT_ORANGE_LT, q[1], qn[1]);
        atDrawHeatCell(innerX,               contentTop + cellH + gap,
                       cellW, cellH,
                       "Fill-ins",
                       "low urgency · low effort", AT_BLUE, q[2], qn[2]);
        atDrawHeatCell(innerX + cellW + gap, contentTop + cellH + gap,
                       cellW, cellH,
                       "Procrastination graveyard",
                       "low urgency · high effort", AT_RED_DIM, q[3], qn[3]);
    }

    if (atShowAddModal)
        atDrawAddModal(GetScreenWidth(), GetScreenHeight());
}

static void atOpenAddModal()
{
    atShowAddModal      = true;
    atSearchFocused     = false;
    atIsEditingDescription = false;
    atNewFocusField     = 0;
    atNewPriority       = 2;
    atNewRepeatType     = 0;
    atNewRepeatDays[0]  = '\0';
    atNewError[0]       = '\0';
    atNewTitle[0]       = '\0';
    atNewDuration[0]    = '\0';

    const char* currentFilter = getCurrentCategoryFilter();
    atNewCategoryIndex = -1;
    if (currentFilter[0] != '\0')
    {
        Category* cats = getCategoryStore();
        int count      = getCategoryCount();
        int i = 0;
        while (i < count)
        {
            if (textsAreEqual(cats[i].name, currentFilter) == true)
            {
                atNewCategoryIndex = i;
                break;
            }
            i = i + 1;
        }
    }

    time_t tt      = time(NULL) + 7 * 24 * 3600;
    struct tm tm_  = *localtime(&tt);
    int newDeadlineYear  = tm_.tm_year + 1900;
    int newDeadlineMonth = tm_.tm_mon + 1;
    int newDeadlineDay   = tm_.tm_mday;
    buildDateText(atNewDeadline, newDeadlineYear, newDeadlineMonth, newDeadlineDay);
}

static bool atDrawTextField(const char* label, char* buf, int bufMax,
                             int x, int y, int w, int h, int fieldIndex,
                             const char* placeholder)
{
    Rectangle r   = { (float)x, (float)y, (float)w, (float)h };
    bool focused  = (atNewFocusField == fieldIndex);
    if (atClickIn(r))
        atNewFocusField = fieldIndex;

    DrawText(label, x, y - 22, 14, AT_GREY);

    DrawRectangleRounded(r, 0.25f, 8, focused ? AT_PANEL_LT : AT_PANEL);
    DrawRectangleRoundedLines(r, 0.25f, 8, 1.5f, focused ? AT_ORANGE : AT_BORDER);

    int len = textLength(buf);
    if (len == 0 && !focused)
    {
        DrawText(placeholder, x + 12, y + h / 2 - 9, 16, AT_DARK);
    }
    else
    {
        DrawText(buf, x + 12, y + h / 2 - 9, 16, AT_WHITE);
    }
    if (focused && ((int)(GetTime() * 2) % 2 == 0))
    {
        int cx = x + 12 + MeasureText(buf, 16);
        DrawRectangle(cx + 2, y + 8, 2, h - 16, AT_ORANGE_LT);
    }

    if (focused)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key >= 32 && key <= 126 && len < bufMax - 1)
            {
                buf[len] = (char)key;
                len = len + 1;
                buf[len] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
            buf[len - 1] = '\0';
    }
    return atHoverIn(r);
}

static void atDrawAddModal(int sw, int sh)
{
    DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 170 });

    int pw = 520;
    int ph = 580;
    int px = sw / 2 - pw / 2;
    int py = sh / 2 - ph / 2;

    DrawRectangleRounded(
        { (float)(px - 2), (float)(py - 2), (float)(pw + 4), (float)(ph + 4) },
        0.06f, 8, AT_ORANGE);
    DrawRectangleRounded(
        { (float)px, (float)py, (float)pw, (float)ph },
        0.06f, 8, AT_PANEL);

    const char* title = "New Task";
    DrawText(title, px + pw / 2 - MeasureText(title, 26) / 2, py + 24, 26, AT_WHITE);

    int fieldX = px + 28;
    int fieldW = pw - 56;
    int fieldH = 40;
    int curY   = py + 84;

    atDrawTextField("Title", atNewTitle, sizeof(atNewTitle),
                    fieldX, curY, fieldW, fieldH, 0, "What needs doing?");
    curY = curY + fieldH + 36;

    DrawText("Priority", fieldX, curY - 22, 14, AT_GREY);
    const char* prioLabels[3] = { "Low", "Medium", "High" };
    int prioX = fieldX;
    int pi = 0;
    while (pi < 3)
    {
        int pillW = 0;
        if (atDrawPill(prioLabels[pi], prioX, curY, fieldH - 8, atNewPriority == pi + 1, &pillW))
            atNewPriority = pi + 1;
        prioX = prioX + pillW + 8;
        pi    = pi + 1;
    }
    curY = curY + fieldH + 36;

    int halfW = (fieldW - 16) / 2;
    atDrawTextField("Deadline (YYYY-MM-DD)", atNewDeadline, sizeof(atNewDeadline),
                    fieldX, curY, halfW, fieldH, 1, "2026-05-17");
    atDrawTextField("Duration (minutes)", atNewDuration, sizeof(atNewDuration),
                    fieldX + halfW + 16, curY, halfW, fieldH, 2, "30");
    curY = curY + fieldH + 36;

    DrawText("Repeat", fieldX, curY - 22, 14, AT_GREY);
    const char* repeatLabels[5] = { "None", "Daily", "Weekly", "Monthly", "Every N days" };
    int repeatX = fieldX;
    int ri = 0;
    while (ri < 5)
    {
        int pillW = 0;
        if (atDrawPill(repeatLabels[ri], repeatX, curY, fieldH - 8, atNewRepeatType == ri, &pillW))
            atNewRepeatType = ri;
        repeatX = repeatX + pillW + 8;
        ri      = ri + 1;
    }
    curY = curY + fieldH;

    if (atNewRepeatType == 4)
    {
        atDrawTextField("Days between repeats", atNewRepeatDays, sizeof(atNewRepeatDays),
                        fieldX, curY + 14, halfW, fieldH, 3, "e.g. 3");
        curY = curY + fieldH + 14;
    }
    curY = curY + 22;

    int numberOfCategories = getCategoryCount();
    if (numberOfCategories > 0)
    {
        DrawText("List (optional)", fieldX, curY - 22, 14, AT_GREY);
        Category* cats = getCategoryStore();

        int catPillX   = fieldX;
        int catPillY   = curY;
        int catPillH   = fieldH - 8;
        int maxPillX   = fieldX + fieldW;

        int noneW = 0;
        if (atDrawPill("None", catPillX, catPillY, catPillH, atNewCategoryIndex == -1, &noneW))
            atNewCategoryIndex = -1;
        catPillX = catPillX + noneW + 8;

        int ci = 0;
        while (ci < numberOfCategories)
        {
            int estimatedW = MeasureText(cats[ci].name, 16) + 28;
            if (catPillX + estimatedW > maxPillX)
            {
                catPillX = fieldX;
                catPillY = catPillY + catPillH + 8;
            }

            int pillW = 0;
            if (atDrawPill(cats[ci].name, catPillX, catPillY, catPillH, atNewCategoryIndex == ci, &pillW))
                atNewCategoryIndex = ci;
            catPillX = catPillX + pillW + 8;
            ci       = ci + 1;
        }

        curY = catPillY + catPillH + 28;
    }

    if (atNewError[0])
    {
        DrawText(atNewError, fieldX, curY, 14, AT_RED);
    }

    int btnW = 130, btnH = 44;
    int btnY = py + ph - btnH - 22;

    if (atDrawSmallBtn("Save Task", px + pw - btnW * 2 - 16, btnY, btnW, btnH))
    {
        if (atNewTitle[0] == '\0')
        {
            copyText(atNewError, "Please enter a title.");
        }
        else if (textLength(atNewDeadline) != 10 ||
                 atNewDeadline[4] != '-' ||
                 atNewDeadline[7] != '-')
        {
            copyText(atNewError, "Deadline must look like YYYY-MM-DD.");
        }
        else if (atNewRepeatType == 4 && atNewRepeatDays[0] == '\0')
        {
            copyText(atNewError, "Enter how many days between repeats.");
        }
        else
        {
            Task newTask = {};
            copyText(newTask.title,    atNewTitle);
            copyText(newTask.deadline, atNewDeadline);
            newTask.priority  = atNewPriority;
            newTask.duration  = atoi(atNewDuration);
            newTask.completed = false;
            newTask.repeatType = atNewRepeatType;

            if (atNewRepeatType == 1) newTask.repeatInterval = 1;
            if (atNewRepeatType == 2) newTask.repeatInterval = 7;
            if (atNewRepeatType == 3) newTask.repeatInterval = 30;
            if (atNewRepeatType == 4) newTask.repeatInterval = atoi(atNewRepeatDays);
            if (newTask.repeatInterval <= 0 && atNewRepeatType > 0)
                newTask.repeatInterval = 1;

            if (atNewCategoryIndex >= 0 && atNewCategoryIndex < getCategoryCount())
            {
                Category* cats = getCategoryStore();
                int copyLen    = 0;
                while (copyLen < 63 && cats[atNewCategoryIndex].name[copyLen] != '\0')
                {
                    newTask.categoryName[copyLen] = cats[atNewCategoryIndex].name[copyLen];
                    copyLen = copyLen + 1;
                }
                newTask.categoryName[copyLen] = '\0';
            }
            else
            {
                newTask.categoryName[0] = '\0';
            }

            newTask.description[0] = '\0';

            const char* user  = getLoggedInUser();
            int         newId = saveNewTask(user, newTask);
            if (newId > 0)
            {
                newTask.id     = newId;
                addTask(newTask);
                atShowAddModal = false;
                atNewError[0]  = '\0';
            }
            else
            {
                copyText(atNewError, "Could not save task. Try again.");
            }
        }
    }

    if (atDrawGhostBtn("Cancel", px + pw - btnW - 16, btnY, btnW, btnH) ||
        IsKeyPressed(KEY_ESCAPE))
    {
        atShowAddModal = false;
        atNewError[0]  = '\0';
    }

    if (IsKeyPressed(KEY_TAB))
    {
        int maxField = (atNewRepeatType == 4) ? 4 : 3;
        atNewFocusField = (atNewFocusField + 1) % maxField;
    }
}
