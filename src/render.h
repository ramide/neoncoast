#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "road.h"
#include "race.h"

#define MAX_CLOUDS 12

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
} Render;

void render_init(Render *render);
void render_update(Render *render, float dt, const Stage *stage);
void render_draw_sky(const Render *render);
void render_draw_clouds(const Render *render, float playerZ);
void render_draw_road(const Render *render, const Stage *stage, float playerZ);
void render_draw_car(float steer, Color color, float speed);
void render_draw_opponent(const Stage *stage, float worldX, float worldZ, Color color, float playerZ);
void render_draw_traffic(const Stage *stage, float worldX, float worldZ, Color color, float playerZ, float speed);
void render_draw_scenery(SceneryObject *scenery, int count, const Stage *stage, float playerZ);
Color render_get_sky_color(float timeOfDay);
Color render_get_ambient_color(float timeOfDay);

#endif
