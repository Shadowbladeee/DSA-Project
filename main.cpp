#include <iostream>
#include <string>

using namespace std;

struct SpeciesData {
    int pokedexNumber;
    string name;
    string type1;
    string type2;
    int hp;
    int attack;
    int defense;
    int spAttack;
    int spDefense;
    int speed;
    int baseStatTotal;
    float heightM;
    float weightKg;
    string abilities;
    string hiddenAbility;
    int generation;
    int captureRate;
    bool isLegendary;
    bool isMythical;
    string color;
    string habitat;
    string genus;
    string flavorText;
};

struct PokemonNode {
    int pokemonID;
    string nickname;
    string gender;
    int level;
    string nature;
    string ability;
    bool isShiny;
    string dateCaught;

    const SpeciesData* species;

    PokemonNode* prev;
    PokemonNode* next;

    PokemonNode(int instanceID, const SpeciesData* speciesRef, string nick, 
                string gndr, int lvl, string nat, string abil, bool shiny, string date)
        : pokemonID(instanceID), species(speciesRef), 
          nickname(nick.empty() ? speciesRef->name : nick),
          gender(gndr), level(lvl), nature(nat), ability(abil), isShiny(shiny), dateCaught(date),
          prev(nullptr), next(nullptr) {}
};

class DoublyLinkedList {
private:
    PokemonNode* head;
    PokemonNode* tail;
    int nextID;

public:
    DoublyLinkedList() {
        head = nullptr;
        tail = nullptr;
        nextID = 1;
    }

    void append(const SpeciesData* species, string nick, string gndr, 
                int lvl, string nat, string abil, bool shiny, string date) 
    {
        PokemonNode* newNode = new PokemonNode(
            nextID++, species, nick, gndr, lvl, nat, abil, shiny, date
        );

        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }
};

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