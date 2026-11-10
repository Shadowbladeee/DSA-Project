#include "include/CSVParser.h"
#include "include/CatchMinigame.h"
#include "include/ConsoleUtils.h"
#include "include/PCSystem.h"
#include "include/PokemonSummary.h"
#include "include/SaveSystem.h"
#include "include/TypeTags.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>
#include <sstream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifdef _WIN32

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")

void setupConsoleAndFullscreen()
{
    SetConsoleOutputCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    Sleep(200);

    INPUT in[2] = {};
    in[0].type = INPUT_KEYBOARD;
    in[0].ki.wVk = VK_F11;

    in[1].type = INPUT_KEYBOARD;
    in[1].ki.wVk = VK_F11;
    in[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, in, sizeof(INPUT));
}

#endif

using namespace std;
const string SAVE_FILE = "pokemon_save.csv";

int readChoice();
int readMenuChoice(const string &prompt, int maxOption, bool allowZero = true);
int readNonNegativeNumber(const string &prompt);
bool askYesNo(const string &prompt);
void displayParty(const DoublyLinkedList &party);

const PokemonNode *nodeAt(const DoublyLinkedList &list, int position)
{
    const PokemonNode *cur = list.getHead();
    for (int i = 1; cur != nullptr && i < position; ++i)
        cur = cur->next;
    return cur;
}

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

PokemonNode *getPokemonAtPosition(const DoublyLinkedList &list, int position)
{
    if (position < 1 || position > list.getCount())
        return nullptr;

    PokemonNode *pokemon = list.getHead();
    for (int currentPosition = 1; currentPosition < position; currentPosition++)
        pokemon = pokemon->next;

    return pokemon;
}

void changePokemonNickname(PokemonNode *pokemon)
{
    if (pokemon == nullptr)
    {
        cout << "[ERROR] Could not find that Pokémon." << endl;
        return;
    }

    cout << "Current nickname: " << pokemon->nickname << endl;
    cout << "Enter a new nickname (blank to cancel): ";
    string nickname;
    if (!getline(cin, nickname))
    {
        cout << "[INPUT] Could not read the nickname; no changes were made." << endl;
        return;
    }

    if (nickname.find_first_not_of(" \t") == string::npos)
    {
        cout << "Nickname change cancelled." << endl;
        return;
    }

    const string oldNickname = pokemon->nickname;
    pokemon->nickname = nickname;
    cout << "[SUCCESS] Renamed " << oldNickname << " to " << pokemon->nickname << "." << endl;
}

void handlePartySubMenu(DoublyLinkedList &party, PCSystem &pc, int slot)
{
    if (slot < 1 || slot > party.getCount())
    {
        cout << "[ERROR] Invalid party slot position." << endl;
        return;
    }

    clearScreen();
    cout << "\n--- PARTY SLOT " << slot << " ACTIONS ---" << endl;
    cout << "1. Deposit to Active Box [" << pc.getCurrentBoxName() << "]" << endl;
    cout << "2. Swap Position in Party" << endl;
    cout << "3. Release Pokémon" << endl;
    cout << "4. View Pokémon Summary" << endl;
    cout << "5. Change Nickname" << endl;
    cout << "0. Cancel" << endl;
    int action = readMenuChoice("Choice: ", 5);
    if (action == 0)
        return;

    if (action == 1)
    {
        if (party.getCount() <= 1)
        {
            cout << "[ERROR] Minimum 1 Pokémon required in party!" << endl;
            return;
        }
        if (pc.getCurrentBoxCount() >= BOX_CAPACITY)
        {
            cout << "[ERROR] Active box is full!" << endl;
            return;
        }

        PokemonNode *moved = party.removeByPosition(slot);
        if (moved)
        {
            pc.getCurrentList().insertNode(moved);
            cout << "[SUCCESS] Deposited " << moved->nickname << " into [" << pc.getCurrentBoxName() << "]." << endl;
        }
    }
    else if (action == 2)
    {
        if (party.getCount() < 2)
        {
            cout << "[ERROR] Need at least 2 Pokémon in party to swap!" << endl;
            return;
        }
        int targetPos = readMenuChoice("Swap slot " + to_string(slot) + " with slot # (0 to cancel): ",
                                       party.getCount());
        if (targetPos == 0)
            return;
        if (party.swapNodes(slot, targetPos))
        {
            cout << "[SUCCESS] Swapped slot " << slot << " and " << targetPos << "." << endl;
        }
        else
        {
            cout << "[ERROR] Invalid target position." << endl;
        }
    }
    else if (action == 3)
    {
        if (party.getCount() <= 1)
        {
            cout << "[ERROR] Cannot release your last Pokémon!" << endl;
            return;
        }

        const PokemonNode *p = nodeAt(party, slot);
        if (!askYesNo("Release " + p->nickname + "? This can't be undone."))
        {
            cout << "Release cancelled." << endl;
            return;
        }

        if (party.releaseByPosition(slot))
        {
            cout << "[SUCCESS] Released Pokémon at slot " << slot << "." << endl;
        }
        else
        {
            cout << "[ERROR] Failed to release Pokémon." << endl;
        }
    }
    else if (action == 4)
    {
        printPokemonSummary(getPokemonAtPosition(party, slot), true, true);
    }
    else if (action == 5)
    {
        changePokemonNickname(getPokemonAtPosition(party, slot));
    }
}

