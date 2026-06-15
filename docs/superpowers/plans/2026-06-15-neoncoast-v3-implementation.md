# NeonCoast v3 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Major gameplay and visual overhaul: faster speed, wider road, OGG music, enemy cars matching player size, scenic background overhaul with parallax mountains and expanded scenery types.

**Architecture:** 6 independent tasks modifying race.c/h, render.c/h, audio.c/h, road.h, main.c. OGG music files downloaded from OpenGameArt (CC0).

**Tech Stack:** C, raylib 5.0, CMake

---

## File Structure

### Files to Modify (by task)

| Task | Files | Responsibility |
|------|-------|----------------|
| 1 | `src/road.h`, `src/race.c` | Speed/dimensions constants |
| 2 | `src/render.c` | Enemy car rendering size/matching |
| 3 | `src/audio.h`, `src/audio.c`, `src/main.c`, `CMakeLists.txt` | OGG music playback system |
| 4 | `src/race.h`, `src/race.c`, `src/render.c` | Scenery type expansion and rendering |
| 5 | `src/render.h`, `src/render.c`, `src/main.c` | Parallax mountain/hill background |
| 6 | `assets/music/*.ogg`, `README.md` | Asset download, build verify, credits |

### New Assets

| Asset | Source | License |
|-------|--------|---------|
| `assets/music/midnight_drive.ogg` | OpenGameArt (congusbongus) | CC0 |
| `assets/music/ganymede.ogg` | OpenGameArt (congusbongus) | CC0 |

---

### Task 1: Increase Speed + Wider Road

**Files:**
- Modify: `src/road.h` (line 7)
- Modify: `src/race.c` (lines 6, 7, 64, 87)

- [ ] **Step 1.1: Widen ROAD_WIDTH in road.h**

Change the road width constant from 2000 to 5000:

```c
// src/road.h line 7 — change:
#define ROAD_WIDTH 2000.0f
// to:
#define ROAD_WIDTH 5000.0f
```

This will make the road half-width (roadHalfWidth = ROAD_WIDTH * 0.4f) grow from 800 to 2000, giving 4x the lateral space.

- [ ] **Step 1.2: Double MAX_SPEED in race.c**

```c
// src/race.c line 6 — change:
#define MAX_SPEED 600.0f
// to:
#define MAX_SPEED 1200.0f
```

- [ ] **Step 1.3: Increase ACCEL_RATE in race.c**

```c
// src/race.c line 7 — change:
#define ACCEL_RATE 300.0f
// to:
#define ACCEL_RATE 500.0f
```

- [ ] **Step 1.4: Widen traffic car spawn spread**

Traffic cars currently spawn at `(rand() % 3 - 1) * 300.0f` which gives positions at -300, 0, or 300. With the wider road, spread them further:

```c
// src/race.c line 64 — change:
t->pos.x = (rand() % 3 - 1) * 300.0f;
// to:
t->pos.x = (rand() % 5 - 2) * 500.0f;
```

This gives positions at -1000, -500, 0, 500, or 1000 — spreading traffic across the wider 4000-wide road surface (half-width 2000).

- [ ] **Step 1.5: Push scenery further from wider road**

Roadside scenery needs to sit outside the wider road boundary:

```c
// src/race.c line 87 — change:
float roadEdgeOffset = 800.0f + (rand() % 1200);
// to:
float roadEdgeOffset = 2000.0f + (rand() % 2000);
```

This moves scenery from range 800-2000 to range 2000-4000, keeping it outside the 2000 half-width road.

- [ ] **Step 1.6: Verify roadHalfWidth auto-scales**

Line 175 of race.c: `float roadHalfWidth = ROAD_WIDTH * 0.4f;`
With ROAD_WIDTH=5000, this becomes 2000 (was 800). This is automatic — no code change needed. Verify the value is correct for the new road width.

- [ ] **Step 1.7: Build and test**

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Run the game and verify:
- Car accelerates faster and reaches higher top speed
- Road visual is visibly wider
- Traffic cars are spread across the road
- Scenery objects appear further from the road edges
- Car doesn't clip through road boundaries at high speed

- [ ] **Step 1.8: Commit**

```bash
git add -A
git commit -m "feat: boost speed 2x, widen road to 5000"
```

---

### Task 2: Enemy Cars Same Size As Player

**Files:**
- Modify: `src/render.c` — `render_draw_opponent` (lines 183-196), `render_draw_traffic` (lines 213-224)

**Context:** The actual codebase already has cabin/windshield rendering for both opponent and traffic cars. This task resizes them to match the player car's proportions (bodyW=80, bodyH=36 at screen coords) at equivalent scale, and adjusts existing cabin/windshield/wheel positions for the new car size.

- [ ] **Step 2.1: Resize opponent cars in render_draw_opponent**

The opponent car body is currently 108×54 world units. With scale at typical opponent distance (z≈300, scale=300/200=1.5), that's 162×81 px on screen. The player car is 80×36 px at bottom of screen. To make opponent cars match player proportions at equivalent scale:

