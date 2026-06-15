# NeonCoast V4 — Polish & Visual Overhaul Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix core gameplay feel — speed, steering, visibility, audio, day/night, and collision feedback

**Architecture:** Incremental fixes across race.c, render.c, audio.h/c, road.h with minimal structural changes

**Tech Stack:** raylib 5.0, C11, CMake

> ⚠️ **Safety:** Before each step's build/commit, always run `git diff` to preview exact changes before staging. This catches unintended modifications before they're committed.

---

## Task 1: Speed Boost + Smoother Road Rendering

**Files:**
- Modify: `src/race.c` — MAX_SPEED 1200→4800, ACCEL_RATE 500→2000, BRAKE_RATE 400→1600
- Modify: `src/road.h` — DRAW_DISTANCE 150→200
- Modify: `src/render.c` — subdivide road segments with interpolated curves for smooth appearance, add speed lines

### Step 1.1: Increase speed constants in race.c

Find these lines near the top of race.c:
```c
#define MAX_SPEED 1200.0f
#define ACCEL_RATE 500.0f
#define BRAKE_RATE 400.0f
```
Change to:
```c
#define MAX_SPEED 4800.0f
#define ACCEL_RATE 2000.0f
#define BRAKE_RATE 1600.0f
```

### Step 1.2: Increase DRAW_DISTANCE in road.h

Find:
```c
#define DRAW_DISTANCE 150
```
Change to:
```c
#define DRAW_DISTANCE 200
```

### Step 1.3: Rewrite render_draw_road in render.c with segment subdivision

The current render_draw_road draws one trapezoid per segment from back to front. Replace the segment drawing loop with sub-segment interpolation.

Open render.c, find the `render_draw_road` function. Replace the segment drawing loop (the `for (int n = DRAW_DISTANCE - 1; n >= 0; n--)` block) with this:

The key changes:
- Inside the loop, subdivide each segment into 4 pieces
- Interpolate curve between consecutive segments for smooth transition
- Add speed lines (thin dashed lines on the road surface that rush past)

Here's the replacement code for the inner segment drawing logic:

> **Important:** Keep the cumulative curve offset array that computes `dx[]` before the draw loop. Look for code near the start of `render_draw_road`:
> ```c
> float dx[DRAW_DISTANCE + 2] = {0};
> for (int n = 0; n < DRAW_DISTANCE + 1; n++) {
>     dx[n+1] = dx[n] + (float)stage->segments[(playerSeg + n) % TOTAL_SEGMENTS].curve;
> }
> ```
> **Preserve this block exactly as-is.** The subdivision loop below interpolates between `dx[n]` and `dx[n+1]` for smooth curves.

```c
    // Sub-division factor for smooth curves
    #define SUBDIV 4
    
    for (int n = DRAW_DISTANCE - 1; n >= 0; n--) {
        // For each segment, draw SUBDIV sub-trapezoids with interpolated cumulative offsets
        for (int s = 0; s < SUBDIV; s++) {
            float t0 = (float)s / SUBDIV;
            float t1 = (float)(s + 1) / SUBDIV;
            
            // Perspective: Z depth for each sub-segment edge
            float z0 = (n + t0) * SEGMENT_LENGTH - playerOffset;
            float z1 = (n + t1) * SEGMENT_LENGTH - playerOffset;
            
            if (z0 <= 0 || z1 <= 0) continue;
            
            // Interpolate cumulative curve offset between dx[n] and dx[n+1]
            float cumCurve0 = dx[n] + (dx[n+1] - dx[n]) * t0;
            float cumCurve1 = dx[n] + (dx[n+1] - dx[n]) * t1;
            
            // Screen Y positions
            float scale0 = focalLength / z0;
            float scale1 = focalLength / z1;
            float y0 = HORIZON_Y + (int)(scale0 * 100.0f);
            float y1 = HORIZON_Y + (int)(scale1 * 100.0f);
            
            if (y0 >= SCREEN_HEIGHT || y1 >= SCREEN_HEIGHT) continue;
            
            float x0 = SCREEN_WIDTH/2 + cumCurve0 * scale0;
            float x1 = SCREEN_WIDTH/2 + cumCurve1 * scale1;
            
            float w0 = roadHalfWidth * scale0;
            float w1 = roadHalfWidth * scale1;
            
            // Road surface color based on time of day
            Color roadColor;
            if (render->timeOfDay < 0.2f || render->timeOfDay > 0.8f) {
                roadColor = (Color){ 60, 60, 70, 255 }; // dark
            } else if (render->timeOfDay < 0.3f || render->timeOfDay > 0.7f) {
                roadColor = (Color){ 100, 100, 110, 255 }; // dusk/dawn
            } else {
                roadColor = (Color){ 140, 140, 150, 255 }; // day
            }
            
            // Draw road trapezoid (two triangles)
            DrawTriangle(
                (Vector2){ x0 - w0, y0 }, (Vector2){ x0 + w0, y0 },
                (Vector2){ x1 - w1, y1 }, roadColor);
            DrawTriangle(
                (Vector2){ x0 + w0, y0 }, (Vector2){ x1 - w1, y1 },
                (Vector2){ x1 + w1, y1 }, roadColor);
            
            // Rumble strips (road edges) - alternating red/white
            Color rumbleColor = ((n + s) % 2 == 0) ? (Color){ 255, 255, 255, 200 } : (Color){ 200, 20, 20, 200 };
            
            // Left rumble strip
            DrawTriangle(
                (Vector2){ x0 - w0, y0 }, (Vector2){ x0 - w0 + w0*0.06f, y0 },
                (Vector2){ x1 - w1, y1 }, rumbleColor);
            DrawTriangle(
                (Vector2){ x0 - w0 + w0*0.06f, y0 }, (Vector2){ x1 - w1, y1 },
                (Vector2){ x1 - w1 + w1*0.06f, y1 }, rumbleColor);
            
            // Right rumble strip
            DrawTriangle(
                (Vector2){ x0 + w0 - w0*0.06f, y0 }, (Vector2){ x0 + w0, y0 },
                (Vector2){ x1 + w1 - w1*0.06f, y1 }, rumbleColor);
            DrawTriangle(
                (Vector2){ x0 + w0, y0 }, (Vector2){ x1 + w1 - w1*0.06f, y1 },
                (Vector2){ x1 + w1, y1 }, rumbleColor);
            
            // Center lane markings (dashed) - streak past at speed for motion feel
            if ((n + s) % 8 < 4) {
                float lw0 = w0 * 0.02f;
                float lw1 = w1 * 0.02f;
                Color markColor = (Color){ 255, 255, 255, 150 };
                DrawTriangle(
                    (Vector2){ x0 - lw0, y0 }, (Vector2){ x0 + lw0, y0 },
                    (Vector2){ x1 - lw1, y1 }, markColor);
                DrawTriangle(
                    (Vector2){ x0 + lw0, y0 }, (Vector2){ x1 - lw1, y1 },
                    (Vector2){ x1 + lw1, y1 }, markColor);
            }
        }
    }
```

