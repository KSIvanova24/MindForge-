#include "../include/screens/tasks.h"
#include "../include/data.h"
#include "../include/db.h"
#include "../include/accounts.h"
#include "raylib.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

static const Color AT_BG          = {  22,  18,  14, 255 };
static const Color AT_PANEL       = {  35,  28,  20, 255 };
static const Color AT_PANEL_LT    = {  46,  37,  27, 255 };
static const Color AT_BORDER      = {  55,  46,  36, 255 };
static const Color AT_WHITE       = { 255, 255, 255, 255 };
static const Color AT_GREY        = { 180, 160, 140, 255 };
static const Color AT_DARK        = { 100,  85,  70, 255 };
static const Color AT_ORANGE      = { 234, 108,  15, 255 };
static const Color AT_ORANGE_LT   = { 251, 146,  60, 255 };
static const Color AT_RED         = { 220,  80,  60, 255 };
static const Color AT_RED_DIM     = { 120,  40,  30, 255 };
static const Color AT_GREEN       = {  80, 200, 100, 255 };
static const Color AT_BLUE        = {  90, 140, 220, 255 };

enum AT_Filter   { ATF_ALL = 0, ATF_ACTIVE, ATF_DONE, ATF_OVERDUE, ATF_HIGHPRI, ATF_COUNT };
enum AT_View     { ATV_LIST = 0, ATV_HEATMAP };

static const char* AT_FILTER_LABELS[ATF_COUNT] = {
    "All", "Active", "Done", "Overdue", "High Priority"
};

static bool   atSelected[MAX_TASKS]   = {};
static float  atFadeTimer[MAX_TASKS]  = {};

static AT_Filter   atFilter         = ATF_ALL;
static AT_View     atView           = ATV_LIST;
static char        atSearchBuf[64]  = {};
static bool        atSearchFocused  = false;
static float       atScroll         = 0.0f;
static int         atStreak         = 0;
static int         atLastCompleteDay = 0;

static bool  atShowAddModal      = false;
static char  atNewTitle[128]     = {};
static char  atNewDeadline[16]   = {};
static int   atNewPriority       = 2;
static char  atNewDuration[8]    = {};
static int   atNewFocusField     = 0;
static char  atNewError[128]     = {};

static int atParseToDay(const char* s)
{
    int y = 0, m = 0, d = 0;
    if (!s || sscanf(s, "%d-%d-%d", &y, &m, &d) != 3) return 0;
    struct tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon  = m - 1;
    t.tm_mday = d;
    t.tm_hour = 12;
    time_t tt = mktime(&t);
    return (int)(tt / 86400);
}

static int atTodayDay()
{
    time_t now = time(NULL);
    struct tm t = *localtime(&now);
    t.tm_hour = 12; t.tm_min = 0; t.tm_sec = 0;
    return (int)(mktime(&t) / 86400);
}

static int atDaysFromToday(const char* deadline)
{
    int d = atParseToDay(deadline);
    if (d == 0) return 9999;
    return d - atTodayDay();
}

static const char* atTodayStr()
{
    static char buf[16];
    time_t now = time(NULL);
    struct tm t = *localtime(&now);
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    return buf;
}

