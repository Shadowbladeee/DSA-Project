#include "../include/SaveMenu.h"
#include "../include/CatchMinigame.h"
#include "../include/ConsoleUtils.h"
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
            waitForContinue();
        }
    }
    else if (choice == 2)
    {
        if (!saveFileExists(SAVE_FILE))
        {
            cout << "[ERROR] No save file found." << endl;
            waitForContinue();
            return;
        }
        if (!askYesNo("Loading replaces your current party and boxes. Continue?"))
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
            waitForContinue();
        }
    }
}
