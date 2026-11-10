#ifndef POKEDEX_MENU_H
#define POKEDEX_MENU_H

#include "Species.h"
#include <set>
#include <vector>

void handlePokedexMenu(const std::vector<SpeciesData *> &pokedex,
                       const std::set<int> &registeredSpecies,
                       const std::set<int> &encounteredSpecies);

#endif
