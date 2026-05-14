#include "../include/screens/statistics.h"
#include "../include/data.h"
#include "raylib.h"
#include <cstdio>

void drawStatisticsScreen(int contentX, int contentWidth, int screenHeight)
{
    Color background = { 22, 18, 14, 255 };
    Color cardFill = { 35, 28, 20, 255 };
    Color orange = { 234, 108, 15, 255 };
    Color orangeLight = { 251, 146, 60, 255 };
    Color white = { 255, 255, 255, 255 };
    Color grey = { 180, 160, 140, 255 };
    Color darkGrey = { 55, 46, 36, 255 };
    Color green = { 80, 200, 100, 255 };
    Color red = { 220, 80, 60, 255 };
    Color blue = { 90, 140, 220, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, background);

    int margin = 40;
    DrawText("Statistics", contentX + margin, margin, 32, white);

    Task* taskList = getTaskStore();
    int totalTasks = getTaskCount();
    int completedTasks = 0;
    int lowPriorityCount = 0;
    int medPriorityCount = 0;
    int highPriorityCount = 0;
    int totalMinutes = 0;

    int taskIndex = 0;
    while (taskIndex < totalTasks)
    {
        if (taskList[taskIndex].completed == true)
        {
            completedTasks = completedTasks + 1;
        }

        if (taskList[taskIndex].priority == 1)
        {
            lowPriorityCount = lowPriorityCount + 1;
        }
        else if (taskList[taskIndex].priority == 2)
        {
            medPriorityCount = medPriorityCount + 1;
        }
        else if (taskList[taskIndex].priority == 3)
        {
            highPriorityCount = highPriorityCount + 1;
        }

        totalMinutes = totalMinutes + taskList[taskIndex].duration;

        taskIndex = taskIndex + 1;
    }

    int pendingTasks = totalTasks - completedTasks;

    int donutCenterX = contentX + margin + 130;
    int donutCenterY = margin + 160;
    int donutOuterRadius = 90;
    int donutInnerRadius = 58;

    DrawCircle(donutCenterX, donutCenterY, (float)donutOuterRadius, darkGrey);

    if (totalTasks > 0 && completedTasks > 0)
    {
        float completionAngle = (float)completedTasks / (float)totalTasks * 360.0f;
        DrawRing(
            { (float)donutCenterX, (float)donutCenterY },
            (float)donutInnerRadius,
            (float)donutOuterRadius,
            -90.0f,
            -90.0f + completionAngle,
            32,
            green
        );
    }

    DrawCircle(donutCenterX, donutCenterY, (float)donutInnerRadius, background);

    int completionPercent = 0;
    if (totalTasks > 0)
    {
        completionPercent = (completedTasks * 100) / totalTasks;
    }

    char percentText[8] = {};
    snprintf(percentText, sizeof(percentText), "%d%%", completionPercent);
    int percentTextWidth = MeasureText(percentText, 26);
    DrawText(percentText, donutCenterX - percentTextWidth / 2, donutCenterY - 13, 26, white);

    int legendX = donutCenterX + donutOuterRadius + 36;
    int legendY = donutCenterY - 30;

    DrawRectangle(legendX, legendY, 14, 14, green);
    DrawText(TextFormat("Completed %d", completedTasks), legendX + 22, legendY - 1, 18, white);

    DrawRectangle(legendX, legendY + 28, 14, 14, darkGrey);
    DrawText(TextFormat("Pending %d", pendingTasks), legendX + 22, legendY + 27, 18, grey);

    DrawText("Completion Rate", contentX + margin, donutCenterY + donutOuterRadius + 14, 15, grey);

    int barSectionY = donutCenterY + donutOuterRadius + 50;
    int barSectionX = contentX + margin;
    int barSectionW = contentWidth - margin * 2;

    DrawText("Tasks by Priority", barSectionX, barSectionY, 22, orangeLight);
    DrawRectangle(barSectionX, barSectionY + 32, barSectionW, 1, darkGrey);

    int barChartStartY = barSectionY + 48;
    int barHeight = 32;
    int barGap = 12;
    int maxBarWidth = barSectionW - 180;

    int lowBarFillW = 0;
    if (totalTasks > 0)
    {
        lowBarFillW = (lowPriorityCount * maxBarWidth) / totalTasks;
    }
    DrawText("Low", barSectionX, barChartStartY + barHeight / 2 - 9, 18, green);
    DrawText(TextFormat("%d", lowPriorityCount), barSectionX + 52, barChartStartY + barHeight / 2 - 9, 18, grey);
    DrawRectangleRounded({ (float)(barSectionX + 100), (float)barChartStartY, (float)maxBarWidth, (float)barHeight }, 0.4f, 8, cardFill);
    if (lowBarFillW > 0)
    {
        DrawRectangleRounded({ (float)(barSectionX + 100), (float)barChartStartY, (float)lowBarFillW, (float)barHeight }, 0.4f, 8, green);
    }

    int medBarY = barChartStartY + barHeight + barGap;
    int medBarFillW = 0;
    if (totalTasks > 0)
    {
        medBarFillW = (medPriorityCount * maxBarWidth) / totalTasks;
    }
    DrawText("Med", barSectionX, medBarY + barHeight / 2 - 9, 18, orangeLight);
    DrawText(TextFormat("%d", medPriorityCount), barSectionX + 52, medBarY + barHeight / 2 - 9, 18, grey);
    DrawRectangleRounded({ (float)(barSectionX + 100), (float)medBarY, (float)maxBarWidth, (float)barHeight }, 0.4f, 8, cardFill);
    if (medBarFillW > 0)
    {
        DrawRectangleRounded({ (float)(barSectionX + 100), (float)medBarY, (float)medBarFillW, (float)barHeight }, 0.4f, 8, orangeLight);
    }

    int highBarY = medBarY + barHeight + barGap;
    int highBarFillW = 0;
    if (totalTasks > 0)
    {
        highBarFillW = (highPriorityCount * maxBarWidth) / totalTasks;
    }
    DrawText("High", barSectionX, highBarY + barHeight / 2 - 9, 18, red);
    DrawText(TextFormat("%d", highPriorityCount), barSectionX + 52, highBarY + barHeight / 2 - 9, 18, grey);
    DrawRectangleRounded({ (float)(barSectionX + 100), (float)highBarY, (float)maxBarWidth, (float)barHeight }, 0.4f, 8, cardFill);
    if (highBarFillW > 0)
    {
        DrawRectangleRounded({ (float)(barSectionX + 100), (float)highBarY, (float)highBarFillW, (float)barHeight }, 0.4f, 8, red);
    }

    int summaryY = highBarY + barHeight + 40;
    int summaryCardW = (barSectionW - 16) / 2;
    int totalHours = totalMinutes / 60;
    int leftoverMins = totalMinutes % 60;

    DrawRectangleRounded({ (float)barSectionX, (float)summaryY, (float)summaryCardW, 80.0f }, 0.18f, 8, cardFill);
    DrawRectangle(barSectionX, summaryY, 4, 80, blue);
    DrawText("Total Tasks", barSectionX + 20, summaryY + 14, 15, grey);
    DrawText(TextFormat("%d", totalTasks), barSectionX + 20, summaryY + 36, 28, white);

    int rightSummaryCardX = barSectionX + summaryCardW + 16;
    DrawRectangleRounded({ (float)rightSummaryCardX, (float)summaryY, (float)summaryCardW, 80.0f }, 0.18f, 8, cardFill);
    DrawRectangle(rightSummaryCardX, summaryY, 4, 80, orange);
    DrawText("Est. Duration", rightSummaryCardX + 20, summaryY + 14, 15, grey);
    DrawText(TextFormat("%dh %dm", totalHours, leftoverMins), rightSummaryCardX + 20, summaryY + 36, 28, white);
}
