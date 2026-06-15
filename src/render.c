#include "render.h"
#include <math.h>
#include <stdlib.h>

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

    for (int i = 0; i < MAX_CLOUDS; i++) {
        render->clouds[i].x = (float)(rand() % 4000) - 2000;
        render->clouds[i].y = 200.0f + (float)(rand() % 400);
        render->clouds[i].z = (float)(rand() % (TOTAL_SEGMENTS * 2)) - (float)TOTAL_SEGMENTS;
        render->clouds[i].speed = 10.0f + (float)(rand() % 30);
        render->clouds[i].scale = 0.5f + (float)(rand() % 10) * 0.1f;
    }
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
    render->timeOfDay += dt * 0.002f;
    if (render->timeOfDay > stage->timeEnd) {
        render->timeOfDay = stage->timeStart;
    }
    render->skyColor = render_get_sky_color(render->timeOfDay);
    render->ambientColor = render_get_ambient_color(render->timeOfDay);

    float sunAngle = render->timeOfDay * PI * 2.0f;
    render->sunPosition = (Vector3){
        cosf(sunAngle) * 5000, sinf(sunAngle) * 5000, 0
    };
    render->moonPosition = (Vector3){
        cosf(sunAngle + PI) * 5000, sinf(sunAngle + PI) * 5000, 0
    };
}

void render_draw_sky(const Render *render) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, render->skyColor);
}

void render_draw_clouds(const Render *render, float playerZ) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        const Cloud *c = &render->clouds[i];
        float relativeZ = c->z - playerZ;
        if (relativeZ <= 0 || relativeZ > DRAW_DISTANCE * SEGMENT_LENGTH) continue;

        float scale = FOCAL_LENGTH / relativeZ;
        float screenX = SCREEN_WIDTH * 0.5f + c->x * scale;
        float screenY = HORIZON_Y + (c->y - CAMERA_HEIGHT) * scale;
        float cloudW = 120.0f * c->scale * scale;
        float cloudH = 30.0f * c->scale * scale;

        if (cloudW < 8.0f) continue;

        Color cloudColor = render->skyColor;
        cloudColor = ColorBrightness(cloudColor, 40);
        cloudColor.a = 180;

        DrawEllipse((int)screenX, (int)screenY, (int)cloudW, (int)cloudH, cloudColor);
        DrawEllipse((int)(screenX - cloudW * 0.4f), (int)(screenY + cloudH * 0.2f),
                    (int)(cloudW * 0.6f), (int)(cloudH * 0.7f), cloudColor);
        DrawEllipse((int)(screenX + cloudW * 0.3f), (int)(screenY + cloudH * 0.1f),
                    (int)(cloudW * 0.5f), (int)(cloudH * 0.6f), cloudColor);
    }
}

