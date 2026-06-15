#include "config.h"
#include <stdio.h>
#include <string.h>

void config_init(GameConfig *config) {
    config->musicVolume = 0.7f;
    config->sfxVolume = 0.8f;
    config->deadzone = 0.15f;
    config->fullscreen = false;
    config->vsync = true;
    config->screenWidth = SCREEN_WIDTH;
    config->screenHeight = SCREEN_HEIGHT;
}

bool config_load(GameConfig *config, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;

    char buf[1024];
    size_t len = fread(buf, 1, sizeof(buf) - 1, f);
    buf[len] = '\0';
    fclose(f);

    char *ptr;

    ptr = strstr(buf, "\"musicVolume\"");
    if (ptr) sscanf(ptr, "\"musicVolume\" : %f", &config->musicVolume);

    ptr = strstr(buf, "\"sfxVolume\"");
    if (ptr) sscanf(ptr, "\"sfxVolume\" : %f", &config->sfxVolume);

    ptr = strstr(buf, "\"deadzone\"");
    if (ptr) sscanf(ptr, "\"deadzone\" : %f", &config->deadzone);

    ptr = strstr(buf, "\"fullscreen\"");
    if (ptr) config->fullscreen = (strstr(ptr, "true") != NULL);

    ptr = strstr(buf, "\"vsync\"");
    if (ptr) config->vsync = (strstr(ptr, "true") != NULL);

    ptr = strstr(buf, "\"screenWidth\"");
    if (ptr) sscanf(ptr, "\"screenWidth\" : %d", &config->screenWidth);

    ptr = strstr(buf, "\"screenHeight\"");
    if (ptr) sscanf(ptr, "\"screenHeight\" : %d", &config->screenHeight);

    return true;
}

bool config_save(const GameConfig *config, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "{\n");
    fprintf(f, "  \"musicVolume\" : %.2f,\n", config->musicVolume);
    fprintf(f, "  \"sfxVolume\" : %.2f,\n", config->sfxVolume);
    fprintf(f, "  \"deadzone\" : %.2f,\n", config->deadzone);
    fprintf(f, "  \"fullscreen\" : %s,\n", config->fullscreen ? "true" : "false");
    fprintf(f, "  \"vsync\" : %s,\n", config->vsync ? "true" : "false");
    fprintf(f, "  \"screenWidth\" : %d,\n", config->screenWidth);
    fprintf(f, "  \"screenHeight\" : %d\n", config->screenHeight);
    fprintf(f, "}\n");

    fclose(f);
    return true;
}
