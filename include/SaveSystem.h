#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "CSVParser.h"
#include "DoublyLinkedList.h"
#include "PCSystem.h"
#include <string>
#include <vector>

bool saveFileExists(const std::string &filename);
bool saveGame(const std::string &filename, PCSystem &pc, DoublyLinkedList &party);
bool loadGame(const std::string &filename, PCSystem &pc, DoublyLinkedList &party,
              const std::vector<SpeciesData *> &pokedex, int maxPartySize);

#endif