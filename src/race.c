#include "race.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SPEED 4800.0f
#define ACCEL_RATE 2000.0f
#define BRAKE_RATE 1600.0f
#define FRICTION 0.998f
#define GEAR_COUNT 6
#define GEAR_RPM_SHIFT 0.85f
#define GEAR_SHIFT_COOLDOWN 0.3f

void race_init(Race *race, StageType stageType, int playerCarIndex) {
    memset(race, 0, sizeof(Race));
    road_init(&race->stage, stageType);
    road_generate(&race->stage);

    race->playerIndex = 0;
    race->countdown = 3;
    race->countdownTimer = 0;

    Racer *player = &race->racers[0];
    player->isPlayer = true;
    player->car = car_create((CarType)playerCarIndex, WHITE);
    player->pos = (Vector3){ 0, 0, 0 };
    player->lap = 1;
    player->bestLap = 9999.0f;
    player->aiSpeedMult = 1.0f;
    player->currentGear = 1;
    player->gearRPM = 0;
    player->gearShiftTime = 0;
    player->autoshift = true;

    Color aiColors[] = { RED, BLUE, GREEN };
    for (int i = 1; i < MAX_RACERS; i++) {
        Racer *ai = &race->racers[i];
        ai->isPlayer = false;
        ai->car = car_create((CarType)((i + playerCarIndex) % MAX_CARS), aiColors[i - 1]);
        ai->pos = (Vector3){ (float)((i - 2) * 400), 0, (float)(i * 500 + 800) };
        ai->targetX = ai->pos.x;
        ai->currentLane = (int)(ai->pos.x / 700.0f);
        if (ai->currentLane < -2) ai->currentLane = -2;
        if (ai->currentLane > 2) ai->currentLane = 2;
        ai->aiLaneTimer = 2.0f + (float)(rand() % 300) / 100.0f;
        ai->lap = 1;
        ai->bestLap = 9999.0f;
        ai->aiSpeedMult = 0.85f + (i * 0.05f);
        ai->aiOffset = (i - 2) * 200.0f;
    }

    race_generate_traffic(race);
    race_generate_scenery(race);
}

void race_generate_traffic(Race *race) {
    race->trafficCount = 0;
    Color trafficColors[] = {
        (Color){ 200, 200, 50, 255 }, (Color){ 50, 150, 200, 255 },
        (Color){ 200, 50, 200, 255 }, (Color){ 100, 200, 100, 255 },
        (Color){ 200, 100, 50, 255 }, (Color){ 150, 150, 150, 255 }
    };

    for (int i = 0; i < MAX_TRAFFIC && race->trafficCount < MAX_TRAFFIC; i++) {
        TrafficCar *t = &race->traffic[race->trafficCount];
        t->pos.z = (float)(800 + rand() % 15000);
        t->pos.x = (rand() % 5 - 2) * 500.0f;
        t->speed = 150.0f + (rand() % 100);
        t->color = trafficColors[rand() % 6];
        t->active = true;
        t->baseX = t->pos.x;
        t->curveX = 0;
        race->trafficCount++;
    }
}

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
        if (i < 80) {
            s->worldZ = (float)(rand() % 5000);
        } else {
            s->worldZ = (float)(rand() % TOTAL_SEGMENTS) * SEGMENT_LENGTH;
        }
        s->rightSide = (rand() % 2 == 0);
        float roadEdgeOffset = 800.0f + (rand() % 1200);

        int roll = rand() % 100;

        if (stageType == STAGE_SAHARA) {
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

    // Second pass: close scenery near road (houses, lamps) - visible at close range
    for (int i = 0; i < MAX_SCENERY && race->sceneryCount < MAX_SCENERY; i++) {
        SceneryObject *s = &race->scenery[race->sceneryCount];
        s->worldZ = (float)(rand() % TOTAL_SEGMENTS) * SEGMENT_LENGTH;
        s->rightSide = (rand() % 2 == 0);
        float closeOffset = 300.0f + (rand() % 500);
        s->worldX = s->rightSide ? closeOffset : -closeOffset;
        s->scale = 0.4f + (rand() % 3) * 0.15f;
        if (rand() % 4 == 0) {
            s->type = SCENERY_LAMP;
            s->color = (Color){ 200, 200, 150, 255 };
        } else {
            s->type = SCENERY_HOUSE;
            switch (stageType) {
                case STAGE_TOKYO:
                case STAGE_NYC:
                    s->color = (Color){ 60 + rand()%100, 60 + rand()%100, 80 + rand()%80, 255 };
                    break;
                case STAGE_SAHARA:
                    s->color = (Color){ 160 + rand()%40, 140 + rand()%40, 100 + rand()%30, 255 };
                    break;
                case STAGE_INDIA:
                    s->color = (Color){ 180 + rand()%60, 100 + rand()%40, 80 + rand()%40, 255 };
                    break;
                default:
                    s->color = (Color){ 130 + rand()%50, 120 + rand()%40, 100 + rand()%30, 255 };
                    break;
            }
        }
        race->sceneryCount++;
    }
}

