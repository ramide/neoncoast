#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MENU_ITEM_HEIGHT 40
#define MENU_START_Y 200

void ui_draw_menu_items(const char **items, int count, int selected, InputSource source) {
    for (int i = 0; i < count; i++) {
        Color color = (i == selected) ? YELLOW : WHITE;
        int y = MENU_START_Y + i * MENU_ITEM_HEIGHT;

        if (i == selected) {
            DrawText(TextFormat("> %s", items[i]), 400, y, 24, color);
        } else {
            DrawText(TextFormat("  %s", items[i]), 400, y, 24, color);
        }
    }
}

void ui_draw_stats_bar(const char *label, float value, float x, float y) {
    DrawText(label, (int)x, (int)y, 16, WHITE);
    DrawRectangle((int)x + 120, (int)y, 100, 16, DARKGRAY);
    DrawRectangle((int)x + 120, (int)y, (int)(100 * value), 16, GREEN);
}

const char* ui_get_input_label(InputSource source, const char *kb, const char *gp) {
    return (source == INPUT_GAMEPAD) ? gp : kb;
}

void ui_draw_attract_mode(float time, InputSource source) {
    DrawText("NEON COAST", 300, 150, 48, YELLOW);
    DrawText("A Cozy OutRun Experience", 380, 210, 20, LIGHTGRAY);

    float carX = 640 + sinf(time * 2.0f) * 200;
    DrawRectangle((int)carX - 40, 350, 80, 30, RED);
    DrawCircle((int)carX - 25, 385, 10, BLACK);
    DrawCircle((int)carX + 25, 385, 10, BLACK);

    if ((int)(time * 2) % 2 == 0) {
        DrawText("PRESS ANY KEY", 480, 500, 24, WHITE);
    }
}

void ui_draw_main_menu(MenuState *menu, InputSource source) {
    DrawText("NEON COAST", 350, 80, 48, YELLOW);

    static const char *items[] = {
        "Just Race",
        "Settings",
        "High Scores",
        "Exit"
    };
    menu->itemCount = 4;
    ui_draw_menu_items(items, menu->itemCount, menu->selectedItem, source);
}

void ui_draw_racer_select(MenuState *menu, InputSource source) {
    DrawText("SELECT YOUR RACER", 380, 80, 32, YELLOW);

    static const char *racers[] = {
        "You (Player)",
        "Sakura",
        "Marco",
        "Priya"
    };
    menu->itemCount = 4;
    ui_draw_menu_items(racers, menu->itemCount, menu->selectedItem, source);

    DrawText("Press ENTER to confirm", 420, 450, 18, LIGHTGRAY);
}

void ui_draw_car_select(MenuState *menu, InputSource source, CarType selected, Color *carColor) {
    DrawText("SELECT YOUR CAR", 380, 60, 32, YELLOW);

    static const char *cars[] = {
        "Sport Coupe",
        "Muscle Car",
        "Electric GT",
        "Supercar",
        "Compact EV"
    };
    menu->itemCount = 5;
    ui_draw_menu_items(cars, menu->itemCount, menu->selectedItem, source);

    float y = 350;
    CarStats stats = car_get_stats(selected);
    ui_draw_stats_bar("Speed", stats.speed, 400, y);
    ui_draw_stats_bar("Accel", stats.acceleration, 400, y + 25);
    ui_draw_stats_bar("Handling", stats.handling, 400, y + 50);

    DrawText("Color: [R] [G] [B] [Y] [W]", 400, y + 80, 16, LIGHTGRAY);
    DrawRectangle(650, y + 75, 20, 20, *carColor);
}

void ui_draw_stage_select(MenuState *menu, InputSource source, StageType selected) {
    DrawText("SELECT STAGE", 420, 60, 32, YELLOW);

    static const char *stages[] = {
        "Coastal Highway - California",
        "Tokyo Highway - Japan",
        "Temple Run - India",
        "Sunset Strip - Mediterranean",
        "Night Drive - NYC",
        "Tropical Island - Hawaii",
        "Desert Run - Sahara",
        "Highland Run - Guatemala"
    };
    menu->itemCount = 8;
    ui_draw_menu_items(stages, menu->itemCount, menu->selectedItem, source);
}

