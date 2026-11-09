#ifndef SPECIES_H
#define SPECIES_H

#include <string>

struct SpeciesData
{
    int internalID;
    int pokedexID;
    std::string name;
    std::string type1;
    std::string type2;

    int hp;
    int attack;
    int defense;
    int spAttack;
    int spDefense;
    int speed;
    int baseStatTotal;

    float heightM;
    float weightKg;

    std::string abilities;
    std::string hiddenAbility;

    int generation;
    bool isLegendary;
    bool isMythical;

    std::string color;
    std::string shape;
    std::string eggGroups;
    std::string habitat;

    int captureRate;

    std::string genus;
    std::string flavorText;

    std::string spritePath;
};

#endif