void render_draw_road(const Render *render, const Stage *stage, float playerZ) {
    int playerSeg = (int)(playerZ / SEGMENT_LENGTH);
    float playerOffset = fmodf(playerZ, SEGMENT_LENGTH);

    float dx[DRAW_DISTANCE];
    float totalDx = 0;

    for (int n = 0; n < DRAW_DISTANCE; n++) {
        int segIndex = (playerSeg + n + 1) % TOTAL_SEGMENTS;
        Segment *seg = road_get_segment(stage, segIndex);
        totalDx += seg->curve;
        dx[n] = totalDx;
    }

    Color shoulderL = (Color){ 255, 0, 0, 255 };
    Color shoulderR = (Color){ 255, 0, 0, 255 };
    Color rumble = (Color){ 255, 255, 255, 255 };
    Color laneMark = (Color){ 255, 255, 255, 180 };

    for (int n = DRAW_DISTANCE - 1; n >= 0; n--) {
        float z = (n + 1) * SEGMENT_LENGTH - playerOffset;
        if (z <= 0) continue;

        float scale = FOCAL_LENGTH / z;
        float farZ = z + SEGMENT_LENGTH;
        float farScale = FOCAL_LENGTH / farZ;

        float roadW = ROAD_WIDTH * scale;
        float bottomY = HORIZON_Y + CAMERA_HEIGHT * scale;
        float topY = HORIZON_Y + CAMERA_HEIGHT * farScale;
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
                // Grass
                DrawRectangle(0, (int)drawY, SCREEN_WIDTH, (int)(drawH + 1), grassColor);

                // Rumble strips
                float edgeW = roadW * 0.06f;
                DrawRectangle((int)(screenX - roadW / 2), (int)drawY, (int)(edgeW + 1), (int)(drawH + 1), shoulderL);
                DrawRectangle((int)(screenX + roadW / 2 - edgeW), (int)drawY, (int)(edgeW + 1), (int)(drawH + 1), shoulderR);

                // Road surface
                float innerW = roadW * 0.88f;
                DrawRectangle((int)(screenX - innerW / 2), (int)drawY, (int)(innerW + 1), (int)(drawH + 1), roadColor);

                // Lane markings (dashed, scroll with playerZ)
                float laneW = roadW * 0.02f;
                float lanePos1 = screenX - roadW * 0.25f;
                float lanePos2 = screenX + roadW * 0.25f;

                float dashLen = SEGMENT_LENGTH * 6.0f;
                float dashPos = fmodf(playerZ, dashLen * 2);
                float segStart = (n + 1) * SEGMENT_LENGTH - playerOffset;
                float segEnd = segStart + SEGMENT_LENGTH;
                float dashStart = dashPos - playerOffset + n * SEGMENT_LENGTH;
                bool visible1 = fmodf(dashStart + SEGMENT_LENGTH * 0.5f, dashLen * 2) < dashLen;
                bool visible2 = fmodf(dashStart + SEGMENT_LENGTH * 0.5f + dashLen, dashLen * 2) < dashLen;

                if (visible1 && segH > 2.0f) {
                    DrawRectangle((int)(lanePos1 - laneW / 2), (int)(drawY + segH * 0.2f), (int)laneW, (int)(segH * 0.6f), laneMark);
                }
                if (visible2 && segH > 2.0f) {
                    DrawRectangle((int)(lanePos2 - laneW / 2), (int)(drawY + segH * 0.2f), (int)laneW, (int)(segH * 0.6f), laneMark);
                }
            }
        }
    }
}

void render_draw_opponent(const Stage *stage, float worldX, float worldZ, Color color, float playerZ) {
    float relativeZ = worldZ - playerZ;
    if (relativeZ <= 0 || relativeZ > DRAW_DISTANCE * SEGMENT_LENGTH) return;

    float scale = FOCAL_LENGTH / relativeZ;
    float screenX = SCREEN_WIDTH * 0.5f + worldX * scale;
    float screenY = HORIZON_Y + CAMERA_HEIGHT * scale;

    float segIndex = worldZ / SEGMENT_LENGTH;
    int idx = ((int)segIndex) % TOTAL_SEGMENTS;
    Segment *seg = road_get_segment(stage, idx);
    screenX += seg->curve * scale * SEGMENT_LENGTH;

    float carW = 108.0f * scale;
    float carH = 54.0f * scale;
    if (carW < 6.0f) carW = 6.0f;
    if (carH < 3.0f) carH = 3.0f;

    DrawRectangle((int)(screenX - carW / 2), (int)(screenY - carH / 2), (int)carW, (int)carH, color);
    DrawRectangle((int)(screenX - carW * 0.3f), (int)(screenY - carH * 0.55f - 10 * scale),
                  (int)(carW * 0.6f), (int)(12 * scale), ColorBrightness(color, -40));
    DrawRectangle((int)(screenX - carW * 0.25f), (int)(screenY - carH * 0.55f - 8 * scale),
                  (int)(carW * 0.5f), (int)(8 * scale), (Color){ 100, 180, 255, 180 });
    DrawCircle((int)(screenX - carW * 0.35f), (int)(screenY + carH * 0.35f), carW * 0.10f, BLACK);
    DrawCircle((int)(screenX + carW * 0.35f), (int)(screenY + carH * 0.35f), carW * 0.10f, BLACK);
    DrawCircle((int)(screenX - carW * 0.3f), (int)(screenY + carH * 0.45f), carW * 0.06f, RED);
    DrawCircle((int)(screenX + carW * 0.3f), (int)(screenY + carH * 0.45f), carW * 0.06f, RED);
}

