#include "../include/presentation.h"
#include "../include/accounts.h"
#include "../include/db.h"
#include "../include/data.h"
#include "../include/screens/dashboard.h"
#include "../include/screens/tasks.h"
#include "../include/screens/archive.h"
#include "../include/screens/calendar.h"
#include "../include/screens/statistics.h"
#include "../include/screens/profile.h"
#include "../include/screens/login.h"

#include <cstdio>
#include <cstring>
#include "../include/logic.h"

static bool s_deleteConfirmIsOpen    = false;
static char s_deleteErrorMessage[128] = {};
static int  s_focusedInputBox        = -1;
static char s_newUsernameText[64]    = {};
static char s_usernameResultMessage[128] = {};
static bool s_usernameChangeSucceeded   = false;
static char s_currentPasswordText[64]   = {};
static char s_newPasswordText[64]       = {};
static char s_confirmPasswordText[64]   = {};
static char s_passwordResultMessage[128] = {};
static bool s_passwordChangeSucceeded   = false;

static bool s_tasksDropdownOpen      = false;
static bool s_showNewCategoryInput   = false;
static char s_newCategoryNameBuf[64] = {};

int SIDEBAR_W = 300;
int BTN_H     = 62;
int BTN_GAP   = 8;
int BTN_X     = 16;
int BTN_W     = 268;
int NAV_START = 110;

Color COLOR_SIDEBAR  = {  28,  24,  20, 255 };
Color COLOR_HOVER    = {  45,  38,  30, 255 };
Color COLOR_ACTIVE   = { 234, 108,  15, 255 };
Color COLOR_ACCENT   = { 251, 146,  60, 255 };
Color COLOR_DIVIDER  = {  55,  46,  36, 255 };
Color COLOR_BRAND_A  = { 251, 146,  60, 255 };
Color COLOR_WHITE    = { 255, 255, 255, 255 };
Color COLOR_GREY     = { 180, 160, 140, 255 };
Color COLOR_DOT_IDLE = { 120, 100,  80, 255 };

static int s_newCategoryColorIndex = 0;

static Color parseHexColor(const char* hexString)
{
    Color resultColor = { 234, 108, 15, 255 };

    bool hexStringIsValid = (hexString != nullptr && hexString[0] == '#');
    if (hexStringIsValid == false)
    {
        return resultColor;
    }

    char redChars[3];
    redChars[0] = hexString[1];
    redChars[1] = hexString[2];
    redChars[2] = '\0';

    char greenChars[3];
    greenChars[0] = hexString[3];
    greenChars[1] = hexString[4];
    greenChars[2] = '\0';

    char blueChars[3];
    blueChars[0] = hexString[5];
    blueChars[1] = hexString[6];
    blueChars[2] = '\0';

    int redValue   = 0;
    int greenValue = 0;
    int blueValue  = 0;

    int scanResult = 0;
    scanResult = sscanf(redChars,   "%x", &redValue);
    scanResult = sscanf(greenChars, "%x", &greenValue);
    scanResult = sscanf(blueChars,  "%x", &blueValue);
    (void)scanResult;

    resultColor.r = (unsigned char)redValue;
    resultColor.g = (unsigned char)greenValue;
    resultColor.b = (unsigned char)blueValue;

    return resultColor;
}

bool settingsDeleteModalIsOpen()
{
    return s_deleteConfirmIsOpen;
}

void drawOneButton(const char* label, int x, int y, int w, int h, bool isActive, bool isHovered)
{
    if (isActive == true)
    {
        DrawRectangle(x, y + 10, 4, h - 20, COLOR_ACCENT);
        DrawRectangleRounded({ (float)(x + 4), (float)y, (float)(w - 4), (float)h }, 0.28f, 8, COLOR_ACTIVE);
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_WHITE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_WHITE);
    }
    else if (isHovered == true)
    {
        DrawRectangleRounded({ (float)x, (float)y, (float)w, (float)h }, 0.28f, 8, COLOR_HOVER);
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_DOT_IDLE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_GREY);
    }
    else
    {
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_DOT_IDLE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_GREY);
    }
}

static void drawDropdownItem(const char* label, int x, int y, int w, int h, bool isActive, bool isHovered, bool isIndented)
{
    int textX = x + 52;
    if (isIndented)
    {
        textX = x + 68;
    }

    if (isActive == true)
    {
        DrawRectangleRounded({ (float)(x + 4), (float)y, (float)(w - 4), (float)h }, 0.28f, 8, { 50, 38, 24, 255 });
        DrawRectangle(x + 4, y + 6, 3, h - 12, COLOR_ACCENT);
        DrawText(label, textX, y + h / 2 - 9, 18, COLOR_ACCENT);
    }
    else if (isHovered == true)
    {
        DrawRectangleRounded({ (float)(x + 4), (float)y, (float)(w - 4), (float)h }, 0.28f, 8, { 40, 33, 26, 255 });
        DrawText(label, textX, y + h / 2 - 9, 18, COLOR_WHITE);
    }
    else
    {
        DrawText(label, textX, y + h / 2 - 9, 18, COLOR_GREY);
    }
}

