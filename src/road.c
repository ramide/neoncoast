#include "road.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

static void add_road(Stage *stage, int entry, int length, float curve, float hill) {
    for (int i = 0; i < length && (entry + i) < TOTAL_SEGMENTS; i++) {
        stage->segments[entry + i].curve = curve;
        stage->segments[entry + i].hill = hill;
    }
}

void road_init(Stage *stage, StageType type) {
    memset(stage, 0, sizeof(Stage));
    stage->type = type;
    stage->segmentCount = TOTAL_SEGMENTS;

    switch (type) {
        case STAGE_COASTAL:
            stage->name = "Coastal Highway";
            stage->location = "California";
            stage->timeStart = 0.15f; stage->timeEnd = 0.35f;
            stage->musicTempo = 95;
            break;
        case STAGE_TOKYO:
            stage->name = "Tokyo Highway";
            stage->location = "Japan";
            stage->timeStart = 0.7f; stage->timeEnd = 0.95f;
            stage->musicTempo = 110;
            break;
        case STAGE_INDIA:
            stage->name = "Temple Run";
            stage->location = "India";
            stage->timeStart = 0.4f; stage->timeEnd = 0.6f;
            stage->musicTempo = 100;
            break;
        case STAGE_MEDITERRANEAN:
            stage->name = "Sunset Strip";
            stage->location = "Mediterranean";
            stage->timeStart = 0.55f; stage->timeEnd = 0.75f;
            stage->musicTempo = 90;
            break;
        case STAGE_NYC:
            stage->name = "Night Drive";
            stage->location = "NYC";
            stage->timeStart = 0.75f; stage->timeEnd = 0.95f;
            stage->musicTempo = 105;
            break;
        case STAGE_HAWAII:
            stage->name = "Tropical Island";
            stage->location = "Hawaii";
            stage->timeStart = 0.5f; stage->timeEnd = 0.7f;
            stage->musicTempo = 95;
            break;
        case STAGE_SAHARA:
            stage->name = "Desert Run";
            stage->location = "Sahara";
            stage->timeStart = 0.5f; stage->timeEnd = 0.7f;
            stage->musicTempo = 90;
            break;
        case STAGE_GUATEMALA:
            stage->name = "Highland Run";
            stage->location = "Guatemala";
            stage->timeStart = 0.25f; stage->timeEnd = 0.5f;
            stage->musicTempo = 100;
            break;
    }
}

void road_generate(Stage *stage) {
    int seg = 0;

    add_road(stage, seg, 50, 0, 0);
    seg += 50;

    for (int section = 0; section < 50; section++) {
        int length = 30 + (rand() % 70);
        float curve = (rand() % 700 - 350) / 1000.0f;
        float hill = (rand() % 100 - 30) / 300.0f;

        if (stage->type == STAGE_GUATEMALA) hill *= 2.0f;
        if (stage->type == STAGE_SAHARA) hill *= 0.3f;

        add_road(stage, seg, length, curve, hill);
        seg += length;

        // Add short straight connector between curves
        if (section < 49) {
            int straightLen = 20 + (rand() % 30);
            add_road(stage, seg, straightLen, 0, 0);
            seg += straightLen;
        }
    }

    while (seg < TOTAL_SEGMENTS) {
        stage->segments[seg].curve = 0;
        stage->segments[seg].hill = 0;
        seg++;
    }
}

const Segment* road_get_segment(const Stage *stage, int index) {
    if (stage->segmentCount == 0) return &stage->segments[0];
    return &stage->segments[index % stage->segmentCount];
}
