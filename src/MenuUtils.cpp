#include "../include/MenuUtils.h"
#include "../include/ConsoleUtils.h"
#include "../include/TypeTags.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

string fillBar(int count, int capacity, int width)
{
    if (capacity <= 0)
        capacity = 1;

    int filled = (count * width + capacity - 1) / capacity; // round up, so 1 Pokémon still shows
    filled = max(0, min(width, filled));

    string bar = "[";
    for (int i = 0; i < width; ++i)
        bar += (i < filled) ? "█" : "░";
    return bar + "]";
}


const vector<string> POKEMON_TYPES = {
    "Normal", "Fire", "Water", "Electric", "Grass", "Ice",
    "Fighting", "Poison", "Ground", "Flying", "Psychic", "Bug",
    "Rock", "Ghost", "Dragon", "Dark", "Steel", "Fairy"};

void printTypeChoice(size_t index, const string &type, bool useTag)
{
    const size_t visibleWidth = type.size() + (useTag ? 2 : 0);
    cout << right << setw(2) << (index + 1) << ". "
         << (useTag ? colorTypeTag(type) : type);
    if (visibleWidth < 14)
        cout << string(14 - visibleWidth, ' ');
}

string chooseType()
{
    clearScreen();
    cout << "\n--- SELECT TYPE ---" << endl;
    for (size_t i = 0; i < POKEMON_TYPES.size(); ++i)
    {
        printTypeChoice(i, POKEMON_TYPES[i], true);
        if ((i + 1) % 3 == 0)
            cout << endl;
    }
    cout << right << endl;
    cout << " 0. Cancel" << endl;
    int pick = readMenuChoice("Choice: ", static_cast<int>(POKEMON_TYPES.size()));
    if (pick == 0)
        return "";
    return POKEMON_TYPES[pick - 1];
}

const vector<string> GENERATION_LABELS = {
    "Gen 1 (Kanto)", "Gen 2 (Johto)", "Gen 3 (Hoenn)",
    "Gen 4 (Sinnoh)", "Gen 5 (Unova)", "Gen 6 (Kalos)",
    "Gen 7 (Alola)", "Gen 8 (Galar + Hisui)", "Gen 9 (Paldea)"};

int chooseGeneration()
{
    clearScreen();
    cout << "\n--- SELECT GENERATION ---" << endl;
    for (size_t i = 0; i < GENERATION_LABELS.size(); ++i)
    {
        cout << (i + 1) << ". " << GENERATION_LABELS[i] << endl;
    }
    cout << "0. Cancel" << endl;
    int pick = readMenuChoice("Choice: ", static_cast<int>(GENERATION_LABELS.size()));
    return pick;
}

int promptDexNumber()
{
    return readNonNegativeNumber("Enter Dex number (0 to cancel): ");
}

string toLowerStr(string s)
{
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return s;
}

int readChoice()
{
    string line;
    if (!getline(cin, line))
        return 0;

    istringstream input(line);
    int value = 0;
    if (!(input >> value) || !(input >> ws).eof())
        return -1;
    return value;
}

int readMenuChoice(const string &prompt, int maxOption, bool allowZero)
{
    while (true)
    {
        cout << prompt;
        const int choice = readChoice();
        if (choice == 0 && cin.eof())
            return 0;
        if ((allowZero && choice == 0) || (choice >= 1 && choice <= maxOption))
            return choice;

        cout << "[ERROR] Invalid Choice." << endl;
    }
}

int readNonNegativeNumber(const string &prompt)
{
    while (true)
    {
        cout << prompt;
        const int number = readChoice();
        if (number == 0 && cin.eof())
            return 0;
        if (number >= 0)
            return number;

        if (number == -1)
            cout << "[ERROR] Please enter a number." << endl;
        else
            cout << "[ERROR] The number cannot be negative." << endl;
    }
}

bool askYesNo(const string &prompt)
{
    string answer;
    while (true)
    {
        cout << prompt << " (y/n): ";
        if (!getline(cin, answer))
            return false;

        size_t start = answer.find_first_not_of(" \t");
        size_t end = answer.find_last_not_of(" \t");
        string normalized = (start == string::npos)
                               ? ""
                               : toLowerStr(answer.substr(start, end - start + 1));

        if (normalized == "y" || normalized == "yes")
            return true;
        if (normalized == "n" || normalized == "no")
            return false;

        cout << "[ERROR] Please type y or n." << endl;
    }
}


int chooseSecondType(const string &firstType)
{
    vector<string> options = POKEMON_TYPES;
    options.push_back("Any");
    options.push_back("Mono");
    string error;

    while (true)
    {
        clearScreen();
        cout << "\n--- SELECT TYPE 2 ---" << endl;
        for (size_t i = 0; i < options.size(); ++i)
        {
            printTypeChoice(i, options[i], i < POKEMON_TYPES.size());
            if ((i + 1) % 3 == 0)
                cout << endl;
        }
        cout << right << endl;
        cout << "(Any = any second type, Mono = single-type only)" << endl;
        if (!error.empty())
            cout << "[ERROR] " << error << endl;
        cout << " 0. Cancel" << endl;

        const int pick = readMenuChoice("Choice: ", static_cast<int>(options.size()));
        if (pick == 0 || pick > static_cast<int>(POKEMON_TYPES.size()))
            return pick;
        if (toLowerStr(options[pick - 1]) != toLowerStr(firstType))
            return pick;

        error = "Type 2 must differ from Type 1. Choose Mono for a single-type Pokémon.";
    }
}


// Returns a slot number, 0 = back, or NAV_NEXT / NAV_PREV
int readSlotOrNav(const string &prompt, int maxSlot)
{
    while (true)
    {
        cout << prompt;
        string line;
        if (!getline(cin, line))
            return 0;

        size_t s = line.find_first_not_of(" \t");
        size_t e = line.find_last_not_of(" \t");
        string t = (s == string::npos) ? "" : toLowerStr(line.substr(s, e - s + 1));

        if (t == "n")
            return NAV_NEXT;
        if (t == "p")
            return NAV_PREV;
        if (t == "g")
            return NAV_GRID;

        istringstream in(t);
        int v = 0;
        if (in >> v && (in >> ws).eof() && v >= 0 && v <= maxSlot)
            return v;

        cout << "[ERROR] Invalid Choice." << endl;
    }
}
