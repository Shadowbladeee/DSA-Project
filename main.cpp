#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// fixed pkmn data
struct SpeciesData
{
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
    bool isLegendary;
    bool isMythical;

    string color;
    string shape;
    string eggGroups;
    string habitat;

    int captureRate;

    string genus;
    string flavorText;
};

// Dynamic pkmn data
struct PokemonNode
{
    int pokemonID;
    string nickname;
    string gender;
    int level;
    string nature;
    string ability;
    bool isShiny;
    string dateCaught;

    const SpeciesData *species;

    PokemonNode *prev;
    PokemonNode *next;

    PokemonNode(int instanceID, const SpeciesData *speciesRef,
                string nick, string gndr, int lvl,
                string nat, string abil, bool shiny, string date)
        : pokemonID(instanceID),
          species(speciesRef),
          nickname(nick.empty() ? speciesRef->name : nick),
          gender(gndr),
          level(lvl),
          nature(nat),
          ability(abil),
          isShiny(shiny),
          dateCaught(date),
          prev(nullptr),
          next(nullptr) {}
};

// DLL wow
class DoublyLinkedList
{
private:
    PokemonNode *head;
    PokemonNode *tail;
    int nextID;

public:
    DoublyLinkedList()
    {
        head = nullptr;
        tail = nullptr;
        nextID = 1;
    }

    void append(const SpeciesData *species, string nick, string gndr,
                int lvl, string nat, string abil,
                bool shiny, string date)
    {
        PokemonNode *newNode = new PokemonNode(
            nextID++, species, nick, gndr, lvl,
            nat, abil, shiny, date);

        if (head == nullptr)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }
};

// CSV file detection
bool checkCSVFile(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "[ERROR] Could not open file: " << filename << endl;
        cerr << "Please verify that 'pokemon_complete.csv' is in the root project folder." << endl;
        return false;
    }

    file.close();
    return true;
};

// parse CSV
vector<string> parseCSVLine(const string &line)
{
    vector<string> result;
    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++)
    {
        char ch = line[i];

        if (ch == '"')
        {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                current += '"';
                i++;
            }
            else
            {
                inQuotes = !inQuotes;
            }
        }
        else if (ch == ',' && !inQuotes)
        {
            result.push_back(current);
            current.clear();
        }
        else
        {
            current += ch;
        }
    }

    result.push_back(current);

    if (!result.empty() && !result.back().empty() &&
        result.back().back() == '\r')
    {
        result.back().pop_back();
    }

    return result;
}

// safestoi helper
int safeStoi(const string &str, int column, int defaultValue = 0)
{
    if (str.empty())
    {
        cerr << "[WARNING] Empty value at column "
             << column << ". Using default: "
             << defaultValue << endl;

        return defaultValue;
    }

    try
    {
        size_t pos;
        int value = stoi(str, &pos);

        if (pos != str.size())
        {
            throw invalid_argument("Invalid characters");
        }

        return value;
    }
    catch (const exception &e)
    {
        cerr << "[WARNING] Invalid value at column "
             << column << ": [" << str << "]"
             << ". Using default: " << defaultValue << endl;

        return defaultValue;
    }
}

// load database
vector<SpeciesData *> loadSpeciesDatabase(const string &filename)
{
    vector<SpeciesData *> database;
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "[ERROR] Could not open " << filename << endl;
        return database;
    }

    string line;
    getline(file, line);

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        vector<string> tokens = parseCSVLine(line);
        if (tokens.size() < 30)
            continue;

        SpeciesData *species = new SpeciesData;
        species->pokedexNumber = safeStoi(tokens[0], 0);
        species->name = tokens[1];
        species->type1 = tokens[2];
        species->type2 = tokens[3];
        species->hp = safeStoi(tokens[4], 4);
        species->attack = safeStoi(tokens[5], 5);
        species->defense = safeStoi(tokens[6], 6);
        species->spAttack = safeStoi(tokens[7], 7);
        species->spDefense = safeStoi(tokens[8], 8);
        species->speed = safeStoi(tokens[9], 9);
        species->baseStatTotal = safeStoi(tokens[10], 10);
        species->heightM = tokens[11].empty() ? 0.0f : stof(tokens[11]);
        species->weightKg = tokens[12].empty() ? 0.0f : stof(tokens[12]);

        // tokens[13] ignored (base_experience)

        species->abilities = tokens[14];
        species->hiddenAbility = tokens[15];
        species->generation = safeStoi(tokens[16], 16);
        species->isLegendary = (tokens[17] == "1" || tokens[17] == "True" || tokens[17] == "true");
        species->isMythical = (tokens[18] == "1" || tokens[18] == "True" || tokens[18] == "true");

        // tokens[19] ignored (is_baby)

        species->color = tokens[20];
        species->shape = tokens[21];
        species->eggGroups = tokens[22];
        species->habitat = tokens[23];

        // tokens[24] ignored (growth_rate)

        species->captureRate = tokens[25].empty() ? 45 : safeStoi(tokens[25], 25, 45);

        // tokens[26] ignored (base_happiness)

        species->genus = tokens[27];

        // tokens[28] ignored (evolution_chain_id)

        species->flavorText = tokens[29];

        // tokens[30] ignored (sprite_url)

        database.push_back(species);
    }

    file.close();
    return database;
}

// lookup helper
const SpeciesData *getSpeciesByPokedexNumber(const vector<SpeciesData *> &database, int dexNum)
{
    for (const auto *species : database)
    {
        if (species->pokedexNumber == dexNum)
        {
            return species;
        }
    }
    return nullptr;
}

int main()
{
    // Variables
    int choice = -1;
    string csvFilename = "pokemon_complete.csv";

    if (!checkCSVFile(csvFilename))
    {
        return 1;
    }

    vector<SpeciesData *> pokedex = loadSpeciesDatabase(csvFilename);

    if (pokedex.empty())
    {
        cout << "[ERROR] Database failed to load species." << endl;
        return 1;
    }

    DoublyLinkedList pcBox;

    cout << "Bill's PC initialized!" << endl;

    while (choice != 0)
    {
        cout << "\n Command list" << endl;
        cout << "1. Move Pokémon" << endl;
        cout << "2. Withdraw Pokémon" << endl;
        cout << "3. Deposit Pokémon" << endl;
        cout << "0. See Ya!" << endl;
        cout << "Enter command: ";

        // handle valid integer input
        if (cin >> choice)
        {
            if (choice == 0)
            {
                cout << "Shutting down PC. Goodbye!" << endl;
            }
            else if (choice == 1)
            {
                cout << "[placeholder]" << endl;
            }
            else
            {
                cout << "Invalid command. Please try again." << endl;
            }
        }
        else
        {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number." << endl;
        }
    }

    for (auto *species : pokedex)
    {
        delete species;
    }

    return 0;
}