static void atSnoozeOneDay(char* deadline)
{
    int y = 0, m = 0, d = 0;
    if (sscanf(deadline, "%d-%d-%d", &y, &m, &d) != 3) return;
    struct tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon  = m - 1;
    t.tm_mday = d + 1;
    t.tm_hour = 12;
    mktime(&t);
    snprintf(deadline, 16, "%04d-%02d-%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
}

static const char* atDurationLabel(int dur)
{
    if (dur <= 0)  return "";
    if (dur <= 15) return "5-15m";
    if (dur <= 60) return "30m-1h";
    return "1h+";
}

static bool atIsQuick(int dur)         { return dur > 0 && dur <= 15; }
static bool atLowEffort(int dur)       { return dur > 0 && dur <= 30; }
static bool atHighUrgency(int days)    { return days <= 3; }

static void atBumpStreakOnComplete()
{
    int today = atTodayDay();
    if (atLastCompleteDay == 0)         atStreak = 1;
    else if (today == atLastCompleteDay) ;
    else if (today == atLastCompleteDay + 1) atStreak++;
    else                                  atStreak = 1;
    atLastCompleteDay = today;
}

static void atLower(const char* in, char* out, int outMax)
{
    int i = 0;
    while (in[i] && i < outMax - 1) {
        char c = in[i];
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
        out[i] = c;
        i++;
    }
    out[i] = '\0';
}

static bool atMatchesSearch(const Task& t)
{
    if (atSearchBuf[0] == '\0') return true;
    char a[128], b[64];
    atLower(t.title, a, 128);
    atLower(atSearchBuf, b, 64);
    return strstr(a, b) != nullptr;
}

static bool atMatchesFilter(const Task& t)
{
    int days = atDaysFromToday(t.deadline);
    switch (atFilter) {
        case ATF_ALL:     return true;
        case ATF_ACTIVE:  return !t.completed;
        case ATF_DONE:    return  t.completed;
        case ATF_OVERDUE: return !t.completed && days < 0;
        case ATF_HIGHPRI: return !t.completed && t.priority >= 3;
        default:          return true;
    }
}

enum AT_Group { ATG_OVERDUE, ATG_TODAY, ATG_WEEK, ATG_LATER, ATG_DONE, ATG_COUNT };
static const char* AT_GROUP_LABELS[ATG_COUNT] = {
    "Overdue", "Today", "This Week", "Later", "Done"
};

static AT_Group atGroupFor(const Task& t)
{
    if (t.completed) return ATG_DONE;
    int d = atDaysFromToday(t.deadline);
    if (d < 0)  return ATG_OVERDUE;
    if (d == 0) return ATG_TODAY;
    if (d <= 7) return ATG_WEEK;
    return ATG_LATER;
}

static bool atClickIn(Rectangle r)
{

    if (atShowAddModal) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        Rectangle modal = { (float)(sw / 2 - 260), (float)(sh / 2 - 235),
                            520.0f, 470.0f };
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
    int textW = MeasureText(label, 16);
    int w     = textW + 28;
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool hov  = atHoverIn(r);

    Color bg     = active ? AT_ORANGE : (hov ? AT_PANEL_LT : AT_PANEL);
    Color border = active ? AT_ORANGE_LT : AT_BORDER;
    Color txt    = active ? AT_WHITE : (hov ? AT_WHITE : AT_GREY);

    DrawRectangleRounded(r, 0.5f, 8, bg);
    DrawRectangleRoundedLines(r, 0.5f, 8, 1.5f, border);
    DrawText(label, x + 14, y + h / 2 - 8, 16, txt);

    if (outW) *outW = w;
    return atClickIn(r);
}

static bool atDrawSmallBtn(const char* label, int x, int y, int w, int h,
                            bool danger = false)
{
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool hov = atHoverIn(r);
    Color base = danger ? Color{ 180, 50, 30, 255 } : AT_ORANGE;
    Color hl   = danger ? Color{ 220, 70, 50, 255 } : AT_ORANGE_LT;
    DrawRectangleRounded(r, 0.3f, 8, hov ? hl : base);
    int fs = 16, tw = MeasureText(label, fs);
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs, AT_WHITE);
    return atClickIn(r);
}

static bool atDrawGhostBtn(const char* label, int x, int y, int w, int h)
{
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool hov = atHoverIn(r);
    DrawRectangleRoundedLines(r, 0.3f, 8, 1.5f, hov ? AT_ORANGE_LT : AT_BORDER);
    int fs = 16, tw = MeasureText(label, fs);
    DrawText(label, x + w / 2 - tw / 2, y + h / 2 - fs / 2, fs,
             hov ? AT_ORANGE_LT : AT_GREY);
    return atClickIn(r);
}

