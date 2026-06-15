#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TARGET_FPS 60
#define FIXED_DT (1.0f / 60.0f)

#define MAX_RACERS 4
#define MAX_LAPS 3
#define MAX_STAGES 8
#define MAX_CARS 5
#define MAX_HIGHSCORES 10

#ifndef DEG2RAD
#define DEG2RAD (PI / 180.0f)
#endif
#ifndef RAD2DEG
#define RAD2DEG (180.0f / PI)
#endif

#define COLOR_SKY_DAWN (Color){ 255, 183, 130, 255 }
#define COLOR_SKY_DAY (Color){ 135, 206, 235, 255 }
#define COLOR_SKY_DUSK (Color){ 255, 140, 66, 255 }
#define COLOR_SKY_NIGHT (Color){ 25, 25, 60, 255 }
#define COLOR_OCEAN (Color){ 0, 105, 148, 255 }
#define COLOR_ROAD (Color){ 80, 80, 80, 255 }
#define COLOR_GRASS (Color){ 34, 139, 34, 255 }

typedef enum {
    BOOT,
    ATTRACT_MODE,
    MAIN_MENU,
    RACER_SELECT,
    CAR_SELECT,
    STAGE_SELECT,
    COUNTDOWN,
    RACING,
    FINISH_SCREEN,
    SETTINGS,
    HIGHSCORES,
    EXIT_REQUESTED
} GameState;

typedef struct {
    float x, y, z;
} Vec3;

static inline Vec3 vec3(float x, float y, float z) {
    return (Vec3){ x, y, z };
}

static inline Vector3 v3(Vec3 v) {
    return (Vector3){ v.x, v.y, v.z };
}

static inline Vec3 from_v3(Vector3 v) {
    return (Vec3){ v.x, v.y, v.z };
}

#endif
