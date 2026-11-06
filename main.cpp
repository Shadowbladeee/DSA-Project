#include "include/CSVParser.h"
#include "include/PCSystem.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

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

    // Seed initial test Pokemon into Box 1
    const SpeciesData *bulbasaur = getSpeciesByPokedexNumber(pokedex, 1);
    if (bulbasaur)
    {
        pc.getCurrentList().append(bulbasaur, "Neil", "Male", 5, "Overgrow", "Overgrow", false, "2026-11-06");
    }

    int choice = -1;
    while (choice != 0)
    {
        cout << "\n=== BILL'S PC (" << pc.getCurrentBoxName() << ") ===" << endl;
        cout << "1. Display Current Box" << endl;
        cout << "2. Switch Active Box" << endl;
        cout << "3. Rename Current Box" << endl;
        cout << "4. View All Boxes Overview" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter command: ";

        if (cin >> choice)
        {
            if (choice == 1)
            {
                pc.displayCurrentBox();
            }
            else if (choice == 2)
            {
                int boxNum;
                cout << "Enter Box Number (1-30): ";
                cin >> boxNum;
                pc.switchBox(boxNum);
            }
            else if (choice == 3)
            {
                string newName;
                cout << "Enter new box name: ";
                cin.ignore();
                getline(cin, newName);
                pc.renameCurrentBox(newName);
            }
            else if (choice == 4)
            {
                pc.listAllBoxes();
            }
        }
        else
        {
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    // Clean up allocated Pokedex memory on exit
    for (auto *species : pokedex)
    {
        delete species;
    }

    return 0;
}