```c
// src/render.c, render_draw_opponent function, lines 183-186:

// Change from:
float carW = 108.0f * scale;
float carH = 54.0f * scale;
if (carW < 6.0f) carW = 6.0f;
if (carH < 3.0f) carH = 3.0f;

// To:
float carW = 150.0f * scale;
float carH = 70.0f * scale;
if (carW < 6.0f) carW = 6.0f;
if (carH < 3.0f) carH = 3.0f;
```

- [ ] **Step 2.2: Adjust opponent cabin/windshield positions**

The existing cabin and windshield rectangles need to be shifted for the larger car body:

```c
// src/render.c, render_draw_opponent, lines 189-192:

// Change from (cabin):
DrawRectangle((int)(screenX - carW * 0.3f), (int)(screenY - carH * 0.55f - 10 * scale),
              (int)(carW * 0.6f), (int)(12 * scale), ColorBrightness(color, -40));
// (windshield):
DrawRectangle((int)(screenX - carW * 0.25f), (int)(screenY - carH * 0.55f - 8 * scale),
              (int)(carW * 0.5f), (int)(8 * scale), (Color){ 100, 180, 255, 180 });

// To (cabin - taller and positioned for larger car):
DrawRectangle((int)(screenX - carW * 0.3f), (int)(screenY - carH * 0.55f - 14 * scale),
              (int)(carW * 0.6f), (int)(16 * scale), ColorBrightness(color, -40));
// (windshield - taller for larger car):
DrawRectangle((int)(screenX - carW * 0.25f), (int)(screenY - carH * 0.55f - 12 * scale),
              (int)(carW * 0.5f), (int)(12 * scale), (Color){ 100, 180, 255, 180 });
```

Also verify the wheel circles (lines 193-196) still look proportional. They reference `carW` so they auto-scale:
- Rear wheel: `screenX - carW * 0.35f`, radius `carW * 0.10f`
- Front wheel: `screenX + carW * 0.35f`, radius `carW * 0.10f`
- Tail lights: radius `carW * 0.06f`
These are all proportional to carW, so they adjust automatically.

- [ ] **Step 2.3: Resize traffic cars in render_draw_traffic**

```c
// src/render.c, render_draw_traffic function, lines 213-216:

// Change from:
float carW = 90.0f * scale;
float carH = 40.0f * scale;
if (carW < 5.0f) carW = 5.0f;
if (carH < 3.0f) carH = 3.0f;

// To:
float carW = 130.0f * scale;
float carH = 55.0f * scale;
if (carW < 5.0f) carW = 5.0f;
if (carH < 3.0f) carH = 3.0f;
```

- [ ] **Step 2.4: Adjust traffic cabin/windshield positions**

The existing traffic cabin (line 219-220) and wheels (lines 221-224) reference carW/H so they auto-scale. Just verify the cabin offset looks right:

```c
// Keep as-is (they auto-scale via carW/carH):
DrawRectangle((int)(screenX - carW * 0.25f), (int)(screenY - carH * 0.55f - 8 * scale),
              (int)(carW * 0.5f), (int)(10 * scale), ColorBrightness(color, -30));
// Wheels - auto-scale via carW:
DrawCircle((int)(screenX - carW * 0.3f), (int)(screenY + carH * 0.35f), carW * 0.09f, BLACK);
DrawCircle((int)(screenX + carW * 0.3f), (int)(screenY + carH * 0.35f), carW * 0.09f, BLACK);
// Tail lights - auto-scale via carW:
DrawCircle((int)(screenX - carW * 0.25f), (int)(screenY + carH * 0.42f), carW * 0.05f, RED);
DrawCircle((int)(screenX + carW * 0.25f), (int)(screenY + carH * 0.42f), carW * 0.05f, RED);
```

These lines already use `carW * X.XXf` for all positions and sizes, so they scale correctly with the new carW=130. No changes needed here.

- [ ] **Step 2.5: Build and test**

```bash
cd build
cmake --build .
```

Run the game and verify:
- Opponent cars are larger and more visible at a distance
- Opponent cars have proportional cabin/windshield/wheels
- Traffic cars are similarly scaled up
- All visual proportions look correct (not stretched or squashed)

- [ ] **Step 2.6: Commit**

```bash
git add -A
git commit -m "feat: enemy cars match player size with cabin/windshield"
```

---

### Task 3: OGG Music System

**Files:**
- Modify: `src/audio.h` — replace AudioEngine struct and remove synth/sfx functions
- Modify: `src/audio.c` — full rewrite for OGG playback via raylib Music API
- Create: `assets/music/` directory
- Modify: `CMakeLists.txt` — copy assets to build directory
- Modify: `src/main.c` — no sfx calls to remove (they don't exist in main.c), audio API unchanged

**Background:** The current audio system is a procedural synthwave engine using raylib's AudioStream. This task replaces it with OGG music file playback using raylib's built-in `Music` API (which supports OGG natively). The old synth code and sfx stubs are removed.

- [ ] **Step 3.1: Rewrite audio.h**

Replace the entire file with the new OGG-based header:

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
} AudioEngine;

