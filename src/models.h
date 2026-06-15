#ifndef MODELS_H
#define MODELS_H

#include "common.h"

typedef enum {
    CAR_SPORT_COUPE,
    CAR_MUSCLE,
    CAR_ELECTRIC,
    CAR_SUPERCAR,
    CAR_COMPACT_EV
} CarType;

typedef struct {
    float speed;
    float acceleration;
    float handling;
    Color color;
    bool hasHeadlights;
} CarStats;

typedef struct {
    Model model;
    CarType type;
    CarStats stats;
    Color color;
} Car;

void models_init(void);
void models_cleanup(void);
Car car_create(CarType type, Color color);
const char* car_get_name(CarType type);
CarStats car_get_stats(CarType type);

#endif
