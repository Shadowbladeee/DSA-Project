#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "CSVParser.h"
#include "DoublyLinkedList.h"
#include "PCSystem.h"
#include <set>
#include <string>
#include <vector>

bool saveFileExists(const std::string &filename);
bool saveGame(const std::string &filename, PCSystem &pc, DoublyLinkedList &party,
              const std::set<int> &registeredSpecies,
              const std::set<int> &encounteredSpecies);
bool loadGame(const std::string &filename, PCSystem &pc, DoublyLinkedList &party,
              const std::vector<SpeciesData *> &pokedex, int maxPartySize,
              std::set<int> &registeredSpecies,
              std::set<int> &encounteredSpecies);

#endif