#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "CSVParser.h"
#include "DoublyLinkedList.h"
#include "PCSystem.h"
#include <vector>

const int MAX_PARTY_SIZE = 6;

void startExpedition(const std::vector<SpeciesData *> &speciesDatabase,
                     DoublyLinkedList &party, PCSystem &pc);

#endif