static void update_racer(Racer *racer, float dt, InputState input, const Stage *stage) {
    if (racer->finished) return;

    if (racer->isPlayer) {
        if (input.throttle > 0) {
            racer->speed += input.throttle * ACCEL_RATE * racer->car.stats.acceleration * dt;
        } else if (input.brake > 0) {
            racer->speed -= input.brake * BRAKE_RATE * dt;
        } else {
            racer->speed *= powf(FRICTION, dt * 60);
        }

        // Lane-switch steering
        int newLane = racer->currentLane;
        if (input.steer < -0.5f && !racer->steerPrevLeft) {
            newLane--;
            if (newLane < -2) newLane = -2;
        }
        racer->steerPrevLeft = (input.steer < -0.5f);
        if (input.steer > 0.5f && !racer->steerPrevRight) {
            newLane++;
            if (newLane > 2) newLane = 2;
        }
        racer->steerPrevRight = (input.steer > 0.5f);
        if (newLane != racer->currentLane) {
            racer->currentLane = newLane;
            racer->targetX = racer->currentLane * 700.0f;
            racer->laneSwitchTimer = 0.3f;
        }
        racer->pos.x += (racer->targetX - racer->pos.x) * fminf(1.0f, dt * 10.0f);
        if (racer->laneSwitchTimer > 0) racer->laneSwitchTimer -= dt;
    } else {
        racer->speed = MAX_SPEED * racer->aiSpeedMult * racer->car.stats.speed;
        racer->aiLaneTimer -= dt;
        if (racer->aiLaneTimer <= 0) {
            int newLane = rand() % 5 - 2;
            racer->currentLane = newLane;
            racer->targetX = racer->currentLane * 700.0f;
            racer->aiLaneTimer = 2.0f + (float)(rand() % 300) / 100.0f;
        }
        racer->pos.x += (racer->targetX - racer->pos.x) * fminf(1.0f, dt * 8.0f);
    }

    float maxSpd = MAX_SPEED * racer->car.stats.speed;
    racer->speed = fmaxf(-maxSpd * 0.3f, fminf(maxSpd, racer->speed));

    if (racer->gearShiftTime > 0) {
        racer->gearShiftTime -= dt;
    }

    float gearRatios[GEAR_COUNT] = { 3.5f, 2.5f, 1.8f, 1.3f, 1.0f, 0.8f };
    float maxGearSpeed = maxSpd * gearRatios[racer->currentGear - 1] / gearRatios[0];
    racer->gearRPM = fminf(1.0f, racer->speed / maxGearSpeed);

    if (racer->isPlayer && racer->autoshift) {
        if (racer->gearRPM >= GEAR_RPM_SHIFT && racer->currentGear < GEAR_COUNT && racer->gearShiftTime <= 0) {
            racer->currentGear++;
            racer->gearRPM = 0.5f;
            racer->gearShiftTime = GEAR_SHIFT_COOLDOWN;
        }
        if (racer->gearRPM < 0.2f && racer->currentGear > 1 && racer->gearShiftTime <= 0) {
            racer->currentGear--;
            racer->gearRPM = 0.7f;
            racer->gearShiftTime = GEAR_SHIFT_COOLDOWN;
        }
    }

    racer->pos.z += racer->speed * dt;
    // Lane-switch handles pos.x update — no analog steering position needed

    float roadHalfWidth = ROAD_WIDTH * 0.4f;
    if (fabsf(racer->pos.x) > roadHalfWidth) {
        racer->pos.x = copysignf(roadHalfWidth, racer->pos.x);
        racer->speed *= 0.90f;
    }

    float lapDistance = TOTAL_SEGMENTS * SEGMENT_LENGTH;
    if (racer->pos.z >= racer->lap * lapDistance) {
        float lapTime = racer->lapTime;
        if (lapTime < racer->bestLap && racer->lap > 1) {
            racer->bestLap = lapTime;
        }
        racer->lap++;
        racer->lapTime = 0;

        if (racer->lap > MAX_LAPS) {
            racer->finished = true;
        }
    }

    racer->lapTime += dt;
    racer->totalTime += dt;
}