void render_draw_traffic(const Stage *stage, float worldX, float worldZ, Color color, float playerZ, float speed) {
    (void)speed;
    float relativeZ = worldZ - playerZ;
    if (relativeZ <= 0 || relativeZ > DRAW_DISTANCE * SEGMENT_LENGTH) return;

    float scale = FOCAL_LENGTH / relativeZ;
    float screenX = SCREEN_WIDTH * 0.5f + worldX * scale;
    float screenY = HORIZON_Y + CAMERA_HEIGHT * scale;

    float segIndex = worldZ / SEGMENT_LENGTH;
    int idx = ((int)segIndex) % TOTAL_SEGMENTS;
    Segment *seg = road_get_segment(stage, idx);
    screenX += seg->curve * scale * SEGMENT_LENGTH;

    float carW = 90.0f * scale;
    float carH = 40.0f * scale;
    if (carW < 5.0f) carW = 5.0f;
    if (carH < 3.0f) carH = 3.0f;

    DrawRectangle((int)(screenX - carW / 2), (int)(screenY - carH / 2), (int)carW, (int)carH, color);
    DrawRectangle((int)(screenX - carW * 0.25f), (int)(screenY - carH * 0.55f - 8 * scale),
                  (int)(carW * 0.5f), (int)(10 * scale), ColorBrightness(color, -30));
    DrawCircle((int)(screenX - carW * 0.3f), (int)(screenY + carH * 0.35f), carW * 0.09f, BLACK);
    DrawCircle((int)(screenX + carW * 0.3f), (int)(screenY + carH * 0.35f), carW * 0.09f, BLACK);
    DrawCircle((int)(screenX - carW * 0.25f), (int)(screenY + carH * 0.42f), carW * 0.05f, RED);
    DrawCircle((int)(screenX + carW * 0.25f), (int)(screenY + carH * 0.42f), carW * 0.05f, RED);
}

void render_draw_scenery(SceneryObject *scenery, int count, const Stage *stage, float playerZ) {
    for (int i = 0; i < count; i++) {
        SceneryObject *s = &scenery[i];
        float relativeZ = s->worldZ - playerZ;
        if (relativeZ <= 0 || relativeZ > DRAW_DISTANCE * SEGMENT_LENGTH) continue;

        float scale = FOCAL_LENGTH / relativeZ;
        float screenX = SCREEN_WIDTH * 0.5f + s->worldX * scale;
        float screenY = HORIZON_Y + CAMERA_HEIGHT * scale;

        int segIdx = (int)(s->worldZ / SEGMENT_LENGTH) % TOTAL_SEGMENTS;
        Segment *seg = road_get_segment(stage, segIdx);
        screenX += seg->curve * scale * SEGMENT_LENGTH;

        if (screenX < -200 || screenX > SCREEN_WIDTH + 200) continue;

        switch (s->type) {
            case SCENERY_TREE: {
                float trunkW = 8.0f * s->scale * scale;
                float trunkH = 60.0f * s->scale * scale;
                float crownR = 25.0f * s->scale * scale;
                if (trunkW < 2) continue;
                DrawRectangle((int)(screenX - trunkW / 2), (int)(screenY - trunkH),
                              (int)trunkW, (int)trunkH, (Color){ 100, 60, 30, 255 });
                DrawCircle((int)screenX, (int)(screenY - trunkH - crownR * 0.5f),
                           (int)crownR, s->color);
                break;
            }
            case SCENERY_PALM: {
                float trunkW = 6.0f * s->scale * scale;
                float trunkH = 80.0f * s->scale * scale;
                float frondR = 35.0f * s->scale * scale;
                if (trunkW < 2) continue;
                DrawRectangle((int)(screenX - trunkW / 2), (int)(screenY - trunkH),
                              (int)trunkW, (int)trunkH, (Color){ 120, 80, 40, 255 });
                DrawCircle((int)screenX, (int)(screenY - trunkH), (int)frondR, s->color);
                DrawCircle((int)(screenX - frondR * 0.5f), (int)(screenY - trunkH + frondR * 0.3f),
                           (int)(frondR * 0.6f), s->color);
                DrawCircle((int)(screenX + frondR * 0.5f), (int)(screenY - trunkH + frondR * 0.3f),
                           (int)(frondR * 0.6f), s->color);
                break;
            }
            case SCENERY_BUILDING: {
                float buildW = 60.0f * s->scale * scale;
                float buildH = 120.0f * s->scale * scale;
                if (buildW < 4) continue;
                DrawRectangle((int)(screenX - buildW / 2), (int)(screenY - buildH),
                              (int)buildW, (int)buildH, s->color);
                Color windowColor = (Color){ 200, 220, 255, 200 };
                int winRows = 3;
                int winCols = 2;
                for (int wy = 0; wy < winRows; wy++) {
                    for (int wx = 0; wx < winCols; wx++) {
                        float winX = screenX - buildW * 0.3f + wx * buildW * 0.35f;
                        float winY = screenY - buildH * 0.8f + wy * buildH * 0.25f;
                        DrawRectangle((int)winX, (int)winY,
                                      (int)(buildW * 0.15f), (int)(buildH * 0.12f), windowColor);
                    }
                }
                break;
            }
            case SCENERY_SIGN: {
                float signW = 30.0f * s->scale * scale;
                float signH = 15.0f * s->scale * scale;
                float poleH = 40.0f * s->scale * scale;
                if (signW < 3) continue;
                DrawRectangle((int)(screenX - 2 * scale), (int)(screenY - poleH),
                              (int)(4 * scale), (int)poleH, (Color){ 150, 150, 150, 255 });
                DrawRectangle((int)(screenX - signW / 2), (int)(screenY - poleH - signH),
                              (int)signW, (int)signH, (Color){ 50, 50, 180, 255 });
                break;
            }
        }
    }
}

