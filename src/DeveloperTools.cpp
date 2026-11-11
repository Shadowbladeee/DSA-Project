#include "../include/DeveloperTools.h"
#include "../include/ConsoleUtils.h"
#include "../include/CatchMinigame.h"
#include "../include/MenuUtils.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace
{
enum class PokedexStatus
{
    Unseen,
    Seen,
    Caught
};

string trim(string value)
{
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos)
        return "";
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

string lower(string value)
{
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char ch) { return static_cast<char>(tolower(ch)); });
    return value;
}

void displayDeveloperMenu()
{
    clearScreen();
    printMenuHeader("DEVELOPER TOOLS");
    cout << "1. Modify Pokédex progress" << endl;
    cout << "2. Fill PC with all loaded Pokémon" << endl;
    cout << "3. Empty all PC boxes" << endl;
    cout << "4. Spawn Pokémon by species name" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
}

void displayProgressScopeMenu()
{
    clearScreen();
    printMenuHeader("MODIFY POKÉDEX PROGRESS");
    cout << "1. Change one Dex entry (includes its forms)" << endl;
    cout << "2. Change all Dex entries" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
}

void displayStatusMenu(int dexNumber, bool allEntries)
{
    clearScreen();
    printMenuHeader("SET POKÉDEX STATUS");
    if (allEntries)
        cout << "Target: all Dex entries" << endl;
    else
        cout << "Target: Dex #" << dexNumber << " and its forms" << endl;
    cout << "1. Unseen" << endl;
    cout << "2. Seen (not caught)" << endl;
    cout << "3. Caught" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
}

bool matchesStatusTarget(const SpeciesData *species, int dexNumber,
                         bool allEntries)
{
    return species != nullptr &&
           (allEntries || species->pokedexID == dexNumber);
}

void setPokedexStatus(const vector<SpeciesData *> &pokedex,
                      int dexNumber, bool allEntries, PokedexStatus status,
                      set<int> &registeredSpecies,
                      set<int> &encounteredSpecies)
{
    for (const SpeciesData *species : pokedex)
    {
        if (!matchesStatusTarget(species, dexNumber, allEntries))
            continue;

        if (status == PokedexStatus::Unseen)
        {
            registeredSpecies.erase(species->internalID);
            encounteredSpecies.erase(species->internalID);
        }
        else
        {
            encounteredSpecies.insert(species->internalID);
            if (status == PokedexStatus::Caught)
                registeredSpecies.insert(species->internalID);
            else
                registeredSpecies.erase(species->internalID);
        }
    }

    cout << "[SUCCESS] Pokédex progress updated." << endl;
    waitForContinue();
}

void modifyPokedexProgress(const vector<SpeciesData *> &pokedex,
                           set<int> &registeredSpecies,
                           set<int> &encounteredSpecies)
{
    displayProgressScopeMenu();
    const int scope = readMenuChoice("\nChoice: ", 2, true,
                                     displayProgressScopeMenu);
    if (scope == 0)
        return;

    const bool allEntries = scope == 2;
    int dexNumber = 0;
    if (!allEntries)
    {
        clearScreen();
        printMenuHeader("MODIFY ONE DEX ENTRY");
        cout << "Enter a Dex number from 1 to 1025 (0 to cancel)." << endl;
        dexNumber = readMenuChoice(
            "\nDex number: ", 1025, true, []() {
                clearScreen();
                printMenuHeader("MODIFY ONE DEX ENTRY");
                cout << "Enter a Dex number from 1 to 1025 (0 to cancel)." << endl;
            });
        if (dexNumber == 0)
            return;

        const bool exists = any_of(pokedex.begin(), pokedex.end(),
                                   [dexNumber](const SpeciesData *species) {
                                       return species != nullptr &&
                                              species->pokedexID == dexNumber;
                                   });
        if (!exists)
        {
            reportInvalidInput("No data is available for that Dex number.");
            return;
        }
    }

    displayStatusMenu(dexNumber, allEntries);
    const int statusChoice = readMenuChoice(
        "\nChoice: ", 3, true,
        [dexNumber, allEntries]() { displayStatusMenu(dexNumber, allEntries); });
    if (statusChoice == 0)
        return;

    const PokedexStatus status = statusChoice == 1
                                     ? PokedexStatus::Unseen
                                     : statusChoice == 2
                                           ? PokedexStatus::Seen
                                           : PokedexStatus::Caught;
    setPokedexStatus(pokedex, dexNumber, allEntries, status,
                     registeredSpecies, encounteredSpecies);
}

void clearPC(PCSystem &pc)
{
    for (int boxNumber = 1; boxNumber <= MAX_BOXES; ++boxNumber)
    {
        DoublyLinkedList &box = pc.getList(boxNumber);
        while (!box.isEmpty())
            box.releaseByPosition(1);
    }
}

