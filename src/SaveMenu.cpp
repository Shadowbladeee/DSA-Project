#include "../include/SaveMenu.h"
#include "../include/CatchMinigame.h"
#include "../include/ConsoleUtils.h"
#include "../include/GameSettings.h"
#include "../include/MenuUtils.h"
#include "../include/SaveSystem.h"
#include <iostream>

using namespace std;

extern const string SAVE_FILE = "pokemon_save.csv";

void handleSaveMenu(PCSystem &pc, DoublyLinkedList &party,
                    const vector<SpeciesData *> &pokedex, set<int> &registeredSpecies,
                    set<int> &encounteredSpecies)
{
    clearScreen();
    printMenuHeader("SAVE / LOAD");
    cout << "1. Save" << endl;
    cout << "2. Load" << endl;
    printMenuSeparator();
    cout << "0. Back" << endl;
    int choice = readMenuChoice("\nChoice: ", 2);
    if (choice == 0)
        return;

    if (choice == 1)
    {
        clearScreen();
        cout << "Saving game..." << endl;
        timeDelay(500);
        if (saveGame(SAVE_FILE, pc, party, registeredSpecies, encounteredSpecies))
        {
            cout << "[SUCCESS] Saved to " << SAVE_FILE << "." << endl;
            waitForContinue();
        }
        else
        {
            // saveGame reports the error and waits for acknowledgment.
        }
    }
    else if (choice == 2)
    {
        if (!saveFileExists(SAVE_FILE))
        {
            reportError("No save file found.");
            return;
        }
        SaveFileInfo info;
        string infoError;
        clearScreen();
        printMenuHeader("SAVE FILE INFORMATION");
        if (readSaveFileInfo(SAVE_FILE, pokedex, info, infoError))
        {
            cout << "Last saved: "
                 << (info.hasTimestamp ? info.savedAt : "Not available (older save)") << endl;
            cout << "Party: " << info.partyCount << "/" << MAX_PARTY_SIZE << endl;
            cout << "Pokédex registered: " << info.registeredDexCount << "/1025" << endl;
            cout << "Pokédex encountered: " << info.encounteredDexCount << "/1025" << endl;
        }
        else
        {
            cout << "[WARNING] Could not read save summary: " << infoError << endl;
            cout << "The full save validation will happen during loading." << endl;
        }
        cout << "Backup available: "
             << (saveFileExists(SAVE_FILE + ".bak") ? "Yes" : "No") << endl;
        waitForContinue();
        if (getGameSettings().confirmOverwrite &&
            !askYesNo("Loading replaces your current party and boxes. Continue?"))
        {
            cout << "Load cancelled." << endl;
            waitForContinue();
            return;
        }
        clearScreen();
        cout << "Loading game..." << endl;
        timeDelay(500);
        if (loadGame(SAVE_FILE, pc, party, pokedex, MAX_PARTY_SIZE,
                     registeredSpecies, encounteredSpecies))
        {
            cout << "[SUCCESS] Loaded " << SAVE_FILE << "." << endl;
            waitForContinue();
        }
        else
        {
            // loadGame reports the error and waits for acknowledgment.
        }
    }
}
