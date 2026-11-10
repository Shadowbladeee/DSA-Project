#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "../include/CatchMinigame.h"
#include "../include/ConsoleUtils.h"
#include "../include/PokemonSummary.h"

using namespace std;

namespace
{

    // --- Data Structures ---
    struct Inventory
    {
        int balls = 10;
        const int maxBalls = 15;
        int rocks = 5;
        const int maxRocks = 10;
        int bait = 5;
        const int maxBait = 10;
    };

    struct ExpeditionStats
    {
        int encountered = 0;
        int caught = 0;
        int pokemonFled = 0;
        int itemsCollected = 0;
    };

    // --- Global/Module State ---
    Inventory playerInv;
    ExpeditionStats stats;
    bool expeditionOver = false;
    bool lastChanceUsed = false;
    int endingBonus = 0;
    const vector<SpeciesData *> *speciesDB = nullptr;
    DoublyLinkedList *partyList = nullptr;
    PCSystem *pcSystem = nullptr;
    set<int> *registeredSpecies = nullptr;
    set<int> *encounteredSpecies = nullptr;
    unordered_map<int, unordered_set<string>> usedEncounterModifiers;

    struct EncounterModifiers
    {
        int level;
        string nature;
        string ability;
    };

    mt19937 &getRandomGenerator()
    {
        static mt19937 generator(random_device{}());
        return generator;
    }

    // --- Math & Probability Helpers ---
    double calculateCaptureChance(const SpeciesData &pokemon, double modifier)
    {
        // Base C = 15% + 70% * (Capture Rate / 255)
        double baseC = 15.0 + 70.0 * (pokemon.captureRate / 255.0);
        return max(5.0, min(baseC + modifier, 95.0));
    }

    double calculateFleeChance(const SpeciesData &pokemon, double modifier)
    {
        // Base F = 5% + 35% * (1 - Capture Rate / 255)
        double baseF = 5.0 + 35.0 * (1.0 - (pokemon.captureRate / 255.0));
        return max(2.0, min(baseF + modifier, 80.0));
    }

    bool rollPercentage(double chancePercent)
    {
        uniform_int_distribution<int> distribution(0, 9999);
        const double roll = distribution(getRandomGenerator()) / 100.0;
        return roll < chancePercent;
    }

    bool rollShinyChance()
    {
        static uniform_int_distribution<int> distribution(1, 4096);
        return distribution(getRandomGenerator()) == 1;
    }

    int getEncounterLevel(const SpeciesData &pokemon)
    {
        int minBaseExperience = numeric_limits<int>::max();
        int maxBaseExperience = 0;
        for (const SpeciesData *species : *speciesDB)
        {
            if (species->baseExperience <= 0)
                continue;

            minBaseExperience = min(minBaseExperience, species->baseExperience);
            maxBaseExperience = max(maxBaseExperience, species->baseExperience);
        }

        if (pokemon.baseExperience <= 0 || minBaseExperience == numeric_limits<int>::max() ||
            maxBaseExperience <= minBaseExperience)
        {
            return 1;
        }

        const double normalized =
            static_cast<double>(pokemon.baseExperience - minBaseExperience) /
            (maxBaseExperience - minBaseExperience);
        return 1 + static_cast<int>(normalized * 99.0 + 0.5);
    }

    EncounterModifiers generateEncounterModifiers(const SpeciesData &pokemon)
    {
        static const vector<string> natures = {
            "Hardy", "Lonely", "Brave", "Adamant", "Naughty",
            "Bold", "Docile", "Relaxed", "Impish", "Lax",
            "Timid", "Hasty", "Serious", "Jolly", "Naive",
            "Modest", "Mild", "Quiet", "Bashful", "Rash",
            "Calm", "Gentle", "Sassy", "Careful", "Quirky"};

        vector<string> abilities;
        size_t start = 0;
        while (start <= pokemon.abilities.size())
        {
            const size_t separator = pokemon.abilities.find('|', start);
            const string ability = pokemon.abilities.substr(
                start, separator == string::npos ? string::npos : separator - start);
            if (!ability.empty())
                abilities.push_back(ability);
            if (separator == string::npos)
                break;
            start = separator + 1;
        }
        if (abilities.empty())
            abilities.push_back("");

        vector<pair<string, string>> available;
        unordered_set<string> &used = usedEncounterModifiers[pokemon.internalID];
        for (const string &nature : natures)
        {
            for (const string &ability : abilities)
            {
                const string signature = nature + "|" + ability;
                if (used.find(signature) == used.end())
                    available.emplace_back(nature, ability);
            }
        }

        if (available.empty())
        {
            for (const string &nature : natures)
            {
                for (const string &ability : abilities)
                    available.emplace_back(nature, ability);
            }
        }

        uniform_int_distribution<int> chooseModifier(0, static_cast<int>(available.size()) - 1);
        const pair<string, string> &selected = available[chooseModifier(getRandomGenerator())];
        used.insert(selected.first + "|" + selected.second);
        return {getEncounterLevel(pokemon), selected.first, selected.second};
    }

