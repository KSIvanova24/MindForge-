#include "../include/screens/dashboard.h"
#include "../include/data.h"
#include "../include/db.h"
#include <cstring>

void drawDashboardScreen(int contentX, int contentWidth, int screenHeight)
{
    Color background   = {  22,  18,  14, 255 };
    Color cardFill     = {  35,  28,  20, 255 };
    Color orange       = { 234, 108,  15, 255 };
    Color orangeBright = { 251, 146,  60, 255 };
    Color white        = { 255, 255, 255, 255 };
    Color grey         = { 180, 160, 140, 255 };
    Color green        = {  80, 200, 100, 255 };
    Color red          = { 220,  80,  60, 255 };
    Color lineColor    = {  55,  46,  36, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, background);

    int margin = 40;
    DrawText("Dashboard", contentX + margin, margin, 32, white);

    Task* tasks = getTaskStore();
    int   totalCount = getTaskCount();
    int   completedCount = 0;
    int   pendingCount = 0;

    for (int i = 0; i < totalCount; i++)
    {
        if (tasks[i].completed) completedCount++;
        else                     pendingCount++;
    }

    int cardTop    = margin + 70;
    int cardHeight = 120;
    int cardGap    = 16;
    int cardWidth  = (contentWidth - margin * 2 - cardGap * 2) / 3;

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

    int sectionTop = cardTop + cardHeight + 48;
    DrawText("Upcoming Tasks", contentX + margin, sectionTop, 24, orangeBright);
    DrawRectangle(contentX + margin, sectionTop + 36, contentWidth - margin * 2, 1, lineColor);

    if (totalCount == 0)
    {
        DrawText("No tasks yet.", contentX + margin, sectionTop + 60, 22, grey);
        DrawText("Open All Tasks to add your first task.",
                 contentX + margin, sectionTop + 92, 18, grey);
        return;
    }

    int pendingIndices[MAX_TASKS];
    int pendingTotal = 0;

    for (int i = 0; i < totalCount; i++)
    {
        if (!tasks[i].completed)
            pendingIndices[pendingTotal++] = i;
    }

    for (int pass = 0; pass < pendingTotal - 1; pass++)
    {
        for (int compare = 0; compare < pendingTotal - pass - 1; compare++)
        {
            const char* thisDeadline = tasks[pendingIndices[compare]].deadline;
            const char* nextDeadline = tasks[pendingIndices[compare + 1]].deadline;

            if (strcmp(thisDeadline, nextDeadline) > 0)
            {
                int temp = pendingIndices[compare];
                pendingIndices[compare] = pendingIndices[compare + 1];
                pendingIndices[compare + 1] = temp;
            }
        }
    }

    int rowsToShow = pendingTotal < 6 ? pendingTotal : 6;

    if (rowsToShow == 0)
        DrawText("All done — nothing pending!", contentX + margin, sectionTop + 56, 20, grey);

    int rowTop    = sectionTop + 48;
    int rowHeight = 62;
    int rowGap    = 8;
    int rowWidth  = contentWidth - margin * 2;

    const char* priorityText[]  = { "?",  "Low",  "Med",         "High" };
    Color       priorityColor[] = { grey,  green,  orangeBright,  red   };

    for (int i = 0; i < rowsToShow; i++)
    {
        Task& task = tasks[pendingIndices[i]];

        int rowY = rowTop + i * (rowHeight + rowGap);
        int rowX = contentX + margin;

        DrawRectangleRounded({ (float)rowX, (float)rowY, (float)rowWidth, (float)rowHeight }, 0.16f, 8, cardFill);

        int priority = (task.priority >= 1 && task.priority <= 3) ? task.priority : 1;
        DrawRectangle(rowX, rowY, 4, rowHeight, priorityColor[priority]);

        DrawText(task.title, rowX + 20, rowY + rowHeight / 2 - 11, 20, white);

        DrawText(priorityText[priority], rowX + rowWidth - 220, rowY + rowHeight / 2 - 9, 16, priorityColor[priority]);
        DrawText(task.deadline,          rowX + rowWidth - 155, rowY + rowHeight / 2 - 9, 16, grey);

        int checkSize = 26;
        int checkX    = rowX + rowWidth - checkSize - 10;
        int checkY    = rowY + rowHeight / 2 - checkSize / 2;

        Rectangle checkboxArea = { (float)checkX, (float)checkY, (float)checkSize, (float)checkSize };
        bool mouseOverCheckbox = CheckCollisionPointRec(GetMousePosition(), checkboxArea);

        Color checkBorderColor = mouseOverCheckbox ? orangeBright : grey;
        DrawRectangleRounded({ (float)(checkX - 2), (float)(checkY - 2), (float)(checkSize + 4), (float)(checkSize + 4) }, 0.3f, 8, checkBorderColor);
        DrawRectangleRounded(checkboxArea, 0.3f, 8, background);

        if (mouseOverCheckbox && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            task.completed = true;
            updateTaskCompleted(task.id, true);
        }
    }
}
