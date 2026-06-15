#ifndef GAME_H
#define GAME_H

#include "common.h"

typedef struct {
    GameState current;
    GameState previous;
    float stateTime;
    float transitionAlpha;
    bool transitioning;
} Game;

void game_init(Game *game);
void game_update(Game *game, float dt);
void game_set_state(Game *game, GameState next);
void game_render(const Game *game);

#endif