void drawSidebar(AppScreen current, AppScreen* outHovered, int screenHeight)
{
    DrawRectangle(0, 0, SIDEBAR_W, screenHeight, COLOR_SIDEBAR);
    DrawRectangleGradientH(SIDEBAR_W - 10, 0, 10, screenHeight, { 0, 0, 0, 0 }, { 0, 0, 0, 60 });

    DrawText("Task", 28, 34, 36, COLOR_BRAND_A);
    DrawText("Dash", 28 + MeasureText("Task", 36) + 8, 34, 36, COLOR_WHITE);

    DrawRectangle(20, 88, SIDEBAR_W - 40, 1, COLOR_DIVIDER);

    *outHovered = (AppScreen)-1;

    Vector2 mousePosition = GetMousePosition();

    int dashboardButtonY  = NAV_START;
    int tasksButtonY      = dashboardButtonY + BTN_H + BTN_GAP;

    int dropdownItemH     = 36;
    int dropdownTotalH    = 0;

    if (s_tasksDropdownOpen == true)
    {
        int numberOfCategoryItems = getCategoryCount();
        dropdownTotalH = dropdownItemH;
        dropdownTotalH = dropdownTotalH + numberOfCategoryItems * dropdownItemH;
        dropdownTotalH = dropdownTotalH + dropdownItemH;
        if (s_showNewCategoryInput == true)
        {
            dropdownTotalH = dropdownTotalH + 44;
        }
    }

    int statisticsButtonY = tasksButtonY + BTN_H + BTN_GAP + dropdownTotalH;
    int archiveButtonY    = statisticsButtonY + BTN_H + BTN_GAP;
    int calendarButtonY   = archiveButtonY + BTN_H + BTN_GAP;

    Rectangle dashboardArea = { (float)BTN_X, (float)dashboardButtonY, (float)BTN_W, (float)BTN_H };
    bool mouseOverDashboard = CheckCollisionPointRec(mousePosition, dashboardArea);
    if (mouseOverDashboard == true)
    {
        *outHovered = SCREEN_DASHBOARD;
    }
    bool dashIsActive = (current == SCREEN_DASHBOARD);
    drawOneButton("Dashboard", BTN_X, dashboardButtonY, BTN_W, BTN_H, dashIsActive, mouseOverDashboard);

    Rectangle tasksArea = { (float)BTN_X, (float)tasksButtonY, (float)BTN_W, (float)BTN_H };
    bool mouseOverTasks = CheckCollisionPointRec(mousePosition, tasksArea);
    if (mouseOverTasks == true)
    {
        *outHovered = SCREEN_ALL_TASKS;
    }
    if (mouseOverTasks == true && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        s_tasksDropdownOpen = !s_tasksDropdownOpen;
        if (s_tasksDropdownOpen == false)
        {
            s_showNewCategoryInput = false;
        }
    }
    bool tasksIsActive = (current == SCREEN_ALL_TASKS);

    const char* tasksArrow = s_tasksDropdownOpen ? "v Tasks" : "> Tasks";
    drawOneButton(tasksArrow, BTN_X, tasksButtonY, BTN_W, BTN_H, tasksIsActive, mouseOverTasks);

    if (s_tasksDropdownOpen == true)
    {
        const char* currentFilter = getCurrentCategoryFilter();

        int itemY = tasksButtonY + BTN_H + 2;

        Rectangle allTasksArea = { (float)(BTN_X + 8), (float)itemY, (float)(BTN_W - 8), (float)dropdownItemH };
        bool mouseOverAllTasks = CheckCollisionPointRec(mousePosition, allTasksArea);
        if (mouseOverAllTasks == true)
        {
            *outHovered = SCREEN_ALL_TASKS;
        }
        if (mouseOverAllTasks == true && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            setCurrentCategoryFilter("");
            *outHovered = SCREEN_ALL_TASKS;
        }

        bool allTasksIsActive = (currentFilter[0] == '\0');
        drawDropdownItem("All Tasks", BTN_X + 8, itemY, BTN_W - 8, dropdownItemH, allTasksIsActive, mouseOverAllTasks, true);

        itemY = itemY + dropdownItemH;

        int numberOfCategories = getCategoryCount();
        Category* categoryList = getCategoryStore();

        bool categoryClickedThisFrame = false;

        int catIndex = 0;
        while (catIndex < numberOfCategories)
        {
            Rectangle catArea = { (float)(BTN_X + 8), (float)itemY, (float)(BTN_W - 8), (float)dropdownItemH };
            bool mouseOverCat = CheckCollisionPointRec(mousePosition, catArea);

            int  deleteBtnSize = 18;
            int  deleteBtnX    = BTN_X + BTN_W - deleteBtnSize - 8;
            int  deleteBtnY    = itemY + (dropdownItemH - deleteBtnSize) / 2;
            Rectangle deleteBtnArea = { (float)deleteBtnX, (float)deleteBtnY, (float)deleteBtnSize, (float)deleteBtnSize };
            bool mouseOverDeleteBtn = CheckCollisionPointRec(mousePosition, deleteBtnArea);

            if (mouseOverDeleteBtn == true && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                char deletedCategoryName[64] = {};
                int  nameCharIndex = 0;
                while (nameCharIndex < 63 && categoryList[catIndex].name[nameCharIndex] != '\0')
                {
                    deletedCategoryName[nameCharIndex] = categoryList[catIndex].name[nameCharIndex];
                    nameCharIndex = nameCharIndex + 1;
                }
                deletedCategoryName[nameCharIndex] = '\0';

                deleteCategoryFromDb(categoryList[catIndex].id);
                removeCategoryAtIndex(catIndex);

                bool deletedCategoryWasActive = textsAreEqual(currentFilter, deletedCategoryName);
                if (deletedCategoryWasActive == true)
                {
                    setCurrentCategoryFilter("");
                }

                numberOfCategories = getCategoryCount();
                categoryList       = getCategoryStore();
                categoryClickedThisFrame = true;
                continue;
            }

            if (mouseOverCat == true)
            {
                *outHovered = SCREEN_ALL_TASKS;
            }
            if (mouseOverCat == true && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                setCurrentCategoryFilter(categoryList[catIndex].name);
                *outHovered = SCREEN_ALL_TASKS;
                categoryClickedThisFrame = true;
            }

            bool catIsActive = textsAreEqual(currentFilter, categoryList[catIndex].name);

            drawDropdownItem(categoryList[catIndex].name, BTN_X + 8, itemY, BTN_W - 8, dropdownItemH, catIsActive, mouseOverCat, true);

            Color categoryDotColor = parseHexColor(categoryList[catIndex].color);
            int   dotCenterX       = BTN_X + 56;
            int   dotCenterY       = itemY + dropdownItemH / 2;
            DrawCircle(dotCenterX, dotCenterY, 5, categoryDotColor);

            if (mouseOverCat == true)
            {
                Color deleteBtnColor = { 120, 80, 80, 255 };
                if (mouseOverDeleteBtn == true)
                {
                    deleteBtnColor = { 220, 60, 60, 255 };
                }
                DrawText("x", deleteBtnX + 5, deleteBtnY + 3, 14, deleteBtnColor);
            }

            itemY = itemY + dropdownItemH;
            catIndex = catIndex + 1;
        }

        Rectangle newListArea = { (float)(BTN_X + 8), (float)itemY, (float)(BTN_W - 8), (float)dropdownItemH };
        bool mouseOverNewList = CheckCollisionPointRec(mousePosition, newListArea);
        if (mouseOverNewList == true)
        {
            *outHovered = (AppScreen)(-2);
        }
        if (mouseOverNewList == true && categoryClickedThisFrame == false && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            s_showNewCategoryInput = !s_showNewCategoryInput;
            if (s_showNewCategoryInput == false)
            {
                s_newCategoryNameBuf[0] = '\0';
            }
        }

        drawDropdownItem("+ New List", BTN_X + 8, itemY, BTN_W - 8, dropdownItemH, s_showNewCategoryInput, mouseOverNewList, true);

        itemY = itemY + dropdownItemH;

        if (s_showNewCategoryInput == true)
        {
            int inputBoxX = BTN_X + 12;
            int inputBoxY = itemY;
            int inputBoxW = BTN_W - 24;
            int inputBoxH = 36;

            Rectangle inputArea = { (float)inputBoxX, (float)inputBoxY, (float)inputBoxW, (float)inputBoxH };

            DrawRectangleRounded(inputArea, 0.25f, 8, { 42, 35, 26, 255 });
            DrawRectangleRoundedLines(inputArea, 0.25f, 8, 1.5f, COLOR_ACCENT);

            int nameLen = 0;
            while (s_newCategoryNameBuf[nameLen] != '\0')
            {
                nameLen = nameLen + 1;
            }

            if (nameLen == 0)
            {
                DrawText("List name...", inputBoxX + 10, inputBoxY + 10, 15, { 100, 85, 70, 255 });
            }
            else
            {
                DrawText(s_newCategoryNameBuf, inputBoxX + 10, inputBoxY + 10, 15, COLOR_WHITE);
            }

            const char* colorChoices[5] = { "#E46C0F", "#50C864", "#5A8CDC", "#DC503C", "#9B59B6" };

            int colorCircleY       = inputBoxY + inputBoxH + 8;
            int colorCircleRadius  = 9;
            int colorCircleSpacing = 26;

            int colorIndex = 0;
            while (colorIndex < 5)
            {
                int circleX = inputBoxX + 14 + colorIndex * colorCircleSpacing;

                Color circleColor = parseHexColor(colorChoices[colorIndex]);
                DrawCircle((float)circleX, (float)(colorCircleY + colorCircleRadius), (float)colorCircleRadius, circleColor);

                bool thisColorIsSelected = (s_newCategoryColorIndex == colorIndex);
                if (thisColorIsSelected == true)
                {
                    DrawCircleLines((float)circleX, (float)(colorCircleY + colorCircleRadius), (float)(colorCircleRadius + 2), COLOR_WHITE);
                }

                Vector2 mousePos   = GetMousePosition();
                float   distanceX  = (float)(mousePos.x - circleX);
                float   distanceY  = (float)(mousePos.y - (colorCircleY + colorCircleRadius));
                float   distanceSq = distanceX * distanceX + distanceY * distanceY;
                float   radiusSq   = (float)(colorCircleRadius * colorCircleRadius);

                bool mouseIsOverCircle = (distanceSq <= radiusSq);
                bool mouseWasClicked   = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

                if (mouseIsOverCircle == true && mouseWasClicked == true)
                {
                    s_newCategoryColorIndex = colorIndex;
                }

                colorIndex = colorIndex + 1;
            }

            int typedChar = GetCharPressed();
            while (typedChar > 0)
            {
                if (typedChar >= 32 && typedChar <= 126 && nameLen < 63)
                {
                    s_newCategoryNameBuf[nameLen] = (char)typedChar;
                    nameLen = nameLen + 1;
                    s_newCategoryNameBuf[nameLen] = '\0';
                }
                typedChar = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && nameLen > 0)
            {
                s_newCategoryNameBuf[nameLen - 1] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) && nameLen > 0)
            {
                const char* colorChoices[5] = { "#E46C0F", "#50C864", "#5A8CDC", "#DC503C", "#9B59B6" };
                const char* loggedInUser = getLoggedInUser();
                int newId = saveNewCategory(loggedInUser, s_newCategoryNameBuf, colorChoices[s_newCategoryColorIndex]);
                if (newId > 0)
                {
                    Category freshCategory = {};
                    freshCategory.id = newId;

                    int copyIndex = 0;
                    while (copyIndex < 63 && s_newCategoryNameBuf[copyIndex] != '\0')
                    {
                        freshCategory.name[copyIndex] = s_newCategoryNameBuf[copyIndex];
                        copyIndex = copyIndex + 1;
                    }
                    freshCategory.name[copyIndex] = '\0';

                    int colorCopyIndex = 0;
                    while (colorCopyIndex < 7 && colorChoices[s_newCategoryColorIndex][colorCopyIndex] != '\0')
                    {
                        freshCategory.color[colorCopyIndex] = colorChoices[s_newCategoryColorIndex][colorCopyIndex];
                        colorCopyIndex = colorCopyIndex + 1;
                    }
                    freshCategory.color[colorCopyIndex] = '\0';

                    addCategoryToStore(freshCategory);
                    s_newCategoryNameBuf[0] = '\0';
                    s_showNewCategoryInput  = false;
                    s_newCategoryColorIndex = 0;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                s_newCategoryNameBuf[0] = '\0';
                s_showNewCategoryInput  = false;
                s_newCategoryColorIndex = 0;
            }

            int formAreaBottom = colorCircleY + colorCircleRadius * 2 + 8;
            Rectangle formBoundingRect = { (float)BTN_X, (float)(inputBoxY - 4), (float)BTN_W, (float)(formAreaBottom - inputBoxY + 4) };
            bool mouseInsideForm    = CheckCollisionPointRec(mousePosition, formBoundingRect);
            bool mouseInsideNewList = CheckCollisionPointRec(mousePosition, newListArea);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouseInsideForm == false && mouseInsideNewList == false)
            {
                s_newCategoryNameBuf[0] = '\0';
                s_showNewCategoryInput  = false;
                s_newCategoryColorIndex = 0;
            }
        }
    }

    Rectangle statisticsArea = { (float)BTN_X, (float)statisticsButtonY, (float)BTN_W, (float)BTN_H };
    bool mouseOverStatistics = CheckCollisionPointRec(mousePosition, statisticsArea);
    if (mouseOverStatistics == true)
    {
        *outHovered = SCREEN_STATISTICS;
    }
    bool statsIsActive = (current == SCREEN_STATISTICS);
    drawOneButton("Statistics", BTN_X, statisticsButtonY, BTN_W, BTN_H, statsIsActive, mouseOverStatistics);

    Rectangle archiveArea = { (float)BTN_X, (float)archiveButtonY, (float)BTN_W, (float)BTN_H };
    bool mouseOverArchive = CheckCollisionPointRec(mousePosition, archiveArea);
    if (mouseOverArchive == true)
    {
        *outHovered = SCREEN_ARCHIVE;
    }
    bool archiveIsActive = (current == SCREEN_ARCHIVE);
    drawOneButton("Archive", BTN_X, archiveButtonY, BTN_W, BTN_H, archiveIsActive, mouseOverArchive);

    Rectangle calendarArea = { (float)BTN_X, (float)calendarButtonY, (float)BTN_W, (float)BTN_H };
    bool mouseOverCalendar = CheckCollisionPointRec(mousePosition, calendarArea);
    if (mouseOverCalendar == true)
    {
        *outHovered = SCREEN_CALENDAR;
    }
    bool calendarIsActive = (current == SCREEN_CALENDAR);
    drawOneButton("Calendar", BTN_X, calendarButtonY, BTN_W, BTN_H, calendarIsActive, mouseOverCalendar);

    DrawRectangle(20, screenHeight - 180, SIDEBAR_W - 40, 1, COLOR_DIVIDER);

    const char* bottomLabels[3]   = { "My Profile", "Settings", "Logout" };
    AppScreen   bottomScreens[3]  = { SCREEN_PROFILE, SCREEN_SETTINGS, SCREEN_LOGOUT };

    int bottomGroupStartY = screenHeight - 160 - (BTN_H + BTN_GAP);

    int i = 0;
    while (i < 3)
    {
        int buttonX = BTN_X;
        int buttonY = bottomGroupStartY + i * (BTN_H + BTN_GAP);

        Rectangle buttonArea = { (float)buttonX, (float)buttonY, (float)BTN_W, (float)BTN_H };
        bool mouseIsOverButton = CheckCollisionPointRec(mousePosition, buttonArea);

        if (mouseIsOverButton == true)
        {
            *outHovered = bottomScreens[i];
        }

        bool thisButtonIsActive = false;
        if (bottomScreens[i] != SCREEN_LOGOUT && current == bottomScreens[i])
        {
            thisButtonIsActive = true;
        }

        drawOneButton(bottomLabels[i], buttonX, buttonY, BTN_W, BTN_H, thisButtonIsActive, mouseIsOverButton);

        i = i + 1;
    }
}