void handleBoxSubMenu(PCSystem &pc, DoublyLinkedList &party, int slot)
{
    DoublyLinkedList &boxList = pc.getCurrentList();
    if (slot < 1 || slot > boxList.getCount())
    {
        cout << "[ERROR] Invalid box slot position." << endl;
        return;
    }

    clearScreen();
    cout << "\n--- BOX SLOT " << slot << " ACTIONS ---" << endl;
    cout << "1. Withdraw to Party" << endl;
    cout << "2. Swap Position in Box" << endl;
    cout << "3. Move to another Box" << endl;
    cout << "4. Release Pokémon" << endl;
    cout << "5. View Pokémon Summary" << endl;
    cout << "6. Change Nickname" << endl;
    cout << "0. Cancel" << endl;
    int action = readMenuChoice("Choice: ", 6);
    if (action == 0)
        return;

    if (action == 1)
    {
        if (party.getCount() >= MAX_PARTY_SIZE)
        {
            cout << "[ERROR] Party is full!" << endl;
            return;
        }

        PokemonNode *moved = boxList.removeByPosition(slot);
        if (moved)
        {
            party.insertNode(moved);
            cout << "[SUCCESS] Withdrew " << moved->nickname << " to party." << endl;
        }
    }
    else if (action == 2)
    {
        if (boxList.getCount() < 2)
        {
            cout << "[ERROR] Need at least 2 Pokémon in box to swap!" << endl;
            return;
        }
        int targetPos = readMenuChoice("Swap slot " + to_string(slot) + " with slot # (0 to cancel): ",
                                       boxList.getCount());
        if (targetPos == 0)
            return;
        if (boxList.swapNodes(slot, targetPos))
        {
            cout << "[SUCCESS] Swapped slot " << slot << " and " << targetPos << "." << endl;
        }
        else
        {
            cout << "[ERROR] Invalid target position." << endl;
        }
    }
    else if (action == 3)
    {
        int targetBox = readMenuChoice("Enter Target Box (1-30): ", MAX_BOXES, false);
        int srcBox = pc.getCurrentBoxIndex() + 1;
        if (targetBox == srcBox)
        {
            cout << "[ERROR] That is already the active box." << endl;
            return;
        }

        if (pc.moveBetweenBoxesByPosition(srcBox, targetBox, slot))
        {
            cout << "[SUCCESS] Moved slot " << slot << " to Box " << targetBox << "." << endl;
        }
        else
        {
            cout << "[ERROR] Transfer failed (check box number or target capacity)." << endl;
        }
    }
    else if (action == 4)
    {
        const PokemonNode *p = nodeAt(boxList, slot);
        if (!askYesNo("Release " + p->nickname + "? This can't be undone."))
        {
            cout << "Release cancelled." << endl;
            return;
        }

        if (boxList.releaseByPosition(slot))
        {
            cout << "[SUCCESS] Released Pokémon at slot " << slot << "." << endl;
        }
        else
        {
            cout << "[ERROR] Failed to release Pokémon." << endl;
        }
    }
    else if (action == 5)
    {
        printPokemonSummary(getPokemonAtPosition(boxList, slot), true, true);
    }
    else if (action == 6)
    {
        changePokemonNickname(getPokemonAtPosition(boxList, slot));
    }
}

