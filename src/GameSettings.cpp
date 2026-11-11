#include "../include/GameSettings.h"

GameSettings &getGameSettings()
{
    static GameSettings settings;
    return settings;
}
