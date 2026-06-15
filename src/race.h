#ifndef RACE_H
#define RACE_H

#include "common.h"
#include "road.h"
#include "models.h"
#include "input.h"

#define MAX_TRAFFIC 15
#define MAX_SCENERY 80

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
    float smoothedSteer;
    int currentGear;
    float gearRPM;
    float gearShiftTime;
    bool autoshift;
    bool isDrifting;
    float driftAngle;
    float driftScore;
} Racer;

typedef struct {
    Vector3 pos;
    float speed;
    float steer;
    Color color;
    bool active;
    float baseX;
    float curveX;
} TrafficCar;

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

typedef struct {
    SceneryType type;
    float worldZ;
    float worldX;
    float scale;
    Color color;
    bool rightSide; // true = right of road, false = left
} SceneryObject;

typedef struct {
    Racer racers[MAX_RACERS];
    int playerIndex;
    Stage stage;
    float raceTime;
    bool started;
    bool finished;
    int countdown;
    float countdownTimer;
    TrafficCar traffic[MAX_TRAFFIC];
    int trafficCount;
    SceneryObject scenery[MAX_SCENERY];
    int sceneryCount;
    bool playerCollided;
    float collisionTimer;
} Race;

void race_init(Race *race, StageType stageType, int playerCarIndex);
void race_update(Race *race, float dt, InputState input);
int race_get_player_position(const Race *race);
void race_generate_traffic(Race *race);
void race_generate_scenery(Race *race);

#endif