    bool readMenuChoice(const string &prompt, int minChoice, int maxChoice, int &choice)
    {
        string line;
        while (true)
        {
            cout << "\n" << prompt;
            if (!getline(cin, line))
                return false;

            istringstream input(line);
            if (input >> choice && (input >> ws).eof() &&
                choice >= minChoice && choice <= maxChoice)
            {
                return true;
            }

            cout << "[ERROR] Invalid Choice.\n";
        }
    }

    // --- Systems Implementation ---

    void showInventory()
    {
        clearScreen();
        cout << "\n====================================\n";
        cout << "        CURRENT INVENTORY           \n";
        cout << "====================================\n";
        cout << "  Safari Balls : " << playerInv.balls << " / " << playerInv.maxBalls << "\n";
        cout << "  Rocks        : " << playerInv.rocks << " / " << playerInv.maxRocks << "\n";
        cout << "  Bait         : " << playerInv.bait << " / " << playerInv.maxBait << "\n";
        cout << "====================================\n\n";
        waitForContinue();
    }

    enum class ItemDiscovery
    {
        Nothing,
        SafariBall,
        Rock,
        Bait
    };

    void collectItems(int &inventoryCount, int inventoryMax, const string &singularName,
                      const string &pluralName, const string &inventoryName)
    {
        uniform_int_distribution<int> chance(0, 99);
        const int requested = chance(getRandomGenerator()) < 20
                                  ? uniform_int_distribution<int>(2, 5)(getRandomGenerator())
                                  : 1;
        const int collected = min(requested, inventoryMax - inventoryCount);

        if (collected == 0)
        {
            cout << "\n[FOUND ITEM] You found " << singularName
                 << ", but your bag is full!\n";
            return;
        }

        inventoryCount += collected;
        stats.itemsCollected += collected;

        if (requested == 1)
        {
            cout << "\n[FOUND ITEM] You found " << singularName
                 << "! (" << inventoryName << ": " << inventoryCount << ")\n";
        }
        else
        {
            cout << "\n[FOUND ITEM] You found a handful of " << pluralName;
            if (collected < requested)
                cout << ", but only " << collected << " fit in your bag";
            else
                cout << " (" << collected << ")";
            cout << "! (" << inventoryName << ": " << inventoryCount << ")\n";
        }
    }

    ItemDiscovery discoverItem()
    {
        uniform_int_distribution<int> distribution(0, 99);
        const int roll = distribution(getRandomGenerator());

        if (roll < 65)
        {
            cout << "\n[EXPLORE] You searched the area but found nothing.\n";
            return ItemDiscovery::Nothing;
        }
        else if (roll < 80)
        { // 15% Ball
            collectItems(playerInv.balls, playerInv.maxBalls, "a Safari Ball",
                         "Safari Balls", "Balls");
            return ItemDiscovery::SafariBall;
        }
        else if (roll < 90)
        { // 10% Rock
            collectItems(playerInv.rocks, playerInv.maxRocks, "a Rock",
                         "rocks", "Rocks");
            return ItemDiscovery::Rock;
        }
        else
        { // 10% Bait
            collectItems(playerInv.bait, playerInv.maxBait, "some Bait",
                         "Bait", "Bait");
            return ItemDiscovery::Bait;
        }
    }

    bool generateEncounter(const SpeciesData *&pokemon)
    {
        if (speciesDB == nullptr || speciesDB->empty())
        {
            cout << "[ERROR] The Pokémon database is empty; cannot generate an encounter.\n";
            return false;
        }

        uniform_int_distribution<size_t> distribution(0, speciesDB->size() - 1);
        const size_t idx = distribution(getRandomGenerator());
        pokemon = (*speciesDB)[idx];
        return true;
    }