void printSearchResults(const string &title, const vector<SearchResult> &results)
{
    clearScreen();
    cout << "\n=== " << title << " ===" << endl;
    if (results.empty())
    {
        cout << "(No matches)" << endl;
        waitForContinue();
        return;
    }
    for (const auto &r : results)
    {
        cout << "Box " << r.boxNumber << " (" << r.boxName << ") Slot " << r.slot
             << ": " << r.nickname << " (#" << right << setfill('0') << setw(3) << r.dexNumber
             << setfill(' ') << " " << r.speciesName << ") "
             << colorTypeTag(r.type1)
             << (r.type2.empty() ? "" : " " + colorTypeTag(r.type2)) << " Gen "
             << r.generation << endl;
    }
    cout << results.size() << " found." << endl;
    waitForContinue();
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
              [](unsigned char c) { return tolower(c); });
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

void handleSaveMenu(PCSystem &pc, DoublyLinkedList &party, const vector<SpeciesData *> &pokedex)
{
    clearScreen();
    cout << "\n--- SAVE / LOAD ---" << endl;
    cout << "1. Save" << endl;
    cout << "2. Load" << endl;
    cout << "0. Back" << endl;
    int choice = readMenuChoice("Choice: ", 2);
    if (choice == 0)
        return;

    if (choice == 1)
    {
        clearScreen();
        cout << "Saving game..." << endl;
        timeDelay(500);
        if (saveGame(SAVE_FILE, pc, party))
        {
            cout << "[SUCCESS] Saved to " << SAVE_FILE << "." << endl;
            waitForContinue();
        }
    }
    else if (choice == 2)
    {
        if (!saveFileExists(SAVE_FILE))
        {
            cout << "[ERROR] No save file found." << endl;
            return;
        }
        if (!askYesNo("Loading replaces your current party and boxes. Continue?"))
        {
            cout << "Load cancelled." << endl;
            return;
        }
        clearScreen();
        cout << "Loading game..." << endl;
        timeDelay(500);
        if (loadGame(SAVE_FILE, pc, party, pokedex, MAX_PARTY_SIZE))
        {
            cout << "[SUCCESS] Loaded " << SAVE_FILE << "." << endl;
            waitForContinue();
        }
    }
}

int chooseSecondType(const string &firstType)
{
    vector<string> options = POKEMON_TYPES;
    options.push_back("Any");
    options.push_back("Mono");

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
        cout << " 0. Cancel" << endl;

        const int pick = readMenuChoice("Choice: ", static_cast<int>(options.size()));
        if (pick == 0 || pick > static_cast<int>(POKEMON_TYPES.size()))
            return pick;
        if (toLowerStr(options[pick - 1]) != toLowerStr(firstType))
            return pick;

        cout << "[ERROR] Type 2 must be different from Type 1." << endl;
    }
}

bool nameLess(const PokemonNode &a, const PokemonNode &b)
{
    string na = toLowerStr(a.species ? a.species->name : "");
    string nb = toLowerStr(b.species ? b.species->name : "");
    if (na != nb)
        return na < nb;
    return toLowerStr(a.nickname) < toLowerStr(b.nickname);
}

bool dexLess(const PokemonNode &a, const PokemonNode &b)
{
    int da = a.species ? a.species->pokedexID : 0;
    int db = b.species ? b.species->pokedexID : 0;
    if (da != db)
        return da < db;
    return toLowerStr(a.nickname) < toLowerStr(b.nickname);
}

int typeRank(const string &type)
{
    const string t = toLowerStr(type);
    for (size_t i = 0; i < POKEMON_TYPES.size(); ++i)
    {
        if (toLowerStr(POKEMON_TYPES[i]) == t)
            return static_cast<int>(i);
    }
    return static_cast<int>(POKEMON_TYPES.size());
}

NodeLess typeOrder(bool descending)
{
    return [descending](const PokemonNode &a, const PokemonNode &b) {
        int ra = typeRank(a.species ? a.species->type1 : "");
        int rb = typeRank(b.species ? b.species->type1 : "");
        if (ra != rb)
            return descending ? (ra > rb) : (ra < rb);
        return dexLess(a, b);
    };
}

