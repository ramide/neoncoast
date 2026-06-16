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
    render->skyColor = render_get_sky_color(render->timeOfDay);
    render->ambientColor = render_get_ambient_color(render->timeOfDay);
    render->shakeTimer = 0;
    render->shakeIntensity = 0;

    for (int i = 0; i < MAX_CLOUDS; i++) {
        render->clouds[i].x = (float)(rand() % 4000) - 2000;
        render->clouds[i].y = 200.0f + (float)(rand() % 400);
        render->clouds[i].z = (float)(rand() % (TOTAL_SEGMENTS * 2)) - (float)TOTAL_SEGMENTS;
        render->clouds[i].speed = 10.0f + (float)(rand() % 30);
        render->clouds[i].scale = 0.5f + (float)(rand() % 10) * 0.1f;
    }
}

static Color color_lerp(Color a, Color b, float t) {
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

Color render_get_sky_color(float timeOfDay) {
    struct { float t; Color c; } kf[] = {
        { 0.00f, (Color){ 25, 25, 60, 255 } },
        { 0.18f, (Color){ 25, 25, 60, 255 } },
        { 0.25f, (Color){ 60, 40, 50, 255 } },
        { 0.30f, (Color){ 255, 120, 60, 255 } },
        { 0.35f, (Color){ 135, 206, 235, 255 } },
        { 0.65f, (Color){ 135, 206, 235, 255 } },
        { 0.70f, (Color){ 255, 100, 50, 255 } },
        { 0.78f, (Color){ 60, 40, 50, 255 } },
        { 0.82f, (Color){ 25, 25, 60, 255 } },
        { 1.00f, (Color){ 25, 25, 60, 255 } },
    };
    int n = sizeof(kf) / sizeof(kf[0]);
    int i;
    for (i = 0; i < n - 1; i++) {
        if (timeOfDay >= kf[i].t && timeOfDay <= kf[i+1].t) break;
    }
    if (i >= n - 1) i = n - 2;
    float t = (timeOfDay - kf[i].t) / (kf[i+1].t - kf[i].t);
    return color_lerp(kf[i].c, kf[i+1].c, t);
}

Color render_get_ambient_color(float timeOfDay) {
    struct { float t; Color c; } kf[] = {
        { 0.00f, (Color){ 40, 40, 80, 255 } },
        { 0.18f, (Color){ 40, 40, 80, 255 } },
        { 0.25f, (Color){ 80, 60, 50, 255 } },
        { 0.30f, (Color){ 100, 80, 60, 255 } },
        { 0.35f, (Color){ 200, 200, 220, 255 } },
        { 0.65f, (Color){ 200, 200, 220, 255 } },
        { 0.70f, (Color){ 100, 80, 60, 255 } },
        { 0.78f, (Color){ 60, 50, 40, 255 } },
        { 0.82f, (Color){ 40, 40, 80, 255 } },
        { 1.00f, (Color){ 40, 40, 80, 255 } },
    };
    int n = sizeof(kf) / sizeof(kf[0]);
    int i;
    for (i = 0; i < n - 1; i++) {
        if (timeOfDay >= kf[i].t && timeOfDay <= kf[i+1].t) break;
    }
    if (i >= n - 1) i = n - 2;
    float t = (timeOfDay - kf[i].t) / (kf[i+1].t - kf[i].t);
    return color_lerp(kf[i].c, kf[i+1].c, t);
}

void render_update(Render *render, float dt, const Stage *stage) {
    render->timeOfDay += dt * 0.0003f;
    if (render->timeOfDay > 1.0f) render->timeOfDay -= 1.0f;
    render->skyColor = render_get_sky_color(render->timeOfDay);
    render->ambientColor = render_get_ambient_color(render->timeOfDay);

    float sunAngle = render->timeOfDay * PI * 2.0f;
    render->sunPosition = (Vector3){
        cosf(sunAngle) * 5000, sinf(sunAngle) * 5000, 0
    };
    render->moonPosition = (Vector3){
        cosf(sunAngle + PI) * 5000, sinf(sunAngle + PI) * 5000, 0
    };
    if (render->shakeTimer > 0) {
        render->shakeTimer -= dt;
        if (render->shakeTimer < 0) {
            render->shakeTimer = 0;
            render->shakeIntensity = 0;
        }
    }
}

void render_shake(Render *render, float intensity, float duration) {
    render->shakeIntensity = intensity;
    render->shakeTimer = duration;
}

void render_draw_sky(const Render *render) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, render->skyColor);
}

