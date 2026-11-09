#include "include/CSVParser.h"
#include "include/PCSystem.h"
#include "include/SaveSystem.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <functional>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using namespace std;

const int MAX_PARTY_SIZE = 6;
const string SAVE_FILE = "pokemon_save.csv";

int readChoice();
bool askYesNo(const string &prompt);

const PokemonNode *nodeAt(const DoublyLinkedList &list, int position)
{
    const PokemonNode *cur = list.getHead();
    for (int i = 1; cur != nullptr && i < position; ++i)
        cur = cur->next;
    return cur;
}

bool validChoice(int pick, int maxOption)
{
    if (pick >= 1 && pick <= maxOption)
        return true;
    if (pick != 0)
        cout << "[ERROR] Invalid choice." << endl;
    return false;
}

void displayParty(const DoublyLinkedList &party)
{
    cout << "\n=== CURRENT PARTY (" << party.getCount() << "/" << MAX_PARTY_SIZE << ") ===" << endl;
    if (party.isEmpty())
    {
        cout << "(Empty)" << endl;
        return;
    }
    party.displayBox();
}

void handlePartySubMenu(DoublyLinkedList &party, PCSystem &pc, int slot)
{
    if (slot < 1 || slot > party.getCount())
    {
        cout << "[ERROR] Invalid party slot position." << endl;
        return;
    }

    cout << "\n--- PARTY SLOT " << slot << " ACTIONS ---" << endl;
    cout << "1. Deposit to Active Box [" << pc.getCurrentBoxName() << "]" << endl;
    cout << "2. Swap Position in Party" << endl;
    cout << "3. Release Pokémon" << endl;
    cout << "0. Cancel" << endl;
    cout << "Choice: ";

    int action = readChoice();
    if (!validChoice(action, 3))
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
        cout << "Swap slot " << slot << " with slot #: ";
        int targetPos = readChoice();
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
}

void handleBoxSubMenu(PCSystem &pc, DoublyLinkedList &party, int slot)
{
    DoublyLinkedList &boxList = pc.getCurrentList();
    if (slot < 1 || slot > boxList.getCount())
    {
        cout << "[ERROR] Invalid box slot position." << endl;
        return;
    }

    cout << "\n--- BOX SLOT " << slot << " ACTIONS ---" << endl;
    cout << "1. Withdraw to Party" << endl;
    cout << "2. Swap Position in Box" << endl;
    cout << "3. Move to another Box" << endl;
    cout << "4. Release Pokémon" << endl;
    cout << "0. Cancel" << endl;
    cout << "Choice: ";

    int action = readChoice();
    if (!validChoice(action, 4))
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
        cout << "Swap slot " << slot << " with slot #: ";
        int targetPos = readChoice();
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
        cout << "Enter Target Box (1-30): ";
        int targetBox = readChoice();
        int srcBox = pc.getCurrentBoxIndex() + 1;

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
}

void printSearchResults(const string &title, const vector<SearchResult> &results)
{
    cout << "\n=== " << title << " ===" << endl;
    if (results.empty())
    {
        cout << "(No matches)" << endl;
        return;
    }
    for (const auto &r : results)
    {
        cout << "Box " << r.boxNumber << " (" << r.boxName << ") Slot " << r.slot
             << ": " << r.nickname << " (#" << right << setfill('0') << setw(3) << r.dexNumber
             << setfill(' ') << " " << r.speciesName << ") ["
             << r.type1 << (r.type2.empty() ? "" : "/" + r.type2) << "] Gen "
             << r.generation << endl;
    }
    cout << results.size() << " found." << endl;
}

const vector<string> POKEMON_TYPES = {
    "Normal", "Fire", "Water", "Electric", "Grass", "Ice",
    "Fighting", "Poison", "Ground", "Flying", "Psychic", "Bug",
    "Rock", "Ghost", "Dragon", "Dark", "Steel", "Fairy"};

string chooseType()
{
    cout << "\n--- SELECT TYPE ---" << endl;
    for (size_t i = 0; i < POKEMON_TYPES.size(); ++i)
    {
        cout << right << setw(2) << (i + 1) << ". "
             << left << setw(10) << POKEMON_TYPES[i];
        if ((i + 1) % 3 == 0)
            cout << endl;
    }
    cout << right;
    cout << " 0. Cancel" << endl;
    cout << "Choice: ";

    int pick = 0;
    if (!(cin >> pick))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "[ERROR] Invalid input." << endl;
        return "";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (pick == 0)
        return "";
    if (pick < 1 || pick > static_cast<int>(POKEMON_TYPES.size()))
    {
        cout << "[ERROR] Invalid type number." << endl;
        return "";
    }
    return POKEMON_TYPES[pick - 1];
}

const vector<string> GENERATION_LABELS = {
    "Gen 1 (Kanto)", "Gen 2 (Johto)", "Gen 3 (Hoenn)",
    "Gen 4 (Sinnoh)", "Gen 5 (Unova)", "Gen 6 (Kalos)",
    "Gen 7 (Alola)", "Gen 8 (Galar + Hisui)", "Gen 9 (Paldea)"};