static bool drawTextInputBox(
    char*       textBuffer,
    int         maxCharacters,
    const char* labelText,
    int         posX,
    int         posY,
    int         boxWidth,
    int         boxHeight,
    bool        isCurrentlyFocused,
    bool        hideTextAsStars)
{
    Color boxBackgroundColor;
    Color boxBorderColor;

    if (isCurrentlyFocused == true)
    {
        boxBackgroundColor = { 42,  35,  26, 255 };
        boxBorderColor     = { 234, 108,  15, 255 };
    }
    else
    {
        boxBackgroundColor = { 32,  27,  21, 255 };
        boxBorderColor     = {  55,  46,  36, 255 };
    }

    DrawText(labelText, posX, posY - 24, 17, { 180, 160, 140, 255 });

    DrawRectangleRounded(
        { (float)(posX - 2), (float)(posY - 2), (float)(boxWidth + 4), (float)(boxHeight + 4) },
        0.22f, 8, boxBorderColor
    );

    DrawRectangleRounded(
        { (float)posX, (float)posY, (float)boxWidth, (float)boxHeight },
        0.22f, 8, boxBackgroundColor
    );

    int characterCount = 0;
    while (textBuffer[characterCount] != '\0')
    {
        characterCount = characterCount + 1;
    }

    char displayText[256];
    int charIndex = 0;
    while (charIndex < characterCount && charIndex < 255)
    {
        if (hideTextAsStars == true)
        {
            displayText[charIndex] = '*';
        }
        else
        {
            displayText[charIndex] = textBuffer[charIndex];
        }
        charIndex = charIndex + 1;
    }
    displayText[charIndex] = '\0';

    int textFontSize = 19;
    int textDrawX    = posX + 14;
    int textDrawY    = posY + boxHeight / 2 - textFontSize / 2;
    DrawText(displayText, textDrawX, textDrawY, textFontSize, { 255, 255, 255, 255 });

    if (isCurrentlyFocused == true)
    {
        int cursorX = posX + 14 + MeasureText(displayText, textFontSize);

        float currentTime  = (float)GetTime();
        int   timeAsInt    = (int)(currentTime * 2.0f);
        bool  cursorIsOn   = (timeAsInt % 2 == 0);

        if (cursorIsOn == true)
        {
            DrawRectangle(cursorX + 2, posY + 10, 2, boxHeight - 20, { 251, 146, 60, 255 });
        }

        int typedCharacter = GetCharPressed();
        while (typedCharacter > 0)
        {
            bool characterIsPrintable = (typedCharacter >= 32 && typedCharacter <= 126);
            bool thereIsRoomForMore   = (characterCount < maxCharacters - 1);

            if (characterIsPrintable == true && thereIsRoomForMore == true)
            {
                textBuffer[characterCount]     = (char)typedCharacter;
                textBuffer[characterCount + 1] = '\0';
            }

            typedCharacter = GetCharPressed();
        }

        bool backspaceWasPressed = IsKeyPressed(KEY_BACKSPACE);
        bool thereAreCharacters  = (characterCount > 0);
        if (backspaceWasPressed == true && thereAreCharacters == true)
        {
            textBuffer[characterCount - 1] = '\0';
        }
    }

    Vector2   mousePosition   = GetMousePosition();
    Rectangle boxHitArea      = { (float)posX, (float)posY, (float)boxWidth, (float)boxHeight };
    bool      mouseIsInBox    = CheckCollisionPointRec(mousePosition, boxHitArea);
    bool      mouseWasClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    return mouseIsInBox && mouseWasClicked;
}