void handleSortMenu(PCSystem &pc, DoublyLinkedList &party)
{
    clearScreen();
    cout << "\n--- SORT ---" << endl;
    cout << "1. Active Box [" << pc.getCurrentBoxName() << "]" << endl;
    cout << "2. Party" << endl;
    cout << "0. Back" << endl;
    int target = readMenuChoice("Choice: ", 2);
    if (target == 0)
        return;

    DoublyLinkedList &list = (target == 1) ? pc.getCurrentList() : party;
    if (list.getCount() < 2)
    {
        cout << "[ERROR] Need at least 2 Pokémon to sort." << endl;
        return;
    }

    clearScreen();
    cout << "\n--- SORT BY ---" << endl;
    cout << "1. Name" << endl;
    cout << "2. Type" << endl;
    cout << "3. Dex Number" << endl;
    cout << "0. Back" << endl;
    int field = readMenuChoice("Choice: ", 3);
    if (field == 0)
        return;

    if (field == 1)
    {
        clearScreen();
        cout << "\n1. Ascending (A-Z)" << endl;
        cout << "2. Descending (Z-A)" << endl;
        cout << "0. Back" << endl;
        int dir = readMenuChoice("Choice: ", 2);
        if (dir == 0)
            return;

        if (dir == 1)
            list.sort(nameLess);
        else
            list.sort([](const PokemonNode &a, const PokemonNode &b) { return nameLess(b, a); });

        clearScreen();
        cout << "[SUCCESS] Sorted by name." << endl;
        list.displayBox();
        waitForContinue();
    }
    else if (field == 2)
    {
        clearScreen();
        cout << "\n1. Ascending (Normal -> Fairy)" << endl;
        cout << "2. Descending (Fairy -> Normal)" << endl;
        cout << "0. Back" << endl;
        int dir = readMenuChoice("Choice: ", 2);
        if (dir == 0)
            return;

        list.sort(typeOrder(dir == 2));

        clearScreen();
        cout << "[SUCCESS] Sorted by type ("
             << (dir == 1 ? "ascending" : "descending") << ")." << endl;
        list.displayBox();
        waitForContinue();
    }
    else if (field == 3)
    {
        clearScreen();
        cout << "\n1. Ascending (low -> high)" << endl;
        cout << "2. Descending (high -> low)" << endl;
        cout << "0. Back" << endl;
        int dir = readMenuChoice("Choice: ", 2);
        if (dir == 0)
            return;

        if (dir == 1)
            list.sort(dexLess);
        else
            list.sort([](const PokemonNode &a, const PokemonNode &b) { return dexLess(b, a); });

        clearScreen();
        cout << "[SUCCESS] Sorted by Dex number ("
             << (dir == 1 ? "ascending" : "descending") << ")." << endl;
        list.displayBox();
        waitForContinue();
    }
}

const int NAV_NEXT = -2;
const int NAV_PREV = -3;
const int NAV_GRID = -4;

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

bool boxGridView = true;

// Visible characters: skips colour codes and counts each UTF-8 character once
int visibleWidth(const string &s)
{
    int w = 0;
    bool inEscape = false;
    for (unsigned char c : s)
    {
        if (inEscape)
        {
            if (c == 'm')
                inEscape = false;
            continue;
        }
        if (c == 0x1B)
        {
            inEscape = true;
            continue;
        }
        if ((c & 0xC0) != 0x80)
            ++w;
    }
    return w;
}

string padTo(const string &s, int width)
{
    return s + string(max(0, width - visibleWidth(s)), ' ');
}

string fitText(const string &s, int width)
{
    if (visibleWidth(s) <= width)
        return s;

    string out;
    int n = 0;
    size_t i = 0;
    while (i < s.size() && n < width - 1)
    {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
        out += s.substr(i, len);
        i += len;
        ++n;
    }
    return out + ".";
}

