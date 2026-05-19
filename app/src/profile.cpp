#include "../include/screens/profile.h"
#include "../include/accounts.h"
#include "../include/data.h"
#include "raylib.h"

void drawProfileScreen(int contentX, int contentWidth, int screenHeight)
{
    Color darkBackground    = {  22,  18,  14, 255 };
    Color cardBackground    = {  35,  28,  20, 255 };
    Color orangeColor       = { 234, 108,  15, 255 };
    Color orangeLight       = { 251, 146,  60, 255 };
    Color whiteColor        = { 255, 255, 255, 255 };
    Color greyColor         = { 180, 160, 140, 255 };
    Color greenColor        = {  80, 200, 100, 255 };
    Color redColor          = { 220,  80,  60, 255 };
    Color dividerColor      = {  55,  46,  36, 255 };
    Color avatarInnerColor  = {  50,  32,  12, 255 };
    Color gradientTopColor  = {  42,  28,  14, 255 };
    Color barBackgroundColor = { 50,  38,  28, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, darkBackground);
    DrawRectangleGradientV(contentX, 0, contentWidth, 240, gradientTopColor, darkBackground);

    int leftMargin = 40;
    int centerX = contentX + contentWidth / 2;

    const char* loggedInUsername = getLoggedInUser();

    const char* displayName = "Guest";
    if (loggedInUsername != nullptr && loggedInUsername[0] != '\0')
    {
        displayName = loggedInUsername;
    }

    int avatarCenterY = 116;
    int avatarRadius  = 62;

    float avatarBorderRadius = (float)(avatarRadius + 5);
    DrawCircle(centerX, avatarCenterY, avatarBorderRadius, orangeColor);
    DrawCircle(centerX, avatarCenterY, (float)avatarRadius, avatarInnerColor);

    char firstLetter = displayName[0];

    // subtract 32 to convert lowercase ascii to uppercase
    if (firstLetter >= 'a' && firstLetter <= 'z')
    {
        firstLetter = firstLetter - 32;
    }

    char avatarInitialText[2];
    avatarInitialText[0] = firstLetter;
    avatarInitialText[1] = '\0';

    int initialFontSize  = 56;
    int initialTextWidth = MeasureText(avatarInitialText, initialFontSize);
    int initialDrawX     = centerX - initialTextWidth / 2;
    int initialDrawY     = avatarCenterY - initialFontSize / 2;

    DrawText(avatarInitialText, initialDrawX, initialDrawY, initialFontSize, orangeLight);

    int nameFontSize  = 30;
    int nameTextWidth = MeasureText(displayName, nameFontSize);
    int nameDrawX     = centerX - nameTextWidth / 2;
    int nameDrawY     = avatarCenterY + avatarRadius + 20;

    DrawText(displayName, nameDrawX, nameDrawY, nameFontSize, whiteColor);

    int accentLineWidth = nameTextWidth + 48;
    int accentLineX     = centerX - accentLineWidth / 2;
    int accentLineY     = nameDrawY + nameFontSize + 10;
    DrawRectangleRounded(
        { (float)accentLineX, (float)accentLineY, (float)accentLineWidth, 3.0f },
        0.5f, 4, orangeColor
    );

    int dividerY = accentLineY + 22;
    DrawRectangle(contentX + leftMargin, dividerY, contentWidth - leftMargin * 2, 1, dividerColor);

    Task* allTasks   = getTaskStore();
    int   totalTasks = getTaskCount();

    int completedTaskCount = 0;
    int pendingTaskCount   = 0;

    for (int i = 0; i < totalTasks; i = i + 1)
    {
        if (allTasks[i].completed == true)
        {
            completedTaskCount = completedTaskCount + 1;
        }
        else
        {
            pendingTaskCount = pendingTaskCount + 1;
        }
    }

    int statsHeadingY = dividerY + 28;
    DrawText("Statistics", contentX + leftMargin, statsHeadingY, 22, orangeLight);

    int cardTop    = statsHeadingY + 44;
    int cardHeight = 112;
    int cardGap    = 16;
    int cardWidth  = (contentWidth - leftMargin * 2 - cardGap * 2) / 3;

    int card1X = contentX + leftMargin;
    DrawRectangleRounded(
        { (float)card1X, (float)cardTop, (float)cardWidth, (float)cardHeight },
        0.18f, 8, cardBackground
    );
    DrawRectangle(card1X, cardTop, 4, cardHeight, orangeColor);
    DrawText(TextFormat("%d", totalTasks), card1X + 20, cardTop + 16, 44, whiteColor);
    DrawText("Total Tasks",                card1X + 20, cardTop + 76, 17, greyColor);

    int card2X = contentX + leftMargin + cardWidth + cardGap;
    DrawRectangleRounded(
        { (float)card2X, (float)cardTop, (float)cardWidth, (float)cardHeight },
        0.18f, 8, cardBackground
    );
    DrawRectangle(card2X, cardTop, 4, cardHeight, greenColor);
    DrawText(TextFormat("%d", completedTaskCount), card2X + 20, cardTop + 16, 44, greenColor);
    DrawText("Completed",                          card2X + 20, cardTop + 76, 17, greyColor);

    int card3X = contentX + leftMargin + (cardWidth + cardGap) * 2;
    DrawRectangleRounded(
        { (float)card3X, (float)cardTop, (float)cardWidth, (float)cardHeight },
        0.18f, 8, cardBackground
    );
    DrawRectangle(card3X, cardTop, 4, cardHeight, redColor);
    DrawText(TextFormat("%d", pendingTaskCount), card3X + 20, cardTop + 16, 44, redColor);
    DrawText("Pending",                          card3X + 20, cardTop + 76, 17, greyColor);

    int rateHeadingY = cardTop + cardHeight + 38;
    DrawText("Completion Rate", contentX + leftMargin, rateHeadingY, 22, orangeLight);

    float completionRate = 0.0f;
    if (totalTasks > 0)
    {
        completionRate = (float)completedTaskCount / (float)totalTasks;
    }

    int barY      = rateHeadingY + 42;
    int barWidth  = contentWidth - leftMargin * 2 - 90;
    int barHeight = 22;
    DrawRectangleRounded(
        { (float)(contentX + leftMargin), (float)barY, (float)barWidth, (float)barHeight },
        0.5f, 8, barBackgroundColor
    );

    int filledWidth = (int)((float)barWidth * completionRate);

    if (filledWidth > 4)
    {
        DrawRectangleRounded(
            { (float)(contentX + leftMargin), (float)barY, (float)filledWidth, (float)barHeight },
            0.5f, 8, greenColor
        );
    }

    int percentFontSize = 20;
    int percentX        = contentX + leftMargin + barWidth + 14;
    int percentY        = barY + barHeight / 2 - percentFontSize / 2;
    DrawText(TextFormat("%.0f%%", completionRate * 100.0f), percentX, percentY, percentFontSize, whiteColor);

    int recentHeadingY = barY + barHeight + 44;
    DrawText("Recent Tasks", contentX + leftMargin, recentHeadingY, 22, orangeLight);
    DrawRectangle(contentX + leftMargin, recentHeadingY + 34, contentWidth - leftMargin * 2, 1, dividerColor);

    if (totalTasks == 0)
    {
        DrawText(
            "No tasks yet.  Head to All Tasks to add your first one.",
            contentX + leftMargin, recentHeadingY + 58, 19, greyColor
        );
        return;
    }

    int numberOfTasksToShow = 5;
    if (totalTasks < 5)
    {
        numberOfTasksToShow = totalTasks;
    }

    int rowHeight = 56;
    int rowGap    = 6;
    int rowWidth  = contentWidth - leftMargin * 2;

    // iterate newest-first so the most recently added task appears at the top
    for (int i = 0; i < numberOfTasksToShow; i = i + 1)
    {
        int taskIndex    = totalTasks - 1 - i;
        Task currentTask = allTasks[taskIndex];

        int rowY = recentHeadingY + 46 + i * (rowHeight + rowGap);
        int rowX = contentX + leftMargin;

        DrawRectangleRounded(
            { (float)rowX, (float)rowY, (float)rowWidth, (float)rowHeight },
            0.16f, 8, cardBackground
        );

        int priority = currentTask.priority;
        if (priority < 1 || priority > 3)
        {
            priority = 1;
        }

        Color accentBarColor = greyColor;
        if (priority == 1)
        {
            accentBarColor = greenColor;
        }
        else if (priority == 2)
        {
            accentBarColor = orangeLight;
        }
        else if (priority == 3)
        {
            accentBarColor = redColor;
        }

        DrawRectangle(rowX, rowY, 4, rowHeight, accentBarColor);

        Color titleColor = whiteColor;
        if (currentTask.completed == true)
        {
            titleColor = greyColor;
        }

        DrawText(currentTask.title, rowX + 20, rowY + rowHeight / 2 - 10, 19, titleColor);

        const char* statusLabel = "Low";
        Color       statusColor = greenColor;

        if (currentTask.completed == true)
        {
            statusLabel = "Done";
            statusColor = greenColor;
        }
        else if (priority == 2)
        {
            statusLabel = "Med";
            statusColor = orangeLight;
        }
        else if (priority == 3)
        {
            statusLabel = "High";
            statusColor = redColor;
        }

        DrawText(statusLabel,          rowX + rowWidth - 200, rowY + rowHeight / 2 - 8, 15, statusColor);
        DrawText(currentTask.deadline, rowX + rowWidth - 130, rowY + rowHeight / 2 - 8, 15, greyColor);
    }
}
