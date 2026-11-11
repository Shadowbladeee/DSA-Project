#ifndef DEVELOPER_TOOLS_H
#define DEVELOPER_TOOLS_H

#include "PCSystem.h"
#include <set>
#include <vector>

void handleDeveloperTools(const std::vector<SpeciesData *> &pokedex,
                          PCSystem &pc, DoublyLinkedList &party,
                          std::set<int> &registeredSpecies,
                          std::set<int> &encounteredSpecies);

#endif
