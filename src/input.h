#ifndef INPUT_H
#define INPUT_H

#include "common.h"

typedef enum {
    INPUT_KEYBOARD,
    INPUT_GAMEPAD
} InputSource;

typedef struct {
    float steer;
    float throttle;
    float brake;
    bool  handbrake;
    bool  confirm;
    bool  back;
    bool  pause;
} InputState;

typedef struct {
    InputState state;
    InputSource activeSource;
    float deadzone;
    int gamepadIndex;
} Input;

void input_init(Input *input);
void input_update(Input *input);
InputSource input_get_active(const Input *input);

#endif