void ui_draw_settings(MenuState *menu, InputSource source, GameConfig *config) {
    DrawText("SETTINGS", 460, 60, 32, YELLOW);

    static const char *items[] = {
        "Music Volume",
        "SFX Volume",
        "Input Deadzone",
        "Fullscreen",
        "V-Sync",
        "Back"
    };
    menu->itemCount = 6;
    ui_draw_menu_items(items, menu->itemCount, menu->selectedItem, source);

    float y = MENU_START_Y;
    DrawText(TextFormat("%.0f%%", config->musicVolume * 100), 700, y, 18, WHITE);
    DrawText(TextFormat("%.0f%%", config->sfxVolume * 100), 700, y + 40, 18, WHITE);
    DrawText(TextFormat("%.2f", config->deadzone), 700, y + 80, 18, WHITE);
    DrawText(config->fullscreen ? "ON" : "OFF", 700, y + 120, 18, WHITE);
    DrawText(config->vsync ? "ON" : "OFF", 700, y + 160, 18, WHITE);
}

void ui_draw_highscores(InputSource source, const HighScoreTable *table) {
    DrawText("HIGH SCORES", 420, 60, 32, YELLOW);

    if (table->count == 0) {
        DrawText("No scores yet!", 500, 200, 24, LIGHTGRAY);
        return;
    }

    DrawText("Name              Time    Stage          Car", 150, 120, 16, YELLOW);

    for (int i = 0; i < table->count; i++) {
        const HighScoreEntry *e = &table->entries[i];
        DrawText(TextFormat("%-18s %6.1fs  %-14s %s",
                           e->name, e->time,
                           e->stage < 8 ? "Stage" : "Unknown",
                           e->car < 5 ? "Car" : "Unknown"),
                150, 150 + i * 25, 16, WHITE);
    }
}

void ui_draw_countdown(int count) {
    const char *text;
    switch (count) {
        case 3: text = "3"; break;
        case 2: text = "2"; break;
        case 1: text = "1"; break;
        case 0: text = "GO!"; break;
        default: text = ""; break;
    }

    int size = (count == 0) ? 72 : 96;
    Color color = (count == 0) ? GREEN : RED;
    int textW = MeasureText(text, size);
    int x = (SCREEN_WIDTH - textW) / 2;
    int y = (SCREEN_HEIGHT - size) / 2;
    DrawRectangle(x - 20, y - 10, textW + 40, size + 20, Fade(BLACK, 0.5f));
    DrawText(text, x, y, size, color);
}

void ui_draw_fps_counter(void) {
    int fps = GetFPS();
    char buf[32];
    snprintf(buf, sizeof(buf), "FPS: %d", fps);
    int textW = MeasureText(buf, 18);
    Color color = fps >= 55 ? GREEN : (fps >= 30 ? YELLOW : RED);
    DrawText(buf, SCREEN_WIDTH - textW - 15, 10, 18, color);
}

void ui_draw_speed_lines(float speed) {
    if (speed < 2400.0f) return;
    float intensity = (speed - 2400.0f) / 2400.0f;
    int lineCount = (int)(intensity * 20) + 5;
    for (int i = 0; i < lineCount; i++) {
        int x = GetRandomValue(0, SCREEN_WIDTH);
        int y = GetRandomValue(SCREEN_HEIGHT / 3, SCREEN_HEIGHT);
        int len = GetRandomValue(20, 60) + (int)(intensity * 40);
        unsigned char alpha = (unsigned char)(30 + intensity * 50);
        DrawLine(x, y, x, y + len, (Color){ 255, 255, 255, alpha });
    }
}

void ui_draw_brake_effect(bool braking) {
    if (!braking) return;
    DrawRectangle(0, SCREEN_HEIGHT - 60, SCREEN_WIDTH, 60, Fade(RED, 0.15f));
}

