#include "../include/screens/profile.h"
#include "../include/accounts.h"
#include "../include/data.h"
#include "raylib.h"

void drawProfileScreen(int contentX, int contentWidth, int screenHeight)
{
    // -------------------------------------------------------
    // STEP 1: Define all the colors we will use on this page
    // -------------------------------------------------------
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

    // -------------------------------------------------------
    // STEP 2: Fill the background and draw the top gradient
    // -------------------------------------------------------

    // Fill the whole content area with the dark background color
    DrawRectangle(contentX, 0, contentWidth, screenHeight, darkBackground);

    // Draw a gradient band at the very top so the header looks warmer
    DrawRectangleGradientV(contentX, 0, contentWidth, 240, gradientTopColor, darkBackground);

    // -------------------------------------------------------
    // STEP 3: Figure out the username to display
    // -------------------------------------------------------

    // How far from the left edge of the content area we start drawing things
    int leftMargin = 40;

    // The horizontal center of the content area (used for centering things)
    int centerX = contentX + contentWidth / 2;

    // Get the name of the person who is logged in
    const char* loggedInUsername = getLoggedInUser();

    // If nobody is logged in or the name is empty, show "Guest" instead
    const char* displayName = "Guest";
    if (loggedInUsername != nullptr && loggedInUsername[0] != '\0')
    {
        displayName = loggedInUsername;
    }

    // -------------------------------------------------------
    // STEP 4: Draw the avatar circle with the user's initial
    // -------------------------------------------------------

    // The center y position and radius of the avatar circle
    int avatarCenterY = 116;
    int avatarRadius  = 62;

    // Draw a slightly bigger orange circle first — this becomes the border ring
    float avatarBorderRadius = (float)(avatarRadius + 5);
    DrawCircle(centerX, avatarCenterY, avatarBorderRadius, orangeColor);

    // Draw the dark inner circle on top of the orange ring
    DrawCircle(centerX, avatarCenterY, (float)avatarRadius, avatarInnerColor);

    // Get the first character of the username
    char firstLetter = displayName[0];

    // If it is a lowercase letter, convert it to uppercase
    // 'a' is 97 and 'A' is 65, so subtracting 32 makes it uppercase
    if (firstLetter >= 'a' && firstLetter <= 'z')
    {
        firstLetter = firstLetter - 32;
    }

    // Build a tiny string with just that one letter so DrawText can use it
    char avatarInitialText[2];
    avatarInitialText[0] = firstLetter;
    avatarInitialText[1] = '\0';

    // Measure the width of that letter so we can center it
    int initialFontSize  = 56;
    int initialTextWidth = MeasureText(avatarInitialText, initialFontSize);
    int initialDrawX     = centerX - initialTextWidth / 2;
    int initialDrawY     = avatarCenterY - initialFontSize / 2;

    // Draw the initial letter inside the circle
    DrawText(avatarInitialText, initialDrawX, initialDrawY, initialFontSize, orangeLight);

    // -------------------------------------------------------
    // STEP 5: Draw the username text and orange accent line
    // -------------------------------------------------------

    // Measure the username so we can center it
    int nameFontSize  = 30;
    int nameTextWidth = MeasureText(displayName, nameFontSize);
    int nameDrawX     = centerX - nameTextWidth / 2;
    int nameDrawY     = avatarCenterY + avatarRadius + 20;

    // Draw the username below the avatar
    DrawText(displayName, nameDrawX, nameDrawY, nameFontSize, whiteColor);

    // Draw a short orange line under the username as a decorative accent
    int accentLineWidth = nameTextWidth + 48;
    int accentLineX     = centerX - accentLineWidth / 2;
    int accentLineY     = nameDrawY + nameFontSize + 10;
    DrawRectangleRounded(
        { (float)accentLineX, (float)accentLineY, (float)accentLineWidth, 3.0f },
        0.5f, 4, orangeColor
    );

    // -------------------------------------------------------
    // STEP 6: Draw a divider line to separate the header
    // -------------------------------------------------------

    int dividerY = accentLineY + 22;
    DrawRectangle(contentX + leftMargin, dividerY, contentWidth - leftMargin * 2, 1, dividerColor);

    // -------------------------------------------------------
    // STEP 7: Count all the tasks
    // -------------------------------------------------------

    // Get the list of all tasks and the total number of them
    Task* allTasks   = getTaskStore();
    int   totalTasks = getTaskCount();

    // Count how many tasks are completed and how many are still pending
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

    // -------------------------------------------------------
    // STEP 8: Draw the "Statistics" heading
    // -------------------------------------------------------

    int statsHeadingY = dividerY + 28;
    DrawText("Statistics", contentX + leftMargin, statsHeadingY, 22, orangeLight);

    // -------------------------------------------------------
    // STEP 9: Draw the three stat cards
    // -------------------------------------------------------

    // Calculate the size and positions of the three stat cards
    int cardTop    = statsHeadingY + 44;
    int cardHeight = 112;
    int cardGap    = 16;
    int cardWidth  = (contentWidth - leftMargin * 2 - cardGap * 2) / 3;

    // --- Card 1: Total Tasks ---
    int card1X = contentX + leftMargin;
    DrawRectangleRounded(
        { (float)card1X, (float)cardTop, (float)cardWidth, (float)cardHeight },
        0.18f, 8, cardBackground
    );
    DrawRectangle(card1X, cardTop, 4, cardHeight, orangeColor);
    DrawText(TextFormat("%d", totalTasks), card1X + 20, cardTop + 16, 44, whiteColor);
    DrawText("Total Tasks",                card1X + 20, cardTop + 76, 17, greyColor);

    // --- Card 2: Completed Tasks ---
    int card2X = contentX + leftMargin + cardWidth + cardGap;
    DrawRectangleRounded(
        { (float)card2X, (float)cardTop, (float)cardWidth, (float)cardHeight },
        0.18f, 8, cardBackground
    );
    DrawRectangle(card2X, cardTop, 4, cardHeight, greenColor);
    DrawText(TextFormat("%d", completedTaskCount), card2X + 20, cardTop + 16, 44, greenColor);
    DrawText("Completed",                          card2X + 20, cardTop + 76, 17, greyColor);

    // --- Card 3: Pending Tasks ---
    int card3X = contentX + leftMargin + (cardWidth + cardGap) * 2;
    DrawRectangleRounded(
        { (float)card3X, (float)cardTop, (float)cardWidth, (float)cardHeight },
        0.18f, 8, cardBackground
    );
    DrawRectangle(card3X, cardTop, 4, cardHeight, redColor);
    DrawText(TextFormat("%d", pendingTaskCount), card3X + 20, cardTop + 16, 44, redColor);
    DrawText("Pending",                          card3X + 20, cardTop + 76, 17, greyColor);

    // -------------------------------------------------------
    // STEP 10: Draw the completion rate progress bar
    // -------------------------------------------------------

    int rateHeadingY = cardTop + cardHeight + 38;
    DrawText("Completion Rate", contentX + leftMargin, rateHeadingY, 22, orangeLight);

    // Calculate what fraction of tasks are done, as a number between 0.0 and 1.0
    float completionRate = 0.0f;
    if (totalTasks > 0)
    {
        completionRate = (float)completedTaskCount / (float)totalTasks;
    }

    // Draw the grey background track of the progress bar
    int barY      = rateHeadingY + 42;
    int barWidth  = contentWidth - leftMargin * 2 - 90;
    int barHeight = 22;
    DrawRectangleRounded(
        { (float)(contentX + leftMargin), (float)barY, (float)barWidth, (float)barHeight },
        0.5f, 8, barBackgroundColor
    );

    // Calculate how wide the green filled portion should be
    int filledWidth = (int)((float)barWidth * completionRate);

    // Only draw the green fill if it is at least a few pixels wide
    if (filledWidth > 4)
    {
        DrawRectangleRounded(
            { (float)(contentX + leftMargin), (float)barY, (float)filledWidth, (float)barHeight },
            0.5f, 8, greenColor
        );
    }

    // Draw the percentage number to the right of the bar
    int percentFontSize = 20;
    int percentX        = contentX + leftMargin + barWidth + 14;
    int percentY        = barY + barHeight / 2 - percentFontSize / 2;
    DrawText(TextFormat("%.0f%%", completionRate * 100.0f), percentX, percentY, percentFontSize, whiteColor);

    // -------------------------------------------------------
    // STEP 11: Draw the "Recent Tasks" list
    // -------------------------------------------------------

    int recentHeadingY = barY + barHeight + 44;
    DrawText("Recent Tasks", contentX + leftMargin, recentHeadingY, 22, orangeLight);
    DrawRectangle(contentX + leftMargin, recentHeadingY + 34, contentWidth - leftMargin * 2, 1, dividerColor);

    // If there are no tasks at all, show a friendly empty message and stop drawing
    if (totalTasks == 0)
    {
        DrawText(
            "No tasks yet.  Head to All Tasks to add your first one.",
            contentX + leftMargin, recentHeadingY + 58, 19, greyColor
        );
        return;
    }

    // Decide how many tasks to show — at most 5, but fewer if there are not enough
    int numberOfTasksToShow = 5;
    if (totalTasks < 5)
    {
        numberOfTasksToShow = totalTasks;
    }

    // The height of each task row and the gap between rows
    int rowHeight = 56;
    int rowGap    = 6;
    int rowWidth  = contentWidth - leftMargin * 2;

    // Draw each recent task row
    for (int i = 0; i < numberOfTasksToShow; i = i + 1)
    {
        // Start from the last task and go backwards so the newest appears first
        int taskIndex    = totalTasks - 1 - i;
        Task currentTask = allTasks[taskIndex];

        int rowY = recentHeadingY + 46 + i * (rowHeight + rowGap);
        int rowX = contentX + leftMargin;

        // Draw the dark card background for this row
        DrawRectangleRounded(
            { (float)rowX, (float)rowY, (float)rowWidth, (float)rowHeight },
            0.16f, 8, cardBackground
        );

        // Make sure the priority is a valid number (1, 2, or 3)
        int priority = currentTask.priority;
        if (priority < 1 || priority > 3)
        {
            priority = 1;
        }

        // Pick the color of the left accent bar based on priority
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

        // Draw the colored left accent bar
        DrawRectangle(rowX, rowY, 4, rowHeight, accentBarColor);

        // Completed tasks show their title in grey, active tasks in white
        Color titleColor = whiteColor;
        if (currentTask.completed == true)
        {
            titleColor = greyColor;
        }

        // Draw the task title
        DrawText(currentTask.title, rowX + 20, rowY + rowHeight / 2 - 10, 19, titleColor);

        // Pick the status label and its color
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

        // Draw the status label and the deadline on the right side of the row
        DrawText(statusLabel,          rowX + rowWidth - 200, rowY + rowHeight / 2 - 8, 15, statusColor);
        DrawText(currentTask.deadline, rowX + rowWidth - 130, rowY + rowHeight / 2 - 8, 15, greyColor);
    }
}
