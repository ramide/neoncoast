#include "models.h"
#include <stdlib.h>
#include <time.h>

static bool s_initialized = false;

void models_init(void) {
    srand((unsigned int)time(NULL));
    s_initialized = true;
}

void models_cleanup(void) {
    s_initialized = false;
}

static Model build_car_mesh(CarType type) {
    float bodyLength = 2.0f;
    float bodyWidth = 1.0f;
    float bodyHeight = 0.5f;

    switch (type) {
        case CAR_MUSCLE:
            bodyLength = 2.4f; bodyWidth = 1.1f; bodyHeight = 0.6f;
            break;
        case CAR_SUPERCAR:
            bodyLength = 2.2f; bodyWidth = 1.2f; bodyHeight = 0.35f;
            break;
        case CAR_COMPACT_EV:
            bodyLength = 1.6f; bodyWidth = 0.9f; bodyHeight = 0.6f;
            break;
        case CAR_ELECTRIC:
            bodyLength = 2.1f; bodyWidth = 1.0f; bodyHeight = 0.55f;
            break;
        default: break;
    }

    Mesh bodyMesh = GenMeshCube(bodyLength, bodyHeight, bodyWidth);
    return LoadModelFromMesh(bodyMesh);
}

Car car_create(CarType type, Color color) {
    Car car;
    car.type = type;
    car.color = color;
    car.model = build_car_mesh(type);
    car.stats = car_get_stats(type);
    return car;
}

CarStats car_get_stats(CarType type) {
    switch (type) {
        case CAR_SPORT_COUPE: return (CarStats){ 0.8f, 0.8f, 0.6f, WHITE, true };
        case CAR_MUSCLE:      return (CarStats){ 1.0f, 0.6f, 0.4f, WHITE, true };
        case CAR_ELECTRIC:    return (CarStats){ 1.0f, 1.0f, 0.8f, WHITE, true };
        case CAR_SUPERCAR:    return (CarStats){ 1.0f, 1.0f, 1.0f, WHITE, true };
        case CAR_COMPACT_EV:  return (CarStats){ 0.6f, 0.8f, 1.0f, WHITE, true };
        default:              return (CarStats){ 0.8f, 0.8f, 0.6f, WHITE, true };
    }
}

const char* car_get_name(CarType type) {
    switch (type) {
        case CAR_SPORT_COUPE: return "Sport Coupe";
        case CAR_MUSCLE:      return "Muscle Car";
        case CAR_ELECTRIC:    return "Electric GT";
        case CAR_SUPERCAR:    return "Supercar";
        case CAR_COMPACT_EV:  return "Compact EV";
        default:              return "Unknown";
    }
}
