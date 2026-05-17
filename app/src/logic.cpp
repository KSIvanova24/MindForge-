#include "../include/logic.h"

void copyText(char* destination, const char* source)
{
    int i = 0;
    while (source[i] != '\0')
    {
        destination[i] = source[i];
        i = i + 1;
    }
    destination[i] = '\0';
}

bool textsAreEqual(const char* firstText, const char* secondText)
{
    int i = 0;
    while (firstText[i] != '\0' && secondText[i] != '\0')
    {
        if (firstText[i] != secondText[i])
        {
            return false;
        }
        i = i + 1;
    }
    return (firstText[i] == '\0' && secondText[i] == '\0');
}

int compareTexts(const char* firstText, const char* secondText)
{
    int i = 0;
    while (firstText[i] != '\0' && secondText[i] != '\0')
    {
        if (firstText[i] != secondText[i])
        {
            return firstText[i] - secondText[i];
        }
        i = i + 1;
    }
    if (firstText[i] == '\0' && secondText[i] == '\0')
    {
        return 0;
    }
    if (firstText[i] == '\0')
    {
        return -1;
    }
    return 1;
}

int textLength(const char* text)
{
    int length = 0;
    while (text[length] != '\0')
    {
        length = length + 1;
    }
    return length;
}

int appendText(char* destination, int currentPosition, const char* textToAdd)
{
    int i = 0;
    while (textToAdd[i] != '\0')
    {
        destination[currentPosition] = textToAdd[i];
        currentPosition = currentPosition + 1;
        i = i + 1;
    }
    destination[currentPosition] = '\0';
    return currentPosition;
}

int appendNumber(char* destination, int currentPosition, int numberToWrite)
{
    if (numberToWrite == 0)
    {
        destination[currentPosition] = '0';
        currentPosition = currentPosition + 1;
        destination[currentPosition] = '\0';
        return currentPosition;
    }

    char digitsBackwards[12] = {};
    int digitCount = 0;
    int remaining = numberToWrite;
    while (remaining > 0)
    {
        int lastDigit = remaining % 10;
        digitsBackwards[digitCount] = '0' + lastDigit;
        digitCount = digitCount + 1;
        remaining = remaining / 10;
    }

    int digitIndex = digitCount - 1;
    while (digitIndex >= 0)
    {
        destination[currentPosition] = digitsBackwards[digitIndex];
        currentPosition = currentPosition + 1;
        digitIndex = digitIndex - 1;
    }

    destination[currentPosition] = '\0';
    return currentPosition;
}

void buildDateText(char* destination, int year, int month, int day)
{
    int position = 0;

    position = appendNumber(destination, position, year);

    destination[position] = '-';
    position = position + 1;

    if (month < 10)
    {
        destination[position] = '0';
        position = position + 1;
    }
    position = appendNumber(destination, position, month);

    destination[position] = '-';
    position = position + 1;

    if (day < 10)
    {
        destination[position] = '0';
        position = position + 1;
    }
    position = appendNumber(destination, position, day);

    destination[position] = '\0';
}

bool parseDateText(const char* dateText, int* yearOut, int* monthOut, int* dayOut)
{
    int length = textLength(dateText);
    if (length != 10)
    {
        return false;
    }

    if (dateText[4] != '-' || dateText[7] != '-')
    {
        return false;
    }

    int year = 0;
    year = year + (dateText[0] - '0') * 1000;
    year = year + (dateText[1] - '0') * 100;
    year = year + (dateText[2] - '0') * 10;
    year = year + (dateText[3] - '0');

    int month = 0;
    month = month + (dateText[5] - '0') * 10;
    month = month + (dateText[6] - '0');

    int day = 0;
    day = day + (dateText[8] - '0') * 10;
    day = day + (dateText[9] - '0');

    *yearOut  = year;
    *monthOut = month;
    *dayOut   = day;
    return true;
}