Note: The `lerpf` helper function is no longer needed since we interpolate cumulative offsets directly with `dx[n] + (dx[n+1] - dx[n]) * t`. Remove the instruction that says "Also add the lerp helper function at the top of render.c".

### Step 1.4: Build and commit

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
git add -A
git commit -m "feat: boost speed 4x, smooth road curves with subdivision"
```

---

## Task 2: Roadside Scenery (Houses, Lamps, Signs Close to Road)

**Files:**
- Modify: `src/race.h` — add SCENERY_HOUSE, SCENERY_LAMP to SceneryType enum
- Modify: `src/race.c` — update race_generate_scenery for new types close to road
- Modify: `src/render.c` — add render_draw_scenery cases for house and lamp

### Step 2.1: Extend SceneryType enum in race.h

Replace the existing enum:
```c
typedef enum {
    SCENERY_TREE,
    SCENERY_BUILDING,
    SCENERY_PALM,
    SCENERY_SIGN,
    SCENERY_TEMPLE,
    SCENERY_CACTUS,
    SCENERY_ROCK,
    SCENERY_BILLBOARD,
    SCENERY_DEER,
    SCENERY_BIRD
} SceneryType;
```
With:
```c
typedef enum {
    SCENERY_TREE,
    SCENERY_BUILDING,
    SCENERY_PALM,
    SCENERY_SIGN,
    SCENERY_TEMPLE,
    SCENERY_CACTUS,
    SCENERY_ROCK,
    SCENERY_BILLBOARD,
    SCENERY_DEER,
    SCENERY_BIRD,
    SCENERY_HOUSE,
    SCENERY_LAMP
} SceneryType;
```

Also increase MAX_SCENERY:
```c
#define MAX_SCENERY 80   // 60 → 80
```

### Step 2.2: Update race_generate_scenery in race.c

Inside the function, after the main scenery generation loop (after generating distant scenery at 2000-4000 offset), add a second pass for **close roadside scenery**:

Before the closing brace of the for loop (after `race->sceneryCount++;` but still inside the loop), add at the end:

```c
        // Second chance: close scenery near road (houses, lamps)
        if (race->sceneryCount < MAX_SCENERY && rand() % 3 == 0) {
            SceneryObject *s2 = &race->scenery[race->sceneryCount];
            s2->worldZ = (float)(rand() % TOTAL_SEGMENTS) * SEGMENT_LENGTH;
            s2->rightSide = (rand() % 2 == 0);
            float closeOffset = 300.0f + (rand() % 500);
            s2->worldX = s2->rightSide ? closeOffset : -closeOffset;
            s2->scale = 0.4f + (rand() % 3) * 0.15f;
            
            if (rand() % 4 == 0) {
                s2->type = SCENERY_LAMP;
                s2->color = (Color){ 200, 200, 150, 255 };
            } else {
                s2->type = SCENERY_HOUSE;
                // Stage-appropriate house colors
                switch (stageType) {
                    case STAGE_TOKYO:
                    case STAGE_NYC:
                        s2->color = (Color){ 60 + rand()%100, 60 + rand()%100, 80 + rand()%80, 255 };
                        break;
                    case STAGE_SAHARA:
                        s2->color = (Color){ 160 + rand()%40, 140 + rand()%40, 100 + rand()%30, 255 };
                        break;
                    case STAGE_INDIA:
                        s2->color = (Color){ 180 + rand()%60, 100 + rand()%40, 80 + rand()%40, 255 };
                        break;
                    default:
                        s2->color = (Color){ 130 + rand()%50, 120 + rand()%40, 100 + rand()%30, 255 };
                        break;
                }
            }
            race->sceneryCount++;
        }