    string getCurrentDate()
    {
        const time_t now = time(nullptr);
        tm localTime;
#ifdef _WIN32
        if (localtime_s(&localTime, &now) != 0)
            return "";
#else
        if (localtime_r(&now, &localTime) == nullptr)
            return "";
#endif

        char date[11];
        if (strftime(date, sizeof(date), "%Y-%m-%d", &localTime) == 0)
            return "";
        return date;
    }

    void addCaughtPokemon(const SpeciesData *species, const EncounterModifiers &modifiers, bool isShiny)
    {
        registeredSpecies->insert(species->internalID);

        string nickname;
        cout << "Give " << species->name << " a nickname (press Enter to keep its name): ";
        if (!getline(cin, nickname))
        {
            nickname = species->name;
            expeditionOver = true;
        }

        PokemonNode *pokemon = new PokemonNode(species);
        pokemon->isShiny = isShiny;
        pokemon->gender.clear();
        pokemon->level = modifiers.level;
        pokemon->nature = modifiers.nature;
        pokemon->ability = modifiers.ability;
        if (!nickname.empty())
            pokemon->nickname = nickname;
        pokemon->dateCaught = getCurrentDate();

        if (partyList->getCount() < MAX_PARTY_SIZE)
        {
            partyList->insertNode(pokemon);
            cout << pokemon->nickname << " joined your party!\n";
            return;
        }

        int boxNumber = pcSystem->storePokemonInAvailableBox(pokemon);
        if (boxNumber != 0)
        {
            cout << "Your party is full. " << pokemon->nickname
                 << " was sent to Box " << boxNumber << ".\n";
            return;
        }

        delete pokemon;
        cout << "[ERROR] Your party and all PC boxes are full; "
             << species->name << " could not be stored and was released.\n";
    }

    void showWildPokemon(const SpeciesData *pokemon, const EncounterModifiers &modifiers, bool isShiny)
    {
        clearScreen();
        playPokemonCry(pokemon->pokedexID);

        const string formID = to_string(pokemon->internalID);
        const string pokedexID = to_string(pokemon->pokedexID);
        const string spriteFolder = isShiny ? "assets/spritesShiny/" : "assets/sprites/";
        const string formSprite = spriteFolder + formID + ".png";
        const string baseSprite = spriteFolder + pokedexID + ".png";
        const string fallbackSprite = spriteFolder + "0.png";

        string spritePath = fallbackSprite;
        if (ifstream(formSprite, ios::binary).good())
            spritePath = formSprite;
        else if (ifstream(baseSprite, ios::binary).good())
            spritePath = baseSprite;

        renderAutoCenteredSprite(spritePath);

        cout << "\n------------------------------------\n";
        cout << "  A wild ";
        if (isShiny)
            cout << "SHINY ";
        cout << pokemon->name << " appeared!\n";
        cout << "  Level: " << modifiers.level << " | Nature: " << modifiers.nature
             << " | Ability: " << (modifiers.ability.empty() ? "None" : formatAbilities(modifiers.ability)) << "\n";
        cout << "------------------------------------\n";
        timeDelay(1000);
    }

