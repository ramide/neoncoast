#include "render.h"
#include <math.h>

void render_init(Render *render) {
    render->camera = (Camera3D){
        .position = (Vector3){ 0, 1000, -2000 },
        .target = (Vector3){ 0, 0, 1000 },
        .up = (Vector3){ 0, 1, 0 },
        .fovy = 70.0f,
        .projection = CAMERA_PERSPECTIVE
    };
    render->timeOfDay = 0.3f;
}

Color render_get_sky_color(float timeOfDay) {
    if (timeOfDay < 0.2f) return COLOR_SKY_NIGHT;
    if (timeOfDay < 0.3f) return COLOR_SKY_DAWN;
    if (timeOfDay < 0.7f) return COLOR_SKY_DAY;
    if (timeOfDay < 0.8f) return COLOR_SKY_DUSK;
    return COLOR_SKY_NIGHT;
}

Color render_get_ambient_color(float timeOfDay) {
    if (timeOfDay < 0.2f || timeOfDay > 0.85f) return (Color){ 40, 40, 80, 255 };
    if (timeOfDay < 0.3f || timeOfDay > 0.75f) return (Color){ 120, 100, 80, 255 };
    return (Color){ 200, 200, 220, 255 };
}

void render_update(Render *render, float dt, const Stage *stage) {
    float range = stage->timeEnd - stage->timeStart;
    render->timeOfDay += dt * 0.01f;

    if (render->timeOfDay > stage->timeEnd) {
        render->timeOfDay = stage->timeStart;
    }

    render->skyColor = render_get_sky_color(render->timeOfDay);
    render->ambientColor = render_get_ambient_color(render->timeOfDay);

    float sunAngle = render->timeOfDay * PI * 2.0f;
    render->sunPosition = (Vector3){
        cosf(sunAngle) * 5000,
        sinf(sunAngle) * 5000,
        0
    };
    render->moonPosition = (Vector3){
        cosf(sunAngle + PI) * 5000,
        sinf(sunAngle + PI) * 5000,
        0
    };
}

void render_draw_sky(const Render *render) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, render->skyColor);
}

void render_draw_road(const Render *render, const Stage *stage, float playerZ) {
    float baseY = SCREEN_HEIGHT * 0.6f;

    for (int n = DRAW_DISTANCE; n > 0; n--) {
        int segIndex = ((int)playerZ / (int)SEGMENT_LENGTH + n) % TOTAL_SEGMENTS;
        Segment *seg = road_get_segment(stage, segIndex);

        float z = (float)n * SEGMENT_LENGTH - fmodf(playerZ, SEGMENT_LENGTH);
        if (z <= 0) continue;

        float scale = 1.0f / z;
        float screenX = SCREEN_WIDTH / 2.0f + seg->curve * scale * 50000.0f;
        float screenW = ROAD_WIDTH * scale;
        float screenH = 2.0f;

        int stripe = ((int)playerZ / (int)SEGMENT_LENGTH + n) % 2;
        Color roadColor = stripe ? (Color){ 100, 100, 100, 255 } : (Color){ 80, 80, 80, 255 };
        Color grassColor = stripe ? COLOR_GRASS : (Color){ 20, 120, 20, 255 };

        DrawRectangle(0, (int)(baseY - screenH), SCREEN_WIDTH, (int)(screenH + 1), grassColor);
        DrawRectangle((int)(screenX - screenW / 2), (int)(baseY - screenH),
                     (int)screenW, (int)(screenH + 1), roadColor);
    }
}