void render_draw_background(const Render *render, float playerZ, StageType stage) {
    Color mountColor;
    Color hillColor;
    switch (stage) {
        case STAGE_COASTAL:
        case STAGE_HAWAII:
            mountColor = (Color){ 60, 100, 130, 255 };
            hillColor = (Color){ 40, 120, 40, 255 };
            break;
        case STAGE_TOKYO:
        case STAGE_NYC:
            mountColor = (Color){ 20, 20, 40, 255 };
            hillColor = (Color){ 30, 30, 50, 255 };
            break;
        case STAGE_INDIA:
            mountColor = (Color){ 160, 100, 60, 255 };
            hillColor = (Color){ 100, 140, 50, 255 };
            break;
        case STAGE_MEDITERRANEAN:
            mountColor = (Color){ 140, 120, 100, 255 };
            hillColor = (Color){ 60, 130, 60, 255 };
            break;
        case STAGE_SAHARA:
            mountColor = (Color){ 180, 150, 80, 255 };
            hillColor = (Color){ 160, 140, 60, 255 };
            break;
        case STAGE_GUATEMALA:
            mountColor = (Color){ 40, 80, 100, 255 };
            hillColor = (Color){ 30, 110, 40, 255 };
            break;
        default:
            mountColor = (Color){ 60, 100, 130, 255 };
            hillColor = (Color){ 40, 120, 40, 255 };
            break;
    }

    if (render->timeOfDay < 0.2f || render->timeOfDay > 0.8f) {
        mountColor = ColorBrightness(mountColor, -0.5f);
        hillColor = ColorBrightness(hillColor, -0.5f);
    } else if (render->timeOfDay < 0.3f || render->timeOfDay > 0.7f) {
        mountColor = ColorBrightness(mountColor, -0.2f);
        hillColor = ColorBrightness(hillColor, -0.2f);
    }

    float parallaxFar = 0.05f;
    float parallaxMid = 0.15f;
    float offsetFar = fmodf(playerZ * parallaxFar, 1500.0f);
    float offsetMid = fmodf(playerZ * parallaxMid, 1000.0f);

    for (int i = -2; i < 15; i++) {
        float x = i * 140.0f - offsetFar;
        float h = 120.0f + sinf(i * 1.7f + playerZ * 0.0003f) * 60.0f
                  + sinf(i * 3.1f) * 30.0f;

        Vector2 p1 = { x - 100, HORIZON_Y };
        Vector2 p2 = { x + 100, HORIZON_Y };
        Vector2 p3 = { x, HORIZON_Y - h };

        DrawTriangle(p1, p2, p3, mountColor);

        if (h > 150.0f && stage != STAGE_SAHARA) {
            float capH = h * 0.15f;
            Vector2 cp1 = { x - 40.0f, HORIZON_Y - h + capH };
            Vector2 cp2 = { x + 40.0f, HORIZON_Y - h + capH };
            Vector2 cp3 = { x, HORIZON_Y - h };
            DrawTriangle(cp1, cp2, cp3, (Color){ 220, 230, 240, 180 });
        }
    }

    for (int i = -2; i < 20; i++) {
        float x = i * 90.0f - offsetMid;
        float h = 60.0f + sinf(i * 2.3f + playerZ * 0.0008f) * 30.0f
                  + sinf(i * 4.7f) * 15.0f;

        Vector2 p1 = { x - 70, HORIZON_Y };
        Vector2 p2 = { x + 70, HORIZON_Y };
        Vector2 p3 = { x, HORIZON_Y - h };

        DrawTriangle(p1, p2, p3, hillColor);
    }

    if (stage == STAGE_COASTAL || stage == STAGE_HAWAII || stage == STAGE_MEDITERRANEAN) {
        Color oceanColor = (render->timeOfDay < 0.3f || render->timeOfDay > 0.7f)
            ? (Color){ 10, 40, 80, 200 }
            : (Color){ 30, 120, 180, 200 };
        DrawRectangle(0, (int)HORIZON_Y + 5, SCREEN_WIDTH, 30, oceanColor);
    }
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

    float shakeX = 0, shakeY = 0;
    if (render->shakeTimer > 0) {
        shakeX = ((float)GetRandomValue(-100, 100) / 100.0f) * render->shakeIntensity * 5.0f;
        shakeY = ((float)GetRandomValue(-100, 100) / 100.0f) * render->shakeIntensity * 3.0f;
    }

    float dx[DRAW_DISTANCE + 2];
    float cumHill[DRAW_DISTANCE + 2];
    float totalDx = 0;
    float totalHill = 0;

    for (int n = 0; n < DRAW_DISTANCE + 2; n++) {
        int segIndex = (playerSeg + n + 1) % TOTAL_SEGMENTS;
        const Segment *seg = road_get_segment(stage, segIndex);
        totalDx += seg->curve;
        totalHill += seg->hill;
        dx[n] = totalDx;
        cumHill[n] = totalHill;
    }

    // Ground fill below horizon (covers gaps between sub-strips)
    {
        Color groundColor;
        if (render->timeOfDay < 0.2f || render->timeOfDay > 0.8f) {
            groundColor = (Color){ 20, 100, 30, 255 };
        } else if (render->timeOfDay < 0.3f || render->timeOfDay > 0.7f) {
            groundColor = (Color){ 25, 120, 40, 255 };
        } else {
            groundColor = (Color){ 30, 140, 50, 255 };
        }
        DrawRectangle(0, (int)HORIZON_Y, SCREEN_WIDTH, SCREEN_HEIGHT - (int)HORIZON_Y, groundColor);
    }

    #define SUBDIV 8

    for (int n = DRAW_DISTANCE - 1; n >= 0; n--) {
        float segHill = cumHill[n];
        float farSegHill = cumHill[n + 1];

        for (int s = 0; s < SUBDIV; s++) {
            float t0 = (float)s / SUBDIV;
            float t1 = (float)(s + 1) / SUBDIV;

            float z0 = (n + t0) * SEGMENT_LENGTH - playerOffset;
            float z1 = (n + t1) * SEGMENT_LENGTH - playerOffset;

            // Fix: clamp z0 to minimum 1 instead of skipping (prevents gap at screen bottom)
            if (z1 <= 0) continue;
            if (z0 <= 0) z0 = 1.0f;

            float scale0 = FOCAL_LENGTH / z0;
            float scale1 = FOCAL_LENGTH / z1;

            float hillOffset0 = (segHill + (farSegHill - segHill) * t0) * scale0 * 500.0f;
            float hillOffset1 = (segHill + (farSegHill - segHill) * t1) * scale1 * 500.0f;

            float bottomY = HORIZON_Y + CAMERA_HEIGHT * scale0 - hillOffset0 + shakeY;
            float topY = HORIZON_Y + CAMERA_HEIGHT * scale1 - hillOffset1 + shakeY;
            if (bottomY >= SCREEN_HEIGHT) continue;

            float cumCurve0 = dx[n] + (dx[n+1] - dx[n]) * t0;
            float cumCurve1 = dx[n] + (dx[n+1] - dx[n]) * t1;

            float x0 = SCREEN_WIDTH * 0.5f + cumCurve0 * scale0 * SEGMENT_LENGTH + shakeX;
            float x1 = SCREEN_WIDTH * 0.5f + cumCurve1 * scale1 * SEGMENT_LENGTH + shakeX;

            float w0 = ROAD_WIDTH * 0.5f * scale0;
            float w1 = ROAD_WIDTH * 0.5f * scale1;

            // Compute average position and width for a solid rectangle
            float avgX = (x0 + x1) * 0.5f;
            float avgW = (w0 + w1) * 0.5f;
            float rectH = bottomY - topY + 1.0f;
            if (rectH < 1.0f) rectH = 1.0f;

            int rx = (int)(avgX - avgW);
            int ry = (int)floorf(topY);
            int rw = (int)(avgW * 2.0f + 1.0f + 1.0f);
            int rh = (int)ceilf(bottomY) - ry + 1;

            // Grass terrain beside road (full-width strip, alternates for road-like feel)
            {
                Color grassColor = ((n + s) % 2 == 0)
                    ? (Color){ 30, 140, 50, 255 }
                    : (Color){ 25, 120, 40, 255 };
                DrawRectangle(0, ry, SCREEN_WIDTH, rh, grassColor);
            }

            // Road surface color
            Color roadColor;
            if (render->timeOfDay < 0.2f || render->timeOfDay > 0.8f) {
                roadColor = (Color){ 60, 60, 70, 255 };
            } else if (render->timeOfDay < 0.3f || render->timeOfDay > 0.7f) {
                roadColor = (Color){ 100, 100, 110, 255 };
            } else {
                roadColor = (Color){ 140, 140, 150, 255 };
            }
            DrawRectangle(rx, ry, rw, rh, roadColor);

            // Rumble strips (road edges) - thin rectangles on left and right
            int rumbleW = (int)(avgW * 0.06f + 1.0f);
            if (rumbleW > 0) {
                Color rumbleColor = ((n + s) % 2 == 0) ? (Color){ 255, 255, 255, 200 } : (Color){ 200, 20, 20, 200 };
                DrawRectangle(rx, ry, rumbleW, rh, rumbleColor);
                DrawRectangle(rx + rw - rumbleW, ry, rumbleW, rh, rumbleColor);
            }

            // Center lane markings (dashed)
            if ((n + s) % 16 < 8) {
                int laneW = (int)(avgW * 0.02f + 1.0f);
                if (laneW > 0) {
                    Color markColor = (Color){ 255, 255, 255, 150 };
                    DrawRectangle(rx + rw / 2 - laneW / 2, ry, laneW, rh, markColor);
                }
            }
        }
    }
    #undef SUBDIV
}

