#include "include/CSVParser.h"
#include "include/CatchMinigame.h"
#include "include/ConsoleUtils.h"
#include "include/MenuUtils.h"
#include "include/PCMenu.h"
#include "include/PCSystem.h"
#include "include/PokedexMenu.h"
#include "include/SaveMenu.h"
#include "include/SaveSystem.h"
#include <iostream>
#include <set>
#include <string>
#include <vector>
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
void registerListSpecies(set<int> &registeredSpecies, set<int> &encounteredSpecies,
                         const DoublyLinkedList &list)
{
    for (const PokemonNode *pokemon = list.getHead(); pokemon != nullptr; pokemon = pokemon->next)
    {
        if (pokemon->species != nullptr)
        {
            registeredSpecies.insert(pokemon->species->internalID);
            encounteredSpecies.insert(pokemon->species->internalID);
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
        set<int> registeredSpecies;
        set<int> encounteredSpecies;

        bool loaded = false;
        if (saveFileExists(SAVE_FILE))
        {
            loaded = loadGame(SAVE_FILE, pc, party, pokedex, MAX_PARTY_SIZE,
                              registeredSpecies, encounteredSpecies);
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

            registerListSpecies(registeredSpecies, encounteredSpecies, party);
            for (int box = 1; box <= MAX_BOXES; ++box)
                registerListSpecies(registeredSpecies, encounteredSpecies, pc.getList(box));
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
            cout << "5. Pokédex" << endl;
            cout << "0. Exit" << endl;
            choice = readMenuChoice("Select option: ", 5);

            if (choice == 1)
            {
                startExpedition(pokedex, party, pc, registeredSpecies, encounteredSpecies);
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
                handleSaveMenu(pc, party, pokedex, registeredSpecies, encounteredSpecies);
            }
            else if (choice == 5)
            {
                handlePokedexMenu(pokedex, registeredSpecies, encounteredSpecies);
            }
        }

        if (askYesNo("Save before exiting?"))
        {
            if (saveGame(SAVE_FILE, pc, party, registeredSpecies, encounteredSpecies))
                cout << "[SUCCESS] Saved to " << SAVE_FILE << "." << endl;
        }
    }

    for (auto *species : pokedex)
        delete species;
    return 0;
}
