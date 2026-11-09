#include "../include/PokemonSummary.h"
#include <iostream>
#include <iomanip>
#include <cctype>

using namespace std;

string toTitleCase(string text)
{
    if (text.empty())
        return "";

    for (char &c : text)
    {
        if (c == '-')
            c = ' ';
    }

    bool capitalizeNext = true;
    for (char &c : text)
    {
        const unsigned char character = static_cast<unsigned char>(c);
        if (isspace(character))
        {
            capitalizeNext = true;
        }
        else if (capitalizeNext)
        {
            c = static_cast<char>(toupper(character));
            capitalizeNext = false;
        }
        else
        {
            c = static_cast<char>(tolower(character));
        }
    }
    return text;
}

string formatAbilities(string abilitiesStr)
{
    if (abilitiesStr.empty())
        return "";

    size_t pos = 0;
    while ((pos = abilitiesStr.find('|')) != string::npos)
    {
        abilitiesStr.replace(pos, 1, ", ");
    }

    return toTitleCase(abilitiesStr);
}

void printPokemonSummary(const PokemonNode *pokemon)
{
    if (!pokemon)
    {
        cout << "[ERROR] Invalid Pokémon data.\n";
        return;
    }

    const SpeciesData *species = pokemon->species;
    if (!species)
    {
        cout << "[ERROR] Invalid species data.\n";
        return;
    }

    cout << "\n====================================================================\n";
    cout << "  #" << species->pokedexID << " " << toTitleCase(species->name);
    if (!species->genus.empty())
    {
        cout << " (" << toTitleCase(species->genus) << ")";
    }
    cout << "\n  Type: " << toTitleCase(species->type1);
    if (!species->type2.empty())
    {
        cout << " / " << toTitleCase(species->type2);
    }
    cout << "\n====================================================================\n";

    cout << "  [CAUGHT POKÉMON]\n";
    cout << "    Nickname       : " << pokemon->nickname << endl;
    cout << "    Gender         : " << toTitleCase(pokemon->gender) << endl;
    cout << "    Level          : " << pokemon->level << endl;
    cout << "    Nature         : " << toTitleCase(pokemon->nature) << endl;
    cout << "    Current Ability: " << formatAbilities(pokemon->ability) << endl;
    cout << "    Shiny          : " << (pokemon->isShiny ? "Yes" : "No") << endl;
    cout << "    Date Caught    : " << pokemon->dateCaught << endl;

    cout << "--------------------------------------------------------------------\n";

    // Metadata & Classification
    cout << "  [CLASSIFICATION & METADATA]\n";
    cout << "    Generation   : Gen " << species->generation << endl;
    cout << "    Category     : ";
    if (species->isLegendary)
        cout << "Legendary Pokémon\n";
    else if (species->isMythical)
        cout << "Mythical Pokémon\n";
    else
        cout << "Standard Pokémon\n";

    cout << "    Abilities    : " << formatAbilities(species->abilities);
    if (!species->hiddenAbility.empty())
    {
        cout << " (Hidden: " << formatAbilities(species->hiddenAbility) << ")";
    }
    cout << endl;
    cout << "    Capture Rate : " << species->captureRate << endl;

    cout << "--------------------------------------------------------------------\n";

    // Physical Traits & Habitat
    cout << "  [PHYSICAL CHARACTERISTICS]\n";
    cout << "    Height          : " << fixed << setprecision(2)
         << species->heightM << "m" << endl;
    cout << "    Weight          : " << fixed << setprecision(2)
         << species->weightKg << "kg" << endl;
    cout << "    Color           : " << toTitleCase(species->color) << endl;
    cout << "    Shape           : " << toTitleCase(species->shape) << endl;
    cout << "    Egg Groups      : " << formatAbilities(species->eggGroups) << endl;
    if (!species->habitat.empty())
    {
        cout << "    Habitat         : " << toTitleCase(species->habitat) << endl;
    }

    cout << "--------------------------------------------------------------------\n";

    // Base Stats
    cout << "  [STATS] (BST: " << species->baseStatTotal << ")\n";
    cout << "    HP      : " << species->hp << endl;
    cout << "    Attack  : " << species->attack << endl;
    cout << "    Defense : " << species->defense << endl;
    cout << "    Sp. Atk : " << species->spAttack << endl;
    cout << "    Sp. Def : " << species->spDefense << endl;
    cout << "    Speed   : " << species->speed << endl;

    cout << "--------------------------------------------------------------------\n";

    // Pokédex Flavor Text
    cout << "  [DESCRIPTION]\n";
    cout << "   \"" << species->flavorText << "\"\n";
    cout << "====================================================================\n\n";
}