void audio_init(AudioEngine *engine);
void audio_update(AudioEngine *engine, float dt);
void audio_play(AudioEngine *engine, StageType stage);
void audio_stop(AudioEngine *engine);
void audio_set_volume(AudioEngine *engine, float volume);

#endif
```

Key changes from current audio.h:
- Removed `MAX_VOICES`, `AUDIO_BUFFER_FRAMES`, `WaveType`, `Voice`, `Voice` array, `AudioStream stream`, `sampleBuffer`
- Removed `tempo`, `beatTime`, `currentBeat` fields
- Removed `audio_sfx_countdown`, `audio_sfx_finish`, `audio_sfx_collision` function declarations
- Added `Music music` field (raylib type for OGG playback)
- Kept `currentTrack` for OGG track tracking

- [ ] **Step 3.2: Rewrite audio.c**

Replace the entire file content:

```c
#include "audio.h"
#include <string.h>
#include <math.h>

// Stage-to-track mapping: even stages use track 0, odd stages use track 1
static const char *trackFiles[MAX_TRACKS] = {
    "assets/music/midnight_drive.ogg",
    "assets/music/ganymede.ogg"
};

void audio_init(AudioEngine *engine) {
    memset(engine, 0, sizeof(AudioEngine));
    engine->masterVolume = 0.7f;
    engine->playing = false;
    engine->music = (Music){ 0 };
    InitAudioDevice();
}

void audio_play(AudioEngine *engine, StageType stage) {
    if (engine->playing) {
        StopMusicStream(engine->music);
        UnloadMusicStream(engine->music);
    }
    // Map stage to track: even stages use track 0, odd stages use track 1
    engine->currentTrack = (int)stage % MAX_TRACKS;
    engine->music = LoadMusicStream(trackFiles[engine->currentTrack]);
    SetMusicVolume(engine->music, engine->masterVolume);
    PlayMusicStream(engine->music);
    engine->playing = true;
}

void audio_update(AudioEngine *engine, float dt) {
    (void)dt;
    if (!engine->playing) return;
    UpdateMusicStream(engine->music);
    if (!IsMusicStreamPlaying(engine->music)) {
        PlayMusicStream(engine->music);
    }
}

void audio_stop(AudioEngine *engine) {
    if (engine->playing) {
        StopMusicStream(engine->music);
        engine->playing = false;
    }
}

void audio_set_volume(AudioEngine *engine, float volume) {
    engine->masterVolume = fmaxf(0.0f, fminf(1.0f, volume));
    if (engine->playing) {
        SetMusicVolume(engine->music, engine->masterVolume);
    }
}
```

Note: `fmaxf` and `fminf` require `<math.h>` which is included via `common.h` -> `raylib.h` -> `<math.h>`. The explicit `#include <math.h>` is a safety measure.

- [ ] **Step 3.3: Update CMakeLists.txt to copy assets to build dir**

Add after the `add_executable` line (line 10):

```cmake
# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
```

Insert this right after `target_link_libraries(neoncoast raylib)` (line 11) and before `target_include_directories(neoncoast PRIVATE src)` (line 12).

The resulting block (lines 10-12) should look like:

```cmake
add_executable(neoncoast ${GAME_SOURCES})
target_link_libraries(neoncoast raylib)
# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
target_include_directories(neoncoast PRIVATE src)
```

- [ ] **Step 3.4: Verify main.c needs no changes for audio API**

The current main.c already uses the same API surface:
- `audio_init(&audio);` — unchanged
- `audio_update(&audio, FIXED_DT);` — unchanged signature
- `audio_play(&audio, selectedStage);` — unchanged
- `audio_stop(&audio);` — unchanged
- `audio_set_volume(&audio, config.musicVolume);` — unchanged

No sfx calls exist in main.c (the stubs in the old audio.c were never called). No changes needed.

- [ ] **Step 3.5: Build and verify (assets not yet downloaded — will pass with placeholder)**

The build should succeed. Audio will print warnings about missing OGG files until Task 6 downloads them, but the code compiles:

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Expected: Clean compile with no warnings.

- [ ] **Step 3.6: Commit**

```bash
git add -A
git commit -m "feat: OGG music system with Midnight Drive and Ganymede"
```

---

### Task 4: Enhanced Scenery (Temples, Animals, Cacti, etc.)

**Files:**
- Modify: `src/race.h` — add new SceneryType enum values, increase MAX_SCENERY
- Modify: `src/race.c` — extend scenery generation with new types, stage-biased spawning
- Modify: `src/render.c` — add rendering cases for all 6 new scenery types in render_draw_scenery

- [ ] **Step 4.1: Extend SceneryType enum in race.h**

