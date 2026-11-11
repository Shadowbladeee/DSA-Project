#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

struct GameSettings
{
    bool spritesEnabled = true;
    bool soundEnabled = true;
    bool typeTagsEnabled = true;
    bool developerMode = false;
};

GameSettings &getGameSettings();

#endif