void render_draw_car(float steer, Color color, float speed) {
    float carY = SCREEN_HEIGHT * 0.78f;
    float carX = SCREEN_WIDTH / 2.0f + steer * 120.0f;

    float bodyW = 80.0f;
    float bodyH = 36.0f;

    // Shadow
    DrawEllipse((int)carX, (int)(carY + bodyH / 2 + 6), (int)(bodyW * 0.5f), 6, (Color){ 0, 0, 0, 80 });

    // Wheels
    float wheelR = 9.0f;
    Color tireColor = (Color){ 30, 30, 30, 255 };
    DrawCircle((int)(carX - bodyW * 0.3f), (int)(carY + bodyH * 0.45f), wheelR, tireColor);
    DrawCircle((int)(carX + bodyW * 0.3f), (int)(carY + bodyH * 0.45f), wheelR, tireColor);
    DrawCircle((int)(carX - bodyW * 0.3f), (int)(carY - bodyH * 0.45f), wheelR, tireColor);
    DrawCircle((int)(carX + bodyW * 0.3f), (int)(carY - bodyH * 0.45f), wheelR, tireColor);

    // Body
    DrawRectangle((int)(carX - bodyW / 2), (int)(carY - bodyH / 2), (int)bodyW, (int)bodyH, color);

    // Cabin (darker tint)
    Color cabinColor = ColorBrightness(color, -0.3f);
    DrawRectangle((int)(carX - bodyW * 0.3f), (int)(carY - bodyH * 0.5f - 12), (int)(bodyW * 0.6f), 14, cabinColor);
    DrawRectangle((int)(carX - bodyW * 0.25f), (int)(carY - bodyH * 0.5f - 10), (int)(bodyW * 0.5f), 10, (Color){ 100, 180, 255, 200 });

    // Headlights
    bool headlightsOn = (speed > 50.0f);
    if (headlightsOn) {
        DrawCircle((int)(carX - bodyW * 0.3f), (int)(carY - bodyH * 0.45f + 2), 4, YELLOW);
        DrawCircle((int)(carX + bodyW * 0.3f), (int)(carY - bodyH * 0.45f + 2), 4, YELLOW);
    }

    // Taillights
    DrawCircle((int)(carX - bodyW * 0.3f), (int)(carY + bodyH * 0.45f - 2), 4, RED);
    DrawCircle((int)(carX + bodyW * 0.3f), (int)(carY + bodyH * 0.45f - 2), 4, RED);
}
