#pragma once

bool registerAccount(const char* username, const char* password);
bool accountExists(const char* username);
bool validateCredentials(const char* username, const char* password);
bool deleteAccount(const char* username);
bool changeUsername(const char* oldUsername, const char* newUsername);
bool changePassword(const char* username, const char* currentPassword, const char* newPassword);
void setLoggedInUser(const char* username);
void clearLoggedInUser();
const char* getLoggedInUser();