```c
// src/race.h, the SceneryType enum (lines 46-51):

// Replace:
typedef enum {
    SCENERY_TREE,
    SCENERY_BUILDING,
    SCENERY_PALM,
    SCENERY_SIGN
} SceneryType;

// With:
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

- [ ] **Step 4.2: Increase MAX_SCENERY in race.h**

```c
// src/race.h line 10 — change:
#define MAX_SCENERY 40
// to:
#define MAX_SCENERY 60
```

- [ ] **Step 4.3: Extend scenery generation in race.c**

Replace the entire `race_generate_scenery` function in `src/race.c` (lines 74-107):

```c
void race_generate_scenery(Race *race) {
    race->sceneryCount = 0;
    Color treeGreen = (Color){ 30, 130, 30, 255 };
    Color treeDark = (Color){ 20, 90, 20, 255 };
    Color buildingColors[] = {
        (Color){ 140, 110, 90, 255 }, (Color){ 180, 150, 120, 255 },
        (Color){ 90, 110, 130, 255 }, (Color){ 200, 60, 60, 255 }
    };
    Color templeColors[] = {
        (Color){ 200, 160, 80, 255 }, (Color){ 180, 120, 50, 255 },
        (Color){ 220, 180, 100, 255 }
    };
    Color cactusGreen = (Color){ 40, 140, 40, 255 };
    Color rockGray = (Color){ 140, 130, 120, 255 };
    Color billboardColors[] = {
        (Color){ 255, 50, 50, 255 }, (Color){ 50, 255, 100, 255 },
        (Color){ 50, 100, 255, 255 }, (Color){ 255, 255, 50, 255 }
    };
    Color deerBrown = (Color){ 160, 100, 60, 255 };
    Color birdColor = (Color){ 100, 100, 100, 255 };

    StageType stageType = race->stage.type;

    for (int i = 0; i < MAX_SCENERY && race->sceneryCount < MAX_SCENERY; i++) {
        SceneryObject *s = &race->scenery[race->sceneryCount];
        s->worldZ = (float)(rand() % TOTAL_SEGMENTS) * SEGMENT_LENGTH;
        s->rightSide = (rand() % 2 == 0);
        float roadEdgeOffset = 2000.0f + (rand() % 2000);

        // Stage-biased scenery selection
        int roll = rand() % 100;

        if (stageType == STAGE_SAHARA) {
            // Sahara: more cacti and rocks, fewer trees
            if (roll < 40) {
                s->type = SCENERY_CACTUS;
                s->worldX = s->rightSide ? roadEdgeOffset + 50 : -(roadEdgeOffset + 50);
                s->scale = 0.6f + (rand() % 4) * 0.2f;
                s->color = cactusGreen;
            } else if (roll < 65) {
                s->type = SCENERY_ROCK;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 3) * 0.2f;
                s->color = rockGray;
            } else if (roll < 80) {
                s->type = SCENERY_TREE;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.6f + (rand() % 3) * 0.2f;
                s->color = (rand() % 2) ? treeGreen : treeDark;
            } else if (roll < 90) {
                s->type = SCENERY_SIGN;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.8f + (rand() % 3) * 0.2f;
                s->color = (Color){ 50, 50, 180, 255 };
            } else {
                s->type = SCENERY_BILLBOARD;
                s->worldX = s->rightSide ? roadEdgeOffset + 100 : -(roadEdgeOffset + 100);
                s->scale = 0.7f + (rand() % 3) * 0.2f;
                s->color = billboardColors[rand() % 4];
            }
        } else if (stageType == STAGE_INDIA) {
            // India: more temples, some deer
            if (roll < 30) {
                s->type = SCENERY_TEMPLE;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 3) * 0.2f;
                s->color = templeColors[rand() % 3];
            } else if (roll < 50) {
                s->type = SCENERY_TREE;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.6f + (rand() % 4) * 0.2f;
                s->color = (rand() % 2) ? treeGreen : treeDark;
            } else if (roll < 65) {
                s->type = SCENERY_DEER;
                s->worldX = s->rightSide ? roadEdgeOffset + 50 : -(roadEdgeOffset + 50);
                s->scale = 0.5f + (rand() % 2) * 0.2f;
                s->color = deerBrown;
            } else if (roll < 80) {
                s->type = SCENERY_BUILDING;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 3) * 0.3f;
                s->color = buildingColors[rand() % 4];
            } else if (roll < 90) {
                s->type = SCENERY_BIRD;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.6f + (rand() % 2) * 0.2f;
                s->color = birdColor;
            } else {
                s->type = SCENERY_SIGN;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.8f + (rand() % 3) * 0.2f;
                s->color = (Color){ 50, 50, 180, 255 };
            }
        } else if (stageType == STAGE_TOKYO || stageType == STAGE_NYC) {
            // Urban: more billboards, fewer trees
            if (roll < 40) {
                s->type = SCENERY_BUILDING;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 3) * 0.3f;
                s->color = buildingColors[rand() % 4];
            } else if (roll < 60) {
                s->type = SCENERY_BILLBOARD;
                s->worldX = s->rightSide ? roadEdgeOffset + 50 : -(roadEdgeOffset + 50);
                s->scale = 0.7f + (rand() % 3) * 0.2f;
                s->color = billboardColors[rand() % 4];
            } else if (roll < 75) {
                s->type = SCENERY_TREE;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.6f + (rand() % 3) * 0.2f;
                s->color = (rand() % 2) ? treeGreen : treeDark;
            } else if (roll < 85) {
                s->type = SCENERY_SIGN;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.8f + (rand() % 3) * 0.2f;
                s->color = (Color){ 50, 50, 180, 255 };
            } else if (roll < 93) {
                s->type = SCENERY_ROCK;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 2) * 0.2f;
                s->color = rockGray;
            } else {
                s->type = SCENERY_BIRD;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.6f + (rand() % 2) * 0.2f;
                s->color = birdColor;
            }
        } else {
            // Default (Coastal, Mediterranean, Hawaii, Guatemala): more palms, natural mix
            if (roll < 25) {
                s->type = SCENERY_PALM;
                s->worldX = s->rightSide ? roadEdgeOffset + 100 : -(roadEdgeOffset + 100);
                s->scale = 0.8f + (rand() % 4) * 0.2f;
                s->color = treeGreen;
            } else if (roll < 45) {
                s->type = SCENERY_TREE;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.6f + (rand() % 4) * 0.2f;
                s->color = (rand() % 2) ? treeGreen : treeDark;
            } else if (roll < 60) {
                s->type = SCENERY_BUILDING;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 3) * 0.3f;
                s->color = buildingColors[rand() % 4];
            } else if (roll < 70) {
                s->type = SCENERY_SIGN;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.8f + (rand() % 3) * 0.2f;
                s->color = (Color){ 50, 50, 180, 255 };
            } else if (roll < 78) {
                s->type = SCENERY_ROCK;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 3) * 0.2f;
                s->color = rockGray;
            } else if (roll < 86) {
                s->type = SCENERY_DEER;
                s->worldX = s->rightSide ? roadEdgeOffset + 50 : -(roadEdgeOffset + 50);
                s->scale = 0.5f + (rand() % 2) * 0.2f;
                s->color = deerBrown;
            } else if (roll < 93) {
                s->type = SCENERY_BIRD;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.6f + (rand() % 2) * 0.2f;
                s->color = birdColor;
            } else {
                s->type = SCENERY_TEMPLE;
                s->worldX = s->rightSide ? roadEdgeOffset : -roadEdgeOffset;
                s->scale = 0.5f + (rand() % 2) * 0.2f;
                s->color = templeColors[rand() % 3];
            }
        }
        race->sceneryCount++;
    }
}
```

- [ ] **Step 4.4: Add rendering for new scenery types in render.c**

Add new case branches to the switch statement inside `render_draw_scenery` function (src/render.c, lines 243-299). Insert the following cases before the closing `}` of the switch block (after the `SCENERY_SIGN` case, before line 299's closing brace):

```c
            case SCENERY_TEMPLE: {
                float baseW = 50.0f * s->scale * scale;
                float baseH = 60.0f * s->scale * scale;
                float roofH = 30.0f * s->scale * scale;
                if (baseW < 4) continue;
                // Base
                DrawRectangle((int)(screenX - baseW / 2), (int)(screenY - baseH),
                              (int)baseW, (int)baseH, s->color);
                // Roof (triangle) — raylib's DrawTriangle is available via common.h -> raylib.h
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
                if (trunkW < 3) continue;
                // Main trunk
                DrawRectangle((int)(screenX - trunkW / 2), (int)(screenY - trunkH),
                              (int)trunkW, (int)trunkH, s->color);
                // Left arm
                DrawRectangle((int)(screenX - trunkW * 2), (int)(screenY - trunkH * 0.7f),
                              (int)(trunkW * 1.5f), (int)(trunkW * 0.6f), s->color);
                // Right arm
                DrawRectangle((int)(screenX + trunkW * 0.5f), (int)(screenY - trunkH * 0.5f),
                              (int)(trunkW * 1.5f), (int)(trunkW * 0.6f), s->color);
                break;
            }
            case SCENERY_ROCK: {
                float rockW = 25.0f * s->scale * scale;
                float rockH = 15.0f * s->scale * scale;
                if (rockW < 3) continue;
                DrawEllipse((int)screenX, (int)(screenY - rockH / 2),
                            (int)(rockW / 2), (int)(rockH / 2), s->color);
                break;
            }
            case SCENERY_BILLBOARD: {
                float poleW = 4.0f * s->scale * scale;
                float poleH = 60.0f * s->scale * scale;
                float boardW = 50.0f * s->scale * scale;
                float boardH = 30.0f * s->scale * scale;
                if (boardW < 4) continue;
                // Pole
                DrawRectangle((int)(screenX - poleW / 2), (int)(screenY - poleH),
                              (int)poleW, (int)poleH, (Color){ 150, 150, 150, 255 });
                // Billboard face
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
                if (bodyW < 3) continue;
                // Body
                DrawRectangle((int)(screenX - bodyW / 2), (int)(screenY - bodyH - legH),
                              (int)bodyW, (int)bodyH, s->color);
                // Left legs
                DrawRectangle((int)(screenX - bodyW * 0.3f), (int)(screenY - legH),
                              (int)legW, (int)legH, (Color){ 80, 50, 30, 255 });
                // Right legs
                DrawRectangle((int)(screenX + bodyW * 0.2f), (int)(screenY - legH),
                              (int)legW, (int)legH, (Color){ 80, 50, 30, 255 });
                // Neck + head
                DrawRectangle((int)(screenX + bodyW * 0.3f), (int)(screenY - bodyH - legH - neckH),
                              (int)legW * 2, (int)neckH, s->color);
                // Head circle
                DrawCircle((int)(screenX + bodyW * 0.4f), (int)(screenY - bodyH - legH - neckH - 4.0f * scale),
                           4.0f * scale, (Color){ 80, 50, 30, 255 });
                break;
            }
            case SCENERY_BIRD: {
                float wingSpan = 25.0f * s->scale * scale;
                if (wingSpan < 4) continue;
                float birdY = screenY - 80.0f * scale; // In the sky above scenery
                // V-shape wings (two lines forming a V)
                DrawLine((int)screenX, (int)birdY,
                         (int)(screenX - wingSpan / 2), (int)(birdY - 8.0f * scale), s->color);
                DrawLine((int)screenX, (int)birdY,
                         (int)(screenX + wingSpan / 2), (int)(birdY - 8.0f * scale), s->color);
                break;
            }
