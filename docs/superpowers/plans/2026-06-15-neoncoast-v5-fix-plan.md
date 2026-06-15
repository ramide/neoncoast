# NeonCoast V5 — Core Fixes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix all 8 gameplay issues: road rendering, speed, steering, audio, enemy cars, scenery, curves, collision.

**Architecture:** Targeted fixes to race.c, render.c, audio.c, road.c — minimal structural changes, maximum impact.

**Tech Stack:** raylib 5.0, C11, CMake

---

## Task 1: Fix Speed — Reduce Friction

**Files:**
- Modify: `src/race.c:9`

- [ ] **Step 1.1:** Change FRICTION

Find:
```c
#define FRICTION 0.985f
```
Change to:
```c
#define FRICTION 0.998f
```

- [ ] **Step 1.2:** Commit
```bash
git add -A
git commit -m "fix: reduce FRICTION 0.985→0.998 so terminal velocity approaches MAX_SPEED"
```

---

## Task 2: Fix Road Rendering — Replace Triangles with Rectangles + Overlap

**Files:**
- Modify: `src/render.c` — rewrite the segment drawing loop in render_draw_road
- Modify: `src/render.c` — fix z0 clip so bottom of road is never cut off

- [ ] **Step 2.1:** Rewrite the road drawing loop

The current approach uses DrawTriangle for each sub-segment (2 triangles per trapezoid), which causes 1px rasterization gaps. Replace with a rectangle-per-sub-segment approach that produces solid road rendering.

Replace the inner drawing loop inside render_draw_road (lines 198-287) with this:

```c
    #define SUBDIV 8

    for (int n = DRAW_DISTANCE - 1; n >= 0; n--) {
        float segHill = cumHill[n];
        float farSegHill = cumHill[n + 1];

        for (int s = 0; s < SUBDIV; s++) {
            float t0 = (float)s / SUBDIV;
            float t1 = (float)(s + 1) / SUBDIV;

            float z0 = (n + t0) * SEGMENT_LENGTH - playerOffset;
            float z1 = (n + t1) * SEGMENT_LENGTH - playerOffset;
            
            // Fix: clamp z0 to minimum 1 instead of skipping
            if (z1 <= 0) continue;
            if (z0 <= 0) z0 = 1.0f;

            float scale0 = FOCAL_LENGTH / z0;
            float scale1 = FOCAL_LENGTH / z1;

            float hillOffset0 = (segHill + (farSegHill - segHill) * t0) * scale0 * 500.0f;
            float hillOffset1 = (segHill + (farSegHill - segHill) * t1) * scale1 * 500.0f;

            float bottomY = HORIZON_Y + CAMERA_HEIGHT * scale0 - hillOffset0;
            float topY = HORIZON_Y + CAMERA_HEIGHT * scale1 - hillOffset1;
            if (bottomY >= SCREEN_HEIGHT) continue;

            float cumCurve0 = dx[n] + (dx[n+1] - dx[n]) * t0;
            float cumCurve1 = dx[n] + (dx[n+1] - dx[n]) * t1;

            float x0 = SCREEN_WIDTH * 0.5f + cumCurve0 * scale0 * SEGMENT_LENGTH;
            float x1 = SCREEN_WIDTH * 0.5f + cumCurve1 * scale1 * SEGMENT_LENGTH;

            float w0 = ROAD_WIDTH * 0.5f * scale0;
            float w1 = ROAD_WIDTH * 0.5f * scale1;

            // Compute average position and size for the rectangle
            float avgX = (x0 + x1) * 0.5f;
            float avgY = (topY + bottomY) * 0.5f;
            float avgW = (w0 + w1) * 0.5f;
            float rectH = bottomY - topY + 1.0f; // +1 for overlap between adjacent sub-segments
            
            if (rectH < 1.0f) rectH = 1.0f;

            // Draw road surface as one solid rectangle
            // Using DrawRectangle with center at (avgX, avgY), width = avgW*2, height = rectH
            // This produces a solid rectangle with no gaps
            int rx = (int)(avgX - avgW);
            int ry = (int)topY;
            int rw = (int)(avgW * 2.0f + 1.0f); // +1 for overlap
            int rh = (int)(bottomY - topY + 1.0f);

            // Road surface color based on time of day
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

            // Center lane markings (dashed) - every 8 sub-segments
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
```

