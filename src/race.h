#ifndef RACE_H
#define RACE_H

#include "common.h"
#include "road.h"
#include "models.h"
#include "input.h"

typedef struct {
    Vector3 pos;
    float speed;
    float steer;
    int lap;
    float lapTime;
    float bestLap;
    float totalTime;
    int racePos;
    bool finished;
    bool isPlayer;
    Car car;
    float aiOffset;
    float aiSpeedMult;
} Racer;

typedef struct {
    Racer racers[MAX_RACERS];
    int playerIndex;
    Stage stage;
    float raceTime;
    bool started;
    bool finished;
    int countdown;
    float countdownTimer;
} Race;

void race_init(Race *race, StageType stageType, int playerCarIndex);
void race_update(Race *race, float dt, InputState input);
int race_get_player_position(const Race *race);

#endif