```

### Step 2.3: Add rendering cases for house and lamp in render.c

**Important:** The `render_draw_scenery` function needs access to `timeOfDay` for the lamp glow. Update the function declaration in `render.h` and the function signature in `render.c` to accept `const Render *render`:

> In **src/render.h**, change line 34 from:
> ```c
> void render_draw_scenery(SceneryObject *scenery, int count, const Stage *stage, float playerZ);
> ```
> to:
> ```c
> void render_draw_scenery(const Render *render, SceneryObject *scenery, int count, const Stage *stage, float playerZ);
> ```
> 
> In **src/render.c**, update the function definition to match (add `const Render *render` as first parameter).
> 
> In **src/main.c**, find all calls to `render_draw_scenery(...)` and add `&render,` as the first argument. After the render.h declaration change, calls like:
> ```c
> render_draw_scenery(race.scenery, race.sceneryCount, &race.stage, race.racers[0].pos.z);
> ```
> become:
> ```c
> render_draw_scenery(&render, race.scenery, race.sceneryCount, &race.stage, race.racers[0].pos.z);
> ```

Inside render_draw_scenery's switch statement, add these cases before the closing brace:

```c
            case SCENERY_HOUSE: {
                float houseW = 40.0f * s->scale * scale;
                float houseH = 30.0f * s->scale * scale;
                float roofOverhang = 6.0f * s->scale * scale;
                if (houseW < 4) continue;
                // Main body
                DrawRectangle((int)(screenX - houseW / 2), (int)(screenY - houseH),
                              (int)houseW, (int)houseH, s->color);
                // Roof (triangle)
                DrawTriangle(
                    (Vector2){ screenX - houseW / 2 - roofOverhang, screenY - houseH },
                    (Vector2){ screenX + houseW / 2 + roofOverhang, screenY - houseH },
                    (Vector2){ screenX, screenY - houseH - 15.0f * s->scale * scale },
                    ColorBrightness(s->color, -20));
                // Window (small rectangle)
                float winW = 8.0f * s->scale * scale;
                float winH = 8.0f * s->scale * scale;
                if (winW > 2) {
                    DrawRectangle((int)(screenX - houseW / 4 - winW / 2), (int)(screenY - houseH + 6.0f * scale),
                                  (int)winW, (int)winH, (Color){ 255, 255, 200, 200 });
                    DrawRectangle((int)(screenX + houseW / 4 - winW / 2), (int)(screenY - houseH + 6.0f * scale),
                                  (int)winW, (int)winH, (Color){ 255, 255, 200, 200 });
                }
                // Door
                float doorW = 8.0f * s->scale * scale;
                float doorH = 14.0f * s->scale * scale;
                if (doorW > 2) {
                    DrawRectangle((int)(screenX - doorW / 2), (int)(screenY - doorH),
                                  (int)doorW, (int)doorH, ColorBrightness(s->color, -30));
                }
                break;
            }
            case SCENERY_LAMP: {
                float poleW = 3.0f * s->scale * scale;
                float poleH = 50.0f * s->scale * scale;
                float lightR = 8.0f * s->scale * scale;
                if (poleW < 2) continue;
                // Pole
                DrawRectangle((int)(screenX - poleW / 2), (int)(screenY - poleH),
                              (int)poleW, (int)poleH, (Color){ 80, 80, 80, 255 });
                // Light sphere (glow)
                Color lightColor;
                if (render->timeOfDay < 0.25f || render->timeOfDay > 0.75f) {
                    lightColor = (Color){ 255, 220, 100, 180 }; // On at night
                } else {
                    lightColor = (Color){ 255, 220, 100, 40 };  // Dim during day
                }
                DrawCircle((int)screenX, (int)(screenY - poleH - lightR), lightR, lightColor);
                // Bright center
                DrawCircle((int)screenX, (int)(screenY - poleH - lightR), lightR * 0.4f, 
                           (Color){ 255, 255, 200, 220 });
                break;
            }
```

### Step 2.4: Build and commit

```bash
cd build
cmake --build .
git add -A
git commit -m "feat: roadside houses and lamps close to road"
```

---

## Task 3: Bigger Enemy Cars

**Files:**
- Modify: `src/render.c` — increase opponent carW 150→220, carH 70→110; traffic carW 130→200, carH 55→90

### Step 3.1: Resize opponent cars in render_draw_opponent (render.c)

Find the lines in render_draw_opponent:
```c
float carW = 150.0f * scale;
float carH = 70.0f * scale;
```
Change to:
```c
float carW = 220.0f * scale;
float carH = 110.0f * scale;
```

### Step 3.2: Adjust opponent cabin proportions

Find the cabin/windshield drawing code. Currently uses carW/carH multipliers. The existing code auto-scales because it uses:
```c
DrawRectangle((int)(screenX - carW * 0.25f), ...);
```
These multiply by carW and carH, so they auto-scale. Verify they look proportional.

### Step 3.3: Resize traffic cars in render_draw_traffic (render.c)

Find:
```c
float carW = 130.0f * scale;
float carH = 55.0f * scale;
```
Change to:
```c
float carW = 200.0f * scale;
float carH = 90.0f * scale;
```

### Step 3.4: Build and commit

```bash
cd build
cmake --build .
git add -A
git commit -m "feat: bigger enemy cars - opponent 220x110, traffic 200x90"
```

---

## Task 4: Audio Fix + Procedural SFX

**Files:**
- Modify: `src/audio.h` — add sfx_init, sfx_countdown, sfx_collision, sfx_finish declarations
- Modify: `src/audio.c` — add procedural sound generation and SFX functions
- Modify: `src/race.h` — add `playerCollided` bool to Race struct
- Modify: `src/race.c` — set `playerCollided` on collision
- Modify: `src/main.c` — wire countdown/finish/collision SFX calls

### Step 4.1: Update audio.h

Replace the file with:

```c
#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "road.h"

