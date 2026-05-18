#include "../include/screens/calendar.h"
#include "../include/data.h"
#include "../include/logic.h"
#include "raylib.h"
#include <ctime>

static int calYear        = 0;
static int calMonth       = 0;
static int calSelectedDay = -1;

static bool calInitialized = false;

static int calDaysInMonth(int year, int month)
{
    if (month == 2)
    {
        bool isLeapYear = false;
        if (year % 4 == 0)
        {
            isLeapYear = true;
        }
        if (year % 100 == 0)
        {
            isLeapYear = false;
        }
        if (year % 400 == 0)
        {
            isLeapYear = true;
        }
        if (isLeapYear == true)
        {
            return 29;
        }
        return 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        return 30;
    }
    return 31;
}

static int calFirstWeekday(int year, int month)
{
    struct tm timeInfo = {};
    timeInfo.tm_year = year - 1900;
    timeInfo.tm_mon  = month - 1;
    timeInfo.tm_mday = 1;
    mktime(&timeInfo);
    return timeInfo.tm_wday;
}

static const char* calMonthName(int month)
{
    if (month == 1)  return "January";
    if (month == 2)  return "February";
    if (month == 3)  return "March";
    if (month == 4)  return "April";
    if (month == 5)  return "May";
    if (month == 6)  return "June";
    if (month == 7)  return "July";
    if (month == 8)  return "August";
    if (month == 9)  return "September";
    if (month == 10) return "October";
    if (month == 11) return "November";
    return "December";
}

