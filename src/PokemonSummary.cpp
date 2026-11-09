#include "../include/PokemonSummary.h"
#include <iostream>
#include <iomanip>

using namespace std;

void printPokemonSummary(const SpeciesData *species)
{
    if (!species)
    {
        cout << "[ERROR] Invalid species data.\n";
        return;
    }

    cout << "\n====================================================================\n";
    cout << "  #" << species->pokedexID << " " << species->name;
    if (!species->genus.empty())
    {
        cout << " (" << species->genus << ")";
    }
    cout << "\n  Type: " << species->type1;
    if (!species->type2.empty())
    {
        cout << " / " << species->type2;
    }
    cout << "\n====================================================================\n";

    // Metadata & Classification
    cout << "  [CLASSIFICATION & METADATA]\n";
    cout << "    Generation   : Gen " << species->generation << "\n";
    cout << "    Category     : ";
    if (species->isLegendary)
        cout << "Legendary Pokémon\n";
    else if (species->isMythical)
        cout << "Mythical Pokémon\n";
    else
        cout << "Standard Pokémon\n";

    cout << "    Abilities    : " << species->abilities;
    if (!species->hiddenAbility.empty())
    {
        cout << " (Hidden: " << species->hiddenAbility << ")";
    }
    cout << "\n";
    cout << "    Capture Rate : " << species->captureRate << "\n";

    cout << "--------------------------------------------------------------------\n";

    // Physical Traits & Habitat
    cout << "  [PHYSICAL CHARACTERISTICS]\n";
    cout << "    Height / Weight : " << fixed << setprecision(2)
         << species->heightM << " m / " << species->weightKg << " kg\n";
    cout << "    Color / Shape   : " << species->color << " / " << species->shape << "\n";
    cout << "    Egg Groups      : " << species->eggGroups << "\n";
    if (!species->habitat.empty())
    {
        cout << "    Habitat         : " << species->habitat << "\n";
    }

    cout << "--------------------------------------------------------------------\n";

    // Base Stats
    cout << "  [STATS] (BST: " << species->baseStatTotal << ")\n";
    cout << "    HP      : " << species->hp << "\n";
    cout << "    Attack  : " << species->attack << "\n";
    cout << "    Defense : " << species->defense << "\n";
    cout << "    Sp. Atk : " << species->spAttack << "\n";
    cout << "    Sp. Def : " << species->spDefense << "\n";
    cout << "    Speed   : " << species->speed << "\n";

    cout << "--------------------------------------------------------------------\n";

    // Pokédex Flavor Text
    cout << "  [DESCRIPTION]\n";
    cout << "   \"" << species->flavorText << "\"\n";
    cout << "====================================================================\n\n";
}