#define MAX_TRACKS 2

typedef struct {
    Music music;
    float masterVolume;
    bool playing;
    int currentTrack;
    Sound sndCountdown;
    Sound sndGo;
    Sound sndCollision;
    Sound sndFinish;
    bool sfxLoaded;
} AudioEngine;

void audio_init(AudioEngine *engine);
void audio_update(AudioEngine *engine, float dt);
void audio_play(AudioEngine *engine, StageType stage);
void audio_stop(AudioEngine *engine);
void audio_set_volume(AudioEngine *engine, float volume);
void audio_sfx_countdown(AudioEngine *engine, int tick);
void audio_sfx_collision(AudioEngine *engine);
void audio_sfx_finish(AudioEngine *engine);

#endif
```

### Step 4.2: Rewrite audio.c with procedural SFX generation

Replace audio.c content with:

```c
#include "audio.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

// Stage-to-track mapping: even stages use track 0, odd stages use track 1
static const char *trackFiles[MAX_TRACKS] = {
    "assets/music/midnight_drive.ogg",
    "assets/music/ganymede.ogg"
};

// Generate a simple sine wave sample buffer
static Wave wave_generate_sine(float freq, float duration, float volume) {
    int sampleRate = 44100;
    int samples = (int)(sampleRate * duration);
    unsigned char *data = (unsigned char *)malloc(samples * 2); // 16-bit mono
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float val = sinf(t * 2.0f * (float)PI * freq);
        short s = (short)(val * 32767.0f * volume);
        data[i * 2] = (unsigned char)(s & 0xFF);
        data[i * 2 + 1] = (unsigned char)((s >> 8) & 0xFF);
    }
    Wave wave = { 0 };
    wave.data = data;
    wave.frameCount = (unsigned int)samples;
    wave.sampleRate = (unsigned int)sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    return wave;
}

// Generate a noise burst
static Wave wave_generate_noise(float duration, float volume) {
    int sampleRate = 44100;
    int samples = (int)(sampleRate * duration);
    unsigned char *data = (unsigned char *)malloc(samples * 2);
    for (int i = 0; i < samples; i++) {
        float val = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        // Apply envelope (fade out)
        float env = 1.0f - (float)i / samples;
        short s = (short)(val * 32767.0f * volume * env);
        data[i * 2] = (unsigned char)(s & 0xFF);
        data[i * 2 + 1] = (unsigned char)((s >> 8) & 0xFF);
    }
    Wave wave = { 0 };
    wave.data = data;
    wave.frameCount = (unsigned int)samples;
    wave.sampleRate = (unsigned int)sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    return wave;
}

void audio_init(AudioEngine *engine) {
    memset(engine, 0, sizeof(AudioEngine));
    engine->masterVolume = 0.7f;
    engine->playing = false;
    engine->sfxLoaded = false;
    InitAudioDevice();
    
    // Generate procedural SFX
    Wave countWave = wave_generate_sine(600.0f, 0.15f, 0.5f);
    Wave goWave = wave_generate_sine(880.0f, 0.3f, 0.6f);
    Wave crashWave = wave_generate_noise(0.2f, 0.7f);
    Wave finishWave = wave_generate_sine(440.0f, 0.5f, 0.5f);
    
    engine->sndCountdown = LoadSoundFromWave(countWave);
    free(countWave.data);
    engine->sndGo = LoadSoundFromWave(goWave);
    free(goWave.data);
    engine->sndCollision = LoadSoundFromWave(crashWave);
    free(crashWave.data);
    engine->sndFinish = LoadSoundFromWave(finishWave);
    free(finishWave.data);
    engine->sfxLoaded = true;
}

void audio_play(AudioEngine *engine, StageType stage) {
    if (!engine->sfxLoaded) return;
    if (engine->playing) {
        StopMusicStream(engine->music);
        UnloadMusicStream(engine->music);
    }
    engine->currentTrack = (int)stage % MAX_TRACKS;
    engine->music = LoadMusicStream(trackFiles[engine->currentTrack]);
    SetMusicVolume(engine->music, engine->masterVolume);
    PlayMusicStream(engine->music);
    engine->playing = true;
}

void audio_update(AudioEngine *engine, float dt) {
    (void)dt;
    if (!engine->playing || !engine->sfxLoaded) return;
    UpdateMusicStream(engine->music);
    if (!IsMusicStreamPlaying(engine->music)) {
        PlayMusicStream(engine->music);
    }
}

void audio_stop(AudioEngine *engine) {
    if (engine->playing && engine->sfxLoaded) {
        StopMusicStream(engine->music);
        engine->playing = false;
    }
}

void audio_set_volume(AudioEngine *engine, float volume) {
    engine->masterVolume = fmaxf(0.0f, fminf(1.0f, volume));
    if (engine->playing && engine->sfxLoaded) {
        SetMusicVolume(engine->music, engine->masterVolume);
    }
}