void fillPCWithAllPokemon(const vector<SpeciesData *> &pokedex,
                          PCSystem &pc, set<int> &registeredSpecies,
                          set<int> &encounteredSpecies)
{
    const size_t availableSlots =
        static_cast<size_t>(MAX_BOXES) * BOX_CAPACITY;
    const size_t pokemonCount = count_if(
        pokedex.begin(), pokedex.end(),
        [](const SpeciesData *species) { return species != nullptr; });
    if (pokemonCount > availableSlots)
    {
        reportError("There are more loaded Pokémon than the PC can hold.");
        return;
    }

    displayDeveloperMenu();
    if (!askYesNo("Replace all PC box contents with every loaded Pokémon?",
                  displayDeveloperMenu))
        return;

    clearPC(pc);
    int boxNumber = 1;
    for (const SpeciesData *species : pokedex)
    {
        if (species == nullptr)
            continue;

        if (pc.getList(boxNumber).getCount() >= BOX_CAPACITY)
            ++boxNumber;
        pc.getList(boxNumber).append(species, species->name, "Unknown", 1,
                                     "", "Hardy", false, "");
        registeredSpecies.insert(species->internalID);
        encounteredSpecies.insert(species->internalID);
    }
    pc.setCurrentBox(1);
    cout << "[SUCCESS] Added " << pokemonCount
         << " Pokémon/forms to the PC." << endl;
    waitForContinue();
}

void emptyPC(PCSystem &pc)
{
    displayDeveloperMenu();
    if (!askYesNo("Release every Pokémon from all PC boxes? Party will be unchanged.",
                  displayDeveloperMenu))
        return;

    clearPC(pc);
    cout << "[SUCCESS] All PC boxes are empty. Party was unchanged." << endl;
    waitForContinue();
}

void displaySpawnDestinationMenu()
{
    clearScreen();
    printMenuHeader("SPAWN POKÉMON");
    cout << "Spawn level: 5" << endl;
    cout << "1. Send to Party" << endl;
    cout << "2. Send to PC" << endl;
    printMenuSeparator();
    cout << "0. Cancel" << endl;
}

const SpeciesData *selectSpeciesByName(const vector<SpeciesData *> &pokedex)
{
    while (true)
    {
        clearScreen();
        printMenuHeader("SPAWN POKÉMON");
        cout << "Enter an exact species name (blank to cancel)." << endl;
        cout << "Species name: " << flush;
        string query;
        if (!getline(cin, query) || trim(query).empty())
            return nullptr;

        const string normalizedQuery = lower(trim(query));
        vector<const SpeciesData *> matches;
        for (const SpeciesData *species : pokedex)
        {
            if (species != nullptr && lower(species->name) == normalizedQuery)
                matches.push_back(species);
        }

        if (matches.empty())
        {
            reportInvalidInput("No species matches that exact name.");
            continue;
        }
        if (matches.size() == 1)
            return matches.front();

        clearScreen();
        printMenuHeader("SELECT SPECIES FORM");
        for (size_t i = 0; i < matches.size(); ++i)
        {
            cout << (i + 1) << ". " << matches[i]->name
                 << " (Dex #" << matches[i]->pokedexID
                 << ", ID " << matches[i]->internalID << ")" << endl;
        }
        printMenuSeparator();
        cout << "0. Cancel" << endl;
        const int selected = readMenuChoice(
            "\nChoice: ", static_cast<int>(matches.size()), true, [&matches]() {
                clearScreen();
                printMenuHeader("SELECT SPECIES FORM");
                for (size_t i = 0; i < matches.size(); ++i)
                    cout << (i + 1) << ". " << matches[i]->name
                         << " (Dex #" << matches[i]->pokedexID
                         << ", ID " << matches[i]->internalID << ")" << endl;
                printMenuSeparator();
                cout << "0. Cancel" << endl;
            });
        return selected == 0 ? nullptr : matches[selected - 1];
    }
}

void spawnPokemon(const vector<SpeciesData *> &pokedex, PCSystem &pc,
                  DoublyLinkedList &party, set<int> &registeredSpecies,
                  set<int> &encounteredSpecies)
{
    const SpeciesData *species = selectSpeciesByName(pokedex);
    if (species == nullptr)
        return;

    displaySpawnDestinationMenu();
    const int destination = readMenuChoice(
        "\nChoice: ", 2, true, displaySpawnDestinationMenu);
    if (destination == 0)
        return;

    if (destination == 1 && party.getCount() >= MAX_PARTY_SIZE)
    {
        reportError("The party is full.");
        return;
    }

    PokemonNode *pokemon = new PokemonNode(species);
    pokemon->level = 5;
    if (destination == 1)
    {
        party.insertNode(pokemon);
        cout << "[SUCCESS] Spawned " << species->name
             << " at level 5 in the Party." << endl;
    }
    else
    {
        const int boxNumber = pc.storePokemonInAvailableBox(pokemon);
        if (boxNumber == 0)
        {
            delete pokemon;
            reportError("All PC boxes are full.");
            return;
        }
        cout << "[SUCCESS] Spawned " << species->name
             << " at level 5 in Box " << boxNumber << "." << endl;
    }

    registeredSpecies.insert(species->internalID);
    encounteredSpecies.insert(species->internalID);
    waitForContinue();
}
}

void handleDeveloperTools(const vector<SpeciesData *> &pokedex,
                          PCSystem &pc, DoublyLinkedList &party,
                          set<int> &registeredSpecies,
                          set<int> &encounteredSpecies)
{
    while (true)
    {
        displayDeveloperMenu();
        const int choice = readMenuChoice("\nChoice: ", 4, true,
                                          displayDeveloperMenu);
        if (choice == 0)
            return;
        if (choice == 1)
            modifyPokedexProgress(pokedex, registeredSpecies, encounteredSpecies);
        else if (choice == 2)
            fillPCWithAllPokemon(pokedex, pc, registeredSpecies, encounteredSpecies);
        else if (choice == 3)
            emptyPC(pc);
        else
            spawnPokemon(pokedex, pc, party, registeredSpecies, encounteredSpecies);
    }
}
