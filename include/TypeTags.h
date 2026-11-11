#ifndef TYPE_TAGS_H
#define TYPE_TAGS_H

#include "GameSettings.h"
#include <cctype>
#include <string>

inline std::string colorTypeTag(const std::string &type)
{
    std::string label = type;
    for (char &character : label)
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    if (!getGameSettings().typeTagsEnabled)
        return label;

    struct TypeColor
    {
        const char *name;
        int red;
        int green;
        int blue;
    };

    static const TypeColor colors[] = {
        {"normal", 168, 167, 122},
        {"fire", 238, 129, 48},
        {"water", 99, 144, 240},
        {"electric", 247, 208, 44},
        {"grass", 122, 199, 76},
        {"ice", 150, 217, 214},
        {"fighting", 194, 46, 40},
        {"poison", 163, 62, 161},
        {"ground", 226, 191, 101},
        {"flying", 169, 143, 243},
        {"psychic", 249, 85, 135},
        {"bug", 166, 185, 26},
        {"rock", 182, 161, 54},
        {"ghost", 115, 87, 151},
        {"dragon", 111, 53, 252},
        {"dark", 112, 87, 70},
        {"steel", 183, 183, 206},
        {"fairy", 214, 133, 173}};

    std::string normalized = type;
    for (char &character : normalized)
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));

    for (const TypeColor &color : colors)
    {
        if (normalized == color.name)
        {
            const int brightness = 299 * color.red + 587 * color.green + 114 * color.blue;
            const char *textColor = brightness < 145000 ? "97" : "30";
            return "\033[" + std::string(textColor) + ";48;2;" +
                   std::to_string(color.red) + ";" + std::to_string(color.green) + ";" +
                   std::to_string(color.blue) + "m " + label + " \033[0m";
        }
    }

    return "[" + label + "]";
}

#endif