void audio_sfx_countdown(AudioEngine *engine, int tick) {
    if (!engine->sfxLoaded) return;
    if (tick <= 0) {
        PlaySound(engine->sndGo);
    } else {
        PlaySound(engine->sndCountdown);
    }
}

void audio_sfx_collision(AudioEngine *engine) {
    if (!engine->sfxLoaded) return;
    PlaySound(engine->sndCollision);
}

void audio_sfx_finish(AudioEngine *engine) {
    if (!engine->sfxLoaded) return;
    PlaySound(engine->sndFinish);
}
```

Note: The `LoadSoundFromWave` function is available in raylib 5.0. We free the wave data after loading since raylib copies the audio data internally.

### Step 4.3: Add playerCollided field to Race struct in race.h

In the Race struct, add after existing fields (such as `int sceneryCount;`):
```c
    bool playerCollided;    // set true on player collision (for audio)
```

### Step 4.4: Set playerCollided in race.c collision detection

In race.c, inside the collision detection logic, when a collision is detected involving the player racer:
```c
    race->playerCollided = true;
```

### Step 4.5: Wire SFX calls in main.c

In main.c's update section for the COUNTDOWN state:
```c
    case COUNTDOWN:
        // After countdown timer logic
        if (race.countdown != prevCountdown) {
            audio_sfx_countdown(&audio, race.countdown);
        }
```

In main.c's update section for RACING state:
```c
    if (race.playerCollided) {
        audio_sfx_collision(&audio);
        race.playerCollided = false;
    }
    
    if (race.finished && !prevFinished) {
        audio_sfx_finish(&audio);
    }
```

### Step 4.6: Build and commit

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
git add -A
git commit -m "feat: procedural SFX - countdown, collision, finish sounds"
```

---

## Task 5: Smooth Arcade Steering

**Files:**
- Modify: `src/race.c` — improve steering feel, reduce auto-center, add boundary spring

### Step 5.1: Change steering constants in race.c

Find these lines near the top of race.c:
```c
#define STEER_SPEED 3.0f
```
Change to:
```c
#define STEER_SPEED 5.0f
```

### Step 5.2: Improve smoothedSteer response

In `update_racer` function, find the line:
```c
racer->smoothedSteer += (input.steer - racer->smoothedSteer) * fminf(1.0f, dt * 8.0f);
```
Change to:
```c
racer->smoothedSteer += (input.steer - racer->smoothedSteer) * fminf(1.0f, dt * 15.0f);
```

### Step 5.3: Reduce auto-center decay

Find:
```c
racer->steer *= powf(0.90f, dt * 60.0f);
```
Change to:
```c
racer->steer *= pow(0.97f, dt * 60.0f);
```

### Step 5.4: Increase lateral movement

Find:
```c
racer->pos.x += racer->steer * racer->speed * dt * 0.5f;
```
Change to:
```c
racer->pos.x += racer->steer * racer->speed * dt * 0.8f;
```

### Step 5.5: Replace hard boundary clamp with spring physics

Find the boundary clamp block:
```c
if (fabsf(racer->pos.x) > roadHalfWidth) {
    racer->speed *= 0.92f;
    racer->pos.x = copysignf(roadHalfWidth, racer->pos.x);
}
```

Replace with:
```c
// Soft boundary spring
float boundary = roadHalfWidth - 100.0f;
if (fabsf(racer->pos.x) > boundary) {
    float overshoot = fabsf(racer->pos.x) - boundary;
    float pushForce = overshoot * 0.5f;
    racer->pos.x -= copysignf(pushForce * dt * 15.0f, racer->pos.x);
    racer->speed *= 0.96f;
}
// Hard clamp (last resort)
if (fabsf(racer->pos.x) > roadHalfWidth) {
    racer->pos.x = copysignf(roadHalfWidth, racer->pos.x);
    racer->speed *= 0.90f;
}
```

### Step 5.6: Build and commit

```bash
cd build
cmake --build .
git add -A
git commit -m "feat: smooth arcade steering with spring boundaries"
```

---

## Task 6: Gradual Day/Night with Smooth Color Lerp

**Files:**
- Modify: `src/render.c` — slow day/night cycle to 5x slower, use smooth color keyframes

### Step 6.1: Slow the day/night cycle

In `render_update` function, find:
```c
render->timeOfDay += dt * 0.002f;
```
Change to:
```c
render->timeOfDay += dt * 0.0003f;
```
And after increment, keep the fmodf to wrap:
```c
if (render->timeOfDay > 1.0f) render->timeOfDay -= 1.0f;
```

### Step 6.2: Rewrite render_get_sky_color with color keyframes

Replace the entire function:

```c
Color render_get_sky_color(float timeOfDay) {
    // Color keyframes: time → color
    struct { float t; Color c; } keyframes[] = {
        { 0.00f, (Color){ 20, 20, 50, 255 } },     // midnight
        { 0.18f, (Color){ 20, 20, 50, 255 } },     // night
        { 0.25f, (Color){ 60, 40, 50, 255 } },     // pre-dawn
        { 0.30f, (Color){ 255, 120, 60, 255 } },   // dawn
        { 0.35f, (Color){ 135, 180, 235, 255 } },  // day
        { 0.65f, (Color){ 135, 180, 235, 255 } },  // day
        { 0.70f, (Color){ 255, 100, 50, 255 } },   // dusk
        { 0.78f, (Color){ 60, 40, 50, 255 } },     // post-dusk
        { 0.82f, (Color){ 20, 20, 50, 255 } },     // night
        { 1.00f, (Color){ 20, 20, 50, 255 } },     // midnight
    };
    int n = sizeof(keyframes) / sizeof(keyframes[0]);
    
    // Find the two keyframes surrounding current time
    int i;
    for (i = 0; i < n - 1; i++) {
        if (timeOfDay >= keyframes[i].t && timeOfDay <= keyframes[i+1].t) {
            break;
        }
    }
    if (i >= n - 1) i = n - 2;
    
    // Lerp between the two keyframes
    float t = (timeOfDay - keyframes[i].t) / (keyframes[i+1].t - keyframes[i].t);
    Color a = keyframes[i].c;
    Color b = keyframes[i+1].c;
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}
```

### Step 6.3: Rewrite render_get_ambient_color similarly

Replace the function:

```c
Color render_get_ambient_color(float timeOfDay) {
    struct { float t; Color c; } keyframes[] = {
        { 0.00f, (Color){ 20, 20, 40, 255 } },     // midnight
        { 0.18f, (Color){ 20, 20, 40, 255 } },     // night
        { 0.25f, (Color){ 40, 30, 40, 255 } },     // pre-dawn
        { 0.30f, (Color){ 80, 60, 50, 255 } },     // dawn
        { 0.35f, (Color){ 200, 200, 200, 255 } },  // day
        { 0.65f, (Color){ 200, 200, 200, 255 } },  // day
        { 0.70f, (Color){ 100, 80, 60, 255 } },    // dusk
        { 0.78f, (Color){ 40, 30, 40, 255 } },     // post-dusk
        { 0.82f, (Color){ 20, 20, 40, 255 } },     // night
        { 1.00f, (Color){ 20, 20, 40, 255 } },     // midnight
    };
    int n = sizeof(keyframes) / sizeof(keyframes[0]);
    int i;
    for (i = 0; i < n - 1; i++) {
        if (timeOfDay >= keyframes[i].t && timeOfDay <= keyframes[i+1].t) break;
    }
    if (i >= n - 1) i = n - 2;
    float t = (timeOfDay - keyframes[i].t) / (keyframes[i+1].t - keyframes[i].t);
    Color a = keyframes[i].c;
    Color b = keyframes[i+1].c;
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}
```

### Step 6.4: Build and commit

```bash
cd build
cmake --build .
git add -A
git commit -m "feat: gradual day/night with smooth color interpolation (5x slower)"
```

---

## Task 7: Collision Feedback

**Files:**
- Modify: `src/race.h` — add collisionTimer to Race struct
- Modify: `src/race.c` — set collisionTimer on impact, add scenery collision
- Modify: `src/render.h` — add shakeTimer, shakeIntensity, render_shake declaration
- Modify: `src/render.c` — add screen shake state and trigger function
- Modify: `src/main.c` — draw collision overlay, wire shake on collision

### Step 7.1: Add collision fields to Race struct in race.h

In the Race struct, after existing fields:
```c
    float collisionTimer;   // > 0 when player just collided
    // playerCollided was added by Task 4 — do NOT add it here
```

### Step 7.2: Add collision fields to Render struct in render.h

In the Render struct, after existing fields:
```c
    float shakeTimer;
    float shakeIntensity;
```

Also add function declaration:
```c
void render_shake(Render *render, float intensity, float duration);
```

### Step 7.3: Set collisionTimer in race.c collisions

In race.c, inside the collision detection logic, when a collision is detected involving the player:
```c
    race->collisionTimer = 0.5f;
    race->playerCollided = true;
```

In `race_update`, decrement collisionTimer:
```c
    if (race->collisionTimer > 0) race->collisionTimer -= dt;
```

### Step 7.4: Add scenery collision in race.c

After the road boundary check inside `update_racer`, add:
```c
// Scenery collision check (trees, rocks, buildings - obstacles)
if (!racer->finished && racer->isPlayer) {
    for (int i = 0; i < race->sceneryCount; i++) {
        SceneryObject *s = &race->scenery[i];
        if (s->type == SCENERY_TREE || s->type == SCENERY_ROCK || 
            s->type == SCENERY_CACTUS || s->type == SCENERY_BUILDING ||
            s->type == SCENERY_HOUSE || s->type == SCENERY_TEMPLE) {
            float dz = fabsf(racer->pos.z - s->worldZ);
            float dx = fabsf(racer->pos.x - s->worldX);
            float collDist = 40.0f + s->scale * 30.0f;
            if (dz < collDist * 0.5f && dx < collDist) {
                racer->speed *= 0.5f;
                // Push car away from obstacle
                if (racer->pos.x > s->worldX) {
                    racer->pos.x += 50.0f;
                } else {
                    racer->pos.x -= 50.0f;
                }
                if (racer->isPlayer) {
                    race->collisionTimer = 0.5f;
                    race->playerCollided = true;
                }
                break; // One collision per frame max
            }
        }
    }
}
```

### Step 7.5: Add screen shake to render.c

In `render_init`, initialize the shake fields:
```c
    render->shakeTimer = 0;
    render->shakeIntensity = 0;
```

