#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "road.h"
#include "race.h"

#define MAX_CLOUDS 12
#define MAX_PARTICLES 50
#define MAX_FLYERS 6

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    Color color;
} Particle;

typedef struct {
    Particle particles[MAX_PARTICLES];
    int count;
} ParticleSystem;

typedef struct {
    float x, z;
    float speed;
    float altitude;
    int type;
    Color color;
} FlyingObject;

typedef struct {
    float x, y, z;
    float speed;
    float scale;
} Cloud;

typedef struct {
    Camera3D camera;
    float timeOfDay;
    Vector3 sunPosition;
    Vector3 moonPosition;
    Color ambientColor;
    Color skyColor;
    Cloud clouds[MAX_CLOUDS];
    float shakeTimer;
    float shakeIntensity;
} Render;

void render_init(Render *render);
void render_update(Render *render, float dt, const Stage *stage);
void render_shake(Render *render, float intensity, float duration);
void render_draw_sky(const Render *render);
void render_draw_background(const Render *render, float playerZ, StageType stage);
void render_draw_clouds(const Render *render, float playerZ);
void render_draw_road(const Render *render, const Stage *stage, float playerZ);
void render_draw_car(float laneOffset, Color color, float speed);
void render_draw_opponent(const Stage *stage, float worldX, float worldZ, Color color, float playerZ);
void render_draw_traffic(const Stage *stage, float worldX, float worldZ, Color color, float playerZ, float speed);
void render_draw_scenery(const Render *render, SceneryObject *scenery, int count, const Stage *stage, float playerZ);
void render_draw_sun(const Render *render, float playerZ);
void render_draw_flyers(const Render *render, float playerZ);
void particle_emit(ParticleSystem *ps, float x, float y, float vx, float vy, float life, Color color);
void particle_update(ParticleSystem *ps, float dt);
void particle_draw(const ParticleSystem *ps);
Color render_get_sky_color(float timeOfDay);
Color render_get_ambient_color(float timeOfDay);

#endif