```

- [ ] **Step 4.5: Build and test**

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Expected: Clean compile (new enum values match between race.h and render.c switch).

Run the game and play through multiple stages. Verify:
- Each stage shows appropriate scenery types (temples in India, cacti in Sahara, billboards in cities)
- New scenery renders correctly (no missing sprites or corruption)
- Deer, birds, rocks appear in natural stages
- Scenery count increased (more objects visible)

- [ ] **Step 4.6: Commit**

```bash
git add -A
git commit -m "feat: enhanced scenery - temples, cacti, rocks, billboards, deer, birds"
```

---

### Task 5: Parallax Background (Mountains, Hills, Skyline)

**Files:**
- Modify: `src/render.h` — declare `render_draw_background`
- Modify: `src/render.c` — implement `render_draw_background` with procedural mountain silhouettes
- Modify: `src/main.c` — call `render_draw_background` in draw switch for RACING, PAUSED, BOOT, ATTRACT_MODE, COUNTDOWN states

- [ ] **Step 5.1: Add render_draw_background declaration to render.h**

Add after the `render_draw_sky` declaration (line 28 of render.h):

```c
void render_draw_background(const Render *render, float playerZ, StageType stage);
```

The updated block (lines 27-30) should look like:

```c
void render_draw_sky(const Render *render);
void render_draw_background(const Render *render, float playerZ, StageType stage);
void render_draw_clouds(const Render *render, float playerZ);
```

- [ ] **Step 5.2: Implement render_draw_background in render.c**

Add the following function after `render_draw_sky` (after line 63, before `render_draw_clouds`):

```c
void render_draw_background(const Render *render, float playerZ, StageType stage) {
    // Color selection based on stage
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

    // Darken colors based on time of day
    // timeOfDay: 0.0-0.2 = night, 0.2-0.3 = dawn, 0.3-0.7 = day, 0.7-0.8 = dusk, 0.8-1.0 = night
    if (render->timeOfDay < 0.2f || render->timeOfDay > 0.8f) {
        mountColor = ColorBrightness(mountColor, -0.5f);
        hillColor = ColorBrightness(hillColor, -0.5f);
    } else if (render->timeOfDay < 0.3f || render->timeOfDay > 0.7f) {
        mountColor = ColorBrightness(mountColor, -0.2f);
        hillColor = ColorBrightness(hillColor, -0.2f);
    }

    // Parallax factors — far mountains scroll slower, mid hills scroll faster
    float parallaxFar = 0.05f;
    float parallaxMid = 0.15f;
    float offsetFar = fmodf(playerZ * parallaxFar, 1500.0f);
    float offsetMid = fmodf(playerZ * parallaxMid, 1000.0f);

    // Far mountains — large triangular silhouettes
    for (int i = -2; i < 15; i++) {
        float x = i * 140.0f - offsetFar;
        float h = 120.0f + sinf(i * 1.7f + playerZ * 0.0003f) * 60.0f
                  + sinf(i * 3.1f) * 30.0f;

        Vector2 p1 = { x - 100, HORIZON_Y };
        Vector2 p2 = { x + 100, HORIZON_Y };
        Vector2 p3 = { x, HORIZON_Y - h };

        DrawTriangle(p1, p2, p3, mountColor);

        // Snow caps for tall mountains (not in desert stages or at night)
        if (h > 150.0f && stage != STAGE_SAHARA) {
            float capH = h * 0.15f;
            Vector2 cp1 = { x - 40.0f, HORIZON_Y - h + capH };
            Vector2 cp2 = { x + 40.0f, HORIZON_Y - h + capH };
            Vector2 cp3 = { x, HORIZON_Y - h };
            DrawTriangle(cp1, cp2, cp3, (Color){ 220, 230, 240, 180 });
        }
    }

    // Mid hills — smaller, closer (more parallax movement)
    for (int i = -2; i < 20; i++) {
        float x = i * 90.0f - offsetMid;
        float h = 60.0f + sinf(i * 2.3f + playerZ * 0.0008f) * 30.0f
                  + sinf(i * 4.7f) * 15.0f;

        Vector2 p1 = { x - 70, HORIZON_Y };
        Vector2 p2 = { x + 70, HORIZON_Y };
        Vector2 p3 = { x, HORIZON_Y - h };

        DrawTriangle(p1, p2, p3, hillColor);
    }

    // Ocean horizon for coastal stages
    if (stage == STAGE_COASTAL || stage == STAGE_HAWAII || stage == STAGE_MEDITERRANEAN) {
        Color oceanColor = (render->timeOfDay < 0.3f || render->timeOfDay > 0.7f)
            ? (Color){ 10, 40, 80, 200 }
            : (Color){ 30, 120, 180, 200 };
        DrawRectangle(0, (int)HORIZON_Y + 5, SCREEN_WIDTH, 30, oceanColor);
    }
}
```

- [ ] **Step 5.3: Add render_draw_background calls in main.c draw section**

In main.c, the draw switch statement (starting at line 294), add `render_draw_background` calls after each `render_draw_sky` call and before `render_draw_clouds` (for states that render clouds) or before `render_draw_road` (for states without clouds):

**BOOT state (line 296-301):**
```c
// After line 296 (render_draw_sky), before line 297 (render_draw_road):
render_draw_background(&render, game.stateTime * 200.0f, selectedStage);
```
Insert between lines 296 and 297. Result:
```c
            case BOOT:
                render_draw_sky(&render);
                render_draw_background(&render, game.stateTime * 200.0f, selectedStage);
                render_draw_road(&render, &race.stage, game.stateTime * 200.0f);
                render_draw_clouds(&render, game.stateTime * 200.0f);