In `render_update`, add shake countdown logic:
```c
    if (render->shakeTimer > 0) {
        render->shakeTimer -= dt;
        if (render->shakeTimer < 0) {
            render->shakeTimer = 0;
            render->shakeIntensity = 0;
        }
    }
```

Add the `render_shake` function:
```c
void render_shake(Render *render, float intensity, float duration) {
    render->shakeIntensity = intensity;
    render->shakeTimer = duration;
}
```

### Step 7.6: Draw collision flash overlay in main.c

In main.c's RACING state draw section, after all other draw calls:
```c
    // Collision flash overlay
    if (race.collisionTimer > 0) {
        unsigned char alpha = (unsigned char)(race.collisionTimer * 2.0f * 120);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 255, 50, 50, alpha });
    }
```

### Step 7.7: Wire collision events in main.c update

In main.c's update section for RACING state:
```c
    if (race.playerCollided) {
        audio_sfx_collision(&audio);
        render_shake(&render, 1.0f, 0.3f);
        race.playerCollided = false;
    }
```

### Step 7.8: Build and commit

```bash
cd build
cmake --build .
git add -A
git commit -m "feat: collision feedback - flash overlay, scenery collision, screen shake"
```

---

## Task 8: Windows .exe Icon Resource Embedding

**Files:**
- Create: `res/resources.rc` — Windows resource file referencing the icon
- Create: `res/neoncoast.ico` — the actual icon file (generated as part of build)
- Create: `tools/genicon.c` — standalone C program to generate the .ico file
- Modify: `CMakeLists.txt` — compile genicon, run it, link .rc on Windows

- [ ] **Step 8.1: Create tools/genicon.c**

This is a standalone C program (no raylib dependency) that generates a valid .ico file with our neon ring design. It writes the .ico binary format directly.

```c
// tools/genicon.c — generates neoncoast.ico, no external dependencies
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static unsigned char pixel[32][32][4];

static void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) return;
    pixel[y][x][0] = b;  // BGRA order for ICO
    pixel[y][x][1] = g;
    pixel[y][x][2] = r;
    pixel[y][x][3] = a;
}

int main(void) {
    // Clear to transparent
    for (int y = 0; y < 32; y++)
        for (int x = 0; x < 32; x++)
            set_pixel(x, y, 0, 0, 0, 0);
    
    // Draw neon ring design
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            float dx = (float)(x - 16);
            float dy = (float)(y - 16);
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist < 6.5f) {
                // White center
                float bright = 1.0f - dist / 6.5f;
                set_pixel(x, y, (unsigned char)(255 * bright), (unsigned char)(255 * bright), 
                          (unsigned char)(255 * bright), (unsigned char)(200 * bright));
            } else if (dist < 8.5f) {
                // Clear gap
                set_pixel(x, y, 10, 0, 25, 0);
            } else if (dist < 11.5f) {
                // Pink ring
                set_pixel(x, y, 255, 50, 150, 255);
            } else if (dist < 13.0f) {
                // Clear gap
                set_pixel(x, y, 10, 0, 25, 0);
            } else if (dist < 15.5f) {
                // Cyan ring with glow
                float fade = 1.0f - (dist - 13.0f) / 2.5f;
                set_pixel(x, y, 0, (unsigned char)(200 * fade), (unsigned char)(255 * fade), (unsigned char)(255 * fade));
            } else if (dist < 17.0f) {
                // Outer glow
                float glow = 1.0f - (dist - 15.5f) / 1.5f;
                set_pixel(x, y, 0, (unsigned char)(100 * glow), (unsigned char)(200 * glow), (unsigned char)(60 * glow));
            }
        }
    }
    
    // Write .ico file
    FILE *f = fopen("neoncoast.ico", "wb");
    if (!f) { perror("fopen"); return 1; }
    
    // ICO header: reserved(2) + type(2=1 for ico) + count(2)
    unsigned char header[] = { 0, 0, 1, 0, 1, 0 };
    fwrite(header, 1, 6, f);
    
    // Directory entry: w, h, colors, reserved, planes, bpp, size(4), offset(4)
    int pixelOffset = 6 + 16;  // header + 1 directory entry
    int pixelSize = 32 * 32 * 4;  // BGRA 32bpp
    unsigned char dir[] = {
        32, 32,           // width, height
        0,                // colors
        0,                // reserved
        1, 0,             // planes
        32, 0,            // bpp
        (unsigned char)(pixelSize & 0xFF),
        (unsigned char)((pixelSize >> 8) & 0xFF),
        (unsigned char)((pixelSize >> 16) & 0xFF),
        (unsigned char)((pixelSize >> 24) & 0xFF),
        (unsigned char)(pixelOffset & 0xFF),
        (unsigned char)((pixelOffset >> 8) & 0xFF),
        (unsigned char)((pixelOffset >> 16) & 0xFF),
        (unsigned char)((pixelOffset >> 24) & 0xFF)
    };
    fwrite(dir, 1, 16, f);
    
    // Write pixel data (BGRA, bottom-up rows)
    for (int y = 31; y >= 0; y--) {
        for (int x = 0; x < 32; x++) {
            fwrite(&pixel[y][x][0], 1, 4, f);
        }
    }
    
    fclose(f);
    printf("Generated neoncoast.ico (%d bytes)\n", pixelOffset + pixelSize);
    return 0;
}
```

Note: `sqrtf` requires `-lm` on Linux, but on Windows/MinGW it's in the default libs. The CMakeLists.txt will handle this.

