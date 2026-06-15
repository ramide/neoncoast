#include "render.h"
#include <math.h>

#define HORIZON_Y      (SCREEN_HEIGHT * 0.45f)
#define FOCAL_LENGTH   300.0f
#define CAMERA_HEIGHT  1500.0f

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
    int playerSeg = (int)(playerZ / SEGMENT_LENGTH);
    float playerOffset = fmodf(playerZ, SEGMENT_LENGTH);

    float dx[DRAW_DISTANCE];
    float dy[DRAW_DISTANCE];
    float totalDx = 0, totalDy = 0;

    for (int n = 0; n < DRAW_DISTANCE; n++) {
        int segIndex = (playerSeg + n + 1) % TOTAL_SEGMENTS;
        Segment *seg = road_get_segment(stage, segIndex);
        totalDx += seg->curve;
        totalDy += seg->hill;
        dx[n] = totalDx;
        dy[n] = totalDy;
    }

    for (int n = DRAW_DISTANCE - 1; n >= 0; n--) {
        float z = (n + 1) * SEGMENT_LENGTH - playerOffset;
        if (z <= 0) continue;

        float scale = FOCAL_LENGTH / z;
        float farZ = z + SEGMENT_LENGTH;
        float farScale = FOCAL_LENGTH / farZ;

        float roadW = ROAD_WIDTH * scale;
        float bottomY = HORIZON_Y + CAMERA_HEIGHT * scale + dy[n] * scale * 0.1f;
        float topY = HORIZON_Y + CAMERA_HEIGHT * farScale + dy[n] * farScale * 0.1f;
        float segH = bottomY - topY;
        if (segH < 1.0f) segH = 1.0f;

        float screenX = SCREEN_WIDTH * 0.5f + dx[n] * scale * SEGMENT_LENGTH;

        int segIndex = (playerSeg + n + 1) % TOTAL_SEGMENTS;
        int stripe = segIndex % 2;
        Color roadColor = stripe ? (Color){ 100, 100, 100, 255 } : (Color){ 80, 80, 80, 255 };
        Color grassColor = stripe ? COLOR_GRASS : (Color){ 20, 120, 20, 255 };

        if (bottomY > HORIZON_Y) {
            float drawY = (topY > HORIZON_Y) ? topY : HORIZON_Y;
            float drawH = bottomY - drawY;
            if (drawH > 0) {
                DrawRectangle(0, (int)drawY, SCREEN_WIDTH, (int)(drawH + 1), grassColor);
                DrawRectangle((int)(screenX - roadW / 2), (int)drawY, (int)(roadW + 1), (int)(drawH + 1), roadColor);
            }
        }
    }
}
