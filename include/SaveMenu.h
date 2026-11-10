#ifndef SAVE_MENU_H
#define SAVE_MENU_H

#include "PCSystem.h"
#include <set>
#include <string>
#include <vector>

extern const std::string SAVE_FILE;

void handleSaveMenu(PCSystem &pc, DoublyLinkedList &party,
                    const std::vector<SpeciesData *> &pokedex,
                    std::set<int> &registeredSpecies,
                    std::set<int> &encounteredSpecies);

#endif
