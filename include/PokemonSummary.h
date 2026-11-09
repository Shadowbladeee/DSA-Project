#ifndef POKEMON_SUMMARY_VIEW_H
#define POKEMON_SUMMARY_VIEW_H

#include "PokemonNode.h"
#include <string>

void renderAutoCenteredSprite(const std::string &filepath, int targetBoxWidth = 32, int targetBoxHeight = 32);
void playPokemonCry(int pokedexID);
void printPokemonSummary(const PokemonNode *pokemon, bool playCry = false, bool showSprite = false);

#endif