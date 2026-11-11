#include "../include/PCMenu.h"
#include "../include/CatchMinigame.h"
#include "../include/ConsoleUtils.h"
#include "../include/MenuUtils.h"
#include "../include/PokemonSummary.h"
#include "../include/TypeTags.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const PokemonNode *nodeAt(const DoublyLinkedList &list, int position)
{
    const PokemonNode *cur = list.getHead();
    for (int i = 1; cur != nullptr && i < position; ++i)
        cur = cur->next;
    return cur;
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

void printPokemonSlotContext(const PokemonNode *pokemon, const string &location,
                             int slot)
{
    if (pokemon == nullptr)
        return;

    printMenuSeparator();
    cout << "Selected: " << pokemon->nickname;
    if (pokemon->species != nullptr)
    {
        cout << " (" << pokemon->species->name << ", #"
             << pokemon->species->pokedexID << ")";
    }
    cout << "\nLocation: " << location << " | Slot " << slot << endl;
    printMenuSeparator();
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
        waitForContinue();
        return;
    }

    while (true)
    {
        clearScreen();
        printMenuHeader("PARTY SLOT ACTIONS");
        printPokemonSlotContext(getPokemonAtPosition(party, slot), "Party", slot);
        cout << "1. Deposit to Active Box [" << pc.getCurrentBoxName() << "]" << endl;
        cout << "2. Swap Position in Party" << endl;
        cout << "3. Change Nickname" << endl;
        cout << "4. View Pokédex Entry" << endl;
        cout << "5. Release Pokémon" << endl;
        printMenuSeparator();
        cout << "0. Cancel" << endl;
        int action = readMenuChoice("\nChoice: ", 5);
        if (action == 0)
            break;

        bool shouldReturn = false;
        if (action == 1)
        {
            if (party.getCount() <= 1)
            {
                cout << "[ERROR] Minimum 1 Pokémon required in party!" << endl;
                waitForContinue();
                continue;
            }
            if (pc.getCurrentBoxCount() >= BOX_CAPACITY)
            {
                cout << "[ERROR] Active box is full!" << endl;
                waitForContinue();
                continue;
            }

            PokemonNode *moved = party.removeByPosition(slot);
            if (moved)
            {
                pc.getCurrentList().insertNode(moved);
                cout << "[SUCCESS] Deposited " << moved->nickname << " into [" << pc.getCurrentBoxName() << "]." << endl;
                shouldReturn = true;
            }
        }
        else if (action == 2)
        {
            if (party.getCount() < 2)
            {
                cout << "[ERROR] Need at least 2 Pokémon in party to swap!" << endl;
                waitForContinue();
                continue;
            }
            int targetPos = readMenuChoice("\nSwap slot " + to_string(slot) + " with slot # (0 to cancel): ",
                                           party.getCount());
            if (targetPos == 0)
                continue;
            if (party.swapNodes(slot, targetPos))
            {
                cout << "[SUCCESS] Swapped slot " << slot << " and " << targetPos << "." << endl;
                shouldReturn = true;
            }
            else
            {
                cout << "[ERROR] Invalid target position." << endl;
            }
        }
        else if (action == 3)
        {
            changePokemonNickname(getPokemonAtPosition(party, slot));
        }
        else if (action == 4)
        {
            printPokemonSummary(getPokemonAtPosition(party, slot), true, true);
            continue;
        }
        else if (action == 5)
        {
            if (party.getCount() <= 1)
            {
                cout << "[ERROR] Cannot release your last Pokémon!" << endl;
                waitForContinue();
                continue;
            }

            const PokemonNode *p = nodeAt(party, slot);
            if (!askYesNo("Release " + p->nickname + "? This can't be undone."))
            {
                cout << "Release cancelled." << endl;
                waitForContinue();
                continue;
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
        else
        {
            cout << "[ERROR] Invalid Choice." << endl;
            waitForContinue();
            continue;
        }

        if (shouldReturn)
            break;
        waitForContinue();
    }
}

void managePartyOutsideBox(DoublyLinkedList &party, PCSystem &)
{
    while (true)
    {
        displayParty(party);
        if (party.isEmpty())
        {
            waitForContinue();
            return;
        }

        int slot = readMenuChoice("Select Slot # to manage (0 to go back): ", party.getCount());
        if (slot == 0)
            return;

        while (true)
        {
            clearScreen();
            printMenuHeader("PARTY SLOT ACTIONS");
            printPokemonSlotContext(getPokemonAtPosition(party, slot), "Party", slot);
            cout << "1. Swap Position in Party" << endl;
            cout << "2. Change Nickname" << endl;
            cout << "3. View Pokédex Entry" << endl;
            printMenuSeparator();
            cout << "0. Cancel" << endl;
            int action = readMenuChoice("\nChoice: ", 3);
            if (action == 0)
                break;

            bool shouldReturnToColumns = false;
            if (action == 1)
            {
                if (party.getCount() < 2)
                {
                    cout << "[ERROR] Need at least 2 Pokémon in party to swap!" << endl;
                    waitForContinue();
                    continue;
                }
                int targetPos = readMenuChoice("\nSwap slot " + to_string(slot) + " with slot # (0 to cancel): ",
                                               party.getCount());
                if (targetPos == 0)
                    continue;
                if (party.swapNodes(slot, targetPos))
                {
                    cout << "[SUCCESS] Swapped slot " << slot << " and " << targetPos << "." << endl;
                    shouldReturnToColumns = true;
                }
                else
                {
                    cout << "[ERROR] Invalid target position." << endl;
                }
            }
            else if (action == 2)
            {
                changePokemonNickname(getPokemonAtPosition(party, slot));
            }
            else if (action == 3)
            {
                printPokemonSummary(getPokemonAtPosition(party, slot), true, true);
                continue;
            }
            else
            {
                cout << "[ERROR] Invalid Choice." << endl;
                waitForContinue();
                continue;
            }

            if (shouldReturnToColumns)
                break;
            waitForContinue();
        }
    }
}

void handlePartySubMenuOutsideBox(DoublyLinkedList &party, PCSystem &pc)
{
    managePartyOutsideBox(party, pc);
}

void handleBoxSubMenu(PCSystem &pc, DoublyLinkedList &party, int slot)
{
    DoublyLinkedList &boxList = pc.getCurrentList();
    if (slot < 1 || slot > boxList.getCount())
    {
        cout << "[ERROR] Invalid box slot position." << endl;
        waitForContinue();
        return;
    }

    while (true)
    {
        clearScreen();
        printMenuHeader("BOX SLOT ACTIONS");
        printPokemonSlotContext(
            getPokemonAtPosition(boxList, slot),
            "Box " + to_string(pc.getCurrentBoxNumber()) + " (" +
                pc.getCurrentBoxName() + ")",
            slot);
        cout << "1. Withdraw to Party" << endl;
        cout << "2. Swap Position in Box" << endl;
        cout << "3. Move to another Box" << endl;
        cout << "4. Change Nickname" << endl;
        cout << "5. View Pokédex Entry" << endl;
        cout << "6. Release Pokémon" << endl;
        printMenuSeparator();
        cout << "0. Cancel" << endl;
        int action = readMenuChoice("\nChoice: ", 6);
        if (action == 0)
            break;

        bool shouldReturn = false;
        if (action == 1)
        {
            if (party.getCount() >= MAX_PARTY_SIZE)
            {
                cout << "[ERROR] Party is full!" << endl;
                waitForContinue();
                continue;
            }

            PokemonNode *moved = boxList.removeByPosition(slot);
            if (moved)
            {
                party.insertNode(moved);
                cout << "[SUCCESS] Withdrew " << moved->nickname << " to party." << endl;
                shouldReturn = true;
            }
        }
        else if (action == 2)
        {
            if (boxList.getCount() < 2)
            {
                cout << "[ERROR] Need at least 2 Pokémon in box to swap!" << endl;
                waitForContinue();
                continue;
            }
            int targetPos = readMenuChoice("\nSwap slot " + to_string(slot) + " with slot # (0 to cancel): ",
                                           boxList.getCount());
            if (targetPos == 0)
                continue;
            if (boxList.swapNodes(slot, targetPos))
            {
                cout << "[SUCCESS] Swapped slot " << slot << " and " << targetPos << "." << endl;
                shouldReturn = true;
            }
            else
            {
                cout << "[ERROR] Invalid target position." << endl;
            }
        }
        else if (action == 3)
        {
            int targetBox = readMenuChoice("\nEnter Target Box (1-30): ", MAX_BOXES, false);
            int srcBox = pc.getCurrentBoxNumber();
            if (targetBox == srcBox)
            {
                cout << "[ERROR] That is already the active box." << endl;
                waitForContinue();
                continue;
            }

            if (pc.moveBetweenBoxesByPosition(srcBox, targetBox, slot))
            {
                cout << "[SUCCESS] Moved slot " << slot << " to Box " << targetBox << "." << endl;
                shouldReturn = true;
            }
            else
            {
                cout << "[ERROR] Transfer failed (check box number or target capacity)." << endl;
            }
        }
        else if (action == 4)
        {
            changePokemonNickname(getPokemonAtPosition(boxList, slot));
        }
        else if (action == 5)
        {
            printPokemonSummary(getPokemonAtPosition(boxList, slot), true, true);
            continue;
        }
        else if (action == 6)
        {
            const PokemonNode *p = nodeAt(boxList, slot);
            if (!askYesNo("Release " + p->nickname + "? This can't be undone."))
            {
                cout << "Release cancelled." << endl;
                waitForContinue();
                continue;
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
        else
        {
            cout << "[ERROR] Invalid Choice." << endl;
            waitForContinue();
            continue;
        }

        if (shouldReturn)
            break;
        waitForContinue();
    }
}

void printSearchResults(const string &title, const vector<SearchResult> &results)
{
    clearScreen();
    printMenuHeader(title);
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
    return [descending](const PokemonNode &a, const PokemonNode &b)
    {
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
    printMenuHeader("SORT POKÉMON");
    cout << "1. Active Box [" << pc.getCurrentBoxName() << "]" << endl;
    cout << "2. Party" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
    int target = readMenuChoice("\nChoice: ", 2);
    if (target == 0)
        return;

    DoublyLinkedList &list = (target == 1) ? pc.getCurrentList() : party;
    if (list.getCount() < 2)
    {
        cout << "[ERROR] Need at least 2 Pokémon to sort." << endl;
        waitForContinue();
        return;
    }

    clearScreen();
    printMenuHeader("SORT BY");
    cout << "1. Name" << endl;
    cout << "2. Type" << endl;
    cout << "3. Dex Number" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
    int field = readMenuChoice("\nChoice: ", 3);
    if (field == 0)
        return;

    if (field == 1)
    {
        clearScreen();
        printMenuHeader("SORT BY NAME");
        cout << "\n1. Ascending (A-Z)" << endl;
        cout << "2. Descending (Z-A)" << endl;
        printMenuSeparator();
        cout << "0. Back" << endl;
        int dir = readMenuChoice("\nChoice: ", 2);
        if (dir == 0)
            return;

        if (dir == 1)
            list.sort(nameLess);
        else
            list.sort([](const PokemonNode &a, const PokemonNode &b)
                      { return nameLess(b, a); });

        clearScreen();
        cout << "[SUCCESS] Sorted by name." << endl;
        list.displayBox();
        waitForContinue();
    }
    else if (field == 2)
    {
        clearScreen();
        printMenuHeader("SORT BY TYPE");
        cout << "\n1. Ascending (Normal -> Fairy)" << endl;
        cout << "2. Descending (Fairy -> Normal)" << endl;
        printMenuSeparator();
        cout << "0. Back" << endl;
        int dir = readMenuChoice("\nChoice: ", 2);
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
        printMenuHeader("SORT BY DEX NUMBER");
        cout << "\n1. Ascending (low -> high)" << endl;
        cout << "2. Descending (high -> low)" << endl;
        printMenuSeparator();
        cout << "0. Back" << endl;
        int dir = readMenuChoice("\nChoice: ", 2);
        if (dir == 0)
            return;

        if (dir == 1)
            list.sort(dexLess);
        else
            list.sort([](const PokemonNode &a, const PokemonNode &b)
                      { return dexLess(b, a); });

        clearScreen();
        cout << "[SUCCESS] Sorted by Dex number ("
             << (dir == 1 ? "ascending" : "descending") << ")." << endl;
        list.displayBox();
        waitForContinue();
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
        size_t len = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3
                                   : (c >= 0xC0)   ? 2
                                                   : 1;
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
        cout << line1 << "\n"
             << line2 << "\n"
             << line3 << "\n\n";
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
    cout << "\n=== BOX " << pc.getCurrentBoxNumber() << "/" << MAX_BOXES << ": "
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
    printMenuHeader("ALL BOXES (> = active)");
    const int active = pc.getCurrentBoxNumber();

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
        printMenuHeader("POKÉMON STORAGE SYSTEM");
        cout << "Active Box: [" << pc.getCurrentBoxName() << "]" << endl;
        cout << "Party: " << party.getCount() << "/" << MAX_PARTY_SIZE << endl;
        cout << "1. Manage Party (" << party.getCount() << "/" << MAX_PARTY_SIZE << ")" << endl;
        cout << "2. Manage Active Box (" << pc.getCurrentBoxCount() << "/" << BOX_CAPACITY << ")" << endl;
        cout << "3. Box Settings (Switch / Rename / Overview)" << endl;
        cout << "4. Search Pokémon in PC" << endl;
        cout << "5. Sort Pokémon" << endl;
        printMenuSeparator();
        cout << "0. Back" << endl;

        choice = readMenuChoice("\nSelect option: ", 5);
        if (choice != -1)
        {
            if (choice == 1)
            {
                while (true)
                {
                    displayParty(party);
                    if (party.isEmpty())
                    {
                        waitForContinue();
                        break;
                    }

                    int slot = readMenuChoice("Select Slot # to manage (0 to go back): ",
                                              party.getCount());
                    if (slot == 0)
                        break;
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
                    {
                        pc.setCurrentBox(pc.getCurrentBoxNumber() >= MAX_BOXES
                                             ? 1
                                             : pc.getCurrentBoxNumber() + 1);
                    }
                    else if (slot == NAV_PREV)
                    {
                        pc.setCurrentBox(pc.getCurrentBoxNumber() == 1
                                             ? MAX_BOXES
                                             : pc.getCurrentBoxNumber() - 1);
                    }
                    else if (slot == NAV_GRID)
                    {
                        boxGridView = !boxGridView;
                    }
                    else
                    {
                        handleBoxSubMenu(pc, party, slot);
                    }
                }
            }
            else if (choice == 3)
            {
                clearScreen();
                printMenuHeader("BOX SETTINGS");
                cout << "1. Switch Active Box" << endl;
                cout << "2. Rename Active Box" << endl;
                cout << "3. View All Boxes Overview" << endl;
                printMenuSeparator();
                cout << "0. Back" << endl;
                int sub = readMenuChoice("\nChoice: ", 3);
                if (sub == 0)
                    continue;

                if (sub == 1)
                {
                    int boxNum = readMenuChoice("\nEnter Box (1-30): ", MAX_BOXES, false);
                    pc.switchBox(boxNum);
                    waitForContinue();
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
                    waitForContinue();
                }
                else if (sub == 3)
                {
                    printBoxOverview(pc);
                }
            }
            else if (choice == 4)
            {
                clearScreen();
                printMenuHeader("SEARCH POKÉMON IN PC");
                cout << "1. By nickname / species" << endl;
                cout << "2. By type" << endl;
                cout << "3. By generation" << endl;
                cout << "4. By Dex number" << endl;
                printMenuSeparator();
                cout << "0. Back" << endl;
                int sub = readMenuChoice("\nChoice: ", 4);
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
                waitForContinue();
            }
        }
        else
        {
            cout << "[ERROR] Please enter a number." << endl;
            waitForContinue();
        }
    }
}
