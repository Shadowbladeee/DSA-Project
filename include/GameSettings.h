#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

struct GameSettings
{
    bool spritesEnabled = true;
    bool soundEnabled = true;
    bool typeTagsEnabled = true;
    bool developerMode = false;
    bool confirmRelease = true;
    bool confirmOverwrite = true;
    int lastPokedexEntry = 1;
};

GameSettings &getGameSettings();

#endif
