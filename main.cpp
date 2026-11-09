#include "include/CSVParser.h"
#include "include/PCSystem.h"
#include "include/PokemonSummary.h"
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32

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

PokemonNode *getPokemonAtPosition(const DoublyLinkedList &list, int position)
{
    if (position < 1 || position > list.getCount())
        return nullptr;

    PokemonNode *pokemon = list.getHead();
    for (int currentPosition = 1; currentPosition < position; currentPosition++)
        pokemon = pokemon->next;

    return pokemon;
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
    cout << "4. View Pokémon Summary" << endl;
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
    else if (action == 4)
    {
        printPokemonSummary(getPokemonAtPosition(party, slot));
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
    cout << "5. View Pokémon Summary" << endl;
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
    else if (action == 5)
    {
        printPokemonSummary(getPokemonAtPosition(boxList, slot));
    }
}

void searchPokemonPC(PCSystem &pc, const string &query)
{
    cout << "\n=== SEARCH RESULTS FOR \"" << query << "\" ===" << endl;
    vector<SearchResult> results = pc.search(query);

    if (results.empty())
    {
        cout << "(No Pokémon found matching \"" << query << "\")" << endl;
        return;
    }
    for (const auto &r : results)
    {
        cout << "Box " << r.boxNumber << " (" << r.boxName << ") Slot " << r.slot
             << ": " << r.nickname << " (" << r.speciesName << ")" << endl;
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

    const SpeciesData *starter = getSpeciesByPokedexNumber(pokedex, 1);
    if (starter)
        party.append(starter, "Neil", "Male", 19, "Overgrow", "Quirky", false, "2026-11-06");

    const SpeciesData *pidgey = getSpeciesByPokedexNumber(pokedex, 16);
    if (pidgey)
        party.append(pidgey, "the biggest bird", "Female", 3, "Keen Eye", "Quirky", false, "2026-11-06");

    const SpeciesData *charmander = getSpeciesByPokedexNumber(pokedex, 4);
    if (charmander)
        party.append(charmander, "fire", "Male", 5, "Blaze", "Quirky", false, "2026-11-06");

    const SpeciesData *squirtle = getSpeciesByPokedexNumber(pokedex, 7);
    if (squirtle)
        pc.getCurrentList().append(squirtle, "ninja turtle", "Male", 5, "Torrent", "Quirky", true, "2026-11-06");

    printPokemonSummary(getPokemonAtPosition(party, 1));

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
                string searchQuery;
                cout << "Enter Pokémon nickname or species to search: ";
                cin.ignore();
                getline(cin, searchQuery);
                searchPokemonPC(pc, searchQuery);
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