```

**ATTRACT_MODE state (lines 302-307):**
```c
// After line 303 (render_draw_sky), before line 304 (render_draw_road):
render_draw_background(&render, game.stateTime * 200.0f, selectedStage);
```
Insert between lines 303 and 304. Result:
```c
            case ATTRACT_MODE:
                render_draw_sky(&render);
                render_draw_background(&render, game.stateTime * 200.0f, selectedStage);
                render_draw_road(&render, &race.stage, game.stateTime * 200.0f);
                render_draw_clouds(&render, game.stateTime * 200.0f);
```

**COUNTDOWN state (lines 320-324):**
```c
// After line 321 (render_draw_sky), before line 322 (render_draw_road):
render_draw_background(&render, race.racers[0].pos.z, selectedStage);
```
Insert between lines 321 and 322. Result:
```c
            case COUNTDOWN:
                render_draw_sky(&render);
                render_draw_background(&render, race.racers[0].pos.z, selectedStage);
                render_draw_road(&render, &race.stage, race.racers[0].pos.z);
```

**RACING state (lines 325-342):**
```c
// After line 326 (render_draw_sky), before line 327 (render_draw_clouds):
render_draw_background(&render, race.racers[0].pos.z, selectedStage);
```
Insert between lines 326 and 327. Result:
```c
            case RACING:
                render_draw_sky(&render);
                render_draw_background(&render, race.racers[0].pos.z, selectedStage);
                render_draw_clouds(&render, race.racers[0].pos.z);
