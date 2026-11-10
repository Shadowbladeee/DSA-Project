#ifndef MENU_UTILS_H
#define MENU_UTILS_H

#include <string>
#include <vector>

extern const std::vector<std::string> POKEMON_TYPES;
extern const std::vector<std::string> GENERATION_LABELS;

const int NAV_NEXT = -2;
const int NAV_PREV = -3;
const int NAV_GRID = -4;

std::string toLowerStr(std::string text);
int readChoice();
int readMenuChoice(const std::string &prompt, int maxOption, bool allowZero = true);
int readNonNegativeNumber(const std::string &prompt);
bool askYesNo(const std::string &prompt);
int chooseSecondType(const std::string &firstType);
std::string chooseType();
int chooseGeneration();
int promptDexNumber();
int readSlotOrNav(const std::string &prompt, int maxSlot);
std::string fillBar(int count, int capacity, int width);

#endif