static void atDrawSearchBox(int x, int y, int w, int h)
{
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    if (atClickIn(r)) atSearchFocused = true;
    else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !atHoverIn(r))
        atSearchFocused = false;

    Color bg = atSearchFocused ? AT_PANEL_LT : AT_PANEL;
    DrawRectangleRounded(r, 0.3f, 8, bg);
    DrawRectangleRoundedLines(r, 0.3f, 8, 1.5f,
        atSearchFocused ? AT_ORANGE : AT_BORDER);

    DrawText("Search", x + 14, y + h / 2 - 8, 16, AT_DARK);

    int len = (int)strlen(atSearchBuf);
    int textX = x + 14 + MeasureText("Search   ", 16);
    if (len == 0 && !atSearchFocused) {
        DrawText("Type to filter tasks...", textX, y + h / 2 - 8, 16, AT_DARK);
    } else {
        DrawText(atSearchBuf, textX, y + h / 2 - 8, 16, AT_WHITE);
        if (atSearchFocused && ((int)(GetTime() * 2) % 2 == 0)) {
            int cx = textX + MeasureText(atSearchBuf, 16);
            DrawRectangle(cx + 2, y + 10, 2, h - 20, AT_ORANGE_LT);
        }
    }

    if (atSearchFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && len < 63) {
                atSearchBuf[len++] = (char)key;
                atSearchBuf[len]   = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
            atSearchBuf[len - 1] = '\0';
        if (IsKeyPressed(KEY_ESCAPE)) {
            atSearchBuf[0] = '\0';
            atSearchFocused = false;
        }
    }
}

static const char* AT_PRIO_LABEL[] = { "?", "Low", "Med", "High" };
static Color       AT_PRIO_COLOR[] = { AT_GREY, AT_GREEN, AT_ORANGE_LT, AT_RED };