```

**PAUSED state (lines 343-360):**
```c
// After line 344 (render_draw_sky), before line 345 (render_draw_clouds):
render_draw_background(&render, race.racers[0].pos.z, selectedStage);
```
Insert between lines 344 and 345. Result:
```c
            case PAUSED:
                render_draw_sky(&render);
                render_draw_background(&render, race.racers[0].pos.z, selectedStage);
                render_draw_clouds(&render, race.racers[0].pos.z);
```

- [ ] **Step 5.4: Build and test**

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Expected: Clean compile (DrawTriangle is from raylib via common.h, HORIZON_Y is defined in render.c).

Run the game and verify:
- Mountain silhouettes are visible behind the road in all stages
- Mountains scroll at a slower rate than the road (parallax effect)
- Hills scroll slightly faster than mountains
- Colors change per stage (blue-gray for coastal, brown for India, dark for NYC/Tokyo)
- Colors darken at dawn/dusk/night
- Snow caps appear on tall mountains (non-desert stages)
- Ocean visible for coastal/Hawaii/Mediterranean stages
- Background renders behind clouds but in front of sky

- [ ] **Step 5.5: Commit**

```bash
git add -A
git commit -m "feat: parallax mountain/hill background with stage-specific colors"
```

---

### Task 6: Download Assets + Build Verify + README

**Files:**
- Create: `assets/music/midnight_drive.ogg`
- Create: `assets/music/ganymede.ogg`
- Modify: `README.md` — add music credits

- [ ] **Step 6.1: Create assets/music directory**

```bash
mkdir -p assets/music
```

- [ ] **Step 6.2: Download OGG music files**

```bash
curl -L -o assets/music/midnight_drive.ogg https://opengameart.org/sites/default/files/midnight_drive.ogg
curl -L -o assets/music/ganymede.ogg https://opengameart.org/sites/default/files/ganymede.ogg
```

Verify files exist and are reasonable size (~3.5 MB each):
```bash
ls -la assets/music/
```

Expected: Each file should be approximately 3.5 MB.

If curl is not available, use an alternative:
- Windows: `Invoke-WebRequest -Uri "https://opengameart.org/sites/default/files/midnight_drive.ogg" -OutFile "assets/music/midnight_drive.ogg"`
- Or use `wget` if available

- [ ] **Step 6.3: Full build and run**

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Run the game and verify:
- OGG music plays during race (check both track 0 and track 1 by selecting stages)
- Music loops correctly when track ends
- Volume control in settings works
- No audio crackling or stuttering

Test at least two stages to verify both music tracks play.

- [ ] **Step 6.4: Update README with music credits**

Add a credits section at the end of `README.md` (before the license section or at the end of the file):

```markdown
## Credits