Also replace the old `#define SUBDIV 4` line (line 198-199) with:
```c
    #define SUBDIV 8
```

Make sure there's only one `#define SUBDIV` in the file — remove the old one if needed.

- [ ] **Step 2.2:** Verify no duplicate SUBDIV defines

Search for `#define SUBDIV` in render.c. If the old `#define SUBDIV 4` at line 199 still exists, remove it. There should be exactly one `#define SUBDIV 8` inside render_draw_road.

- [ ] **Step 2.3:** Commit
```bash
git add -A
git commit -m "fix: road rendering - replace triangles with solid rectangles + z0 clamp"
```

---

## Task 3: Fix Enemy Cars + Traffic Visibility

**Files:**
- Modify: `src/race.c` — place AI opponents ahead of player, traffic near player
- Modify: `src/render.c` — may not need changes (cars already skip if relativeZ<=0)

- [ ] **Step 3.1:** Fix AI opponent starting positions

In race_init function in race.c, after setting up player at (0, 0), find where AI opponents are positioned. Replace their Z initialization:

Find code like:
```c
    // AI opponents
    for (int r = 1; r < MAX_RACERS; r++) {
        Racer *racer = &race->racers[r];
        racer->pos.z = -(float)(r * 200);  // negative Z — behind player!
        ...
    }
```

Replace with:
```c
    // AI opponents — start ahead of player so they're visible
    for (int r = 1; r < MAX_RACERS; r++) {
        Racer *racer = &race->racers[r];
        racer->pos.z = (float)(r * 500 + 800);  // ahead of player
        racer->pos.x = (float)((rand() % 3 - 1) * 400);  // spread across lanes
        ...
    }
```

- [ ] **Step 3.2:** Fix traffic car positions

In race_generate_traffic function, ensure traffic cars are placed near the player's starting position:

Find the traffic generation loop. Replace the random Z generation with:
```c
    // Generate traffic near the player start (Z = 0 to 15000)
    for (int i = 0; i < MAX_TRAFFIC; i++) {
        TrafficCar *tc = &race->traffic[i];
        tc->pos.z = (float)(800 + rand() % 15000);  // ahead of player
        tc->pos.x = (float)((rand() % 5 - 2) * 500);  // spread across lanes
        ...
    }
```

Also modify `race_init` to call `race_generate_traffic` earlier or ensure traffic is visible. The call should happen after player position is initialized.

- [ ] **Step 3.3:** Commit
```bash
git add -A
git commit -m "fix: place AI opponents and traffic ahead of player so they render"
```

---

## Task 4: Fix Scenery Density + Visibility

**Files:**
- Modify: `src/race.h` — MAX_SCENERY 80→200
- Modify: `src/race.c` — generate more scenery near start position, wider variety

- [ ] **Step 4.1:** Increase MAX_SCENERY

In race.h:
```c
#define MAX_SCENERY 200
```

- [ ] **Step 4.2:** Rewrite scenery generation to concentrate near start

In race.c, replace the race_generate_scenery function to generate scenery concentrated near the first 5000 units of track (where the player starts), plus scattered throughout the track.

The function should:
1. Generate 80 objects in the first 5000 units (close to player start)
2. Generate 120 objects scattered across the remaining track
3. Use the same stage-biased selection as before
4. Keep the close-roadside houses/lamps pass

The key change: modify the worldZ generation for the first pass to be:
```c
if (i < 40) {
    s->worldZ = (float)(rand() % 5000);  // concentrated near start
} else {
    s->worldZ = (float)(rand() % TOTAL_SEGMENTS) * SEGMENT_LENGTH;
}
```

- [ ] **Step 4.3:** Commit
```bash
git add -A
git commit -m "fix: increase scenery density to 200, concentrate near player start"
```

---

## Task 5: Fix Road Curves — Distribute Across Entire Track

