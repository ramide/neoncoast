#include "raylib.h"
#include "common.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Neon Coast");
    SetTargetFPS(TARGET_FPS);

    GameState state = BOOT;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Neon Coast - Loading...", 400, 350, 20, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