int chooseGeneration()
{
    cout << "\n--- SELECT GENERATION ---" << endl;
    for (size_t i = 0; i < GENERATION_LABELS.size(); ++i)
    {
        cout << (i + 1) << ". " << GENERATION_LABELS[i] << endl;
    }
    cout << "0. Cancel" << endl;
    cout << "Choice: ";

    int pick = 0;
    if (!(cin >> pick))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "[ERROR] Invalid input." << endl;
        return 0;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (pick < 1 || pick > static_cast<int>(GENERATION_LABELS.size()))
    {
        if (pick != 0)
            cout << "[ERROR] Invalid generation number." << endl;
        return 0;
    }
    return pick;
}

int promptDexNumber()
{
    cout << "Enter Dex number (0 to cancel): ";

    int dex = 0;
    if (!(cin >> dex))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "[ERROR] Please enter a number." << endl;
        return 0;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (dex < 0)
    {
        cout << "[ERROR] Dex number must be positive." << endl;
        return 0;
    }
    return dex;
}

string toLowerStr(string s)
{
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
}

int readChoice()
{
    int v = 0;
    if (!(cin >> v))
    {
        if (cin.eof())
            return 0; // input closed: treat as back / exit
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return v;
}

bool askYesNo(const string &prompt)
{
    cout << prompt << " (y/n): ";
    string answer;
    getline(cin, answer);
    return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
}

void handleSaveMenu(PCSystem &pc, DoublyLinkedList &party, const vector<SpeciesData *> &pokedex)
{
    cout << "\n--- SAVE / LOAD ---" << endl;
    cout << "1. Save" << endl;
    cout << "2. Load" << endl;
    cout << "0. Back" << endl;
    cout << "Choice: ";
    int choice = readChoice();
    if (!validChoice(choice, 2))
        return;

    if (choice == 1)
    {
        if (saveGame(SAVE_FILE, pc, party))
            cout << "[SUCCESS] Saved to " << SAVE_FILE << "." << endl;
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
        if (loadGame(SAVE_FILE, pc, party, pokedex, MAX_PARTY_SIZE))
            cout << "[SUCCESS] Loaded " << SAVE_FILE << "." << endl;
    }
}

int chooseSecondType()
{
    cout << "\n--- SELECT TYPE 2 ---" << endl;

    vector<string> options = POKEMON_TYPES;
    options.push_back("Any");
    options.push_back("Mono");

    for (size_t i = 0; i < options.size(); ++i)
    {
        cout << right << setw(2) << (i + 1) << ". "
             << left << setw(10) << options[i];
        if ((i + 1) % 3 == 0)
            cout << endl;
    }
    cout << right << endl;
    cout << "(Any = any second type, Mono = single-type only)" << endl;
    cout << " 0. Cancel" << endl;
    cout << "Choice: ";

    int pick = readChoice();
    if (pick == -1)
    {
        cout << "[ERROR] Invalid input." << endl;
        return 0;
    }
    if (pick < 0 || pick > static_cast<int>(options.size()))
    {
        cout << "[ERROR] Invalid choice." << endl;
        return 0;
    }
    return pick;
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
    int da = a.species ? a.species->pokedexNumber : 0;
    int db = b.species ? b.species->pokedexNumber : 0;
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
    cout << "\n--- SORT ---" << endl;
    cout << "1. Active Box [" << pc.getCurrentBoxName() << "]" << endl;
    cout << "2. Party" << endl;
    cout << "0. Back" << endl;
    cout << "Choice: ";
    int target = readChoice();
    if (!validChoice(target, 2))
        return;

    DoublyLinkedList &list = (target == 1) ? pc.getCurrentList() : party;
    if (list.getCount() < 2)
    {
        cout << "[ERROR] Need at least 2 Pokémon to sort." << endl;
        return;
    }

    cout << "\n--- SORT BY ---" << endl;
    cout << "1. Name" << endl;
    cout << "2. Type" << endl;
    cout << "3. Dex Number" << endl;
    cout << "0. Back" << endl;
    cout << "Choice: ";
    int field = readChoice();
    if (!validChoice(field, 3))
        return;

    if (field == 1)
    {
        cout << "\n1. Ascending (A-Z)" << endl;
        cout << "2. Descending (Z-A)" << endl;
        cout << "Choice: ";
        int dir = readChoice();
        if (!validChoice(dir, 2))
            return;

        if (dir == 1)
            list.sort(nameLess);
        else
            list.sort([](const PokemonNode &a, const PokemonNode &b) { return nameLess(b, a); });

        cout << "[SUCCESS] Sorted by name." << endl;
        list.displayBox();
    }
    else if (field == 2)
    {
        cout << "\n1. Ascending (Normal -> Fairy)" << endl;
        cout << "2. Descending (Fairy -> Normal)" << endl;
        cout << "Choice: ";
        int dir = readChoice();
        if (!validChoice(dir, 2))
            return;

        list.sort(typeOrder(dir == 2));

        cout << "[SUCCESS] Sorted by type ("
             << (dir == 1 ? "ascending" : "descending") << ")." << endl;
        list.displayBox();
    }
    else if (field == 3)
    {
        cout << "\n1. Ascending (low -> high)" << endl;
        cout << "2. Descending (high -> low)" << endl;
        cout << "Choice: ";
        int dir = readChoice();
        if (!validChoice(dir, 2))
            return;

        if (dir == 1)
            list.sort(dexLess);
        else
            list.sort([](const PokemonNode &a, const PokemonNode &b) { return dexLess(b, a); });

        cout << "[SUCCESS] Sorted by Dex number ("
             << (dir == 1 ? "ascending" : "descending") << ")." << endl;
        list.displayBox();
    }
}

int main()
{
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    string csvFilename = "pokemon_complete.csv";
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

        const SpeciesData *Ursaring = getSpeciesByPokedexNumber(pokedex, 217);
        if (Ursaring)
            pc.getCurrentList().append(Ursaring, "ursaring", "Male", 50, "Guts", "Quick Feet", false, "2026-11-06");

    }

        int choice = -1;
    while (choice != 0)
    {
        cout << "\n=== POKEMON STORAGE SYSTEM ===" << endl;
        cout << "Active Box: [" << pc.getCurrentBoxName() << "]" << endl;
        cout << "1. Manage Party (" << party.getCount() << "/" << MAX_PARTY_SIZE << ")" << endl;
        cout << "2. Manage Active Box (" << pc.getCurrentBoxCount() << "/" << BOX_CAPACITY << ")" << endl;
        cout << "3. Box Settings (Switch / Rename / Overview)" << endl;
        cout << "4. Search Pokémon in PC" << endl;
        cout << "5. Sort Pokémon" << endl;
        cout << "6. Save / Load" << endl;
        cout << "0. Exit" << endl;
        cout << "Select option: ";

        choice = readChoice();
        if (choice != -1)
        {
            if (choice == 1)
            {
                displayParty(party);
                if (!party.isEmpty())
                {
                    cout << "Select Slot # to manage (0 to go back): ";
                    int slot = readChoice();
                    if (slot != 0)
                        handlePartySubMenu(party, pc, slot);
                }
            }
            else if (choice == 2)
            {
                pc.displayCurrentBox();
                if (!pc.getCurrentList().isEmpty())
                {
                    cout << "Select Slot # to manage (0 to go back): ";
                    int slot = readChoice();
                    if (slot != 0)
                        handleBoxSubMenu(pc, party, slot);
                }
            }
            else if (choice == 3)
            {
                cout << "\n--- BOX SETTINGS ---" << endl;
                cout << "1. Switch Active Box" << endl;
                cout << "2. Rename Active Box" << endl;
                cout << "3. View All Boxes Overview" << endl;
                cout << "0. Back" << endl;
                cout << "Choice: ";
                int sub = readChoice();
                if (!validChoice(sub, 3))
                    continue;

                if (sub == 1)
                {
                    cout << "Enter Box (1-30): ";
                    int boxNum = readChoice();
                    pc.switchBox(boxNum);
                }
                else if (sub == 2)
                {
                    string name;
                    cout << "New name: ";
                    getline(cin, name);
                    pc.renameCurrentBox(name);
                }
                else if (sub == 3)
                {
                    pc.listAllBoxes();
                }
            }
            else if (choice == 4)
            {
                cout << "\n--- SEARCH PC ---" << endl;
                cout << "1. By nickname / species" << endl;
                cout << "2. By type" << endl;
                cout << "3. By generation" << endl;
                cout << "4. By Dex number" << endl;
                cout << "0. Back" << endl;
                cout << "Choice: ";
                int sub = readChoice();
                if (!validChoice(sub, 4))
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
                        int second = chooseSecondType();
                        if (second >= 1 && second <= 18)
                        {
                            string type2 = POKEMON_TYPES[second - 1];
                            if (toLowerStr(type2) == toLowerStr(type1))
                                cout << "[ERROR] Type 2 must be different from Type 1." << endl;
                            else
                                printSearchResults("TYPE: " + type1 + "/" + type2 + " (either order)",
                                                   pc.searchByTypePair(type1, type2));
                        }
                        else if (second == 19)
                        {
                            printSearchResults("TYPE: " + type1 + " (any second type)",
                                               pc.searchByType(type1));
                        }
                        else if (second == 20)
                        {
                            printSearchResults("TYPE: " + type1 + " (single-type only)",
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
            else if (choice == 6)
            {
                handleSaveMenu(pc, party, pokedex);
            }
            else if (choice != 0)
            {
                cout << "[ERROR] Invalid choice." << endl;
            }
        }

                else
        {
            cout << "[ERROR] Please enter a number." << endl;
        }
    }

    if (askYesNo("Save before exiting?"))
    {
        if (saveGame(SAVE_FILE, pc, party))
            cout << "[SUCCESS] Saved to " << SAVE_FILE << "." << endl;
    }

    for (auto *species : pokedex)
    {
        delete species;
    }

    return 0;
}