void drawCalendarScreen(int contentX, int contentWidth, int screenHeight)
{
    Color background  = {  22,  18,  14, 255 };
    Color cardFill    = {  35,  28,  20, 255 };
    Color cellToday   = {  50,  38,  24, 255 };
    Color cellSelect  = {  45,  35,  18, 255 };
    Color orange      = { 234, 108,  15, 255 };
    Color orangeLight = { 251, 146,  60, 255 };
    Color white       = { 255, 255, 255, 255 };
    Color grey        = { 180, 160, 140, 255 };
    Color darkGrey    = {  55,  46,  36, 255 };
    Color green       = {  80, 200, 100, 255 };
    Color red         = { 220,  80,  60, 255 };
    Color lineColor   = {  55,  46,  36, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, background);

    if (calInitialized == false)
    {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        calYear  = t->tm_year + 1900;
        calMonth = t->tm_mon + 1;
        calInitialized = true;
    }

    time_t now = time(nullptr);
    struct tm* todayPtr = localtime(&now);
    int todayYear  = todayPtr->tm_year + 1900;
    int todayMonth = todayPtr->tm_mon + 1;
    int todayDay   = todayPtr->tm_mday;

    int margin = 40;

    DrawText("Calendar", contentX + margin, margin, 32, white);

    int headerY = margin + 52;

    int arrowBtnW = 36;
    int arrowBtnH = 36;
    int arrowBtnY = headerY;

    char monthLabel[32] = {};
    int labelPos = 0;
    labelPos = appendText(monthLabel, labelPos, calMonthName(calMonth));
    labelPos = appendText(monthLabel, labelPos, " ");
    labelPos = appendNumber(monthLabel, labelPos, calYear);

    int monthLabelW = MeasureText(monthLabel, 22);
    int monthLabelX = contentX + contentWidth / 2 - monthLabelW / 2;
    DrawText(monthLabel, monthLabelX, headerY + 7, 22, white);

    int prevBtnX = monthLabelX - arrowBtnW - 16;
    int nextBtnX = monthLabelX + monthLabelW + 16;

    Rectangle prevArea = { (float)prevBtnX, (float)arrowBtnY, (float)arrowBtnW, (float)arrowBtnH };
    Rectangle nextArea = { (float)nextBtnX, (float)arrowBtnY, (float)arrowBtnW, (float)arrowBtnH };

    bool mouseOverPrev = CheckCollisionPointRec(GetMousePosition(), prevArea);
    bool mouseOverNext = CheckCollisionPointRec(GetMousePosition(), nextArea);

    Color prevColor = mouseOverPrev ? orangeLight : grey;
    Color nextColor = mouseOverNext ? orangeLight : grey;

    DrawRectangleRounded(prevArea, 0.3f, 8, darkGrey);
    DrawText("<", prevBtnX + 11, arrowBtnY + 8, 18, prevColor);

    DrawRectangleRounded(nextArea, 0.3f, 8, darkGrey);
    DrawText(">", nextBtnX + 11, arrowBtnY + 8, 18, nextColor);

    if (mouseOverPrev && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        calMonth = calMonth - 1;
        if (calMonth < 1)
        {
            calMonth = 12;
            calYear  = calYear - 1;
        }
        calSelectedDay = -1;
    }

    if (mouseOverNext && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        calMonth = calMonth + 1;
        if (calMonth > 12)
        {
            calMonth = 1;
            calYear  = calYear + 1;
        }
        calSelectedDay = -1;
    }

    int gridTop  = headerY + arrowBtnH + 20;
    int gridLeft = contentX + margin;
    int gridW    = contentWidth - margin * 2;

    int cellW = gridW / 7;
    int cellH = 90;

    const char* dayLabels[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    int dayLabelIndex = 0;
    while (dayLabelIndex < 7)
    {
        int labelX = gridLeft + dayLabelIndex * cellW + cellW / 2 - MeasureText(dayLabels[dayLabelIndex], 15) / 2;
        DrawText(dayLabels[dayLabelIndex], labelX, gridTop, 15, grey);
        dayLabelIndex = dayLabelIndex + 1;
    }

    DrawRectangle(gridLeft, gridTop + 24, gridW, 1, lineColor);

    int daysInThisMonth = calDaysInMonth(calYear, calMonth);
    int firstWeekday    = calFirstWeekday(calYear, calMonth);

    int taskIndicesForCell[42][MAX_TASKS];
    int taskCountForCell[42]  = {};

    Task* taskList  = getTaskStore();
    int   taskCount = getTaskCount();

    int taskIndex = 0;
    while (taskIndex < taskCount)
    {
        int taskYear  = 0;
        int taskMonth = 0;
        int taskDay   = 0;
        bool parsed = parseDateText(taskList[taskIndex].deadline, &taskYear, &taskMonth, &taskDay);

        if (parsed == true && taskYear == calYear && taskMonth == calMonth)
        {
            int cellIndex = firstWeekday + taskDay - 1;
            if (cellIndex >= 0 && cellIndex < 42)
            {
                int slotCount = taskCountForCell[cellIndex];
                if (slotCount < MAX_TASKS)
                {
                    taskIndicesForCell[cellIndex][slotCount] = taskIndex;
                    taskCountForCell[cellIndex] = slotCount + 1;
                }
            }
        }
        taskIndex = taskIndex + 1;
    }

    int cellsStartY = gridTop + 32;

    int dayNumber = 1;
    int cellIndex = 0;
    while (cellIndex < 42 && dayNumber <= daysInThisMonth)
    {
        int col = cellIndex % 7;
        int row = cellIndex / 7;

        if (cellIndex < firstWeekday)
        {
            cellIndex = cellIndex + 1;
            continue;
        }

        int cellX = gridLeft + col * cellW;
        int cellY = cellsStartY + row * (cellH + 6);

        bool isToday    = (calYear == todayYear && calMonth == todayMonth && dayNumber == todayDay);
        bool isSelected = (calSelectedDay == dayNumber);

        Color cellBg = cardFill;
        if (isToday == true)
        {
            cellBg = cellToday;
        }
        if (isSelected == true)
        {
            cellBg = cellSelect;
        }

        DrawRectangleRounded({ (float)(cellX + 2), (float)cellY, (float)(cellW - 4), (float)cellH }, 0.15f, 8, cellBg);

        if (isToday == true)
        {
            DrawRectangleRoundedLines({ (float)(cellX + 2), (float)cellY, (float)(cellW - 4), (float)cellH }, 0.15f, 8, 1.5f, orange);
        }

        Color dayNumColor = white;
        if (isToday == true)
        {
            dayNumColor = orangeLight;
        }

        char dayText[4] = {};
        int dayTextPos = 0;
        dayTextPos = appendNumber(dayText, dayTextPos, dayNumber);
        DrawText(dayText, cellX + 8, cellY + 6, 16, dayNumColor);

        int dotCount = taskCountForCell[cellIndex];

        int maxDots = 3;
        int dotsToShow = dotCount;
        if (dotsToShow > maxDots)
        {
            dotsToShow = maxDots;
        }

        int dotRadius  = 4;
        int dotSpacing = 12;
        int dotsStartX = cellX + 8;
        int dotsY      = cellY + 30;

        int dotDrawIndex = 0;
        while (dotDrawIndex < dotsToShow)
        {
            int taskIdx = taskIndicesForCell[cellIndex][dotDrawIndex];
            int priority = taskList[taskIdx].priority;

            Color dotColor = green;
            if (priority == 2) { dotColor = orangeLight; }
            if (priority == 3) { dotColor = red; }
            if (taskList[taskIdx].completed == true) { dotColor = darkGrey; }

            DrawCircle(dotsStartX + dotDrawIndex * dotSpacing, dotsY, dotRadius, dotColor);
            dotDrawIndex = dotDrawIndex + 1;
        }

        if (dotCount > maxDots)
        {
            char moreText[8] = {};
            int morePos = 0;
            morePos = appendText(moreText, morePos, "+");
            morePos = appendNumber(moreText, morePos, dotCount - maxDots);
            DrawText(moreText, dotsStartX + maxDots * dotSpacing + 4, dotsY - 5, 13, grey);
        }

        Rectangle cellArea = { (float)(cellX + 2), (float)cellY, (float)(cellW - 4), (float)cellH };
        bool mouseOverCell = CheckCollisionPointRec(GetMousePosition(), cellArea);
        if (mouseOverCell == true && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (calSelectedDay == dayNumber)
            {
                calSelectedDay = -1;
            }
            else
            {
                calSelectedDay = dayNumber;
            }
        }

        dayNumber  = dayNumber + 1;
        cellIndex  = cellIndex + 1;
    }

    if (calSelectedDay >= 1)
    {
        int totalRows = (firstWeekday + daysInThisMonth + 6) / 7;
        int panelTop  = cellsStartY + totalRows * (cellH + 6) + 16;

        DrawRectangle(gridLeft, panelTop, gridW, 1, lineColor);

        char panelTitle[32] = {};
        int panelTitlePos = 0;
        panelTitlePos = appendText(panelTitle, panelTitlePos, calMonthName(calMonth));
        panelTitlePos = appendText(panelTitle, panelTitlePos, " ");
        panelTitlePos = appendNumber(panelTitle, panelTitlePos, calSelectedDay);
        DrawText(panelTitle, gridLeft, panelTop + 14, 20, orangeLight);

        int selectedCellIndex = firstWeekday + calSelectedDay - 1;
        int taskCount2 = taskCountForCell[selectedCellIndex];

        if (taskCount2 == 0)
        {
            DrawText("No tasks due this day.", gridLeft, panelTop + 44, 16, grey);
        }
        else
        {
            int rowY   = panelTop + 44;
            int rowH   = 40;
            int rowGap = 6;
            int rowW   = gridW;

            int panelTaskIndex = 0;
            while (panelTaskIndex < taskCount2)
            {
                int taskIdx  = taskIndicesForCell[selectedCellIndex][panelTaskIndex];
                Task* task   = &taskList[taskIdx];

                int priority = task->priority;
                if (priority < 1 || priority > 3) { priority = 1; }

                Color priorityColor = green;
                if (priority == 2) { priorityColor = orangeLight; }
                if (priority == 3) { priorityColor = red; }

                DrawRectangleRounded({ (float)gridLeft, (float)rowY, (float)rowW, (float)rowH }, 0.2f, 8, cardFill);
                DrawRectangle(gridLeft, rowY, 4, rowH, priorityColor);

                Color titleColor = white;
                if (task->completed == true) { titleColor = grey; }

                DrawText(task->title, gridLeft + 18, rowY + rowH / 2 - 9, 17, titleColor);

                if (task->completed == true)
                {
                    DrawText("Done", gridLeft + rowW - 60, rowY + rowH / 2 - 9, 14, darkGrey);
                }

                rowY = rowY + rowH + rowGap;
                panelTaskIndex = panelTaskIndex + 1;
            }
        }
    }
}