- [ ] **Step 8.2: Create res/resources.rc**

```rc
// res/resources.rc
IDI_ICON1 ICON "neoncoast.ico"
```

- [ ] **Step 8.3: Update CMakeLists.txt**

Add the genicon tool build and .rc file handling. Insert the following block after the `target_include_directories(neoncoast PRIVATE src)` line:

```cmake
# Windows .exe icon resource
if(WIN32)
    # Build genicon tool (standalone, links m for sqrtf)
    add_executable(genicon EXCLUDE_FROM_ALL tools/genicon.c)
    if(UNIX)
        target_link_libraries(genicon m)
    endif()
    
    # Generate .ico at build time
    set(ICON_OUTPUT "${CMAKE_BINARY_DIR}/neoncoast.ico")
    add_custom_command(
        OUTPUT ${ICON_OUTPUT}
        COMMAND genicon
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS genicon
        COMMENT "Generating neoncoast.ico..."
    )
    
    # Copy .ico to source dir for .rc reference (or reference from binary dir)
    configure_file(${ICON_OUTPUT} ${CMAKE_SOURCE_DIR}/res/neoncoast.ico COPYONLY)
    
    # Add .rc file to sources
    set_property(SOURCE res/resources.rc PROPERTY OBJECT_DEPENDS ${ICON_OUTPUT})
    target_sources(neoncoast PRIVATE res/resources.rc)
endif()
```

Wait — there's a circular problem: we need raylib built to compile genicon? No — genicon.c is standalone (only uses stdio, stdlib, math.h). No raylib dependency! This is the key design choice.

However, there's a timing issue: genicon needs to be compiled and run before the main build. With CMake's add_custom_command, this works because we add the .ico as an OUTPUT dependency of the main target.

Actually, let me simplify the CMake approach. The simplest reliable approach:

```cmake
if(WIN32)
    # Build genicon tool
    add_executable(genicon EXCLUDE_FROM_ALL tools/genicon.c)
    target_link_libraries(genicon m)
    
    # Custom command to generate .ico
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/neoncoast.ico
        COMMAND genicon
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS genicon
        COMMENT "Generating neoncoast.ico..."
    )
    
    # Add .rc file that references the generated .ico
    # Since .rc references the file in source dir, copy it there
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/res/neoncoast.ico
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/neoncoast.ico ${CMAKE_CURRENT_SOURCE_DIR}/res/neoncoast.ico
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/neoncoast.ico
    )
    
    target_sources(neoncoast PRIVATE res/resources.rc)
endif()
```

Hmm, this is getting complex and error-prone. Let me simplify even further.

Actually, the SIMPLEST approach: just include the genicon output as a direct dependency, and use a simpler CMake approach:

```cmake
if(WIN32)
    # Generate icon at configure time using a standalone C program
    add_executable(genicon EXCLUDE_FROM_ALL tools/genicon.c)
    if(UNIX)
        target_link_libraries(genicon m)
    endif()
    
    add_custom_target(generate_icon ALL
        COMMAND genicon
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${CMAKE_CURRENT_BINARY_DIR}/neoncoast.ico 
            ${CMAKE_SOURCE_DIR}/res/neoncoast.ico
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS genicon
        COMMENT "Generating neoncoast.ico..."
    )
    
    add_dependencies(neoncoast generate_icon)
    target_sources(neoncoast PRIVATE res/resources.rc)
endif()
```

This is cleaner. add_custom_target runs the genicon tool and copies the output, and add_dependencies ensures it runs before building the main target.

- [ ] **Step 8.4: Create res/ directory**

```bash
mkdir -p res
```

- [ ] **Step 8.5: Build and verify**

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

On Windows, verify:
- The build succeeds (genicon compiles and runs, .ico is generated)
- Check `res/neoncoast.ico` exists and is 4118 bytes
- Run the game and check the .exe in Explorer shows the neon ring icon
- The window title bar shows the icon (existing SetWindowIcon from v3 should also work)

- [ ] **Step 8.6: Commit**

```bash
git add -A
git commit -m "feat: Windows .exe icon via resource embedding"
```

- [ ] **Step 8.7: Push**

```bash
git push
```

---

## Post-Implementation Verification Checklist

After all tasks are complete:

| Check | Detail |
|-------|--------|
| Speed | Car reaches 4800 max speed, accelerates at 2000 rate |
| Road smoothness | Curves look smoother, no blocky segment edges |
| Speed lines | Dashed center lines rush past at speed |
| Houses | Small houses visible close to road (200-800 units) |
| Lamps | Street lamps visible at night with glow |
| Enemy car size | Opponents are 220x110, clearly visible |
| Traffic car size | Traffic cars are 200x90 |
| Audio music | OGG plays correctly, no crackling |
| SFX countdown | Beeps play during 3-2-1 countdown |
| SFX collision | Crash noise on impact |
| SFX finish | Fanfare on race finish |
| Steering feel | Car doesn't snap to center, smooth arcade feel |
| Boundary spring | Soft push-back at road edge, not hard clamp |
| Day/night | Cycle is 5x slower (~5.5 min real time per full cycle) |
| Sky colors | Smooth gradient transitions, no discrete jumps |
| Collision flash | Red overlay flashes on impact |
| Scenery collision | Trees/rocks stop the car |
| .exe icon | .exe shows neon ring icon in Windows Explorer |