**Files:**
- Modify: `src/road.c` — rewrite road_generate to distribute curves across full track

- [ ] **Step 5.1:** Rewrite road_generate

Replace the current road_generate function. The current approach creates 20 curved sections at the start and then fills with straight road. Replace with curves distributed across the entire track:

```c
void road_generate(Stage *stage) {
    memset(stage->segments, 0, sizeof(Segment) * TOTAL_SEGMENTS);
    
    // Generate 50 curve sections distributed across the track
    for (int i = 0; i < 50; i++) {
        int startSeg = rand() % (TOTAL_SEGMENTS - 60);
        int length = 30 + rand() % 40;  // 30-70 segments per curve
        float curve = ((float)(rand() % 600) / 1000.0f) - 0.3f; // -0.3 to +0.3
        
        // Ensure no overlap with existing curves (skip if already has curve)
        for (int j = startSeg; j < startSeg + length && j < TOTAL_SEGMENTS; j++) {
            if (stage->segments[j].curve == 0.0f) {
                stage->segments[j].curve = curve;
            }
        }
    }
    
    // 20% of segments get a small hill
    for (int i = 0; i < TOTAL_SEGMENTS; i++) {
        if (stage->segments[i].curve != 0.0f && rand() % 5 == 0) {
            stage->segments[i].hill = (float)(rand() % 20) / 100.0f;
        }
    }
    
    // Set time range for day/night
    stage->timeStart = 0.0f;
    stage->timeEnd = 1.0f;
    stage->segmentCount = TOTAL_SEGMENTS;
}
```

- [ ] **Step 5.2:** Commit
```bash
git add -A
git commit -m "fix: distribute road curves across entire track (50 sections, -0.3 to +0.3)"
```

---

## Task 6: Fix Collision Slowdown — Add Cooldown

**Files:**
- Modify: `src/race.c` — add collision cooldown check

- [ ] **Step 6.1:** Add collision cooldown to scenery collision check

In race.c, find the scenery collision code (inside update_racer, likely). Add a cooldown timer:

```c
// Scenery collision check
if (!racer->finished && racer->isPlayer && race->collisionTimer <= 0) {
    for (int i = 0; i < race->sceneryCount; i++) {
        SceneryObject *s = &race->scenery[i];
        // Only collide with solid obstacles
        if (s->type == SCENERY_TREE || s->type == SCENERY_ROCK || 
            s->type == SCENERY_CACTUS || s->type == SCENERY_BUILDING ||
            s->type == SCENERY_HOUSE || s->type == SCENERY_TEMPLE) {
            float dz = fabsf(racer->pos.z - s->worldZ);
            float dx = fabsf(racer->pos.x - s->worldX);
            float collDist = 40.0f + s->scale * 30.0f;
            if (dz < collDist * 0.5f && dx < collDist) {
                racer->speed *= 0.7f;  // 0.5 → 0.7 (less severe)
                if (racer->pos.x > s->worldX) {
                    racer->pos.x += 30.0f;
                } else {
                    racer->pos.x -= 30.0f;
                }
                race->collisionTimer = 0.5f;  // 0.5s cooldown
                race->playerCollided = true;
                break;
            }
        }
    }
}
```

Key changes:
- `race->collisionTimer <= 0` guard prevents repeated collisions
- `racer->speed *= 0.7f` instead of 0.5f (less aggressive)
- Collision push distance reduced from 50 to 30

- [ ] **Step 6.2:** Ensure collisionTimer decrements in race_update

In race_update function, add (if not already there):
```c
if (race->collisionTimer > 0) race->collisionTimer -= dt;
if (race->collisionTimer < 0) race->collisionTimer = 0;
```

- [ ] **Step 6.3:** Commit
```bash
git add -A
git commit -m "fix: add 0.5s cooldown to scenery collision, reduce speed penalty 0.5→0.7"
```

---

### Task 7: Lane-Switch Arcade Steering

**Goal:** Replace analog steering with arcade lane-switching — press left/right once to change lanes, car stays in that lane automatically.