void drawSettingsScreen(int contentX, int contentWidth, int screenHeight, bool* outAccountDeleted)
{
    if (outAccountDeleted != nullptr)
    {
        *outAccountDeleted = false;
    }

    Color darkBackground = {  22,  18,  14, 255 };
    Color orangeColor    = { 234, 108,  15, 255 };
    Color orangeLight    = { 251, 146,  60, 255 };
    Color greyColor      = { 180, 160, 140, 255 };
    Color dividerColor   = {  55,  46,  36, 255 };
    Color errorColor     = { 220,  80,  60, 255 };
    Color successColor   = {  80, 200, 100, 255 };

    DrawRectangle(contentX, 0, contentWidth, screenHeight, darkBackground);

    int leftPadding  = 48;
    int formWidth    = 520;
    int formLeftEdge = contentX + leftPadding;

    DrawText("Settings", formLeftEdge, 44, 32, { 255, 255, 255, 255 });

    const char* currentUsername = getLoggedInUser();

    char signedInLine[96];
    if (currentUsername != nullptr && currentUsername[0] != '\0')
    {
        int signedInPos = 0;
        signedInPos = appendText(signedInLine, signedInPos, "Signed in as:  ");
        signedInPos = appendText(signedInLine, signedInPos, currentUsername);
    }
    else
    {
        copyText(signedInLine, "Signed in as:  -");
    }
    DrawText(signedInLine, formLeftEdge, 90, 18, greyColor);

    DrawRectangle(formLeftEdge, 124, formWidth, 1, dividerColor);

    DrawRectangle(formLeftEdge - 4, 144, 4, 22, orangeLight);
    DrawText("Change Username", formLeftEdge + 6, 144, 20, { 255, 255, 255, 255 });

    int usernameBoxY = 194;

    bool usernameBoxWasClicked = drawTextInputBox(
        s_newUsernameText, 64, "New username",
        formLeftEdge, usernameBoxY, formWidth, 50,
        s_focusedInputBox == 0, false
    );

    if (usernameBoxWasClicked == true)
    {
        s_focusedInputBox          = 0;
        s_usernameResultMessage[0] = '\0';
    }

    bool saveUsernameButtonClicked = drawButton("Save Username", formLeftEdge, usernameBoxY + 64, 192, 46);

    if (saveUsernameButtonClicked == true)
    {
        int newUsernameLength = 0;
        while (s_newUsernameText[newUsernameLength] != '\0')
        {
            newUsernameLength = newUsernameLength + 1;
        }

        if (newUsernameLength == 0)
        {
            copyText(s_usernameResultMessage, "Enter a new username.");
            s_usernameChangeSucceeded = false;
        }
        else if (newUsernameLength < 3)
        {
            copyText(s_usernameResultMessage, "Username must be at least 3 characters.");
            s_usernameChangeSucceeded = false;
        }
        else if (textsAreEqual(s_newUsernameText, currentUsername) == true)
        {
            copyText(s_usernameResultMessage, "That is already your username.");
            s_usernameChangeSucceeded = false;
        }
        else
        {
            bool changeWorked = changeUsername(currentUsername, s_newUsernameText);

            if (changeWorked == false)
            {
                copyText(s_usernameResultMessage, "Username already taken.");
                s_usernameChangeSucceeded = false;
            }
            else
            {
                setLoggedInUser(s_newUsernameText);
                copyText(s_usernameResultMessage, "Username updated!");
                s_usernameChangeSucceeded = true;
                s_newUsernameText[0]      = '\0';
                s_focusedInputBox         = -1;
            }
        }
    }

    if (s_usernameResultMessage[0] != '\0')
    {
        Color messageColor = errorColor;
        if (s_usernameChangeSucceeded == true)
        {
            messageColor = successColor;
        }
        DrawText(s_usernameResultMessage, formLeftEdge, usernameBoxY + 122, 16, messageColor);
    }

    int dividerBetweenSections = usernameBoxY + 150;
    DrawRectangle(formLeftEdge, dividerBetweenSections, formWidth, 1, dividerColor);

    int passwordSectionTop = dividerBetweenSections + 26;

    DrawRectangle(formLeftEdge - 4, passwordSectionTop, 4, 22, orangeLight);
    DrawText("Change Password", formLeftEdge + 6, passwordSectionTop, 20, { 255, 255, 255, 255 });

    int currentPasswordBoxY = passwordSectionTop + 50;
    int newPasswordBoxY     = currentPasswordBoxY + 72;
    int confirmPasswordBoxY = newPasswordBoxY + 72;

    bool currentPasswordBoxClicked = drawTextInputBox(
        s_currentPasswordText, 64, "Current password",
        formLeftEdge, currentPasswordBoxY, formWidth, 50,
        s_focusedInputBox == 1, true
    );
    if (currentPasswordBoxClicked == true)
    {
        s_focusedInputBox          = 1;
        s_passwordResultMessage[0] = '\0';
    }

    bool newPasswordBoxClicked = drawTextInputBox(
        s_newPasswordText, 64, "New password",
        formLeftEdge, newPasswordBoxY, formWidth, 50,
        s_focusedInputBox == 2, true
    );
    if (newPasswordBoxClicked == true)
    {
        s_focusedInputBox          = 2;
        s_passwordResultMessage[0] = '\0';
    }

    bool confirmPasswordBoxClicked = drawTextInputBox(
        s_confirmPasswordText, 64, "Confirm new password",
        formLeftEdge, confirmPasswordBoxY, formWidth, 50,
        s_focusedInputBox == 3, true
    );
    if (confirmPasswordBoxClicked == true)
    {
        s_focusedInputBox          = 3;
        s_passwordResultMessage[0] = '\0';
    }

    bool savePasswordButtonClicked = drawButton("Save Password", formLeftEdge, confirmPasswordBoxY + 66, 192, 46);

    if (savePasswordButtonClicked == true)
    {
        int currentPasswordLength = 0;
        while (s_currentPasswordText[currentPasswordLength] != '\0')
        {
            currentPasswordLength = currentPasswordLength + 1;
        }

        int newPasswordLength = 0;
        while (s_newPasswordText[newPasswordLength] != '\0')
        {
            newPasswordLength = newPasswordLength + 1;
        }

        int confirmPasswordLength = 0;
        while (s_confirmPasswordText[confirmPasswordLength] != '\0')
        {
            confirmPasswordLength = confirmPasswordLength + 1;
        }

        if (currentPasswordLength == 0 || newPasswordLength == 0 || confirmPasswordLength == 0)
        {
            copyText(s_passwordResultMessage, "Please fill in all fields.");
            s_passwordChangeSucceeded = false;
        }
        else if (newPasswordLength < 4)
        {
            copyText(s_passwordResultMessage, "New password must be at least 4 characters.");
            s_passwordChangeSucceeded = false;
        }
        else if (textsAreEqual(s_newPasswordText, s_confirmPasswordText) == false)
        {
            copyText(s_passwordResultMessage, "New passwords do not match.");
            s_passwordChangeSucceeded = false;
        }
        else
        {
            bool changeWorked = changePassword(currentUsername, s_currentPasswordText, s_newPasswordText);

            if (changeWorked == false)
            {
                copyText(s_passwordResultMessage, "Current password is incorrect.");
                s_passwordChangeSucceeded = false;
            }
            else
            {
                copyText(s_passwordResultMessage, "Password updated!");
                s_passwordChangeSucceeded = true;
                s_currentPasswordText[0]  = '\0';
                s_newPasswordText[0]      = '\0';
                s_confirmPasswordText[0]  = '\0';
                s_focusedInputBox         = -1;
            }
        }
    }

    if (s_passwordResultMessage[0] != '\0')
    {
        Color messageColor = errorColor;
        if (s_passwordChangeSucceeded == true)
        {
            messageColor = successColor;
        }
        DrawText(s_passwordResultMessage, formLeftEdge, confirmPasswordBoxY + 126, 16, messageColor);
    }

    int dangerSectionY = confirmPasswordBoxY + 156;

    DrawRectangle(formLeftEdge, dangerSectionY, formWidth, 1, { 80, 30, 20, 255 });

    DrawRectangle(formLeftEdge - 4, dangerSectionY + 22, 4, 22, errorColor);
    DrawText("Danger Zone", formLeftEdge + 6, dangerSectionY + 22, 20, errorColor);

    if (s_deleteErrorMessage[0] != '\0')
    {
        DrawText(s_deleteErrorMessage, formLeftEdge, dangerSectionY + 56, 16, errorColor);
    }

    int deleteButtonY = dangerSectionY + 60;
    if (s_deleteErrorMessage[0] != '\0')
    {
        deleteButtonY = dangerSectionY + 82;
    }

    bool deleteButtonWasClicked = drawButton("Delete my account", formLeftEdge, deleteButtonY, 200, 46, true);

    if (deleteButtonWasClicked == true)
    {
        s_deleteErrorMessage[0] = '\0';

        bool noUserIsLoggedIn = (currentUsername == nullptr || currentUsername[0] == '\0');
        if (noUserIsLoggedIn == true)
        {
            copyText(s_deleteErrorMessage, "No active account.");
        }
        else
        {
            s_deleteConfirmIsOpen = true;
        }
    }

    if (s_deleteConfirmIsOpen == false)
    {
        if (IsKeyPressed(KEY_TAB))
        {
            if (s_focusedInputBox < 0 || s_focusedInputBox > 3)
            {
                s_focusedInputBox = 0;
            }
            else
            {
                s_focusedInputBox = s_focusedInputBox + 1;
                if (s_focusedInputBox > 3)
                {
                    s_focusedInputBox = 0;
                }
            }
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            s_focusedInputBox = -1;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 mousePosition = GetMousePosition();

            float formLeft  = (float)(formLeftEdge - 4);
            float formRight = (float)(formLeftEdge - 4 + formWidth + 8);

            bool mouseIsInsideForm = (mousePosition.x >= formLeft && mousePosition.x <= formRight);
            if (mouseIsInsideForm == false)
            {
                s_focusedInputBox = -1;
            }
        }
    }

    if (s_deleteConfirmIsOpen == true)
    {
        int fullWindowWidth  = GetScreenWidth();
        int fullWindowHeight = GetScreenHeight();

        DrawRectangle(0, 0, fullWindowWidth, fullWindowHeight, { 0, 0, 0, 170 });

        int popupWidth  = 480;
        int popupHeight = 250;
        int popupX      = fullWindowWidth  / 2 - popupWidth  / 2;
        int popupY      = fullWindowHeight / 2 - popupHeight / 2;

        DrawRectangleRounded(
            { (float)(popupX - 2), (float)(popupY - 2), (float)(popupWidth + 4), (float)(popupHeight + 4) },
            0.08f, 8, orangeColor
        );

        DrawRectangleRounded(
            { (float)popupX, (float)popupY, (float)popupWidth, (float)popupHeight },
            0.08f, 8, darkBackground
        );

        const char* popupTitle      = "Delete account?";
        int         popupTitleWidth = MeasureText(popupTitle, 26);
        int         popupTitleX     = popupX + popupWidth / 2 - popupTitleWidth / 2;
        DrawText(popupTitle, popupTitleX, popupY + 36, 26, { 255, 255, 255, 255 });

        const char* warningMessage      = "This will permanently delete your account and all tasks.";
        int         warningMessageWidth = MeasureText(warningMessage, 16);
        int         warningMessageX     = popupX + popupWidth / 2 - warningMessageWidth / 2;
        DrawText(warningMessage, warningMessageX, popupY + 84, 16, greyColor);

        int buttonWidth       = 170;
        int buttonHeight      = 52;
        int buttonGap         = 16;
        int totalButtonsWidth = buttonWidth * 2 + buttonGap;
        int deleteButtonX     = popupX + popupWidth / 2 - totalButtonsWidth / 2;
        int cancelButtonX     = deleteButtonX + buttonWidth + buttonGap;
        int buttonsY          = popupY + popupHeight - 86;

        bool confirmDeleteClicked = drawButton("Delete", deleteButtonX, buttonsY, buttonWidth, buttonHeight, true);

        if (confirmDeleteClicked == true)
        {
            bool accountWasDeleted = deleteAccount(currentUsername);
            bool userIsValid       = (currentUsername != nullptr && currentUsername[0] != '\0');

            if (userIsValid == true && accountWasDeleted == true)
            {
                deleteAllTasksForUser(currentUsername);
                clearLoggedInUser();
                s_deleteConfirmIsOpen   = false;
                s_deleteErrorMessage[0] = '\0';

                if (outAccountDeleted != nullptr)
                {
                    *outAccountDeleted = true;
                }
            }
            else
            {
                copyText(s_deleteErrorMessage, "Could not remove account.");
                s_deleteConfirmIsOpen = false;
            }
        }

        bool cancelButtonClicked = drawOutlineButton("Cancel", cancelButtonX, buttonsY, buttonWidth, buttonHeight);
        bool escapeKeyPressed    = IsKeyPressed(KEY_ESCAPE);

        if (cancelButtonClicked == true || escapeKeyPressed == true)
        {
            s_deleteConfirmIsOpen = false;
        }
    }
}