void printPokemonGrid(const DoublyLinkedList &list, int capacity, int cols)
{
    const int CELL_W = 23;
    const int rows = (capacity + cols - 1) / cols;

    vector<const PokemonNode *> mons;
    for (const PokemonNode *p = list.getHead(); p != nullptr; p = p->next)
        mons.push_back(p);

    for (int r = 0; r < rows; ++r)
    {
        string line1, line2, line3;
        for (int c = 0; c < cols; ++c)
        {
            const int idx = r * cols + c;
            if (idx >= capacity)
                break;

            ostringstream num;
            num << setw(2) << setfill('0') << (idx + 1);

            string top, middle, bottom;
            if (idx < static_cast<int>(mons.size()))
            {
                const PokemonNode *p = mons[idx];
                const SpeciesData *s = p->species;
                top = num.str() + (p->isShiny ? "\033[93m*\033[0m" : " ") +
                      fitText(p->nickname, CELL_W - 4);
                middle = " " + fitText(s ? s->name : "?", 12) + " L" + to_string(p->level);
                if (s)
                    bottom = " " + colorTypeTag(s->type1) +
                             (s->type2.empty() ? "" : " " + colorTypeTag(s->type2));
            }
            else
            {
                top = "\033[90m" + num.str() + " --\033[0m";
            }

            line1 += padTo(top, CELL_W);
            line2 += padTo(middle, CELL_W);
            line3 += padTo(bottom, CELL_W);
        }
        cout << line1 << "\n" << line2 << "\n" << line3 << "\n\n";
    }
}

void displayParty(const DoublyLinkedList &party)
{
    clearScreen();
    cout << "\n=== CURRENT PARTY (" << party.getCount() << "/" << MAX_PARTY_SIZE << ") "
         << fillBar(party.getCount(), MAX_PARTY_SIZE, 6) << " ===" << endl;
    printPokemonGrid(party, MAX_PARTY_SIZE, 3);
}

void showActiveBox(PCSystem &pc)
{
    clearScreen();
    const int count = pc.getCurrentBoxCount();
    cout << "\n=== BOX " << (pc.getCurrentBoxIndex() + 1) << "/" << MAX_BOXES << ": "
         << pc.getCurrentBoxName() << " (" << count << "/" << BOX_CAPACITY << ") "
         << fillBar(count, BOX_CAPACITY, 15) << " ===" << endl;

    if (boxGridView)
    {
        printPokemonGrid(pc.getCurrentList(), BOX_CAPACITY, 5);
        return;
    }
    if (count == 0)
    {
        cout << "(Empty)" << endl;
        return;
    }
    pc.getCurrentList().displayBox();
}

void printBoxOverview(PCSystem &pc)
{
    clearScreen();
    cout << "\n=== ALL BOXES (> = active) ===" << endl;
    const int active = pc.getCurrentBoxIndex() + 1;

    for (int row = 0; row < 10; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            const int b = col * 10 + row + 1;
            string name = pc.getBoxName(b);
            if (name.size() > 10)
                name = name.substr(0, 9) + ".";

            ostringstream cell;
            cell << (b == active ? '>' : ' ') << setw(2) << b << " "
                 << left << setw(10) << name << right
                 << " " << setw(2) << pc.getList(b).getCount() << "/" << BOX_CAPACITY;
            cout << cell.str() << "   ";
        }
        cout << endl;
    }
    waitForContinue();
}

