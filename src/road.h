#ifndef ROAD_H
#define ROAD_H

#include "common.h"

#define SEGMENT_LENGTH 200.0f
#define ROAD_WIDTH 5000.0f
#define DRAW_DISTANCE 200
#define TOTAL_SEGMENTS 6000

typedef enum {
    STAGE_COASTAL,
    STAGE_TOKYO,
    STAGE_INDIA,
    STAGE_MEDITERRANEAN,
    STAGE_NYC,
    STAGE_HAWAII,
    STAGE_SAHARA,
    STAGE_GUATEMALA
} StageType;

typedef struct {
    float curve;
    float hill;
    int scenery;
} Segment;

typedef struct {
    StageType type;
    const char *name;
    const char *location;
    float timeStart;
    float timeEnd;
    int musicTempo;
    Segment segments[TOTAL_SEGMENTS];
    int segmentCount;
} Stage;

void road_init(Stage *stage, StageType type);
void road_generate(Stage *stage);
Segment* road_get_segment(const Stage *stage, int index);

#endif