void ui_draw_hud(const Race *race, InputSource source) {
    const Racer *player = &race->racers[race->playerIndex];

    float speedKmh = player->speed * (300.0f / MAX_SPEED);
    if (speedKmh > 300.0f) speedKmh = 300.0f;
    if (speedKmh < 0.0f) speedKmh = 0.0f;
    DrawText(TextFormat("%d km/h", (int)speedKmh), 20, 20, 28, WHITE);

    const char *posLabels[] = { "", "1st", "2nd", "3rd", "4th" };
    DrawText(posLabels[player->racePos], 20, 55, 24, YELLOW);

    DrawText(TextFormat("Lap %d / %d", player->lap, MAX_LAPS), 20, 85, 20, WHITE);

    DrawText(TextFormat("%.1fs", player->totalTime), 20, 110, 18, LIGHTGRAY);

    if (player->bestLap < 9999.0f) {
        DrawText(TextFormat("Best: %.1fs", player->bestLap), 20, 135, 16, GREEN);
    }

    const char *gearLabels[] = { "N", "1", "2", "3", "4", "5", "6" };
    DrawText(TextFormat("Gear: %s", gearLabels[player->currentGear]), 20, 160, 20, YELLOW);

    DrawRectangle(20, 185, 100, 12, DARKGRAY);
    Color rpmColor = player->gearRPM > 0.8f ? RED : (player->gearRPM > 0.6f ? YELLOW : GREEN);
    DrawRectangle(20, 185, (int)(100 * player->gearRPM), 12, rpmColor);

    // Analog speedometer - bottom right
    {
        float gaugeCX = SCREEN_WIDTH - 100;
        float gaugeCY = SCREEN_HEIGHT - 30;
        float radius = 70.0f;

        DrawRectangle((int)(gaugeCX - radius - 10), (int)(gaugeCY - radius - 10),
                      (int)(radius * 2 + 20), (int)(radius + 20), Fade(BLACK, 0.6f));

        for (int i = 0; i <= 6; i++) {
            float angle = PI + (float)i / 6.0f * PI;
            float innerR = radius - 8;
            int tx1 = (int)(gaugeCX + cosf(angle) * innerR);
            int ty1 = (int)(gaugeCY + sinf(angle) * innerR);
            int tx2 = (int)(gaugeCX + cosf(angle) * radius);
            int ty2 = (int)(gaugeCY + sinf(angle) * radius);
            DrawLine(tx1, ty1, tx2, ty2, WHITE);
        }

        float needleAngle = PI + (speedKmh / 300.0f) * PI;
        int nx = (int)(gaugeCX + cosf(needleAngle) * (radius - 12));
        int ny = (int)(gaugeCY + sinf(needleAngle) * (radius - 12));
        DrawLine((int)gaugeCX, (int)gaugeCY, nx, ny, RED);
        DrawCircle((int)gaugeCX, (int)gaugeCY, 4, RED);

        char spdBuf[16];
        snprintf(spdBuf, sizeof(spdBuf), "%d", (int)speedKmh);
        int spdW = MeasureText(spdBuf, 16);
        DrawText(spdBuf, (int)gaugeCX - spdW / 2, (int)gaugeCY - 22, 16, WHITE);
    }
}

void ui_draw_finish_screen(const Race *race, MenuState *menu, InputSource source) {
    const Racer *player = &race->racers[race->playerIndex];

    DrawText("RACE COMPLETE", 380, 100, 36, YELLOW);

    const char *posLabels[] = { "", "1st Place!", "2nd Place", "3rd Place", "4th Place" };
    DrawText(posLabels[player->racePos], 450, 160, 28, WHITE);

    DrawText(TextFormat("Time: %.1fs", player->totalTime), 450, 210, 22, WHITE);
    if (player->bestLap < 9999.0f) {
        DrawText(TextFormat("Best Lap: %.1fs", player->bestLap), 450, 240, 20, GREEN);
    }

    static const char *items[] = { "Continue", "Retry", "Main Menu" };
    menu->itemCount = 3;
    ui_draw_menu_items(items, menu->itemCount, menu->selectedItem, source);
}

void ui_draw_pause_menu(MenuState *menu, InputSource source) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));

    static const char *items[] = { "Resume", "Restart", "Settings", "Quit to Menu" };
    menu->itemCount = 4;
    ui_draw_menu_items(items, menu->itemCount, menu->selectedItem, source);
}