- Music by [congusbongus](https://opengameart.org/users/congusbongus) (CC0):
  - *Midnight Drive*
  - *Ganymede*
```

Full updated README.md should append this after the existing content. The current last line is line 54 (`MIT`). Add a blank line after line 54 and then the credits.

- [ ] **Step 6.5: Add procedural window icon in main.c**

After `SetTargetFPS(TARGET_FPS);` and `srand((unsigned int)time(NULL));` (around line 24 of main.c), before the `Game game;` declaration, add:

```c
    // Procedural window icon (neon racing ring on dark purple)
    {
        Image icon = GenImageColor(64, 64, (Color){ 10, 0, 25, 255 });
        ImageDrawCircle(&icon, 32, 32, 28, (Color){ 0, 200, 255, 60 });    // outer cyan glow
        ImageDrawCircle(&icon, 32, 32, 24, (Color){ 0, 220, 255, 255 });   // cyan ring
        ImageDrawCircle(&icon, 32, 32, 18, (Color){ 10, 0, 25, 255 });     // clear center
        ImageDrawCircle(&icon, 32, 32, 16, (Color){ 255, 50, 150, 40 });   // pink glow
        ImageDrawCircle(&icon, 32, 32, 14, (Color){ 255, 50, 150, 255 });  // pink ring
        ImageDrawCircle(&icon, 32, 32, 10, (Color){ 10, 0, 25, 255 });     // clear center
        ImageDrawCircle(&icon, 32, 32, 6,  (Color){ 255, 255, 255, 200 }); // white center
        SetWindowIcon(icon);
        UnloadImage(icon);
    }
```

No additional `#include` needed — `common.h` already includes `raylib.h` which provides all Image functions.

- [ ] **Step 6.6: Build and verify icon shows**

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Run the game. Verify the window title bar shows the neon ring icon. Also verify in taskbar/Alt+Tab.

- [ ] **Step 6.7: Commit all remaining changes**

```bash
git add -A
git commit -m "chore: download OGG assets, add window icon, update README credits"
```

- [ ] **Step 6.8: Push**

```bash
git push
```

---

## Post-Implementation Verification Checklist

After all tasks are complete, run through this verification:

| Check | Detail |
|-------|--------|
| Speed | Car reaches 1200 max speed, accelerates at 500 rate |
| Road width | Road is visibly wider, roadHalfWidth is 2000 |
| Traffic spread | Traffic cars positioned across wider road (-1000 to 1000) |
| Scenery offset | Scenery at 2000-4000 from road center |
| Opponent size | Opponent cars render at 150×70 world units |
| Traffic size | Traffic cars render at 130×55 world units |
| OGG music | Music plays in race, stage-mapped to 2 tracks |
| Scenery types | All 10 types visible in appropriate stages |
| Parallax | Mountains scroll slower than hills; stage-specific colors |
| Background | Mountains render behind clouds, in front of sky |
| Window icon | Neon ring icon shows in title bar, taskbar, Alt+Tab |
| Build | Clean compile with no warnings |
| Assets | OGG files present (~3.5 MB each) |

---

## Self-Review

### Spec Coverage
- **Task 1** → covers speed boost (MAX_SPEED 600→1200, ACCEL_RATE 300→500) and wider road (ROAD_WIDTH 2000→5000, traffic spread, scenery offset)
- **Task 2** → covers enemy car size matching (opponent 108→150, 54→70; traffic 90→130, 40→55; cabin/windshield position adjustments)
- **Task 3** → covers OGG music system (audio.h/c rewrite, OGG playback, CMakeLists asset copy, no main.c changes needed)
- **Task 4** → covers enhanced scenery (6 new types: temple, cactus, rock, billboard, deer, bird; MAX_SCENERY 40→60; stage-biased generation; all render cases)
- **Task 5** → covers parallax background (mountain silhouettes, hill silhouettes, stage-specific colors, time-of-day darkening, snow caps, ocean, 5 draw state call sites)
- **Task 6** → covers asset download, build verify, README credits

### Placeholder Scan
No TBD, TODO, or "implement later" found. Every code block contains complete, compilable code.

### Type Consistency
- `SceneryType` enum values in Task 4 match between race.h (definition) and render.c (switch cases)
- `Render`, `StageType`, `playerZ` parameter types consistent across render.h declaration and render.c implementation
- `AudioEngine` struct fields in Task 3 match between audio.h and audio.c
- `HORIZON_Y` used in Task 5's render_draw_background is defined in render.c line 5 — accessible within same file
- `render_draw_background` signature uses `const Render *render` matching existing render functions
