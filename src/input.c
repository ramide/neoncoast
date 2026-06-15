#include "input.h"
#include <math.h>

#define DEADZONE_DEFAULT 0.15f

static float apply_deadzone(float value, float deadzone) {
    if (fabsf(value) < deadzone) return 0.0f;
    return (value - copysignf(deadzone, value)) / (1.0f - deadzone);
}

void input_init(Input *input) {
    input->state = (InputState){0};
    input->activeSource = INPUT_KEYBOARD;
    input->deadzone = DEADZONE_DEFAULT;
    input->gamepadIndex = -1;

    for (int i = 0; i < 4; i++) {
        if (IsGamepadAvailable(i)) {
            input->gamepadIndex = i;
            break;
        }
    }
}

void input_update(Input *input) {
    InputState next = {0};
    InputSource source = input->activeSource;

    if (input->gamepadIndex >= 0 && IsGamepadAvailable(input->gamepadIndex)) {
        float axisL = GetGamepadAxisMovement(input->gamepadIndex, GAMEPAD_AXIS_LEFT_X);
        float axisU = -GetGamepadAxisMovement(input->gamepadIndex, GAMEPAD_AXIS_LEFT_Y);

        if (fabsf(axisL) > input->deadzone || fabsf(axisU) > input->deadzone ||
            IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
            IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) ||
            IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) ||
            IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_MIDDLE_RIGHT) ||
            IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_MIDDLE_LEFT) ||
            IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_UP)) {
            source = INPUT_GAMEPAD;

            next.steer = apply_deadzone(axisL, input->deadzone);
            next.throttle = fmaxf(0.0f, apply_deadzone(axisU, input->deadzone));
            next.brake = fmaxf(0.0f, -apply_deadzone(axisU, input->deadzone));
            next.handbrake = IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
            next.confirm = IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
            next.back = IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
            next.pause = IsGamepadButtonPressed(input->gamepadIndex, GAMEPAD_BUTTON_MIDDLE_RIGHT);
        }
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) ||
        IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) ||
        IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) ||
        IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S) ||
        IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER) ||
        IsKeyDown(KEY_ESCAPE)) {
        source = INPUT_KEYBOARD;

        next.steer = 0.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) next.steer -= 1.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) next.steer += 1.0f;

        next.throttle = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) ? 1.0f : 0.0f;
        next.brake = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S) ? 1.0f : 0.0f;
        next.handbrake = IsKeyDown(KEY_SPACE);
        next.confirm = IsKeyDown(KEY_ENTER);
        next.back = IsKeyDown(KEY_BACKSPACE);
        next.pause = IsKeyDown(KEY_ESCAPE);
    }

    input->activeSource = source;
    input->state = next;
}

InputSource input_get_active(const Input *input) {
    return input->activeSource;
}
