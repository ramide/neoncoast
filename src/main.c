#include "raylib.h"
#include "common.h"
#include "game.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Neon Coast");
    SetTargetFPS(TARGET_FPS);

    Game game;
    game_init(&game);

    double accumulator = 0.0;
    double lastTime = GetTime();

    while (!WindowShouldClose()) {
        double currentTime = GetTime();
        double frameTime = currentTime - lastTime;
        lastTime = currentTime;
        accumulator += frameTime;

        while (accumulator >= FIXED_DT) {
            game_update(&game, FIXED_DT);
            accumulator -= FIXED_DT;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        game_render(&game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
