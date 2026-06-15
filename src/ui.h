#ifndef UI_H
#define UI_H

#include "common.h"
#include "input.h"
#include "config.h"
#include "highscore.h"
#include "road.h"
#include "models.h"
#include "race.h"

typedef struct {
    int selectedItem;
    int itemCount;
    float scrollOffset;
} MenuState;

void ui_draw_attract_mode(float time, InputSource source);
void ui_draw_main_menu(MenuState *menu, InputSource source);
void ui_draw_racer_select(MenuState *menu, InputSource source);
void ui_draw_car_select(MenuState *menu, InputSource source, CarType selected, Color *carColor);
void ui_draw_stage_select(MenuState *menu, InputSource source, StageType selected);
void ui_draw_settings(MenuState *menu, InputSource source, GameConfig *config);
void ui_draw_highscores(InputSource source, const HighScoreTable *table);
void ui_draw_countdown(int count);
void ui_draw_hud(const Race *race, InputSource source);
void ui_draw_finish_screen(const Race *race, MenuState *menu, InputSource source);
void ui_draw_pause_menu(MenuState *menu, InputSource source);

void ui_draw_menu_items(const char **items, int count, int selected, InputSource source);
void ui_draw_stats_bar(const char *label, float value, float x, float y);
const char* ui_get_input_label(InputSource source, const char *kb, const char *gp);

#endif
