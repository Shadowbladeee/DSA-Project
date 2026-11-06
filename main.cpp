#include <iostream>

using namespace std;

int main() {
    // Variables
    int choice = -1;
    cout << "Bill's PC initialized!" << endl;
    
    while (choice != 0) {
        cout << "\n Command list" << endl;
        cout << "1. Move Pokémon" << endl;
        cout << "2. Withdraw Pokémon" << endl;
        cout << "3. Deposit Pokémon" << endl;
        cout << "0. See Ya!" << endl;
        cout << "Enter command: ";

        // handle valid integer input
        if (cin >> choice) {
            if (choice == 0) {
                cout << "Shutting down PC. Goodbye!" << endl;
            } else if (choice == 1) {
                cout << "[placeholder]" << endl;
            } else {
                cout << "Invalid command. Please try again." << endl;
            }
        }
        else {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number." << endl;
        }
    }

    return 0;
}