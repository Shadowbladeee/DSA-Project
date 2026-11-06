#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include "Species.h"
#include <string>
#include <vector>

bool checkCSVFile(const std::string &filename);
std::vector<std::string> parseCSVLine(const std::string &line);
int safeStoi(const std::string &str, int column, int defaultValue = 0);
int parseGeneration(const std::string &str);

std::vector<SpeciesData *> loadSpeciesDatabase(const std::string &filename);
const SpeciesData *getSpeciesByPokedexNumber(const std::vector<SpeciesData *> &database, int dexNum);

#endif