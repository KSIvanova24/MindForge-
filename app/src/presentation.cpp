#include "../include/presentation.h"
#include "../include/accounts.h"
#include "../include/db.h"
#include "../include/screens/dashboard.h"
#include "../include/screens/tasks.h"
#include "../include/screens/statistics.h"
#include "../include/screens/profile.h"
#include "../include/screens/login.h"

#include <cstdio>
#include <cstring>

// -------------------------------------------------------
// REMEMBERED STATE FOR THE SETTINGS PAGE
// These variables keep their values between frames because
// they are declared outside of any function (static means
// they are private to this file only).
// -------------------------------------------------------

// Whether the "are you sure you want to delete?" popup is showing
static bool s_deleteConfirmIsOpen = false;

// An error message to show if the delete fails (empty string = no message)
static char s_deleteErrorMessage[128] = {};

// Which input box on the settings page is currently selected (-1 = none)
// 0 = new username box
// 1 = current password box
// 2 = new password box
// 3 = confirm password box
static int s_focusedInputBox = -1;

// The text the user has typed into the "new username" box
static char s_newUsernameText[64] = {};

// The message shown after trying to save a new username
static char s_usernameResultMessage[128] = {};

// Whether the username result message is a success (true) or an error (false)
static bool s_usernameChangeSucceeded = false;

// The text in the three password boxes
static char s_currentPasswordText[64] = {};
static char s_newPasswordText[64]     = {};
static char s_confirmPasswordText[64] = {};

// The message shown after trying to save a new password
static char s_passwordResultMessage[128] = {};

// Whether the password result message is a success or an error
static bool s_passwordChangeSucceeded = false;

// -------------------------------------------------------
// SIDEBAR LAYOUT CONSTANTS
// These control the size and position of the sidebar.
// -------------------------------------------------------
int SIDEBAR_W = 300;
int BTN_H     = 62;
int BTN_GAP   = 8;
int BTN_X     = 16;
int BTN_W     = 268;
int NAV_START = 110;

// -------------------------------------------------------
// SIDEBAR COLORS
// -------------------------------------------------------
Color COLOR_SIDEBAR  = {  28,  24,  20, 255 };
Color COLOR_HOVER    = {  45,  38,  30, 255 };
Color COLOR_ACTIVE   = { 234, 108,  15, 255 };
Color COLOR_ACCENT   = { 251, 146,  60, 255 };
Color COLOR_DIVIDER  = {  55,  46,  36, 255 };
Color COLOR_BRAND_A  = { 251, 146,  60, 255 };
Color COLOR_WHITE    = { 255, 255, 255, 255 };
Color COLOR_GREY     = { 180, 160, 140, 255 };
Color COLOR_DOT_IDLE = { 120, 100,  80, 255 };

// -------------------------------------------------------
// This function returns true if the delete popup is open.
// Main uses this to block navigation while the popup shows.
// -------------------------------------------------------
bool settingsDeleteModalIsOpen()
{
    return s_deleteConfirmIsOpen;
}

// -------------------------------------------------------
// drawOneButton
// Draws a single navigation button in the sidebar.
// It looks different depending on whether it is selected
// (active) or if the mouse is hovering over it.
// -------------------------------------------------------
void drawOneButton(const char* label, int x, int y, int w, int h, bool isActive, bool isHovered)
{
    // Draw the button differently based on its state
    if (isActive == true)
    {
        // This is the currently selected screen — draw it highlighted in orange
        DrawRectangle(x, y + 10, 4, h - 20, COLOR_ACCENT);
        DrawRectangleRounded({ (float)(x + 4), (float)y, (float)(w - 4), (float)h }, 0.28f, 8, COLOR_ACTIVE);
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_WHITE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_WHITE);
    }
    else if (isHovered == true)
    {
        // The mouse is over this button — draw a subtle highlight
        DrawRectangleRounded({ (float)x, (float)y, (float)w, (float)h }, 0.28f, 8, COLOR_HOVER);
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_DOT_IDLE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_GREY);
    }
    else
    {
        // Normal resting state — just show the dot and grey label
        DrawCircle(x + 32, y + h / 2, 5.0f, COLOR_DOT_IDLE);
        DrawText(label, x + 52, y + h / 2 - 11, 22, COLOR_GREY);
    }
}