static void check_collisions(Race *race) {
    Racer *player = &race->racers[race->playerIndex];

    // Cooldown: don't check again until timer expires
    if (race->collisionTimer > 0) return;

    for (int i = 0; i < race->trafficCount; i++) {
        TrafficCar *t = &race->traffic[i];
        if (!t->active) continue;

        float dz = fabsf(player->pos.z - t->pos.z);
        float dx = fabsf(player->pos.x - t->pos.x);
        if (dz < 400.0f && dx < 400.0f) {
            player->speed *= 0.7f;
            if (player->speed < 100.0f) player->speed = 100.0f;
            t->speed -= 20.0f;
            if (t->speed < 80.0f) t->speed = 80.0f;
            race->playerCollided = true;
            race->collisionTimer = 0.5f;
        }
    }

    for (int i = 1; i < MAX_RACERS; i++) {
        Racer *ai = &race->racers[i];
        float dz = fabsf(player->pos.z - ai->pos.z);
        float dx = fabsf(player->pos.x - ai->pos.x);
        if (dz < 300.0f && dx < 400.0f) {
            player->speed *= 0.8f;
            if (player->speed < 120.0f) player->speed = 120.0f;
            race->playerCollided = true;
            race->collisionTimer = 0.5f;
        }
    }

    // Scenery collision (obstacles on roadside)
    for (int i = 0; i < race->sceneryCount; i++) {
        SceneryObject *s = &race->scenery[i];
        if (s->type != SCENERY_TREE && s->type != SCENERY_ROCK && 
            s->type != SCENERY_CACTUS && s->type != SCENERY_BUILDING &&
            s->type != SCENERY_HOUSE && s->type != SCENERY_TEMPLE) continue;
        float dz = fabsf(player->pos.z - s->worldZ);
        float dx = fabsf(player->pos.x - s->worldX);
        float collDist = 40.0f + s->scale * 30.0f;
        if (dz < collDist * 0.5f && dx < collDist) {
            player->speed *= 0.7f;
            if (player->pos.x > s->worldX) {
                player->pos.x += 30.0f;
            } else {
                player->pos.x -= 30.0f;
            }
            race->playerCollided = true;
            race->collisionTimer = 0.5f;
            break;
        }
    }
}

void race_update(Race *race, float dt, InputState input) {
    if (!race->started) {
        race->countdownTimer += dt;
        if (race->countdownTimer >= 1.0f) {
            race->countdownTimer = 0;
            race->countdown--;
            if (race->countdown <= 0) {
                race->started = true;
            }
        }
        return;
    }

    race->raceTime += dt;

    for (int i = 0; i < MAX_RACERS; i++) {
        InputState racerInput = {0};
        if (i == race->playerIndex) {
            racerInput = input;
        }
        update_racer(&race->racers[i], dt, racerInput, &race->stage);
    }

    for (int i = 0; i < race->trafficCount; i++) {
        TrafficCar *t = &race->traffic[i];
        if (!t->active) continue;

        int segIdx = (int)(t->pos.z / SEGMENT_LENGTH) % TOTAL_SEGMENTS;
        const Segment *seg = road_get_segment(&race->stage, segIdx);
        float targetX = seg->curve * 800.0f + t->baseX;
        t->pos.x += (targetX - t->pos.x) * 0.05f;
        t->pos.z += t->speed * dt;
    }

    for (int i = 0; i < MAX_RACERS; i++) {
        race->racers[i].racePos = 1;
        for (int j = 0; j < MAX_RACERS; j++) {
            if (race->racers[j].lap > race->racers[i].lap ||
                (race->racers[j].lap == race->racers[i].lap &&
                 race->racers[j].pos.z > race->racers[i].pos.z)) {
                race->racers[i].racePos++;
            }
        }
    }

    check_collisions(race);
    if (race->collisionTimer > 0) race->collisionTimer -= dt;

    if (race->racers[race->playerIndex].finished) {
        race->finished = true;
    }
}

int race_get_player_position(const Race *race) {
    return race->racers[race->playerIndex].racePos;
}
