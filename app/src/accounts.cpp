#include "../include/accounts.h"
#include <fstream>
#include <string>

static const char* ACCOUNTS_FILE = "accounts.txt";
static char current_user[64] = {};

bool registerAccount(const char* username, const char* password)
{
    if (!username || !password || username[0] == '\0')
    {
        return false;
    }

    std::ofstream file(ACCOUNTS_FILE, std::ios::app);
    if (!file)
    {
        return false;
    }

    file << username << '\n';
    file << password << '\n';
    return true;
}

bool accountExists(const char* username)
{
    if (!username || username[0] == '\0')
    {
        return false;
    }

    std::ifstream file(ACCOUNTS_FILE);
    std::string line_username;
    std::string line_password;

    while (std::getline(file, line_username))
    {
        std::getline(file, line_password);

        if (line_username == username)
        {
            return true;
        }
    }

    return false;
}

bool validateCredentials(const char* username, const char* password)
{
    if (!username || !password || username[0] == '\0')
    {
        return false;
    }

    std::ifstream file(ACCOUNTS_FILE);
    std::string line_username;
    std::string line_password;

    while (std::getline(file, line_username))
    {
        std::getline(file, line_password);

        if (line_username == username && line_password == password)
        {
            return true;
        }
    }

    return false;
}

bool deleteAccount(const char* username)
{
    if (!username || username[0] == '\0')
    {
        return false;
    }

    std::ifstream input(ACCOUNTS_FILE);
    if (!input)
    {
        return false;
    }

    std::string saved_usernames[256];
    std::string saved_passwords[256];
    int count = 0;

    while (std::getline(input, saved_usernames[count]))
    {
        std::getline(input, saved_passwords[count]);
        count++;
    }
    input.close();

    bool found = false;
    for (int i = 0; i < count; i++)
    {
        if (saved_usernames[i] == username)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        return false;
    }

    std::ofstream output(ACCOUNTS_FILE);
    if (!output)
    {
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        if (saved_usernames[i] != username)
        {
            output << saved_usernames[i] << '\n';
            output << saved_passwords[i] << '\n';
        }
    }

    return true;
}

void setLoggedInUser(const char* username)
{
    if (!username)
    {
        current_user[0] = '\0';
        return;
    }

    int i = 0;
    while (i < 63 && username[i] != '\0')
    {
        current_user[i] = username[i];
        i++;
    }
    current_user[i] = '\0';
}

void clearLoggedInUser()
{
    current_user[0] = '\0';
}

const char* getLoggedInUser()
{
    return current_user;
}