// -------------------------------------------------------
// drawSidebar
// Draws the full left sidebar including the logo,
// main navigation buttons, and bottom buttons.
// outHovered is set to whichever button the mouse is over.
// -------------------------------------------------------
void drawSidebar(AppScreen current, AppScreen* outHovered, int screenHeight)
{
    // Fill the sidebar with its dark background
    DrawRectangle(0, 0, SIDEBAR_W, screenHeight, COLOR_SIDEBAR);

    // Draw a subtle shadow on the right edge of the sidebar
    DrawRectangleGradientH(SIDEBAR_W - 10, 0, 10, screenHeight, { 0, 0, 0, 0 }, { 0, 0, 0, 60 });

    // Draw the two-word app logo at the top
    DrawText("Task", 28, 34, 36, COLOR_BRAND_A);
    DrawText("Dash", 28 + MeasureText("Task", 36) + 8, 34, 36, COLOR_WHITE);

    // Draw a thin line below the logo
    DrawRectangle(20, 88, SIDEBAR_W - 40, 1, COLOR_DIVIDER);

    // The three main navigation buttons
    const char* mainLabels[3]   = { "Dashboard", "Tasks", "Statistics" };
    AppScreen   mainScreens[3]  = { SCREEN_DASHBOARD, SCREEN_ALL_TASKS, SCREEN_STATISTICS };

    // Start with no button hovered
    *outHovered = (AppScreen)-1;

    // Get the current mouse position
    Vector2 mousePosition = GetMousePosition();

    // Draw each of the three main buttons
    for (int i = 0; i < 3; i = i + 1)
    {
        int buttonX = BTN_X;
        int buttonY = NAV_START + i * (BTN_H + BTN_GAP);

        // Check if the mouse is over this button
        Rectangle buttonArea = { (float)buttonX, (float)buttonY, (float)BTN_W, (float)BTN_H };
        bool mouseIsOverButton = CheckCollisionPointRec(mousePosition, buttonArea);

        // If the mouse is over this button, record it as hovered
        if (mouseIsOverButton == true)
        {
            *outHovered = mainScreens[i];
        }

        // Is this the currently shown screen?
        bool thisButtonIsActive = (current == mainScreens[i]);

        drawOneButton(mainLabels[i], buttonX, buttonY, BTN_W, BTN_H, thisButtonIsActive, mouseIsOverButton);
    }

    // Draw a divider line above the bottom buttons
    DrawRectangle(20, screenHeight - 180, SIDEBAR_W - 40, 1, COLOR_DIVIDER);

    // The three bottom buttons: Profile, Settings, Logout
    const char* bottomLabels[3]   = { "My Profile", "Settings", "Logout" };
    AppScreen   bottomScreens[3]  = { SCREEN_PROFILE, SCREEN_SETTINGS, SCREEN_LOGOUT };

    // Calculate where the bottom group of buttons starts
    int bottomGroupStartY = screenHeight - 160 - (BTN_H + BTN_GAP);

    // Draw each bottom button
    for (int i = 0; i < 3; i = i + 1)
    {
        int buttonX = BTN_X;
        int buttonY = bottomGroupStartY + i * (BTN_H + BTN_GAP);

        // Check if the mouse is over this button
        Rectangle buttonArea = { (float)buttonX, (float)buttonY, (float)BTN_W, (float)BTN_H };
        bool mouseIsOverButton = CheckCollisionPointRec(mousePosition, buttonArea);

        if (mouseIsOverButton == true)
        {
            *outHovered = bottomScreens[i];
        }

        // Logout is never shown as "active" even if it was just pressed
        bool thisButtonIsActive = false;
        if (bottomScreens[i] != SCREEN_LOGOUT && current == bottomScreens[i])
        {
            thisButtonIsActive = true;
        }

        drawOneButton(bottomLabels[i], buttonX, buttonY, BTN_W, BTN_H, thisButtonIsActive, mouseIsOverButton);
    }
}

