#ifndef POKEMON_NODE_H
#define POKEMON_NODE_H

#include "Species.h"
#include <string>

struct PokemonNode
{
  int pokemonID;
  std::string nickname;
  std::string gender;
  int level;
  std::string nature;
  std::string ability;
  bool isShiny;
  std::string dateCaught;

  const SpeciesData *species;

  PokemonNode *prev;
  PokemonNode *next;

  PokemonNode() = default;

  PokemonNode(int instanceID, const SpeciesData *speciesRef,
              std::string nick, std::string gndr, int lvl,
              std::string nat, std::string abil, bool shiny, std::string date)
      : pokemonID(instanceID),
        species(speciesRef),
        nickname(nick.empty() ? speciesRef->name : nick),
        gender(gndr),
        level(lvl),
        nature(nat),
        ability(abil),
        isShiny(shiny),
        dateCaught(date),
        prev(nullptr),
        next(nullptr) {}
};

#endif