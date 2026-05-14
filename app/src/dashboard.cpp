#include "../include/screens/dashboard.h"
#include "../include/data.h"
#include "../include/db.h"
#include <cstring>
#include <ctime>
#include <cstdio>

void drawDashboardScreen(int contentX, int contentWidth, int screenHeight)
{
    Color background = { 22, 18, 14, 255 };
    Color cardFill = { 35, 28, 20, 255 };
    Color orange = { 234, 108, 15, 255 };
    Color orangeBright = { 251, 146, 60, 255 };
    Color white = { 255, 255, 255, 255 };
    Color grey = { 180, 160, 140, 255 };
    Color green = { 80, 200, 100, 255 };
    Color red = { 220, 80, 60, 255 };
    Color lineColor = { 55, 46, 36, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, background);

    int margin = 40;
    DrawText("Dashboard", contentX + margin, margin, 32, white);

    Task* tasks = getTaskStore();
    int totalCount = getTaskCount();
    int completedCount = 0;
    int pendingCount = 0;

    int countIndex = 0;
    while (countIndex < totalCount)
    {
        if (tasks[countIndex].completed == true)
        {
            completedCount = completedCount + 1;
        }
        else
        {
            pendingCount = pendingCount + 1;
        }
        countIndex = countIndex + 1;
    }

    int cardTop = margin + 70;
    int cardHeight = 120;
    int cardGap = 16;
    int cardWidth = (contentWidth - margin * 2 - cardGap * 2) / 3;

    int card1Left = contentX + margin;
    DrawRectangleRounded({ (float)card1Left, (float)cardTop, (float)cardWidth, (float)cardHeight }, 0.18f, 8, cardFill);
    DrawRectangle(card1Left, cardTop, 4, cardHeight, orange);
    DrawText(TextFormat("%d", totalCount), card1Left + 20, cardTop + 18, 44, white);
    DrawText("Total Tasks", card1Left + 20, cardTop + 80, 18, grey);

    int card2Left = contentX + margin + cardWidth + cardGap;
    DrawRectangleRounded({ (float)card2Left, (float)cardTop, (float)cardWidth, (float)cardHeight }, 0.18f, 8, cardFill);
    DrawRectangle(card2Left, cardTop, 4, cardHeight, green);
    DrawText(TextFormat("%d", completedCount), card2Left + 20, cardTop + 18, 44, green);
    DrawText("Completed", card2Left + 20, cardTop + 80, 18, grey);

    int card3Left = contentX + margin + (cardWidth + cardGap) * 2;
    DrawRectangleRounded({ (float)card3Left, (float)cardTop, (float)cardWidth, (float)cardHeight }, 0.18f, 8, cardFill);
    DrawRectangle(card3Left, cardTop, 4, cardHeight, red);
    DrawText(TextFormat("%d", pendingCount), card3Left + 20, cardTop + 18, 44, red);
    DrawText("Pending", card3Left + 20, cardTop + 80, 18, grey);

    int progressBarX = contentX + margin;
    int progressBarY = cardTop + cardHeight + 14;
    int progressBarW = contentWidth - margin * 2;
    int progressBarH = 10;

    int completionPercent = 0;
    int progressFillW = 0;
    if (totalCount > 0)
    {
        completionPercent = (completedCount * 100) / totalCount;
        progressFillW = (completedCount * progressBarW) / totalCount;
    }

    DrawRectangleRounded({ (float)progressBarX, (float)progressBarY, (float)progressBarW, (float)progressBarH }, 1.0f, 4, lineColor);
    if (progressFillW > 0)
    {
        DrawRectangleRounded({ (float)progressBarX, (float)progressBarY, (float)progressFillW, (float)progressBarH }, 1.0f, 4, green);
    }

    char progressLabelText[32] = {};
    snprintf(progressLabelText, sizeof(progressLabelText), "%d%% complete", completionPercent);
    int progressLabelW = MeasureText(progressLabelText, 15);
    DrawText(progressLabelText, progressBarX + progressBarW - progressLabelW, progressBarY - 20, 15, grey);

    time_t currentTime = time(nullptr);
    struct tm* localTimePtr = localtime(&currentTime);
    char todayDateString[16] = {};
    int currentYear = localTimePtr->tm_year + 1900;
    int currentMonth = localTimePtr->tm_mon + 1;
    int currentDay = localTimePtr->tm_mday;
    snprintf(todayDateString, sizeof(todayDateString), "%04d-%02d-%02d", currentYear, currentMonth, currentDay);

    int dueTodaySectionY = progressBarY + progressBarH + 24;
    DrawText("Due Today", contentX + margin, dueTodaySectionY, 22, orangeBright);
    DrawRectangle(contentX + margin, dueTodaySectionY + 32, contentWidth - margin * 2, 1, lineColor);

    int dueTodayIndices[MAX_TASKS];
    int dueTodayCount = 0;

    int searchIndex = 0;
    while (searchIndex < totalCount)
    {
        bool taskIsNotCompleted = (tasks[searchIndex].completed == false);
        bool deadlineMatchesToday = (strcmp(tasks[searchIndex].deadline, todayDateString) == 0);
        if (taskIsNotCompleted == true && deadlineMatchesToday == true)
        {
            dueTodayIndices[dueTodayCount] = searchIndex;
            dueTodayCount = dueTodayCount + 1;
        }
        searchIndex = searchIndex + 1;
    }

    int dueTodayRowTop = dueTodaySectionY + 46;
    int dueTodayRowH = 52;
    int dueTodayRowGap = 6;

    if (dueTodayCount == 0)
    {
        DrawText("Nothing due today.", contentX + margin, dueTodayRowTop, 18, grey);
        dueTodayRowTop = dueTodayRowTop + 32;
    }
    else
    {
        int maxDueTodayRows = dueTodayCount;
        if (maxDueTodayRows > 3)
        {
            maxDueTodayRows = 3;
        }

        int dueRowDisplayIndex = 0;
        while (dueRowDisplayIndex < maxDueTodayRows)
        {
            int taskIdx = dueTodayIndices[dueRowDisplayIndex];
            int rowY = dueTodayRowTop + dueRowDisplayIndex * (dueTodayRowH + dueTodayRowGap);
            int rowX = contentX + margin;
            int rowW = contentWidth - margin * 2;

            DrawRectangleRounded({ (float)rowX, (float)rowY, (float)rowW, (float)dueTodayRowH }, 0.16f, 8, cardFill);

            int priorityValue = tasks[taskIdx].priority;
            Color priorityBarColor = green;
            if (priorityValue == 2) { priorityBarColor = orangeBright; }
            if (priorityValue == 3) { priorityBarColor = red; }
            DrawRectangle(rowX, rowY, 4, dueTodayRowH, priorityBarColor);

            DrawText(tasks[taskIdx].title, rowX + 20, rowY + dueTodayRowH / 2 - 10, 18, white);

            dueRowDisplayIndex = dueRowDisplayIndex + 1;
        }

        dueTodayRowTop = dueTodayRowTop + maxDueTodayRows * (dueTodayRowH + dueTodayRowGap);
    }

    int sectionTop = dueTodayRowTop + 24;
    DrawText("Upcoming Tasks", contentX + margin, sectionTop, 24, orangeBright);
    DrawRectangle(contentX + margin, sectionTop + 36, contentWidth - margin * 2, 1, lineColor);

    if (totalCount == 0)
    {
        DrawText("No tasks yet.", contentX + margin, sectionTop + 60, 22, grey);
        DrawText("Open All Tasks to add your first task.", contentX + margin, sectionTop + 92, 18, grey);
        return;
    }

    int pendingIndices[MAX_TASKS];
    int pendingTotal = 0;

    int pendingSearchIndex = 0;
    while (pendingSearchIndex < totalCount)
    {
        if (tasks[pendingSearchIndex].completed == false)
        {
            pendingIndices[pendingTotal] = pendingSearchIndex;
            pendingTotal = pendingTotal + 1;
        }
        pendingSearchIndex = pendingSearchIndex + 1;
    }

    int outerSortPass = 0;
    while (outerSortPass < pendingTotal - 1)
    {
        int innerSortIndex = 0;
        while (innerSortIndex < pendingTotal - outerSortPass - 1)
        {
            const char* thisDeadline = tasks[pendingIndices[innerSortIndex]].deadline;
            const char* nextDeadline = tasks[pendingIndices[innerSortIndex + 1]].deadline;
            if (strcmp(thisDeadline, nextDeadline) > 0)
            {
                int temp = pendingIndices[innerSortIndex];
                pendingIndices[innerSortIndex] = pendingIndices[innerSortIndex + 1];
                pendingIndices[innerSortIndex + 1] = temp;
            }
            innerSortIndex = innerSortIndex + 1;
        }
        outerSortPass = outerSortPass + 1;
    }

    int rowsToShow = pendingTotal;
    if (rowsToShow > 6)
    {
        rowsToShow = 6;
    }

    if (rowsToShow == 0)
    {
        DrawText("All done - nothing pending!", contentX + margin, sectionTop + 56, 20, grey);
    }

    int rowTop = sectionTop + 48;
    int rowHeight = 62;
    int rowGap = 8;
    int rowWidth = contentWidth - margin * 2;

    const char* priorityText[] = { "?", "Low", "Med", "High" };
    Color priorityColor[] = { grey, green, orangeBright, red };

    int rowDisplayIndex = 0;
    while (rowDisplayIndex < rowsToShow)
    {
        Task* task = &tasks[pendingIndices[rowDisplayIndex]];

        int rowY = rowTop + rowDisplayIndex * (rowHeight + rowGap);
        int rowX = contentX + margin;

        DrawRectangleRounded({ (float)rowX, (float)rowY, (float)rowWidth, (float)rowHeight }, 0.16f, 8, cardFill);

        int priority = task->priority;
        if (priority < 1 || priority > 3)
        {
            priority = 1;
        }
        DrawRectangle(rowX, rowY, 4, rowHeight, priorityColor[priority]);

        DrawText(task->title, rowX + 20, rowY + rowHeight / 2 - 11, 20, white);
        DrawText(priorityText[priority], rowX + rowWidth - 220, rowY + rowHeight / 2 - 9, 16, priorityColor[priority]);
        DrawText(task->deadline, rowX + rowWidth - 155, rowY + rowHeight / 2 - 9, 16, grey);

        int checkSize = 26;
        int checkX = rowX + rowWidth - checkSize - 10;
        int checkY = rowY + rowHeight / 2 - checkSize / 2;

        Rectangle checkboxArea = { (float)checkX, (float)checkY, (float)checkSize, (float)checkSize };
        bool mouseOverCheckbox = CheckCollisionPointRec(GetMousePosition(), checkboxArea);

        Color checkBorderColor = grey;
        if (mouseOverCheckbox == true)
        {
            checkBorderColor = orangeBright;
        }

        DrawRectangleRounded({ (float)(checkX - 2), (float)(checkY - 2), (float)(checkSize + 4), (float)(checkSize + 4) }, 0.3f, 8, checkBorderColor);
        DrawRectangleRounded(checkboxArea, 0.3f, 8, background);

        if (mouseOverCheckbox == true && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            task->completed = true;
            updateTaskCompleted(task->id, true);
        }

        rowDisplayIndex = rowDisplayIndex + 1;
    }
}