// -------------------------------------------------------
// drawTextInputBox
// Draws one text input box with a label above it.
// The caller tells us:
//   - which buffer to read/write text into
//   - the label to show above the box
//   - where and how big the box should be
//   - whether this box is currently focused (selected)
//   - whether to hide the text behind asterisks (password mode)
// Returns true if the user clicked on the box this frame.
// -------------------------------------------------------
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
    // Choose the box colors based on whether it is focused or not
    Color boxBackgroundColor;
    Color boxBorderColor;

    if (isCurrentlyFocused == true)
    {
        // Focused box: lighter background and orange border
        boxBackgroundColor = { 42,  35,  26, 255 };
        boxBorderColor     = { 234, 108,  15, 255 };
    }
    else
    {
        // Unfocused box: darker background and dim border
        boxBackgroundColor = { 32,  27,  21, 255 };
        boxBorderColor     = {  55,  46,  36, 255 };
    }

    // Draw the label text above the box (24 pixels above the top edge)
    DrawText(labelText, posX, posY - 24, 17, { 180, 160, 140, 255 });

    // Draw the border by drawing a slightly larger rounded rectangle first
    DrawRectangleRounded(
        { (float)(posX - 2), (float)(posY - 2), (float)(boxWidth + 4), (float)(boxHeight + 4) },
        0.22f, 8, boxBorderColor
    );

    // Draw the box background on top of the border
    DrawRectangleRounded(
        { (float)posX, (float)posY, (float)boxWidth, (float)boxHeight },
        0.22f, 8, boxBackgroundColor
    );

    // Count how many characters are currently in the text buffer
    int characterCount = 0;
    while (textBuffer[characterCount] != '\0')
    {
        characterCount = characterCount + 1;
    }

    // Build the string we will actually display inside the box
    // If it is a password box, show asterisks instead of real characters
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

    // Draw the display text inside the box
    int textFontSize = 19;
    int textDrawX    = posX + 14;
    int textDrawY    = posY + boxHeight / 2 - textFontSize / 2;
    DrawText(displayText, textDrawX, textDrawY, textFontSize, { 255, 255, 255, 255 });

    // If this box is focused, handle typing and show the blinking cursor
    if (isCurrentlyFocused == true)
    {
        // The cursor appears right after the last character
        int cursorX = posX + 14 + MeasureText(displayText, textFontSize);

        // Make the cursor blink by checking if the current time is even or odd
        float currentTime  = (float)GetTime();
        int   timeAsInt    = (int)(currentTime * 2.0f);
        bool  cursorIsOn   = (timeAsInt % 2 == 0);

        if (cursorIsOn == true)
        {
            // Draw the cursor as a thin vertical line
            DrawRectangle(cursorX + 2, posY + 10, 2, boxHeight - 20, { 251, 146, 60, 255 });
        }

        // Handle typing: GetCharPressed returns the character typed this frame
        int typedCharacter = GetCharPressed();
        while (typedCharacter > 0)
        {
            // Only accept printable characters (letters, numbers, symbols)
            bool characterIsPrintable = (typedCharacter >= 32 && typedCharacter <= 126);

            // Only add if there is still room in the buffer
            bool thereIsRoomForMore = (characterCount < maxCharacters - 1);

            if (characterIsPrintable == true && thereIsRoomForMore == true)
            {
                // Add the new character at the end and close the string
                textBuffer[characterCount]     = (char)typedCharacter;
                textBuffer[characterCount + 1] = '\0';
            }

            // Check if another character was typed in the same frame
            typedCharacter = GetCharPressed();
        }

        // Handle backspace: remove the last character if there is one
        bool backspaceWasPressed = IsKeyPressed(KEY_BACKSPACE);
        bool thereAreCharacters  = (characterCount > 0);
        if (backspaceWasPressed == true && thereAreCharacters == true)
        {
            textBuffer[characterCount - 1] = '\0';
        }
    }

    // Check if the user clicked inside this box
    Vector2   mousePosition   = GetMousePosition();
    Rectangle boxHitArea      = { (float)posX, (float)posY, (float)boxWidth, (float)boxHeight };
    bool      mouseIsInBox    = CheckCollisionPointRec(mousePosition, boxHitArea);
    bool      mouseWasClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // Return true only if the mouse is inside the box AND was clicked
    return mouseIsInBox && mouseWasClicked;
}