**Files:**
- Modify: `src/race.h` — add `currentLane`, `targetX`, `laneSwitchTimer` to Racer struct; remove `smoothedSteer`, `isDrifting`, `driftAngle`, `driftScore`
- Modify: `src/race.c` — rewrite steering logic to lane-switch model; remove analog steering code, drift code, spring boundary code
- Modify: `src/render.c` — update render_draw_car to use lane position instead of analog steer
- Modify: `src/main.c` — update render_draw_car call to use lateral offset

- [ ] **Step 7.1: Update Racer struct in race.h**

Remove these fields:
```c
    float smoothedSteer;
    bool isDrifting;
    float driftAngle;
    float driftScore;
```

Add these fields in their place:
```c
    int currentLane;         // -2=far left, -1=left, 0=center, 1=right, 2=far right
    float targetX;           // world X position of target lane center
    float laneSwitchTimer;   // countdown during lane switch for visual sway
    bool steerPrevLeft;      // edge detection for left press
    bool steerPrevRight;     // edge detection for right press
```

- [ ] **Step 7.2: Rewrite steering logic in race.c (update_racer function)**

Find the steering block in `update_racer` (currently around lines 289-330 in race.c). Remove all the old analog steering, drift, and spring boundary code. Replace with:

```c
    // Lane-switch steering (player)
    if (racer->isPlayer) {
        int newLane = racer->currentLane;
        
        // Edge-detected left input
        if (input.steer < -0.5f && !racer->steerPrevLeft) {
            newLane--;
            if (newLane < -2) newLane = -2;
        }
        racer->steerPrevLeft = (input.steer < -0.5f);
        
        // Edge-detected right input
        if (input.steer > 0.5f && !racer->steerPrevRight) {
            newLane++;
            if (newLane > 2) newLane = 2;
        }
        racer->steerPrevRight = (input.steer > 0.5f);
        
        // Apply lane change
        if (newLane != racer->currentLane) {
            racer->currentLane = newLane;
            racer->targetX = racer->currentLane * 700.0f;
            racer->laneSwitchTimer = 0.3f;
        }
        
        // Smoothly move toward target lane position
        racer->pos.x += (racer->targetX - racer->pos.x) * fminf(1.0f, dt * 10.0f);
        
        // Decrement switch timer
        if (racer->laneSwitchTimer > 0) racer->laneSwitchTimer -= dt;
    }
    
    // AI lane logic
    if (!racer->isPlayer && !racer->finished) {
        // AI occasionally changes lanes
        static float aiLaneTimer = 0;
        aiLaneTimer -= dt;
        if (aiLaneTimer <= 0) {
            int newLane = rand() % 5 - 2; // -2 to 2
            racer->currentLane = newLane;
            racer->targetX = racer->currentLane * 700.0f;
            aiLaneTimer = 1.0f + (float)(rand() % 300) / 100.0f; // 1-4 seconds
        }
        racer->pos.x += (racer->targetX - racer->pos.x) * fminf(1.0f, dt * 8.0f);
    }
```

Also remove these #defines and variables that are no longer needed:
- Remove `STEER_SPEED` (it was used in the analog model)
- Remove the old `powf(0.995f, dt * 60.0f)` steer decay
- Remove the old spring boundary code

But keep FRICTION, MAX_SPEED, ACCEL_RATE, BRAKE_RATE — those are still needed.

- [ ] **Step 7.3: Update render_draw_car in render.c**

The current render_draw_car signature takes `float steer` and computes:
```c
float carX = SCREEN_WIDTH / 2.0f + steer * 120.0f;
```

Change the signature to accept `float laneOffset` instead:
```c
void render_draw_car(float laneOffset, Color color, float speed) {
    float carY = SCREEN_HEIGHT * 0.78f;
    float carX = SCREEN_WIDTH / 2.0f + laneOffset * 200.0f;
    // ... rest stays the same
}
```

The laneOffset maps the player's world X position to a screen offset. At max lane (-2 = -1400 world X), offset = -1400/1200 * 200 = -233 pixels from center.