    void handleEncounter(const SpeciesData *pokemon, const EncounterModifiers &modifiers, bool isShiny)
    {
        stats.encountered++;
        encounteredSpecies->insert(pokemon->internalID);
        showWildPokemon(pokemon, modifiers, isShiny);

        double captureMod = 0.0;
        double fleeMod = 0.0;
        bool active = true;

        while (active)
        {
            cout << "\n[ENCOUNTER MENU] (Balls: " << playerInv.balls
                 << " | Rocks: " << playerInv.rocks
                 << " | Bait: " << playerInv.bait << ")\n";
            cout << "1. Throw Safari Ball\n";
            cout << "2. Throw Rock (+20% Catch, +15% Flee)\n";
            cout << "3. Throw Bait (-10% Catch, -15% Flee)\n";
            cout << "4. Run Away\n";

            int action;
            if (!readMenuChoice("Choice: ", 1, 4, action))
            {
                cout << "\n[INPUT] Input closed. Ending the expedition.\n";
                expeditionOver = true;
                return;
            }
            bool turnCompleted = false;

            if (action == 1)
            { // SAFARI BALL
                if (playerInv.balls <= 0)
                {
                    cout << "You have no Safari Balls left!\n";
                    continue;
                }
                playerInv.balls--;
                turnCompleted = true;

                double finalCatch = calculateCaptureChance(*pokemon, captureMod);
                cout << "\nYou threw a Safari Ball...\n";
                timeDelay(1000);

                if (rollPercentage(finalCatch))
                {
                    cout << "\nGOTCHA! " << pokemon->name << " was caught!\n";
                    stats.caught++;
                    addCaughtPokemon(pokemon, modifiers, isShiny);

                    active = false;
                    break;
                }
                else
                {
                    cout << "\nOh no! The Pokémon broke free!\n";
                }
            }
            else if (action == 2)
            { // ROCK
                if (playerInv.rocks <= 0)
                {
                    cout << "\nYou have no Rocks left!\n";
                    continue;
                }
                playerInv.rocks--;
                captureMod = 20.0;
                fleeMod = 15.0;
                turnCompleted = true;
                cout << "\nYou threw a Rock! " << pokemon->name << " looks angry!\n";
            }
            else if (action == 3)
            { // BAIT
                if (playerInv.bait <= 0)
                {
                    cout << "\nYou have no Bait left!\n";
                    continue;
                }
                playerInv.bait--;
                captureMod = -10.0;
                fleeMod = -15.0;
                turnCompleted = true;
                cout << "\nYou threw Bait! " << pokemon->name << " is eating...\n";
            }
            else if (action == 4)
            { // RUN
                cout << "\nYou ran away from the encounter.\n";
                active = false;
                break;
            }
            if (turnCompleted)
            {
                double finalFlee = calculateFleeChance(*pokemon, fleeMod);
                timeDelay(1000);
                if (rollPercentage(finalFlee))
                {
                    cout << "\nThe wild " << pokemon->name << " fled!\n";
                    stats.pokemonFled++;
                    active = false;
                }
                else
                {
                    cout << "\nThe wild " << pokemon->name << " is watching carefully...\n";
                }
            }

            // Check if out of balls mid-encounter
            if (playerInv.balls <= 0 && active)
            {
                cout << "\n[!] You ran out of Safari Balls! The encounter ends.\n";
                active = false;
            }
        }

        captureMod = 0.0;
        fleeMod = 0.0;
    }

    void performExploration(bool isLastChance)
    {
        // The 40% item-table branch yields 26% nothing and 14% an item overall.
        uniform_int_distribution<int> distribution(0, 99);
        const int outcomeRoll = distribution(getRandomGenerator());

        if (outcomeRoll < 60)
        {
            const SpeciesData *wild = nullptr;
            if (!generateEncounter(wild))
            {
                expeditionOver = true;
                return;
            }
            const EncounterModifiers modifiers = generateEncounterModifiers(*wild);
            const bool isShiny = rollShinyChance();

            if (isLastChance)
            {
                stats.encountered++;
                showWildPokemon(wild, modifiers, isShiny);
                cout << "\nYou have no Safari Balls to catch it.\n";
                cout << "The expedition ends because you cannot catch it without Safari Balls.\n";
            }
            else
            {
                handleEncounter(wild, modifiers, isShiny);
            }
        }
        else
        {
            ItemDiscovery result = discoverItem();
            if (isLastChance)
            {
                if (result == ItemDiscovery::SafariBall && playerInv.balls > 0)
                {
                    cout << "\nYou successfully added a Safari Ball. The expedition continues.\n";
                }
                else if (result == ItemDiscovery::Rock)
                {
                    cout << "\nYou found a Rock, but no Safari Balls. The expedition ends.\n";
                }
                else if (result == ItemDiscovery::Bait)
                {
                    cout << "\nYou found Bait, but no Safari Balls. The expedition ends.\n";
                }
                else
                {
                    cout << "\nYou found nothing. The expedition ends because no Safari Balls remain.\n";
                }
            }
        }
    }

    void endExpeditionIfOutOfBalls()
    {
        if (playerInv.balls > 0 || expeditionOver)
            return;

        if (lastChanceUsed)
        {
            cout << "\n[!] No Safari Balls remain. The expedition ends.\n";
            expeditionOver = true;
            endingBonus = 0;
            return;
        }

        lastChanceUsed = true;
        cout << "\n[!] Last Chance: one final exploration attempt!\n";
        cout << "\nYou continue searching the area...\n";
        timeDelay(1000);
        performExploration(true);

        if (expeditionOver)
            return;

        if (playerInv.balls > 0)
        {
            return;
        }
        else
        {
            expeditionOver = true;
        }
    }