// -------------------------------------------------------
// drawSettingsScreen
// Draws the full settings page including:
//   - Change Username form
//   - Change Password form
//   - Danger Zone (delete account)
// -------------------------------------------------------
void drawSettingsScreen(int contentX, int contentWidth, int screenHeight, bool* outAccountDeleted)
{
    // Tell the caller that no account was deleted yet this frame
    if (outAccountDeleted != nullptr)
    {
        *outAccountDeleted = false;
    }

    // -------------------------------------------------------
    // STEP 1: Define colors
    // -------------------------------------------------------
    Color darkBackground = {  22,  18,  14, 255 };
    Color orangeColor    = { 234, 108,  15, 255 };
    Color orangeLight    = { 251, 146,  60, 255 };
    Color greyColor      = { 180, 160, 140, 255 };
    Color dividerColor   = {  55,  46,  36, 255 };
    Color errorColor     = { 220,  80,  60, 255 };
    Color successColor   = {  80, 200, 100, 255 };

    // Fill the whole content area with the dark background
    DrawRectangle(contentX, 0, contentWidth, screenHeight, darkBackground);

    // How far from the left edge the form content starts
    int leftPadding  = 48;
    int formWidth    = 520;
    int formLeftEdge = contentX + leftPadding;

    // -------------------------------------------------------
    // STEP 2: Draw the page header
    // -------------------------------------------------------

    // Page title
    DrawText("Settings", formLeftEdge, 44, 32, { 255, 255, 255, 255 });

    // Show which user is currently logged in
    const char* currentUsername = getLoggedInUser();

    char signedInLine[96];
    if (currentUsername != nullptr && currentUsername[0] != '\0')
    {
        snprintf(signedInLine, sizeof(signedInLine), "Signed in as:  %s", currentUsername);
    }
    else
    {
        snprintf(signedInLine, sizeof(signedInLine), "Signed in as:  -");
    }
    DrawText(signedInLine, formLeftEdge, 90, 18, greyColor);

    // Thin divider line between the header and the form sections
    DrawRectangle(formLeftEdge, 124, formWidth, 1, dividerColor);

    // -------------------------------------------------------
    // STEP 3: Change Username section
    // -------------------------------------------------------

    // Draw the orange bar and section heading
    DrawRectangle(formLeftEdge - 4, 144, 4, 22, orangeLight);
    DrawText("Change Username", formLeftEdge + 6, 144, 20, { 255, 255, 255, 255 });

    // The y position of the username input box
    int usernameBoxY = 194;

    // Draw the input box and find out if the user clicked it
    bool usernameBoxWasClicked = drawTextInputBox(
        s_newUsernameText,
        64,
        "New username",
        formLeftEdge,
        usernameBoxY,
        formWidth,
        50,
        s_focusedInputBox == 0,
        false
    );

    // If the user clicked the username box, focus it and clear any old message
    if (usernameBoxWasClicked == true)
    {
        s_focusedInputBox            = 0;
        s_usernameResultMessage[0]   = '\0';
    }

    // Draw the Save Username button
    bool saveUsernameButtonClicked = drawButton("Save Username", formLeftEdge, usernameBoxY + 64, 192, 46);

    if (saveUsernameButtonClicked == true)
    {
        // Count how long the new username is
        int newUsernameLength = 0;
        while (s_newUsernameText[newUsernameLength] != '\0')
        {
            newUsernameLength = newUsernameLength + 1;
        }

        // Validate the new username before trying to save it
        if (newUsernameLength == 0)
        {
            strcpy(s_usernameResultMessage, "Enter a new username.");
            s_usernameChangeSucceeded = false;
        }
        else if (newUsernameLength < 3)
        {
            strcpy(s_usernameResultMessage, "Username must be at least 3 characters.");
            s_usernameChangeSucceeded = false;
        }
        else if (strcmp(s_newUsernameText, currentUsername) == 0)
        {
            strcpy(s_usernameResultMessage, "That is already your username.");
            s_usernameChangeSucceeded = false;
        }
        else
        {
            // Try to change the username in the database
            bool changeWorked = changeUsername(currentUsername, s_newUsernameText);

            if (changeWorked == false)
            {
                strcpy(s_usernameResultMessage, "Username already taken.");
                s_usernameChangeSucceeded = false;
            }
            else
            {
                // Update the session so the app uses the new name right away
                setLoggedInUser(s_newUsernameText);
                strcpy(s_usernameResultMessage, "Username updated!");
                s_usernameChangeSucceeded  = true;
                s_newUsernameText[0]       = '\0';
                s_focusedInputBox          = -1;
            }
        }
    }

    // Show the username result message in green (success) or red (error)
    if (s_usernameResultMessage[0] != '\0')
    {
        Color messageColor = errorColor;
        if (s_usernameChangeSucceeded == true)
        {
            messageColor = successColor;
        }
        DrawText(s_usernameResultMessage, formLeftEdge, usernameBoxY + 122, 16, messageColor);
    }

    // -------------------------------------------------------
    // STEP 4: Divider between username and password sections
    // -------------------------------------------------------

    int dividerBetweenSections = usernameBoxY + 150;
    DrawRectangle(formLeftEdge, dividerBetweenSections, formWidth, 1, dividerColor);

    // -------------------------------------------------------
    // STEP 5: Change Password section
    // -------------------------------------------------------

    int passwordSectionTop = dividerBetweenSections + 26;

    // Draw the section heading with an orange accent bar
    DrawRectangle(formLeftEdge - 4, passwordSectionTop, 4, 22, orangeLight);
    DrawText("Change Password", formLeftEdge + 6, passwordSectionTop, 20, { 255, 255, 255, 255 });

    // Position the three password input boxes one below the other
    int currentPasswordBoxY = passwordSectionTop + 50;
    int newPasswordBoxY     = currentPasswordBoxY + 72;
    int confirmPasswordBoxY = newPasswordBoxY + 72;

    // Draw the current password box
    bool currentPasswordBoxClicked = drawTextInputBox(
        s_currentPasswordText,
        64,
        "Current password",
        formLeftEdge,
        currentPasswordBoxY,
        formWidth,
        50,
        s_focusedInputBox == 1,
        true
    );
    if (currentPasswordBoxClicked == true)
    {
        s_focusedInputBox          = 1;
        s_passwordResultMessage[0] = '\0';
    }

    // Draw the new password box
    bool newPasswordBoxClicked = drawTextInputBox(
        s_newPasswordText,
        64,
        "New password",
        formLeftEdge,
        newPasswordBoxY,
        formWidth,
        50,
        s_focusedInputBox == 2,
        true
    );
    if (newPasswordBoxClicked == true)
    {
        s_focusedInputBox          = 2;
        s_passwordResultMessage[0] = '\0';
    }

    // Draw the confirm password box
    bool confirmPasswordBoxClicked = drawTextInputBox(
        s_confirmPasswordText,
        64,
        "Confirm new password",
        formLeftEdge,
        confirmPasswordBoxY,
        formWidth,
        50,
        s_focusedInputBox == 3,
        true
    );
    if (confirmPasswordBoxClicked == true)
    {
        s_focusedInputBox          = 3;
        s_passwordResultMessage[0] = '\0';
    }

    // Draw the Save Password button
    bool savePasswordButtonClicked = drawButton("Save Password", formLeftEdge, confirmPasswordBoxY + 66, 192, 46);

    if (savePasswordButtonClicked == true)
    {
        // Count the length of each of the three password fields
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

        // Validate before saving
        if (currentPasswordLength == 0 || newPasswordLength == 0 || confirmPasswordLength == 0)
        {
            strcpy(s_passwordResultMessage, "Please fill in all fields.");
            s_passwordChangeSucceeded = false;
        }
        else if (newPasswordLength < 4)
        {
            strcpy(s_passwordResultMessage, "New password must be at least 4 characters.");
            s_passwordChangeSucceeded = false;
        }
        else if (strcmp(s_newPasswordText, s_confirmPasswordText) != 0)
        {
            strcpy(s_passwordResultMessage, "New passwords do not match.");
            s_passwordChangeSucceeded = false;
        }
        else
        {
            // Try to change the password in the database
            bool changeWorked = changePassword(currentUsername, s_currentPasswordText, s_newPasswordText);

            if (changeWorked == false)
            {
                strcpy(s_passwordResultMessage, "Current password is incorrect.");
                s_passwordChangeSucceeded = false;
            }
            else
            {
                strcpy(s_passwordResultMessage, "Password updated!");
                s_passwordChangeSucceeded      = true;
                s_currentPasswordText[0]       = '\0';
                s_newPasswordText[0]           = '\0';
                s_confirmPasswordText[0]       = '\0';
                s_focusedInputBox              = -1;
            }
        }
    }

    // Show the password result message
    if (s_passwordResultMessage[0] != '\0')
    {
        Color messageColor = errorColor;
        if (s_passwordChangeSucceeded == true)
        {
            messageColor = successColor;
        }
        DrawText(s_passwordResultMessage, formLeftEdge, confirmPasswordBoxY + 126, 16, messageColor);
    }

    // -------------------------------------------------------
    // STEP 6: Danger Zone section (delete account)
    // -------------------------------------------------------

    int dangerSectionY = confirmPasswordBoxY + 156;

    // Draw a red-tinted divider to warn the user
    DrawRectangle(formLeftEdge, dangerSectionY, formWidth, 1, { 80, 30, 20, 255 });

    // Draw the "Danger Zone" heading in red
    DrawRectangle(formLeftEdge - 4, dangerSectionY + 22, 4, 22, errorColor);
    DrawText("Danger Zone", formLeftEdge + 6, dangerSectionY + 22, 20, errorColor);

    // Show any error message from a previous failed delete attempt
    if (s_deleteErrorMessage[0] != '\0')
    {
        DrawText(s_deleteErrorMessage, formLeftEdge, dangerSectionY + 56, 16, errorColor);
    }

    // Push the delete button down a little if there is an error message above it
    int deleteButtonY = dangerSectionY + 60;
    if (s_deleteErrorMessage[0] != '\0')
    {
        deleteButtonY = dangerSectionY + 82;
    }

    // Draw the red delete button
    bool deleteButtonWasClicked = drawButton("Delete my account", formLeftEdge, deleteButtonY, 200, 46, true);

    if (deleteButtonWasClicked == true)
    {
        // Clear any old error message
        s_deleteErrorMessage[0] = '\0';

        // Make sure there is actually a logged-in user to delete
        bool noUserIsLoggedIn = (currentUsername == nullptr || currentUsername[0] == '\0');
        if (noUserIsLoggedIn == true)
        {
            snprintf(s_deleteErrorMessage, sizeof(s_deleteErrorMessage), "No active account.");
        }
        else
        {
            // Open the confirmation popup
            s_deleteConfirmIsOpen = true;
        }
    }

    // -------------------------------------------------------
    // STEP 7: Keyboard navigation between input boxes
    // -------------------------------------------------------

    // Only handle keyboard shortcuts when the delete popup is not open
    if (s_deleteConfirmIsOpen == false)
    {
        // Tab key moves focus to the next input box
        if (IsKeyPressed(KEY_TAB))
        {
            if (s_focusedInputBox < 0 || s_focusedInputBox > 3)
            {
                // No box is focused — start at the first one
                s_focusedInputBox = 0;
            }
            else
            {
                // Move to the next box
                s_focusedInputBox = s_focusedInputBox + 1;

                // Wrap back to the first box after the last one
                if (s_focusedInputBox > 3)
                {
                    s_focusedInputBox = 0;
                }
            }
        }

        // Escape key removes focus from all boxes
        if (IsKeyPressed(KEY_ESCAPE))
        {
            s_focusedInputBox = -1;
        }

        // Clicking outside the form area removes focus from all boxes
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

    // -------------------------------------------------------
    // STEP 8: Delete account confirmation popup
    // -------------------------------------------------------

    if (s_deleteConfirmIsOpen == true)
    {
        // Get the full window size so we can center the popup
        int fullWindowWidth  = GetScreenWidth();
        int fullWindowHeight = GetScreenHeight();

        // Dim the entire screen behind the popup with a semi-transparent black layer
        DrawRectangle(0, 0, fullWindowWidth, fullWindowHeight, { 0, 0, 0, 170 });

        // Calculate the size and center position of the popup box
        int popupWidth  = 480;
        int popupHeight = 250;
        int popupX      = fullWindowWidth  / 2 - popupWidth  / 2;
        int popupY      = fullWindowHeight / 2 - popupHeight / 2;

        // Draw the orange border around the popup (slightly larger rectangle)
        DrawRectangleRounded(
            { (float)(popupX - 2), (float)(popupY - 2), (float)(popupWidth + 4), (float)(popupHeight + 4) },
            0.08f, 8, orangeColor
        );

        // Draw the dark background of the popup
        DrawRectangleRounded(
            { (float)popupX, (float)popupY, (float)popupWidth, (float)popupHeight },
            0.08f, 8, darkBackground
        );

        // Draw the popup title centered inside the popup
        const char* popupTitle      = "Delete account?";
        int         popupTitleWidth = MeasureText(popupTitle, 26);
        int         popupTitleX     = popupX + popupWidth / 2 - popupTitleWidth / 2;
        DrawText(popupTitle, popupTitleX, popupY + 36, 26, { 255, 255, 255, 255 });

        // Draw the warning message centered below the title
        const char* warningMessage      = "This will permanently delete your account and all tasks.";
        int         warningMessageWidth = MeasureText(warningMessage, 16);
        int         warningMessageX     = popupX + popupWidth / 2 - warningMessageWidth / 2;
        DrawText(warningMessage, warningMessageX, popupY + 84, 16, greyColor);

        // Calculate the positions of the two buttons at the bottom of the popup
        int buttonWidth       = 170;
        int buttonHeight      = 52;
        int buttonGap         = 16;
        int totalButtonsWidth = buttonWidth * 2 + buttonGap;
        int deleteButtonX     = popupX + popupWidth / 2 - totalButtonsWidth / 2;
        int cancelButtonX     = deleteButtonX + buttonWidth + buttonGap;
        int buttonsY          = popupY + popupHeight - 86;

        // Draw the red "Delete" button
        bool confirmDeleteClicked = drawButton("Delete", deleteButtonX, buttonsY, buttonWidth, buttonHeight, true);

        if (confirmDeleteClicked == true)
        {
            // Try to delete the account from the database
            bool accountWasDeleted = deleteAccount(currentUsername);

            bool userIsValid = (currentUsername != nullptr && currentUsername[0] != '\0');

            if (userIsValid == true && accountWasDeleted == true)
            {
                // Also delete all the tasks that belonged to this user
                deleteAllTasksForUser(currentUsername);

                // Clear the logged-in user from memory
                clearLoggedInUser();

                // Close the popup
                s_deleteConfirmIsOpen = false;
                s_deleteErrorMessage[0] = '\0';

                // Tell main.cpp that the account was deleted so it can go back to the login screen
                if (outAccountDeleted != nullptr)
                {
                    *outAccountDeleted = true;
                }
            }
            else
            {
                // Something went wrong — show an error and close the popup
                snprintf(s_deleteErrorMessage, sizeof(s_deleteErrorMessage), "Could not remove account.");
                s_deleteConfirmIsOpen = false;
            }
        }

        // Draw the "Cancel" button (outline style, not filled)
        bool cancelButtonClicked = drawOutlineButton("Cancel", cancelButtonX, buttonsY, buttonWidth, buttonHeight);
        bool escapeKeyPressed    = IsKeyPressed(KEY_ESCAPE);

        // Close the popup if Cancel was clicked or Escape was pressed
        if (cancelButtonClicked == true || escapeKeyPressed == true)
        {
            s_deleteConfirmIsOpen = false;
        }
    }
}