void viewPCMenu(PCSystem &pc, DoublyLinkedList &party)
{
    int choice = -1;
    while (choice != 0)
    {
        clearScreen();
        cout << "\n=== POKEMON STORAGE SYSTEM ===" << endl;
        cout << "Active Box: [" << pc.getCurrentBoxName() << "]" << endl;
        cout << "1. Manage Party (" << party.getCount() << "/" << MAX_PARTY_SIZE << ")" << endl;
        cout << "2. Manage Active Box (" << pc.getCurrentBoxCount() << "/" << BOX_CAPACITY << ")" << endl;
        cout << "3. Box Settings (Switch / Rename / Overview)" << endl;
        cout << "4. Search Pokémon in PC" << endl;
        cout << "5. Sort Pokémon" << endl;
        cout << "0. Back" << endl;
        cout << "Select option: ";

        choice = readChoice();
        if (choice != -1)
        {
            if (choice == 1)
            {
                displayParty(party);
                if (!party.isEmpty())
                {
                    int slot = readMenuChoice("Select Slot # to manage (0 to go back): ",
                                              party.getCount());
                    if (slot != 0)
                        handlePartySubMenu(party, pc, slot);
                }
            }
            else if (choice == 2)
            {
                while (true)
                {
                    showActiveBox(pc);
                    int slot = readSlotOrNav("Slot # to manage | N = next | P = previous | G = grid/list | 0 = back: ",
                                             pc.getCurrentList().getCount());
                    if (slot == 0)
                        break;
                    if (slot == NAV_NEXT)
                        pc.setCurrentBox(pc.getCurrentBoxIndex() + 1 >= MAX_BOXES ? 1 : pc.getCurrentBoxIndex() + 2);
                    else if (slot == NAV_PREV)
                        pc.setCurrentBox(pc.getCurrentBoxIndex() == 0 ? MAX_BOXES : pc.getCurrentBoxIndex());
                    else if (slot == NAV_GRID)
                        boxGridView = !boxGridView;
                    else
                        handleBoxSubMenu(pc, party, slot);
                }
            }
            else if (choice == 3)
            {
                clearScreen();
                cout << "\n--- BOX SETTINGS ---" << endl;
                cout << "1. Switch Active Box" << endl;
                cout << "2. Rename Active Box" << endl;
                cout << "3. View All Boxes Overview" << endl;
                cout << "0. Back" << endl;
                int sub = readMenuChoice("Choice: ", 3);
                if (sub == 0)
                    continue;

                if (sub == 1)
                {
                    int boxNum = readMenuChoice("Enter Box (1-30): ", MAX_BOXES, false);
                    pc.switchBox(boxNum);
                }
                else if (sub == 2)
                {
                    string name;
                    do
                    {
                        cout << "New name: ";
                        if (!getline(cin, name))
                            return;
                        if (name.empty())
                            cout << "[ERROR] Box name cannot be empty." << endl;
                    } while (name.empty());
                    pc.renameCurrentBox(name);
                }
                else if (sub == 3)
                {
                    printBoxOverview(pc);
                }
            }
            else if (choice == 4)
            {
                clearScreen();
                cout << "\n--- SEARCH PC ---" << endl;
                cout << "1. By nickname / species" << endl;
                cout << "2. By type" << endl;
                cout << "3. By generation" << endl;
                cout << "4. By Dex number" << endl;
                cout << "0. Back" << endl;
                int sub = readMenuChoice("Choice: ", 4);
                if (sub == 0)
                    continue;

                string query;
                if (sub == 1)
                {
                    cout << "Enter nickname or species: ";
                    getline(cin, query);
                    printSearchResults("SEARCH: \"" + query + "\"", pc.search(query));
                }
                else if (sub == 2)
                {
                    string type1 = chooseType();
                    if (!type1.empty())
                    {
                        int second = chooseSecondType(type1);
                        if (second >= 1 && second <= 18)
                        {
                            string type2 = POKEMON_TYPES[second - 1];
                            printSearchResults("TYPE: " + colorTypeTag(type1) + " / " +
                                                   colorTypeTag(type2) + " (either order)",
                                               pc.searchByTypePair(type1, type2));
                        }
                        else if (second == 19)
                        {
                            printSearchResults("TYPE: " + colorTypeTag(type1) + " (any second type)",
                                               pc.searchByType(type1));
                        }
                        else if (second == 20)
                        {
                            printSearchResults("TYPE: " + colorTypeTag(type1) + " (single-type only)",
                                               pc.searchByMonoType(type1));
                        }
                    }
                }
                else if (sub == 3)
                {
                    int gen = chooseGeneration();
                    if (gen != 0)
                    {
                        printSearchResults("GENERATION: " + GENERATION_LABELS[gen - 1],
                                        pc.searchByGeneration(gen));
                    }
                }
                else if (sub == 4)
                {
                    int dex = promptDexNumber();
                    if (dex != 0)
                    {
                        printSearchResults("DEX #" + to_string(dex), pc.searchByDexNumber(dex));
                    }
                }
            }
            else if (choice == 5)
            {
                handleSortMenu(pc, party);
            }
            else if (choice != 0)
            {
                cout << "[ERROR] Invalid Choice." << endl;
            }
        }
        else
        {
            cout << "[ERROR] Please enter a number." << endl;
        }
    }
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    setupConsoleAndFullscreen();
#endif

    const string csvFilename = "pokemon_complete.csv";
    if (!checkCSVFile(csvFilename))
    {
        cout << "[ERROR] Could not find or open " << csvFilename << endl;
        return 1;
    }

    vector<SpeciesData *> pokedex = loadSpeciesDatabase(csvFilename);
    if (pokedex.empty())
    {
        cout << "[ERROR] Failed to load Pokedex database." << endl;
        return 1;
    }

    {
        PCSystem pc;
        DoublyLinkedList party;

        bool loaded = false;
        if (saveFileExists(SAVE_FILE))
        {
            loaded = loadGame(SAVE_FILE, pc, party, pokedex, MAX_PARTY_SIZE);
            if (loaded)
                cout << "[INFO] Loaded " << SAVE_FILE << "." << endl;
        }

        if (!loaded)
        {
            const SpeciesData *starter = getSpeciesByPokedexNumber(pokedex, 1);
            if (starter)
                party.append(starter, "Neil", "Male", 19, "Overgrow", "Overgrow", false, "2026-11-06");

            const SpeciesData *pidgey = getSpeciesByPokedexNumber(pokedex, 16);
            if (pidgey)
                party.append(pidgey, "the biggest bird", "Female", 3, "Keen Eye", "Keen Eye", false, "2026-11-06");

            const SpeciesData *charmander = getSpeciesByPokedexNumber(pokedex, 4);
            if (charmander)
                party.append(charmander, "fire", "Male", 5, "Blaze", "Blaze", false, "2026-11-06");

            const SpeciesData *squirtle = getSpeciesByPokedexNumber(pokedex, 7);
            if (squirtle)
                pc.getCurrentList().append(squirtle, "ninja turtle", "Male", 5, "Torrent", "Torrent", true, "2026-11-06");

            const SpeciesData *marshtomp = getSpeciesByPokedexNumber(pokedex, 259);
            if (marshtomp)
                pc.getCurrentList().append(marshtomp, "mudkip", "Male", 16, "Torrent", "Damp", false, "2026-11-06");

            const SpeciesData *garchomp = getSpeciesByPokedexNumber(pokedex, 445);
            if (garchomp)
                pc.getCurrentList().append(garchomp, "garchomp", "Male", 50, "Rough Skin", "Jolly", false, "2026-11-06");

            const SpeciesData *gardevoir = getSpeciesByPokedexNumber(pokedex, 282);
            if (gardevoir)
                pc.getCurrentList().append(gardevoir, "gardevoir", "Female", 50, "Synchronize", "Trace", false, "2026-11-06");

            const SpeciesData *ursaring = getSpeciesByPokedexNumber(pokedex, 217);
            if (ursaring)
                pc.getCurrentList().append(ursaring, "ursaring", "Male", 50, "Guts", "Quick Feet", false, "2026-11-06");
        
            const SpeciesData *meowth = getSpeciesByPokedexNumber(pokedex, 52);
            if (meowth)
                pc.getCurrentList().append(meowth, "meowth", "Male", 50, "Pickup", "Technician", false, "2026-11-06");
        }
        
        int choice = -1;
        while (choice != 0)
        {
            clearScreen();
            cout << "\n=== POKEMON ADVENTURE ===" << endl;
            cout << "1. Start an Expedition" << endl;
            cout << "2. View PC" << endl;
            cout << "3. View Party" << endl;
            cout << "4. Save / Load" << endl;
            cout << "0. Exit" << endl;
            choice = readMenuChoice("Select option: ", 4);

            if (choice == 1)
            {
                startExpedition(pokedex, party, pc);
            }
            else if (choice == 2)
            {
                viewPCMenu(pc, party);
            }
            else if (choice == 3)
            {
                displayParty(party);
                waitForContinue();
            }
            else if (choice == 4)
            {
                handleSaveMenu(pc, party, pokedex);
            }
        }

        if (askYesNo("Save before exiting?"))
        {
            if (saveGame(SAVE_FILE, pc, party))
                cout << "[SUCCESS] Saved to " << SAVE_FILE << "." << endl;
        }
    }

    for (auto *species : pokedex)
        delete species;
    return 0;
}