- [ ] **Step 7.4: Update render.h declaration**

Find the render_draw_car declaration in render.h:
```c
void render_draw_car(float steer, Color color, float speed);
```
Change to:
```c
void render_draw_car(float laneOffset, Color color, float speed);
```

- [ ] **Step 7.5: Update main.c call sites**

Find both calls to `render_draw_car` in main.c (in RACING and PAUSED states):
```c
render_draw_car(input.state.steer, race.racers[0].car.color, race.racers[0].speed);
```
Change each to:
```c
render_draw_car(race.racers[0].pos.x / 1200.0f, race.racers[0].car.color, race.racers[0].speed);
```

This uses the player's lateral position (range ~-1400 to +1400) normalized by 1200 (~ -1.16 to +1.16) as the lane offset.

- [ ] **Step 7.6: Commit**
```bash
git add -A
git commit -m "refactor: replace analog steering with arcade lane-switch system"
```

---

### Task 8: Audio OGG Fallback

**Goal:** Gracefully handle missing OGG files — play SFX even if music files aren't found.

**Files:**
- Modify: `src/audio.c` — add OGG load failure detection
- Modify: `src/audio.c` — add IsAudioDeviceReady guards

- [ ] **Step 8.1: Add IsAudioDeviceReady guard in audio_init**

At the top of `audio_init`, after `InitAudioDevice()`:
```c
void audio_init(AudioEngine *engine) {
    memset(engine, 0, sizeof(AudioEngine));
    engine->masterVolume = 0.7f;
    engine->playing = false;
    engine->sfxLoaded = false;
    InitAudioDevice();
    
    if (!IsAudioDeviceReady()) return;  // No audio device — skip all audio
    // ... rest of SFX generation
}
```

- [ ] **Step 8.2: Add IsAudioDeviceReady guard to all other audio functions**

In every audio function that accesses the audio device (audio_play, audio_update, audio_stop, audio_set_volume, audio_sfx_countdown, audio_sfx_collision, audio_sfx_finish), add at the top:
```c
if (!IsAudioDeviceReady()) return;
```

- [ ] **Step 8.3: Add OGG load failure detection in audio_play**

In `audio_play`, after `LoadMusicStream`, add a check:
```c
    engine->music = LoadMusicStream(trackFiles[engine->currentTrack]);
    
    // If OGG file wasn't found, skip music playback (SFX still work)
    if (engine->music.ctxType == 0 || engine->music.frameCount == 0) {
        engine->playing = false;
        return;
    }
    
    SetMusicVolume(engine->music, engine->masterVolume);
    PlayMusicStream(engine->music);
    engine->playing = true;
```

- [ ] **Step 8.4: Guard audio_update against unloaded music**

In `audio_update`:
```c
void audio_update(AudioEngine *engine, float dt) {
    (void)dt;
    if (!IsAudioDeviceReady()) return;
    if (!engine->playing) return;
    if (engine->music.ctxType == 0) return;  // No music loaded
    UpdateMusicStream(engine->music);
    if (!IsMusicStreamPlaying(engine->music)) {
        PlayMusicStream(engine->music);
    }
}
```

- [ ] **Step 8.5: Commit**
```bash
git add -A
git commit -m "fix: add OGG load failure fallback + IsAudioDeviceReady guards"
```

---

## Post-Implementation Verification

Run CI and check each aspect:

| Check | Expected |
|-------|----------|
| Speed | Car accelerates to ~4800, doesn't slow drastically when coasting |
| Road | Solid rectangles, no gaps or see-through areas, bottom of road fills screen |
| Curves | Road curves visibly throughout the entire race, not just first 10 seconds |
| Opponents | AI cars visible ahead of player in COUNTDOWN and RACING states |
| Traffic | Regular cars visible ahead |
| Scenery | Houses, lamps, trees, signs visible close to road in first few seconds |
| Audio | SFX plays on countdown/collision/finish; music plays if OGG found, gracefully skips if not |
| Steering | Left/right press switches lanes, car stays in lane, 5-lane system |
| Collision | Car slows briefly on tree/building hit, then recovers |
