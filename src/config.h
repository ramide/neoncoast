#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

#define CONFIG_PATH "racing_config.json"

typedef struct {
    float musicVolume;
    float sfxVolume;
    float deadzone;
    bool  fullscreen;
    bool  vsync;
    int   screenWidth;
    int   screenHeight;
} GameConfig;

void config_init(GameConfig *config);
bool config_load(GameConfig *config, const char *path);
bool config_save(const GameConfig *config, const char *path);

#endif
