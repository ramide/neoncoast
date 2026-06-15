#include "game.h"
#include <stdio.h>

void game_init(Game *game) {
    game->current = BOOT;
    game->previous = BOOT;
    game->stateTime = 0.0f;
    game->transitionAlpha = 1.0f;
    game->transitioning = false;
}

void game_set_state(Game *game, GameState next) {
    if (game->current == next) return;
    game->previous = game->current;
    game->current = next;
    game->stateTime = 0.0f;
    game->transitioning = true;
    game->transitionAlpha = 0.0f;
}

void game_update(Game *game, float dt) {
    game->stateTime += dt;
    if (game->transitioning) {
        game->transitionAlpha += dt * 3.0f;
        if (game->transitionAlpha >= 1.0f) {
            game->transitionAlpha = 1.0f;
            game->transitioning = false;
        }
    }
}

void game_render(const Game *game) {
    if (game->transitioning) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
            Fade(BLACK, 1.0f - game->transitionAlpha));
    }
}