    void explore()
    {
        if (expeditionOver)
            return;

        if (playerInv.balls <= 0)
        {
            endExpeditionIfOutOfBalls();
            return;
        }

        performExploration(false);

        // Reaching zero triggers the expedition's one final exploration attempt.
        if (playerInv.balls <= 0)
        {
            endExpeditionIfOutOfBalls();
        }
    }

    int calculateScore()
    {
        return (stats.caught * 100) +
               (stats.encountered * 10) +
               (stats.itemsCollected * 5) +
               endingBonus;
    }

    double calculateCatchEfficiency()
    {
        if (stats.encountered == 0)
            return 0.0;

        return (static_cast<double>(stats.caught) / stats.encountered) * 100.0;
    }

    string calculateExpeditionGrade()
    {
        const double catchEfficiency = calculateCatchEfficiency();
        // Cap score progress so repeated encounters cannot be farmed for a higher grade.
        const double scoreProgress = min(100.0, calculateScore() / 1200.0 * 100.0);
        const double gradePoints = catchEfficiency * 0.7 + scoreProgress * 0.3;

        if (gradePoints >= 90.0)
            return "S";
        if (gradePoints >= 87.0)
            return "A+";
        if (gradePoints >= 83.0)
            return "A";
        if (gradePoints >= 80.0)
            return "A-";
        if (gradePoints >= 77.0)
            return "B+";
        if (gradePoints >= 73.0)
            return "B";
        if (gradePoints >= 70.0)
            return "B-";
        if (gradePoints >= 67.0)
            return "C+";
        if (gradePoints >= 63.0)
            return "C";
        if (gradePoints >= 60.0)
            return "C-";
        if (gradePoints >= 57.0)
            return "D+";
        if (gradePoints >= 53.0)
            return "D";
        if (gradePoints >= 50.0)
            return "D-";
        return "F";
    }

    void showExpeditionSummary()
    {
        clearScreen();
        cout << "\n=======================================================\n";
        cout << "               EXPEDITION SUMMARY                      \n";
        cout << "=======================================================\n";
        cout << "  Total Encounters    : " << stats.encountered << "\n";
        cout << "  Pokémon Caught      : " << stats.caught << "\n";
        cout << "  Pokémon Fled        : " << stats.pokemonFled << "\n";
        cout << "  Items Collected     : " << stats.itemsCollected << "\n";
        cout << "  Remaining Supplies  : " << playerInv.balls << " Balls, "
             << playerInv.rocks << " Rocks, " << playerInv.bait << " Bait\n";

        cout << "  Catch Efficiency    : " << fixed << setprecision(2) << calculateCatchEfficiency() << "%\n";
        cout << "  Expedition Grade    : " << calculateExpeditionGrade() << "\n";
        cout << "=======================================================\n\n";
        waitForContinue();
    }

    void runExpedition()
    {
        while (!expeditionOver)
        {
            clearScreen();
            cout << "\n=== SAFARI EXPEDITION MENU ===\n";
            cout << "1. Explore\n";
            cout << "2. View Inventory\n";
            cout << "3. End Expedition\n";

            int choice;
            if (!readMenuChoice("Choice: ", 1, 3, choice))
            {
                cout << "\n[INPUT] Input closed. Ending the expedition.\n";
                expeditionOver = true;
                break;
            }

            if (choice == 1)
            {
                explore();
                if (!expeditionOver)
                    waitForContinue();
            }
            else if (choice == 2)
            {
                showInventory();
            }
            else if (choice == 3)
            {
                if (lastChanceUsed)
                    endingBonus = 50;
                else
                    endingBonus = 100;

                cout << "You decided to wrap up the expedition.\n";
                expeditionOver = true;
            }
        }

        showExpeditionSummary();
    }

} // namespace

void startExpedition(const vector<SpeciesData *> &speciesDatabase,
                     DoublyLinkedList &party, PCSystem &pc,
                     set<int> &registered, set<int> &encountered)
{
    if (speciesDatabase.empty())
    {
        cout << "[ERROR] The Pokémon database is empty; cannot start an expedition.\n";
        return;
    }

    speciesDB = &speciesDatabase;
    partyList = &party;
    pcSystem = &pc;
    registeredSpecies = &registered;
    encounteredSpecies = &encountered;
    playerInv.balls = 10;
    playerInv.rocks = 5;
    playerInv.bait = 5;
    stats = ExpeditionStats();
    expeditionOver = false;
    lastChanceUsed = false;
    endingBonus = 0;
    usedEncounterModifiers.clear();
    runExpedition();
}