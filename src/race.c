#include "race.h"
#include <math.h>
#include <string.h>

#define MAX_SPEED 300.0f
#define ACCEL_RATE 150.0f
#define BRAKE_RATE 200.0f
#define FRICTION 0.98f
#define STEER_SPEED 3.0f

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

    Color aiColors[] = { RED, BLUE, GREEN };
    for (int i = 1; i < MAX_RACERS; i++) {
        Racer *ai = &race->racers[i];
        ai->isPlayer = false;
        ai->car = car_create((CarType)((i + playerCarIndex) % MAX_CARS), aiColors[i - 1]);
        ai->pos = (Vector3){ (i - 1) * 500, 0, -i * 300 };
        ai->lap = 1;
        ai->bestLap = 9999.0f;
        ai->aiSpeedMult = 0.85f + (i * 0.05f);
        ai->aiOffset = (i - 2) * 200.0f;
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

        racer->steer += input.steer * STEER_SPEED * dt;
        racer->steer *= powf(0.9f, dt * 60);
    } else {
        racer->speed = MAX_SPEED * racer->aiSpeedMult * racer->car.stats.speed;

        Segment *seg = road_get_segment(stage, (int)racer->pos.z / (int)SEGMENT_LENGTH);
        float targetX = seg->curve * 1000.0f + racer->aiOffset;
        racer->steer = (targetX - racer->pos.x) * 0.001f;
    }

    float maxSpd = MAX_SPEED * racer->car.stats.speed;
    racer->speed = fmaxf(-maxSpd * 0.3f, fminf(maxSpd, racer->speed));

    racer->pos.z += racer->speed * dt;
    racer->pos.x += racer->steer * racer->speed * dt * 0.5f;

    float roadHalfWidth = ROAD_WIDTH * 0.4f;
    if (fabsf(racer->pos.x) > roadHalfWidth) {
        racer->speed *= 0.95f;
        racer->pos.x = copysignf(roadHalfWidth, racer->pos.x);
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

    if (race->racers[race->playerIndex].finished) {
        race->finished = true;
    }
}

int race_get_player_position(const Race *race) {
    return race->racers[race->playerIndex].racePos;
}
