#include "include/CSVParser.h"
#include "include/PCSystem.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <functional>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using namespace std;

const int MAX_PARTY_SIZE = 6;

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

    int action;
    if (!(cin >> action))
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
        int targetPos;
        cout << "Swap slot " << slot << " with slot #: ";
        cin >> targetPos;
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

    int action;
    if (!(cin >> action))
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
        int targetPos;
        cout << "Swap slot " << slot << " with slot #: ";
        cin >> targetPos;
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
        int targetBox;
        cout << "Enter Target Box (1-30): ";
        cin >> targetBox;

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
             << ": " << r.nickname << " (#" << setfill('0') << setw(3) << r.dexNumber
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

    int choice = -1;
    while (choice != 0)
    {
        cout << "\n=== POKEMON STORAGE SYSTEM ===" << endl;
        cout << "Active Box: [" << pc.getCurrentBoxName() << "]" << endl;
        cout << "1. Manage Party (" << party.getCount() << "/" << MAX_PARTY_SIZE << ")" << endl;
        cout << "2. Manage Active Box (" << pc.getCurrentBoxCount() << "/" << BOX_CAPACITY << ")" << endl;
        cout << "3. Box Settings (Switch / Rename / Overview)" << endl;
        cout << "4. Search Pokémon in PC" << endl;
        cout << "0. Exit" << endl;
        cout << "Select option: ";

        if (cin >> choice)
        {
            if (choice == 1)
            {
                displayParty(party);
                if (!party.isEmpty())
                {
                    int slot;
                    cout << "Select Slot # to manage (0 to go back): ";
                    cin >> slot;
                    if (slot != 0)
                        handlePartySubMenu(party, pc, slot);
                }
            }
            else if (choice == 2)
            {
                pc.displayCurrentBox();
                if (!pc.getCurrentList().isEmpty())
                {
                    int slot;
                    cout << "Select Slot # to manage (0 to go back): ";
                    cin >> slot;
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
                int sub;
                cin >> sub;
                if (sub == 1)
                {
                    int boxNum;
                    cout << "Enter Box (1-30): ";
                    cin >> boxNum;
                    pc.switchBox(boxNum);
                }
                else if (sub == 2)
                {
                    string name;
                    cout << "New name: ";
                    cin.ignore();
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
                int sub = 0;
                if (!(cin >> sub))
                {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    continue;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                string query;
                if (sub == 1)
                {
                    cout << "Enter nickname or species: ";
                    getline(cin, query);
                    printSearchResults("SEARCH: \"" + query + "\"", pc.search(query));
                }
                else if (sub == 2)
                {
                    string type = chooseType();
                    if (!type.empty())
                    {
                        printSearchResults("TYPE: " + type, pc.searchByType(type));
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
        }
        else
        {
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    // Clean allocated Pokedex memory
    for (auto *species : pokedex)
    {
        delete species;
    }

    return 0;
}