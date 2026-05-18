#include "../include/screens/archive.h"
#include "../include/data.h"
#include "../include/db.h"
#include "raylib.h"
#include "../include/logic.h"

void drawArchiveScreen(int contentX, int contentWidth, int screenHeight)
{
    Color background  = {  22,  18,  14, 255 };
    Color cardFill    = {  35,  28,  20, 255 };
    Color white       = { 255, 255, 255, 255 };
    Color grey        = { 180, 160, 140, 255 };
    Color green       = {  80, 200, 100, 255 };
    Color orangeBright = { 251, 146,  60, 255 };
    Color red         = { 220,  80,  60, 255 };
    Color lineColor   = {  55,  46,  36, 255 };
    Color checkColor  = {  65, 160,  80, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, background);

    int margin = 40;
    DrawText("Archive", contentX + margin, margin, 32, white);
    DrawText("Completed tasks", contentX + margin, margin + 44, 18, grey);
    DrawRectangle(contentX + margin, margin + 76, contentWidth - margin * 2, 1, lineColor);

    Task* taskList = getTaskStore();
    int totalTasks = getTaskCount();

    int completedIndices[MAX_TASKS];
    int completedCount = 0;

    int searchIndex = 0;
    while (searchIndex < totalTasks)
    {
        if (taskList[searchIndex].completed == true)
        {
            completedIndices[completedCount] = searchIndex;
            completedCount = completedCount + 1;
        }
        searchIndex = searchIndex + 1;
    }

    int listStartY = margin + 90;

    if (completedCount == 0)
    {
        DrawText("No completed tasks yet.", contentX + margin, listStartY, 20, grey);
        DrawText("Complete tasks in All Tasks to see them here.", contentX + margin, listStartY + 32, 16, grey);
        return;
    }

    char countText[32] = {};
    int countPos = 0;
    countPos = appendNumber(countText, countPos, completedCount);
    countPos = appendText(countText, countPos, " completed");
    DrawText(countText, contentX + contentWidth - margin - MeasureText(countText, 16), margin + 52, 16, grey);

    int rowHeight = 60;
    int rowGap    = 8;
    int rowWidth  = contentWidth - margin * 2;

    const char* priorityLabels[4] = { "?", "Low", "Med", "High" };
    Color priorityColors[4] = { grey, green, orangeBright, red };

    int visibleRows = (screenHeight - listStartY - margin) / (rowHeight + rowGap);
    if (visibleRows > completedCount)
    {
        visibleRows = completedCount;
    }

    int displayIndex = 0;
    while (displayIndex < visibleRows)
    {
        int taskIndex = completedIndices[displayIndex];
        Task* task = &taskList[taskIndex];

        int rowY = listStartY + displayIndex * (rowHeight + rowGap);
        int rowX = contentX + margin;

        DrawRectangleRounded({ (float)rowX, (float)rowY, (float)rowWidth, (float)rowHeight }, 0.16f, 8, cardFill);

        int priority = task->priority;
        if (priority < 1 || priority > 3)
        {
            priority = 1;
        }

        DrawRectangle(rowX, rowY, 4, rowHeight, priorityColors[priority]);

        int checkMarkSize = 20;
        int checkMarkX = rowX + 18;
        int checkMarkY = rowY + rowHeight / 2 - checkMarkSize / 2;
        DrawRectangleRounded({ (float)checkMarkX, (float)checkMarkY, (float)checkMarkSize, (float)checkMarkSize }, 0.3f, 8, checkColor);
        DrawText("v", checkMarkX + 5, checkMarkY + 3, 14, background);

        DrawText(task->title, rowX + 52, rowY + rowHeight / 2 - 10, 18, grey);

        DrawText(priorityLabels[priority], rowX + rowWidth - 260, rowY + rowHeight / 2 - 9, 15, priorityColors[priority]);

        DrawText(task->deadline, rowX + rowWidth - 190, rowY + rowHeight / 2 - 9, 15, grey);

        int restoreBtnW = 80;
        int restoreBtnH = 30;
        int restoreBtnX = rowX + rowWidth - restoreBtnW - 10;
        int restoreBtnY = rowY + rowHeight / 2 - restoreBtnH / 2;

        Rectangle restoreArea = { (float)restoreBtnX, (float)restoreBtnY, (float)restoreBtnW, (float)restoreBtnH };
        bool mouseOverRestore = CheckCollisionPointRec(GetMousePosition(), restoreArea);

        Color restoreBtnBg = { 45, 38, 30, 255 };
        Color restoreBtnText = grey;
        if (mouseOverRestore == true)
        {
            restoreBtnBg   = { 60, 50, 36, 255 };
            restoreBtnText = white;
        }

        DrawRectangleRounded(restoreArea, 0.3f, 8, restoreBtnBg);
        DrawRectangleRoundedLines(restoreArea, 0.3f, 8, 1.0f, lineColor);

        const char* restoreLabel = "Restore";
        int restoreLabelW = MeasureText(restoreLabel, 14);
        DrawText(restoreLabel, restoreBtnX + restoreBtnW / 2 - restoreLabelW / 2, restoreBtnY + restoreBtnH / 2 - 7, 14, restoreBtnText);

        if (mouseOverRestore == true && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            task->completed = false;
            updateTaskCompleted(task->id, false);
        }

        displayIndex = displayIndex + 1;
    }

    if (completedCount > visibleRows)
    {
        int moreCount = completedCount - visibleRows;
        char moreText[32] = {};
        int morePos = 0;
        morePos = appendText(moreText, morePos, "+ ");
        morePos = appendNumber(moreText, morePos, moreCount);
        morePos = appendText(moreText, morePos, " more");
        DrawText(moreText, contentX + margin, listStartY + visibleRows * (rowHeight + rowGap) + 8, 15, grey);
    }
}