static int atDrawTaskRow(int taskIdx, int x, int y, int w, float alpha)
{
    Task* tasks = getTaskStore();
    Task& t = tasks[taskIdx];
    int rowH = 64;

    Color panel = t.completed ? AT_PANEL : AT_PANEL;
    int days = atDaysFromToday(t.deadline);
    bool overdue = !t.completed && days < 0;

    Rectangle r = { (float)x, (float)y, (float)w, (float)rowH };

    Color bg = overdue ? Color{ 60, 28, 24, 255 } : panel;
    bg.a = (unsigned char)(bg.a * alpha);
    DrawRectangleRounded(r, 0.16f, 8, bg);

    int pr = (t.priority >= 1 && t.priority <= 3) ? t.priority : 1;
    Color prCol = AT_PRIO_COLOR[pr];
    prCol.a = (unsigned char)(prCol.a * alpha);
    DrawRectangle(x, y, 4, rowH, prCol);

    int cbSz = 20;
    int cbX  = x + 16;
    int cbY  = y + rowH / 2 - cbSz / 2;
    Rectangle cbR = { (float)cbX, (float)cbY, (float)cbSz, (float)cbSz };
    bool hovCb = atHoverIn(cbR);
    Color cbBorder = atSelected[taskIdx] ? AT_ORANGE_LT : (hovCb ? AT_ORANGE : AT_DARK);
    cbBorder.a = (unsigned char)(cbBorder.a * alpha);
    DrawRectangleRoundedLines(cbR, 0.25f, 8, 1.5f, cbBorder);
    if (atSelected[taskIdx]) {
        Color fill = AT_ORANGE; fill.a = (unsigned char)(fill.a * alpha);
        DrawRectangleRounded({ (float)(cbX+3), (float)(cbY+3),
                               (float)(cbSz-6), (float)(cbSz-6) }, 0.25f, 8, fill);
    }
    if (atClickIn(cbR)) atSelected[taskIdx] = !atSelected[taskIdx];

    Color titleCol = t.completed ? AT_DARK : AT_WHITE;
    titleCol.a = (unsigned char)(titleCol.a * alpha);
    int titleX = cbX + cbSz + 14;
    DrawText(t.title, titleX, y + rowH / 2 - 11, 20, titleCol);
    if (t.completed) {
        int tw = MeasureText(t.title, 20);
        Color line = AT_DARK; line.a = (unsigned char)(line.a * alpha);
        DrawRectangle(titleX, y + rowH / 2, tw, 1, line);
    }

    const char* durLabel = atDurationLabel(t.duration);
    int durX = x + w - 360;
    if (durLabel[0]) {
        int dw = MeasureText(durLabel, 14) + 16;
        Color dBg = atIsQuick(t.duration) ? AT_GREEN : AT_BORDER;
        dBg.a = (unsigned char)(dBg.a * 0.45f * alpha);
        Color dTx = atIsQuick(t.duration) ? AT_GREEN : AT_GREY;
        dTx.a = (unsigned char)(dTx.a * alpha);
        DrawRectangleRounded({ (float)durX, (float)(y + rowH/2 - 12),
                               (float)dw, 24 }, 0.5f, 8, dBg);
        DrawText(durLabel, durX + 8, y + rowH / 2 - 8, 14, dTx);
    }

    Color prTxt = AT_PRIO_COLOR[pr];
    prTxt.a = (unsigned char)(prTxt.a * alpha);
    DrawText(AT_PRIO_LABEL[pr], x + w - 260, y + rowH / 2 - 8, 16, prTxt);

    Color dlCol = overdue ? AT_RED : AT_GREY;
    dlCol.a = (unsigned char)(dlCol.a * alpha);
    DrawText(t.deadline, x + w - 200, y + rowH / 2 - 8, 16, dlCol);
    if (overdue) {
        char od[32];
        snprintf(od, sizeof(od), "%d day%s overdue",
                 -days, (-days) == 1 ? "" : "s");
        Color rd = AT_RED; rd.a = (unsigned char)(rd.a * alpha);
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
    if (t.completed) {
        Color g = AT_GREEN; g.a = (unsigned char)(g.a * alpha);
        DrawCircle(doneX + doneSz / 2, doneY + doneSz / 2, doneSz / 2.0f - 4, g);
    }
    if (atClickIn(doneR) && !t.completed) {
        t.completed = true;
        atFadeTimer[taskIdx] = 1.0f;
        atBumpStreakOnComplete();
        updateTaskCompleted(t.id, true);
    } else if (atClickIn(doneR) && t.completed) {
        t.completed = false;
        atFadeTimer[taskIdx] = 0.0f;
        updateTaskCompleted(t.id, false);
    }

    return rowH;
}

static void atDrawStreakFlame(int x, int y)
{
    if (atStreak <= 0) {
        DrawText("No streak yet", x, y + 6, 16, AT_DARK);
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%d-day streak", atStreak);
    DrawCircle(x + 12, y + 14, 10, AT_ORANGE);
    DrawCircle(x + 12, y + 10, 8,  AT_ORANGE_LT);
    DrawCircle(x + 12, y + 8,  4,  AT_WHITE);
    DrawText(buf, x + 30, y + 6, 18, AT_ORANGE_LT);
}

static int atCountSelected()
{
    int n = 0;
    for (int i = 0; i < getTaskCount(); i++) if (atSelected[i]) n++;
    return n;
}
static void atClearSelection()
{
    for (int i = 0; i < MAX_TASKS; i++) atSelected[i] = false;
}
static void atBulkMarkDone()
{
    Task* tasks = getTaskStore();
    for (int i = 0; i < getTaskCount(); i++) {
        if (atSelected[i] && !tasks[i].completed) {
            tasks[i].completed = true;
            atFadeTimer[i]    = 1.0f;
            atBumpStreakOnComplete();
            updateTaskCompleted(tasks[i].id, true);
        }
    }
    atClearSelection();
}
static void atBulkSnooze()
{
    Task* tasks = getTaskStore();
    for (int i = 0; i < getTaskCount(); i++) {
        if (atSelected[i]) {
            atSnoozeOneDay(tasks[i].deadline);
            updateTaskDeadline(tasks[i].id, tasks[i].deadline);
        }
    }
    atClearSelection();
}
static void atBulkDelete()
{
    Task* tasks = getTaskStore();
    for (int i = getTaskCount() - 1; i >= 0; i--) {
        if (atSelected[i]) {
            deleteTaskFromDb(tasks[i].id);
            removeTask(i);

            for (int j = i; j < MAX_TASKS - 1; j++) {
                atFadeTimer[j] = atFadeTimer[j + 1];
                atSelected[j]  = atSelected[j + 1];
            }
            atFadeTimer[MAX_TASKS - 1] = 0.0f;
            atSelected[MAX_TASKS - 1]  = false;
        }
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

    Color stripe = tint; stripe.a = 60;
    DrawRectangle(x + 4, y, w - 4, 36, stripe);
    DrawText(title, x + 16, y + 8, 18, AT_WHITE);
    DrawText(sub,   x + 16, y + h - 24, 12, AT_DARK);

    Task* tasks = getTaskStore();
    int rowY = y + 48;
    int maxRows = (h - 80) / 26;
    int shown = 0;
    for (int i = 0; i < n && shown < maxRows; i++) {
        Task& t = tasks[taskIdx[i]];
        int days = atDaysFromToday(t.deadline);
        int pr = (t.priority >= 1 && t.priority <= 3) ? t.priority : 1;
        Color dot = AT_PRIO_COLOR[pr];
        DrawCircle(x + 18, rowY + 8, 4, dot);
        char clipped[64];
        snprintf(clipped, sizeof(clipped), "%.40s",
                 t.title);
        DrawText(clipped, x + 30, rowY, 14, AT_WHITE);
        char tail[32];
        if (days < 0)       snprintf(tail, sizeof(tail), "%dd late", -days);
        else if (days == 0) snprintf(tail, sizeof(tail), "today");
        else                snprintf(tail, sizeof(tail), "%dd",      days);
        DrawText(tail, x + w - 60, rowY, 13, AT_GREY);
        rowY += 26;
        shown++;
    }
    if (n == 0) {
        DrawText("(empty)", x + 18, y + 56, 14, AT_DARK);
    } else if (shown < n) {
        char more[32];
        snprintf(more, sizeof(more), "+%d more", n - shown);
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

    DrawText("All Tasks", innerX, margin, 32, AT_WHITE);
    atDrawStreakFlame(innerX + innerW - 420, margin + 4);

    int addBtnW = 140, addBtnH = 38;
    int addBtnX = innerX + innerW - addBtnW;
    int addBtnY = margin + 4;
    if (atDrawSmallBtn("+ New Task", addBtnX, addBtnY, addBtnW, addBtnH))
        atOpenAddModal();

    int pillY = margin + 64;
    int pillH = 32;
    int px = innerX;
    for (int i = 0; i < ATF_COUNT; i++) {
        int w = 0;
        if (atDrawPill(AT_FILTER_LABELS[i], px, pillY, pillH, atFilter == i, &w))
            atFilter = (AT_Filter)i;
        px += w + 8;
    }

    int viewBtnW = 96;
    int viewX = innerX + innerW - viewBtnW * 2 - 8;
    if (atDrawPill("List",    viewX,                pillY, pillH, atView == ATV_LIST,    nullptr))
        atView = ATV_LIST;
    if (atDrawPill("Heatmap", viewX + viewBtnW + 8, pillY, pillH, atView == ATV_HEATMAP, nullptr))
        atView = ATV_HEATMAP;

    int searchY = pillY + pillH + 12;
    atDrawSearchBox(innerX, searchY, innerW - 280, 38);

    Task* tasks = getTaskStore();
    int taskCount = getTaskCount();
    int quickCount = 0;
    for (int i = 0; i < taskCount; i++)
        if (!tasks[i].completed && atIsQuick(tasks[i].duration))
            quickCount++;
    int hintX = innerX + innerW - 268;
    int hintY = searchY + 8;
    if (quickCount > 0) {
        DrawCircle(hintX + 10, hintY + 10, 6, AT_GREEN);
        char hint[64];
        snprintf(hint, sizeof(hint), "%d quick task%s right now",
                 quickCount, quickCount == 1 ? "" : "s");
        DrawText(hint, hintX + 24, hintY + 2, 16, AT_GREEN);
    } else {
        DrawText("No quick tasks pending", hintX, hintY + 2, 14, AT_DARK);
    }

    int bulkY = searchY + 50;
    int bulkH = 0;
    int sel = atCountSelected();
    if (sel > 0) {
        bulkH = 50;
        Rectangle br = { (float)innerX, (float)bulkY, (float)innerW, (float)bulkH };
        DrawRectangleRounded(br, 0.18f, 8, AT_PANEL_LT);
        DrawRectangleRoundedLines(br, 0.18f, 8, 1.5f, AT_ORANGE);
        char selTxt[32];
        snprintf(selTxt, sizeof(selTxt), "%d selected", sel);
        DrawText(selTxt, innerX + 16, bulkY + bulkH / 2 - 9, 18, AT_WHITE);

        int bw = 130, bh = 32;
        int bx = innerX + innerW - bw * 4 - 24;
        int by = bulkY + bulkH / 2 - bh / 2;
        if (atDrawSmallBtn("Mark Done",   bx,                by, bw, bh)) atBulkMarkDone();
        if (atDrawSmallBtn("Snooze +1d",  bx + (bw + 8),     by, bw, bh)) atBulkSnooze();
        if (atDrawSmallBtn("Delete",      bx + (bw + 8) * 2, by, bw, bh, true)) atBulkDelete();
        if (atDrawGhostBtn("Cancel",      bx + (bw + 8) * 3, by, bw, bh)) atClearSelection();
    }

    taskCount = getTaskCount();

    int contentTop = bulkY + (bulkH > 0 ? bulkH + 12 : 8);
    int contentH   = screenHeight - contentTop - 24;
    if (contentH < 80) contentH = 80;

    int idxs[MAX_TASKS];
    int nIdx = 0;
    for (int i = 0; i < taskCount; i++) {
        if (!atMatchesFilter(tasks[i])) continue;
        if (!atMatchesSearch(tasks[i])) continue;

        if (tasks[i].completed && atFadeTimer[i] == 0.0f &&
            atFilter == ATF_ACTIVE) continue;
        idxs[nIdx++] = i;
    }

    float dt = GetFrameTime();
    for (int i = 0; i < taskCount; i++) {
        if (atFadeTimer[i] > 0.0f) {
            atFadeTimer[i] -= dt;
            if (atFadeTimer[i] < 0.0f) atFadeTimer[i] = 0.0f;
        }
    }

    if (atView == ATV_LIST)
    {

        int buckets[ATG_COUNT][MAX_TASKS];
        int bn[ATG_COUNT] = {};
        for (int k = 0; k < nIdx; k++) {
            int i = idxs[k];
            AT_Group g = atGroupFor(tasks[i]);
            buckets[g][bn[g]++] = i;
        }

        for (int g = 0; g < ATG_COUNT; g++) {
            for (int a = 0; a < bn[g] - 1; a++)
                for (int b = a + 1; b < bn[g]; b++)
                    if (strcmp(tasks[buckets[g][a]].deadline,
                               tasks[buckets[g][b]].deadline) > 0) {
                        int tmp = buckets[g][a];
                        buckets[g][a] = buckets[g][b];
                        buckets[g][b] = tmp;
                    }
        }

        if (atHoverIn({ (float)innerX, (float)contentTop, (float)innerW, (float)contentH })) {
            float wheel = GetMouseWheelMove();
            atScroll -= wheel * 36;
        }

        BeginScissorMode(innerX, contentTop, innerW, contentH);
        int curY = contentTop - (int)atScroll;
        int totalDrawn = 0;
        bool empty = true;
        for (int g = 0; g < ATG_COUNT; g++) {
            if (bn[g] == 0) continue;
            empty = false;

            Color hcol = AT_ORANGE_LT;
            if (g == ATG_OVERDUE) hcol = AT_RED;
            if (g == ATG_DONE)    hcol = AT_DARK;
            char hdr[64];
            snprintf(hdr, sizeof(hdr), "%s  (%d)", AT_GROUP_LABELS[g], bn[g]);
            DrawText(hdr, innerX, curY, 18, hcol);
            DrawRectangle(innerX, curY + 28, innerW, 1, AT_BORDER);
            curY += 40;

            for (int k = 0; k < bn[g]; k++) {
                int i = buckets[g][k];
                float alpha = 1.0f;
                if (tasks[i].completed) {
                    if (atFadeTimer[i] > 0.0f) alpha = atFadeTimer[i];
                    else if (atFilter == ATF_ALL) alpha = 0.45f;
                }
                int rh = atDrawTaskRow(i, innerX, curY, innerW, alpha);
                curY += rh + 8;
                totalDrawn++;
            }
            curY += 8;
        }
        EndScissorMode();

        if (empty) {
            const char* msg;
            if (atSearchBuf[0] || atFilter != ATF_ALL) {
                msg = "No tasks match your filter.";
            } else if (taskCount == 0) {
                msg = "No tasks yet. Click \"+ New Task\" to add one.";
            } else {
                msg = "Nothing here yet.";
            }
            DrawText(msg, innerX, contentTop + 20, 18, AT_DARK);
        }

        int totalH = (curY + (int)atScroll) - contentTop;
        int maxScroll = totalH - contentH;
        if (maxScroll < 0) maxScroll = 0;
        if (atScroll < 0)         atScroll = 0;
        if (atScroll > maxScroll) atScroll = (float)maxScroll;
    }

    else
    {

        int q[4][MAX_TASKS];
        int qn[4] = {};
        for (int k = 0; k < nIdx; k++) {
            int i = idxs[k];
            if (tasks[i].completed) continue;
            int days = atDaysFromToday(tasks[i].deadline);
            int u = atHighUrgency(days) ? 1 : 0;
            int e = atLowEffort(tasks[i].duration) ? 0 : 1;
            int qi = (u == 1 && e == 0) ? 0
                   : (u == 1 && e == 1) ? 1
                   : (u == 0 && e == 0) ? 2
                                        : 3;
            q[qi][qn[qi]++] = i;
        }

        int gap = 12;
        int cellW = (innerW - gap) / 2;
        int cellH = (contentH - gap) / 2;

        atDrawHeatCell(innerX,                contentTop,
                        cellW, cellH,
                        "Quick wins",
                        "high urgency · low effort", AT_GREEN,
                        q[0], qn[0]);
        atDrawHeatCell(innerX + cellW + gap,  contentTop,
                        cellW, cellH,
                        "Plan & focus",
                        "high urgency · high effort", AT_ORANGE_LT,
                        q[1], qn[1]);
        atDrawHeatCell(innerX,                contentTop + cellH + gap,
                        cellW, cellH,
                        "Fill-ins",
                        "low urgency · low effort", AT_BLUE,
                        q[2], qn[2]);
        atDrawHeatCell(innerX + cellW + gap,  contentTop + cellH + gap,
                        cellW, cellH,
                        "Procrastination graveyard",
                        "low urgency · high effort", AT_RED_DIM,
                        q[3], qn[3]);
    }

    if (atShowAddModal)
        atDrawAddModal(GetScreenWidth(), GetScreenHeight());
}

static void atOpenAddModal()
{
    atShowAddModal   = true;
    atSearchFocused  = false;
    atNewFocusField  = 0;
    atNewPriority    = 2;
    atNewError[0]    = '\0';
    atNewTitle[0]    = '\0';
    atNewDuration[0] = '\0';

    time_t tt = time(NULL) + 7 * 24 * 3600;
    struct tm tm_ = *localtime(&tt);
    snprintf(atNewDeadline, sizeof(atNewDeadline), "%04d-%02d-%02d",
             tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday);
}

static bool atDrawTextField(const char* label, char* buf, int bufMax,
                             int x, int y, int w, int h, int fieldIndex,
                             const char* placeholder)
{
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    bool focused = (atNewFocusField == fieldIndex);
    if (atClickIn(r)) atNewFocusField = fieldIndex;

    DrawText(label, x, y - 22, 14, AT_GREY);

    DrawRectangleRounded(r, 0.25f, 8, focused ? AT_PANEL_LT : AT_PANEL);
    DrawRectangleRoundedLines(r, 0.25f, 8, 1.5f, focused ? AT_ORANGE : AT_BORDER);

    int len = (int)strlen(buf);
    if (len == 0 && !focused) {
        DrawText(placeholder, x + 12, y + h / 2 - 9, 16, AT_DARK);
    } else {
        DrawText(buf, x + 12, y + h / 2 - 9, 16, AT_WHITE);
    }
    if (focused && ((int)(GetTime() * 2) % 2 == 0)) {
        int cx = x + 12 + MeasureText(buf, 16);
        DrawRectangle(cx + 2, y + 8, 2, h - 16, AT_ORANGE_LT);
    }

    if (focused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && len < bufMax - 1) {
                buf[len++] = (char)key;
                buf[len]   = '\0';
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

    int pw = 520, ph = 470;
    int px = sw / 2 - pw / 2;
    int py = sh / 2 - ph / 2;

    DrawRectangleRounded({ (float)(px - 2), (float)(py - 2),
                           (float)(pw + 4), (float)(ph + 4) }, 0.06f, 8, AT_ORANGE);
    DrawRectangleRounded({ (float)px, (float)py, (float)pw, (float)ph },
                         0.06f, 8, AT_PANEL);

    const char* title = "New Task";
    DrawText(title, px + pw / 2 - MeasureText(title, 26) / 2,
             py + 24, 26, AT_WHITE);

    int fieldX = px + 28;
    int fieldW = pw - 56;
    int fieldH = 40;
    int curY   = py + 84;

    atDrawTextField("Title", atNewTitle, sizeof(atNewTitle),
                     fieldX, curY, fieldW, fieldH, 0, "What needs doing?");
    curY += fieldH + 36;

    DrawText("Priority", fieldX, curY - 22, 14, AT_GREY);
    const char* prioLabels[3] = { "Low", "Medium", "High" };
    int prioX = fieldX;
    for (int i = 0; i < 3; i++) {
        int pillW = 0;
        if (atDrawPill(prioLabels[i], prioX, curY, fieldH - 8,
                        atNewPriority == i + 1, &pillW))
            atNewPriority = i + 1;
        prioX += pillW + 8;
    }
    curY += fieldH + 36;

    int halfW = (fieldW - 16) / 2;
    atDrawTextField("Deadline (YYYY-MM-DD)", atNewDeadline,
                     sizeof(atNewDeadline),
                     fieldX, curY, halfW, fieldH, 1, "2026-05-17");
    atDrawTextField("Duration (minutes)", atNewDuration,
                     sizeof(atNewDuration),
                     fieldX + halfW + 16, curY, halfW, fieldH, 2, "30");
    curY += fieldH + 30;

    if (atNewError[0]) {
        DrawText(atNewError, fieldX, curY, 14, AT_RED);
    }

    int btnW = 130, btnH = 44;
    int btnY = py + ph - btnH - 22;
    if (atDrawSmallBtn("Save Task", px + pw - btnW * 2 - 16,
                        btnY, btnW, btnH))
    {

        if (atNewTitle[0] == '\0') {
            snprintf(atNewError, sizeof(atNewError),
                     "Please enter a title.");
        }
        else if (strlen(atNewDeadline) != 10 ||
                 atNewDeadline[4]  != '-'  ||
                 atNewDeadline[7]  != '-')
        {
            snprintf(atNewError, sizeof(atNewError),
                     "Deadline must look like YYYY-MM-DD.");
        }
        else
        {

            Task t = {};
            snprintf(t.title, sizeof(t.title), "%s", atNewTitle);
            t.priority  = atNewPriority;
            snprintf(t.deadline, sizeof(t.deadline), "%s", atNewDeadline);
            t.duration  = atoi(atNewDuration);
            t.completed = false;

            const char* user = getLoggedInUser();
            int newId = saveNewTask(user, t);
            if (newId > 0) {
                t.id = newId;
                addTask(t);
                atShowAddModal = false;
                atNewError[0]  = '\0';
            } else {
                snprintf(atNewError, sizeof(atNewError),
                         "Could not save task. Try again.");
            }
        }
    }
    if (atDrawGhostBtn("Cancel", px + pw - btnW - 16,
                        btnY, btnW, btnH) ||
        IsKeyPressed(KEY_ESCAPE))
    {
        atShowAddModal = false;
        atNewError[0]  = '\0';
    }

    if (IsKeyPressed(KEY_TAB))
        atNewFocusField = (atNewFocusField + 1) % 3;
}