void render_draw_opponent(const Stage *stage, float worldX, float worldZ, Color color, float playerZ) {
    float relativeZ = worldZ - playerZ;
    if (relativeZ <= 0 || relativeZ > DRAW_DISTANCE * SEGMENT_LENGTH) return;

    float scale = FOCAL_LENGTH / relativeZ;
    float screenX = SCREEN_WIDTH * 0.5f + worldX * scale;
    float screenY = HORIZON_Y + CAMERA_HEIGHT * scale;

    float segIndex = worldZ / SEGMENT_LENGTH;
    int idx = ((int)segIndex) % TOTAL_SEGMENTS;
    const Segment *seg = road_get_segment(stage, idx);
    screenX += seg->curve * scale * SEGMENT_LENGTH;

    float carW = 600.0f * scale;
    float carH = 260.0f * scale;
    if (carW < 6.0f) carW = 6.0f;
    if (carH < 3.0f) carH = 3.0f;

    DrawRectangle((int)(screenX - carW / 2), (int)(screenY - carH / 2), (int)carW, (int)carH, color);
    DrawRectangle((int)(screenX - carW * 0.3f), (int)(screenY - carH * 0.55f - 14 * scale),
                  (int)(carW * 0.6f), (int)(16 * scale), ColorBrightness(color, -40));
    DrawRectangle((int)(screenX - carW * 0.25f), (int)(screenY - carH * 0.55f - 12 * scale),
                  (int)(carW * 0.5f), (int)(12 * scale), (Color){ 100, 180, 255, 180 });
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
    const Segment *seg = road_get_segment(stage, idx);
    screenX += seg->curve * scale * SEGMENT_LENGTH;

    float carW = 540.0f * scale;
    float carH = 240.0f * scale;
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

void render_draw_scenery(const Render *render, SceneryObject *scenery, int count, const Stage *stage, float playerZ) {
    // Depth sort: far-to-near using index array (avoids modifying race state)
    int indices[MAX_SCENERY];
    for (int i = 0; i < count; i++) indices[i] = i;

    for (int i = 1; i < count; i++) {
        int key = indices[i];
        float keyZ = scenery[key].worldZ - playerZ;
        int j = i - 1;
        while (j >= 0 && (scenery[indices[j]].worldZ - playerZ) > keyZ) {
            indices[j + 1] = indices[j];
            j--;
        }
        indices[j + 1] = key;
    }

    for (int i = 0; i < count; i++) {
        SceneryObject *s = &scenery[indices[i]];
        float relativeZ = s->worldZ - playerZ;
        if (relativeZ <= 0 || relativeZ > DRAW_DISTANCE * SEGMENT_LENGTH) continue;

        float scale = FOCAL_LENGTH / relativeZ;
        float screenX = SCREEN_WIDTH * 0.5f + s->worldX * scale;
        float screenY = HORIZON_Y + CAMERA_HEIGHT * scale;

        int segIdx = (int)(s->worldZ / SEGMENT_LENGTH) % TOTAL_SEGMENTS;
        const Segment *seg = road_get_segment(stage, segIdx);
        screenX += seg->curve * scale * SEGMENT_LENGTH;

        if (screenX < -600 || screenX > SCREEN_WIDTH + 600) continue;

        switch (s->type) {
            case SCENERY_TREE: {
                float trunkW = 14.0f * s->scale * scale;
                float trunkH = 100.0f * s->scale * scale;
                float crownR = 45.0f * s->scale * scale;
                if (trunkW < 2) continue;
                DrawRectangle((int)(screenX - trunkW / 2), (int)(screenY - trunkH),
                              (int)trunkW, (int)trunkH, (Color){ 100, 60, 30, 255 });
                DrawCircle((int)screenX, (int)(screenY - trunkH - crownR * 0.5f),
                           (int)crownR, s->color);
                break;
            }
            case SCENERY_PALM: {
                float trunkW = 10.0f * s->scale * scale;
                float trunkH = 130.0f * s->scale * scale;
                float frondR = 55.0f * s->scale * scale;
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
                float buildW = 100.0f * s->scale * scale;
                float buildH = 200.0f * s->scale * scale;
                if (buildW < 3) continue;
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
                float signW = 50.0f * s->scale * scale;
                float signH = 25.0f * s->scale * scale;
                float poleH = 70.0f * s->scale * scale;
                if (signW < 2) continue;
                DrawRectangle((int)(screenX - 2 * scale), (int)(screenY - poleH),
                              (int)(4 * scale), (int)poleH, (Color){ 150, 150, 150, 255 });
                DrawRectangle((int)(screenX - signW / 2), (int)(screenY - poleH - signH),
                              (int)signW, (int)signH, (Color){ 50, 50, 180, 255 });
                break;
            }
            case SCENERY_TEMPLE: {
                float baseW = 80.0f * s->scale * scale;
                float baseH = 100.0f * s->scale * scale;
                float roofH = 50.0f * s->scale * scale;
                if (baseW < 3) continue;
                DrawRectangle((int)(screenX - baseW / 2), (int)(screenY - baseH),
                              (int)baseW, (int)baseH, s->color);
                DrawTriangle(
                    (Vector2){ screenX - baseW * 0.6f, screenY - baseH },
                    (Vector2){ screenX + baseW * 0.6f, screenY - baseH },
                    (Vector2){ screenX, screenY - baseH - roofH },
                    ColorBrightness(s->color, 30));
                break;
            }
            case SCENERY_CACTUS: {
                float trunkW = 12.0f * s->scale * scale;
                float trunkH = 70.0f * s->scale * scale;
                if (trunkW < 1) continue;
                DrawRectangle((int)(screenX - trunkW / 2), (int)(screenY - trunkH),
                              (int)trunkW, (int)trunkH, s->color);
                DrawRectangle((int)(screenX - trunkW * 2), (int)(screenY - trunkH * 0.7f),
                              (int)(trunkW * 1.5f), (int)(trunkW * 0.6f), s->color);
                DrawRectangle((int)(screenX + trunkW * 0.5f), (int)(screenY - trunkH * 0.5f),
                              (int)(trunkW * 1.5f), (int)(trunkW * 0.6f), s->color);
                break;
            }
            case SCENERY_ROCK: {
                float rockW = 25.0f * s->scale * scale;
                float rockH = 15.0f * s->scale * scale;
                if (rockW < 1) continue;
                DrawEllipse((int)screenX, (int)(screenY - rockH / 2),
                            (int)(rockW / 2), (int)(rockH / 2), s->color);
                break;
            }
            case SCENERY_BILLBOARD: {
                float poleW = 4.0f * s->scale * scale;
                float poleH = 60.0f * s->scale * scale;
                float boardW = 50.0f * s->scale * scale;
                float boardH = 30.0f * s->scale * scale;
                if (boardW < 2) continue;
                DrawRectangle((int)(screenX - poleW / 2), (int)(screenY - poleH),
                              (int)poleW, (int)poleH, (Color){ 150, 150, 150, 255 });
                DrawRectangle((int)(screenX - boardW / 2), (int)(screenY - poleH - boardH),
                              (int)boardW, (int)boardH, s->color);
                break;
            }
            case SCENERY_DEER: {
                float bodyW = 20.0f * s->scale * scale;
                float bodyH = 12.0f * s->scale * scale;
                float legH = 15.0f * s->scale * scale;
                float legW = 3.0f * s->scale * scale;
                float neckH = 12.0f * s->scale * scale;
                if (bodyW < 1) continue;
                DrawRectangle((int)(screenX - bodyW / 2), (int)(screenY - bodyH - legH),
                              (int)bodyW, (int)bodyH, s->color);
                DrawRectangle((int)(screenX - bodyW * 0.3f), (int)(screenY - legH),
                              (int)legW, (int)legH, (Color){ 80, 50, 30, 255 });
                DrawRectangle((int)(screenX + bodyW * 0.2f), (int)(screenY - legH),
                              (int)legW, (int)legH, (Color){ 80, 50, 30, 255 });
                DrawRectangle((int)(screenX + bodyW * 0.3f), (int)(screenY - bodyH - legH - neckH),
                              (int)legW * 2, (int)neckH, s->color);
                DrawCircle((int)(screenX + bodyW * 0.4f), (int)(screenY - bodyH - legH - neckH - 4.0f * scale),
                           4.0f * scale, (Color){ 80, 50, 30, 255 });
                break;
            }
            case SCENERY_BIRD: {
                float wingSpan = 25.0f * s->scale * scale;
                if (wingSpan < 2) continue;
                float birdY = screenY - 80.0f * scale;
                DrawLine((int)screenX, (int)birdY,
                         (int)(screenX - wingSpan / 2), (int)(birdY - 8.0f * scale), s->color);
                DrawLine((int)screenX, (int)birdY,
                         (int)(screenX + wingSpan / 2), (int)(birdY - 8.0f * scale), s->color);
                break;
            }
            case SCENERY_HOUSE: {
                float houseW = 70.0f * s->scale * scale;
                float houseH = 50.0f * s->scale * scale;
                float roofOverhang = 8.0f * s->scale * scale;
                if (houseW < 3) continue;
                DrawRectangle((int)(screenX - houseW / 2), (int)(screenY - houseH),
                              (int)houseW, (int)houseH, s->color);
                DrawTriangle(
                    (Vector2){ screenX - houseW / 2 - roofOverhang, screenY - houseH },
                    (Vector2){ screenX + houseW / 2 + roofOverhang, screenY - houseH },
                    (Vector2){ screenX, screenY - houseH - 15.0f * s->scale * scale },
                    ColorBrightness(s->color, -20));
                float winW = 8.0f * s->scale * scale;
                float winH = 8.0f * s->scale * scale;
                if (winW > 2) {
                    Color winColor;
                    if (render->timeOfDay < 0.25f || render->timeOfDay > 0.75f) {
                        winColor = (Color){ 255, 220, 100, 200 };
                    } else {
                        winColor = (Color){ 100, 120, 140, 150 };
                    }
                    DrawRectangle((int)(screenX - houseW / 4 - winW / 2), (int)(screenY - houseH + 6.0f * scale),
                                  (int)winW, (int)winH, winColor);
                    DrawRectangle((int)(screenX + houseW / 4 - winW / 2), (int)(screenY - houseH + 6.0f * scale),
                                  (int)winW, (int)winH, winColor);
                }
                float doorW = 8.0f * s->scale * scale;
                float doorH = 14.0f * s->scale * scale;
                if (doorW > 2) {
                    DrawRectangle((int)(screenX - doorW / 2), (int)(screenY - doorH),
                                  (int)doorW, (int)doorH, ColorBrightness(s->color, -30));
                }
                break;
            }
            case SCENERY_LAMP: {
                float poleW = 4.0f * s->scale * scale;
                float poleH = 80.0f * s->scale * scale;
                float lightR = 12.0f * s->scale * scale;
                if (poleW < 2) continue;
                DrawRectangle((int)(screenX - poleW / 2), (int)(screenY - poleH),
                              (int)poleW, (int)poleH, (Color){ 80, 80, 80, 255 });
                Color lightColor;
                if (render->timeOfDay < 0.25f || render->timeOfDay > 0.75f) {
                    lightColor = (Color){ 255, 220, 100, 180 };
                } else {
                    lightColor = (Color){ 255, 220, 100, 40 };
                }
                DrawCircle((int)screenX, (int)(screenY - poleH - lightR), lightR, lightColor);
                DrawCircle((int)screenX, (int)(screenY - poleH - lightR), lightR * 0.4f,
                           (Color){ 255, 255, 200, 220 });
                break;
            }
        }
    }
}

void render_draw_sun(const Render *render, float playerZ) {
    float sunZ = render->sunPosition.z - playerZ;
    if (sunZ <= 0 || sunZ > DRAW_DISTANCE * SEGMENT_LENGTH) return;

    float scale = FOCAL_LENGTH / sunZ;
    float screenX = SCREEN_WIDTH * 0.5f + render->sunPosition.x * scale;
    float screenY = HORIZON_Y + (render->sunPosition.y - CAMERA_HEIGHT) * scale;

    if (screenY > HORIZON_Y + 50) return;

    float sunR = 30.0f * scale * 5.0f;
    if (sunR < 5.0f) sunR = 5.0f;

    DrawCircle((int)screenX, (int)screenY, (int)sunR, (Color){ 255, 255, 200, 255 });
    DrawCircle((int)screenX, (int)screenY, (int)(sunR * 2.0f), (Color){ 255, 200, 100, 50 });
    DrawCircle((int)screenX, (int)screenY, (int)(sunR * 3.5f), (Color){ 255, 180, 80, 15 });

    if (sunR > 10.0f) {
        for (int i = 1; i <= 3; i++) {
            float flareR = sunR * (0.3f + i * 0.2f);
            float flareAlpha = 40.0f / i;
            DrawCircle((int)screenX, (int)screenY, (int)flareR,
                       (Color){ 255, 220, 150, (unsigned char)flareAlpha });
        }
    }
}

void particle_emit(ParticleSystem *ps, float x, float y, float vx, float vy, float life, Color color) {
    if (ps->count >= MAX_PARTICLES) return;
    Particle *p = &ps->particles[ps->count++];
    p->x = x; p->y = y;
    p->vx = vx; p->vy = vy;
    p->life = life; p->maxLife = life;
    p->color = color;
}

void particle_update(ParticleSystem *ps, float dt) {
    for (int i = 0; i < ps->count; i++) {
        Particle *p = &ps->particles[i];
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->life -= dt;
        if (p->life <= 0) {
            ps->particles[i] = ps->particles[--ps->count];
            i--;
        }
    }
}

void particle_draw(const ParticleSystem *ps) {
    for (int i = 0; i < ps->count; i++) {
        const Particle *p = &ps->particles[i];
        float t = 1.0f - p->life / p->maxLife;
        float alpha = (1.0f - t) * p->color.a;
        float size = 3.0f + t * 6.0f;
        DrawCircle((int)p->x, (int)p->y, (int)size,
                   (Color){ p->color.r, p->color.g, p->color.b, (unsigned char)alpha });
    }
}

void render_draw_flyers(const Render *render, float playerZ) {
    static FlyingObject flyers[MAX_FLYERS] = {0};
    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < MAX_FLYERS; i++) {
            flyers[i].x = (float)(rand() % 6000) - 3000;
            flyers[i].z = (float)(rand() % TOTAL_SEGMENTS) * SEGMENT_LENGTH;
            flyers[i].speed = 200.0f + (float)(rand() % 300);
            flyers[i].altitude = 800.0f + (float)(rand() % 600);
            flyers[i].type = rand() % 2;
            flyers[i].color = flyers[i].type == 0
                ? (Color){ 80, 80, 80, 255 }
                : (Color){ 200, 200, 200, 255 };
        }
        initialized = true;
    }

    for (int i = 0; i < MAX_FLYERS; i++) {
        FlyingObject *f = &flyers[i];
        f->z += f->speed * 0.016f;

        float relativeZ = f->z - playerZ;
        if (relativeZ <= 0 || relativeZ > DRAW_DISTANCE * SEGMENT_LENGTH) continue;

        float scale = FOCAL_LENGTH / relativeZ;
        float screenX = SCREEN_WIDTH * 0.5f + f->x * scale;
        float screenY = HORIZON_Y + (f->altitude - CAMERA_HEIGHT) * scale;

        if (screenX < -50 || screenX > SCREEN_WIDTH + 50) continue;

        if (f->type == 0) {
            float wingW = 15.0f * scale * 3.0f;
            if (wingW < 3) wingW = 3;
            DrawLine((int)screenX, (int)screenY,
                     (int)(screenX - wingW), (int)(screenY - 4.0f * scale * 3.0f), f->color);
            DrawLine((int)screenX, (int)screenY,
                     (int)(screenX + wingW), (int)(screenY - 4.0f * scale * 3.0f), f->color);
        } else {
            float planeW = 20.0f * scale * 3.0f;
            float planeH = 5.0f * scale * 3.0f;
            if (planeW < 4) planeW = 4;
            DrawRectangle((int)(screenX - planeW / 2), (int)(screenY - planeH / 2),
                          (int)planeW, (int)planeH, f->color);
            DrawRectangle((int)(screenX - planeW * 0.4f), (int)(screenY - planeH),
                          (int)(planeW * 0.8f), (int)(planeH * 0.4f), f->color);
        }
    }
}

void render_draw_car(float laneOffset, Color color, float speed) {
    float carY = SCREEN_HEIGHT * 0.78f;
    float carX = SCREEN_WIDTH / 2.0f + laneOffset * 200.0f;

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
