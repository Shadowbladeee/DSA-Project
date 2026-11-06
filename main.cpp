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
    PokemonNode* getHead() const { return head; }
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
SpeciesData pikachu = {
        25, "Pikachu", "Electric", "", 35, 55, 40, 50, 50, 90, 320,
        0.4f, 6.0f, "Static", "Lightning Rod", 1, 190, false, false,
        "Yellow", "Forest", "Mouse Pokemon", "Stores electricity in cheek pouches."
    };

    DoublyLinkedList pcBox;
    pcBox.append(&pikachu, "Sparky", "Male", 15, "Jolly", "Static", false, "2026-11-06");
    pcBox.append(&pikachu, "Zap", "Female", 22, "Timid", "Lightning Rod", true, "2026-11-06");
    pcBox.append(&pikachu, "Volt", "Genderless", 5, "Adamant", "Static", false, "2026-11-06");

    PokemonNode* n1 = pcBox.getHead(); 
    PokemonNode* n2 = n1 ? n1->next : nullptr;
    PokemonNode* n3 = n2 ? n2->next : nullptr;

    if (n1 && n2 && n3) {
        cout << "Node 1: #" << n1->pokemonID << " " << n1->nickname << " (" << n1->species->name << ")" << endl;
        cout << "Node 2: #" << n2->pokemonID << " " << (n2->isShiny ? "★ " : "") << n2->nickname << endl;
        cout << "Node 3: #" << n3->pokemonID << " " << n3->nickname << " Lvl " << n3->level << endl;
    }

    return 0;
}