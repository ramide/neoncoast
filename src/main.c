#include "raylib.h"
#include "common.h"
#include "game.h"
#include "input.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Neon Coast");
    SetTargetFPS(TARGET_FPS);

    Game game;
    game_init(&game);

    Input input;
    input_init(&input);

    double accumulator = 0.0;
    double lastTime = GetTime();

    while (!WindowShouldClose()) {
        double currentTime = GetTime();
        double frameTime = currentTime - lastTime;
        lastTime = currentTime;
        accumulator += frameTime;

        while (accumulator >= FIXED_DT) {
            input_update(&input);

            if (input.state.confirm && game.current == BOOT) {
                game_set_state(&game, ATTRACT_MODE);
            }
            if (input.state.back && game.current != BOOT && game.current != MAIN_MENU) {
                game_set_state(&game, MAIN_MENU);
            }
            if (input.state.confirm && game.current == EXIT_REQUESTED) {
                break;
            }

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
