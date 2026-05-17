#pragma once

void copyText(char* destination, const char* source);

bool textsAreEqual(const char* firstText, const char* secondText);

int compareTexts(const char* firstText, const char* secondText);

int textLength(const char* text);

int appendText(char* destination, int currentPosition, const char* textToAdd);

int appendNumber(char* destination, int currentPosition, int numberToWrite);

void buildDateText(char* destination, int year, int month, int day);

bool parseDateText(const char* dateText, int* yearOut, int* monthOut, int* dayOut);
