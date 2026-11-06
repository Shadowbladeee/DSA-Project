#include "include/CSVParser.h"
#include "include/PCSystem.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const int MAX_PARTY_SIZE = 6;

void displayParty(const DoublyLinkedList &party)
{
    cout << "\n=== CURRENT PARTY (" << party.getCount() << "/" << MAX_PARTY_SIZE << ") ===" << endl;
    if (party.isEmpty())
    {
        cout << "Party is empty." << endl;
        return;
    }
    party.displayBox();
}

void depositPokemon(DoublyLinkedList &party, PCSystem &pc)
{
    if (party.getCount() <= 1)
    {
        cout << "[ERROR] You must keep at least 1 Pokémon in your party at all times!" << endl;
        return;
    }
    if (party.isEmpty())
    {
        cout << "[ERROR] Party is empty!" << endl;
        return;
    }
    if (pc.getCurrentBoxCount() >= BOX_CAPACITY)
    {
        cout << "[ERROR] Box [" << pc.getCurrentBoxName() << "] is full!" << endl;
        return;
    }

    displayParty(party);
    int targetID;
    cout << "Enter Pokémon ID to deposit: ";
    cin >> targetID;

    PokemonNode *moved = party.removeByID(targetID);
    if (moved)
    {
        pc.getCurrentList().insertNode(moved);
        cout << "[SUCCESS] Deposited " << moved->nickname << " into [" << pc.getCurrentBoxName() << "]." << endl;
    }
    else
    {
        cout << "[ERROR] ID not found in party." << endl;
    }
}

void withdrawPokemon(DoublyLinkedList &party, PCSystem &pc)
{
    if (party.getCount() >= MAX_PARTY_SIZE)
    {
        cout << "[ERROR] Party full (Max 6)." << endl;
        return;
    }
    if (pc.getCurrentList().isEmpty())
    {
        cout << "[ERROR] Active box [" << pc.getCurrentBoxName() << "] is empty!" << endl;
        return;
    }

    pc.displayCurrentBox();
    int targetID;
    cout << "Enter Pokémon ID to withdraw: ";
    cin >> targetID;

    PokemonNode *moved = pc.getCurrentList().removeByID(targetID);
    if (moved)
    {
        party.insertNode(moved);
        cout << "[SUCCESS] Withdrew " << moved->nickname << " to party!" << endl;
    }
    else
    {
        cout << "[ERROR] ID not found in active box." << endl;
    }
}

void swapPokemonInList(DoublyLinkedList &list, const string &listName)
{
    if (list.getCount() < 2)
    {
        cout << "[ERROR] Need at least 2 Pokémon in " << listName << " to swap!" << endl;
        return;
    }

    int pos1, pos2;
    cout << "Enter position of first Pokémon: ";
    cin >> pos1;
    cout << "Enter position of second Pokémon: ";
    cin >> pos2;

    if (list.swapNodes(pos1, pos2))
    {
        cout << "[SUCCESS] Swapped slot " << pos1 << " and slot " << pos2 << " in " << listName << "!" << endl;
    }
    else
    {
        cout << "[ERROR] Invalid positions entered (valid range: 1 to " << list.getCount() << ")." << endl;
    }
}

void movePokemonBetweenBoxes(PCSystem &pc)
{
    int srcBox, tgtBox, targetID;
    cout << "Enter Source Box (1-30): ";
    cin >> srcBox;
    cout << "Enter Target Box (1-30): ";
    cin >> tgtBox;
    cout << "Enter Pokémon ID to move: ";
    cin >> targetID;

    if (pc.moveBetweenBoxes(srcBox, tgtBox, targetID))
    {
        cout << "[SUCCESS] Moved Pokémon #" << targetID << " from Box " << srcBox << " to Box " << tgtBox << "." << endl;
    }
    else
    {
        cout << "[ERROR] Transfer failed (check IDs or target box capacity)." << endl;
    }
}

int main()
{
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

    // Seed initial test data
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

    int choice = -1;
    while (choice != 0)
    {
        cout << "\n=== POKEMON STORAGE SYSTEM ===" << endl;
        cout << "Active Box: " << pc.getCurrentBoxName() << endl;
        cout << "1. View Party" << endl;
        cout << "2. View Active PC Box" << endl;
        cout << "3. Deposit Pokémon (Party -> Active Box)" << endl;
        cout << "4. Withdraw Pokémon (Active Box -> Party)" << endl;
        cout << "5. Swap Party Pokémon Positions" << endl;
        cout << "6. Swap Active Box Pokémon Positions" << endl;
        cout << "7. Switch Active Box" << endl;
        cout << "8. Rename Active Box" << endl;
        cout << "9. Move Pokémon Between Boxes" << endl;
        cout << "10. View All Boxes Overview" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter command: ";

        if (cin >> choice)
        {
            if (choice == 1)
                displayParty(party);
            else if (choice == 2)
                pc.displayCurrentBox();
            else if (choice == 3)
                depositPokemon(party, pc);
            else if (choice == 4)
                withdrawPokemon(party, pc);
            else if (choice == 5)
            {
                displayParty(party);
                swapPokemonInList(party, "Party");
            }
            else if (choice == 6)
            {
                pc.displayCurrentBox();
                swapPokemonInList(pc.getCurrentList(), pc.getCurrentBoxName());
            }
            else if (choice == 7)
            {
                int boxNum;
                cout << "Enter Box (1-30): ";
                cin >> boxNum;
                pc.switchBox(boxNum);
            }
            else if (choice == 8)
            {
                string name;
                cout << "New name: ";
                cin.ignore();
                getline(cin, name);
                pc.renameCurrentBox(name);
            }
            else if (choice == 9)
                movePokemonBetweenBoxes(pc);
            else if (choice == 10)
                pc.listAllBoxes();
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