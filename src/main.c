#include "raylib.h"
#include "common.h"
#include "game.h"
#include "input.h"
#include "config.h"
#include "highscore.h"
#include "audio.h"
#include "render.h"
#include "race.h"
#include "ui.h"
#include "models.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Neon Coast");
    SetTargetFPS(TARGET_FPS);
    srand((unsigned int)time(NULL));

    Image icon = GenImageColor(64, 64, (Color){ 10, 0, 25, 255 });
    ImageDrawCircle(&icon, 32, 32, 28, (Color){ 0, 200, 255, 60 });
    ImageDrawCircle(&icon, 32, 32, 24, (Color){ 0, 220, 255, 255 });
    ImageDrawCircle(&icon, 32, 32, 18, (Color){ 10, 0, 25, 255 });
    ImageDrawCircle(&icon, 32, 32, 16, (Color){ 255, 50, 150, 40 });
    ImageDrawCircle(&icon, 32, 32, 14, (Color){ 255, 50, 150, 255 });
    ImageDrawCircle(&icon, 32, 32, 10, (Color){ 10, 0, 25, 255 });
    ImageDrawCircle(&icon, 32, 32, 6,  (Color){ 255, 255, 255, 200 });
    SetWindowIcon(icon);
    UnloadImage(icon);

    Game game;
    game_init(&game);

    Input input;
    input_init(&input);

    GameConfig config;
    config_init(&config);
    config_load(&config, CONFIG_PATH);

    HighScoreTable scores;
    highscore_init(&scores);
    highscore_load(&scores, HIGHSCORE_PATH);

    AudioEngine audio;
    audio_init(&audio);

    Render render;
    render_init(&render);

    models_init();

    MenuState menu = {0};
    Race race;
    memset(&race, 0, sizeof(Race));
    road_init(&race.stage, STAGE_COASTAL);
    road_generate(&race.stage);

    CarType selectedCar = CAR_SPORT_COUPE;
    StageType selectedStage = STAGE_COASTAL;
    Color carColor = WHITE;

    double accumulator = 0.0;
    double lastTime = GetTime();

    bool prevConfirm = false;
    bool prevBack = false;
    bool prevThrottle = false;
    bool prevHandbrake = false;
    int prevCountdown = 3;

    while (!WindowShouldClose()) {
        double currentTime = GetTime();
        double frameTime = currentTime - lastTime;
        lastTime = currentTime;
        accumulator += frameTime;

        input_update(&input);
        audio_update(&audio, FIXED_DT);

        bool confirmPressed = input.state.confirm && !prevConfirm;
        bool backPressed = input.state.back && !prevBack;
        bool throttlePressed = (input.state.throttle > 0.1f) && !prevThrottle;
        bool handbrakePressed = input.state.handbrake && !prevHandbrake;
        prevConfirm = input.state.confirm;
        prevBack = input.state.back;
        prevThrottle = (input.state.throttle > 0.1f);
        prevHandbrake = input.state.handbrake;

        while (accumulator >= FIXED_DT) {
            game_update(&game, FIXED_DT);

            switch (game.current) {
                case BOOT:
                    if (confirmPressed) {
                        game_set_state(&game, ATTRACT_MODE);
                    }
                    break;

                case ATTRACT_MODE:
                    if (confirmPressed || backPressed) {
                        game_set_state(&game, MAIN_MENU);
                        menu.selectedItem = 0;
                    }
                    break;

                case MAIN_MENU:
                    if (input.state.steer < -0.5f && !prevBack) {
                        menu.selectedItem = (menu.selectedItem - 1 + menu.itemCount) % menu.itemCount;
                        prevBack = true;
                    }
                    if (throttlePressed) {
                        menu.selectedItem = (menu.selectedItem + 1) % menu.itemCount;
                    }
                    if (confirmPressed) {
                        switch (menu.selectedItem) {
                            case 0: game_set_state(&game, RACER_SELECT); menu.selectedItem = 0; break;
                            case 1: game_set_state(&game, SETTINGS); menu.selectedItem = 0; break;
                            case 2: game_set_state(&game, HIGHSCORES); break;
                            case 3: game_set_state(&game, EXIT_REQUESTED); break;
                        }
                    }
                    break;

                case RACER_SELECT:
                    if (throttlePressed) menu.selectedItem = (menu.selectedItem + 1) % menu.itemCount;
                    if (input.state.steer < -0.5f && !prevBack) {
                        menu.selectedItem = (menu.selectedItem - 1 + menu.itemCount) % menu.itemCount;
                        prevBack = true;
                    }
                    if (confirmPressed) {
                        game_set_state(&game, CAR_SELECT);
                        menu.selectedItem = selectedCar;
                    }
                    if (backPressed) { game_set_state(&game, MAIN_MENU); menu.selectedItem = 0; }
                    break;

                case CAR_SELECT:
                    if (throttlePressed) menu.selectedItem = (menu.selectedItem + 1) % menu.itemCount;
                    if (input.state.steer < -0.5f && !prevBack) {
                        menu.selectedItem = (menu.selectedItem - 1 + menu.itemCount) % menu.itemCount;
                        prevBack = true;
                    }
                    selectedCar = (CarType)menu.selectedItem;
                    if (handbrakePressed) {
                        Color colors[] = { WHITE, RED, BLUE, GREEN, YELLOW };
                        static int ci = 0;
                        ci = (ci + 1) % 5;
                        carColor = colors[ci];
                    }
                    if (confirmPressed) {
                        game_set_state(&game, STAGE_SELECT);
                        menu.selectedItem = selectedStage;
                    }
                    if (backPressed) { game_set_state(&game, RACER_SELECT); menu.selectedItem = 0; }
                    break;

                case STAGE_SELECT:
                    if (throttlePressed) menu.selectedItem = (menu.selectedItem + 1) % menu.itemCount;
                    if (input.state.steer < -0.5f && !prevBack) {
                        menu.selectedItem = (menu.selectedItem - 1 + menu.itemCount) % menu.itemCount;
                        prevBack = true;
                    }
                    selectedStage = (StageType)menu.selectedItem;
                    if (confirmPressed) {
                        race_init(&race, selectedStage, selectedCar);
                        race.racers[0].car.color = carColor;
                        audio_play(&audio, selectedStage);
                        game_set_state(&game, COUNTDOWN);
                    }
                    if (backPressed) { game_set_state(&game, CAR_SELECT); menu.selectedItem = selectedCar; }
                    break;

                case COUNTDOWN:
                    race_update(&race, FIXED_DT, input.state);
                    if (race.started) {
                        audio_sfx_countdown(&audio, 0);
                        game_set_state(&game, RACING);
                    } else if (race.countdown != prevCountdown) {
                        audio_sfx_countdown(&audio, race.countdown);
                        prevCountdown = race.countdown;
                    }
                    break;

                case RACING:
                    race_update(&race, FIXED_DT, input.state);
                    if (race.playerCollided) {
                        audio_sfx_collision(&audio);
                        render_shake(&render, 1.0f, 0.3f);
                        race.playerCollided = false;
                    }
                    render_update(&render, FIXED_DT, &race.stage);
                    if (input.state.pause && !prevBack) {
                        game_set_state(&game, PAUSED);
                        menu.selectedItem = 0;
                        prevBack = true;
                    }
                    if (race.finished) {
                        audio_sfx_finish(&audio);
                        game_set_state(&game, FINISH_SCREEN);
                        menu.selectedItem = 0;
                        audio_stop(&audio);

                        HighScoreEntry entry;
                        snprintf(entry.name, sizeof(entry.name), "Player");
                        entry.time = race.racers[0].totalTime;
                        entry.stage = selectedStage;
                        entry.car = selectedCar;
                        snprintf(entry.date, sizeof(entry.date), "2026-06-15");
                        highscore_add(&scores, &entry);
                        highscore_save(&scores, HIGHSCORE_PATH);
                    }
                    break;

                case PAUSED:
                    if (input.state.steer < -0.5f && !prevBack) {
                        menu.selectedItem = (menu.selectedItem - 1 + 4) % 4;
                        prevBack = true;
                    }
                    if (throttlePressed) {
                        menu.selectedItem = (menu.selectedItem + 1) % 4;
                    }
                    if (confirmPressed) {
                        switch (menu.selectedItem) {
                            case 0: game_set_state(&game, RACING); break;
                            case 1:
                                race_init(&race, selectedStage, selectedCar);
                                race.racers[0].car.color = carColor;
                                audio_play(&audio, selectedStage);
                                game_set_state(&game, COUNTDOWN);
                                break;
                            case 2:
                                game_set_state(&game, SETTINGS);
                                menu.selectedItem = 0;
                                break;
                            case 3:
                                game_set_state(&game, MAIN_MENU);
                                menu.selectedItem = 0;
                                audio_stop(&audio);
                                break;
                        }
                    }
                    if (backPressed) {
                        game_set_state(&game, RACING);
                    }
                    break;

                case FINISH_SCREEN:
                    if (throttlePressed) menu.selectedItem = (menu.selectedItem + 1) % menu.itemCount;
                    if (input.state.steer < -0.5f && !prevBack) {
                        menu.selectedItem = (menu.selectedItem - 1 + menu.itemCount) % menu.itemCount;
                        prevBack = true;
                    }
                    if (confirmPressed) {
                        switch (menu.selectedItem) {
                            case 0:
                                game_set_state(&game, STAGE_SELECT);
                                menu.selectedItem = selectedStage;
                                break;
                            case 1:
                                race_init(&race, selectedStage, selectedCar);
                                race.racers[0].car.color = carColor;
                                audio_play(&audio, selectedStage);
                                game_set_state(&game, COUNTDOWN);
                                break;
                            case 2: game_set_state(&game, MAIN_MENU); menu.selectedItem = 0; break;
                        }
                    }
                    break;

                case SETTINGS:
                    if (throttlePressed) menu.selectedItem = (menu.selectedItem + 1) % menu.itemCount;
                    if (input.state.steer < -0.5f && !prevBack) {
                        menu.selectedItem = (menu.selectedItem - 1 + menu.itemCount) % menu.itemCount;
                        prevBack = true;
                    }
                    if (confirmPressed) {
                        if (menu.selectedItem == 5) {
                            config_save(&config, CONFIG_PATH);
                            game_set_state(&game, MAIN_MENU);
                            menu.selectedItem = 0;
                        }
                        if (menu.selectedItem == 3) config.fullscreen = !config.fullscreen;
                        if (menu.selectedItem == 4) config.vsync = !config.vsync;
                    }
                    if (menu.selectedItem == 0) config.musicVolume += input.state.steer * FIXED_DT * 0.5f;
                    if (menu.selectedItem == 1) config.sfxVolume += input.state.steer * FIXED_DT * 0.5f;
                    if (menu.selectedItem == 2) config.deadzone += input.state.steer * FIXED_DT * 0.05f;
                    config.musicVolume = fmaxf(0, fminf(1, config.musicVolume));
                    config.sfxVolume = fmaxf(0, fminf(1, config.sfxVolume));
                    config.deadzone = fmaxf(0, fminf(0.5f, config.deadzone));
                    audio_set_volume(&audio, config.musicVolume);
                    break;

                case HIGHSCORES:
                    if (backPressed || confirmPressed) { game_set_state(&game, MAIN_MENU); menu.selectedItem = 0; }
                    break;

                case EXIT_REQUESTED:
                    if (confirmPressed) goto cleanup;
                    if (backPressed) { game_set_state(&game, MAIN_MENU); menu.selectedItem = 3; }
                    break;

                default: break;
            }

            accumulator -= FIXED_DT;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        switch (game.current) {
            case BOOT:
                render_draw_sky(&render);
                render_draw_background(&render, game.stateTime * 200.0f, selectedStage);
                render_draw_road(&render, &race.stage, game.stateTime * 200.0f);
                render_draw_clouds(&render, game.stateTime * 200.0f);
                DrawText("Neon Coast", 350, 150, 48, YELLOW);
                DrawText("Press ENTER to start", 420, 500, 24, WHITE);
                break;
            case ATTRACT_MODE:
                render_draw_sky(&render);
                render_draw_background(&render, game.stateTime * 200.0f, selectedStage);
                render_draw_road(&render, &race.stage, game.stateTime * 200.0f);
                render_draw_clouds(&render, game.stateTime * 200.0f);
                ui_draw_attract_mode(game.stateTime, input_get_active(&input));
                break;
            case MAIN_MENU:
                ui_draw_main_menu(&menu, input_get_active(&input));
                break;
            case RACER_SELECT:
                ui_draw_racer_select(&menu, input_get_active(&input));
                break;
            case CAR_SELECT:
                ui_draw_car_select(&menu, input_get_active(&input), selectedCar, &carColor);
                break;
            case STAGE_SELECT:
                ui_draw_stage_select(&menu, input_get_active(&input), selectedStage);
                break;
            case COUNTDOWN:
                render_draw_sky(&render);
                render_draw_background(&render, race.racers[0].pos.z, selectedStage);
                render_draw_clouds(&render, race.racers[0].pos.z);
                render_draw_road(&render, &race.stage, race.racers[0].pos.z);
                render_draw_scenery(&render, race.scenery, race.sceneryCount, &race.stage, race.racers[0].pos.z);
                for (int t = 0; t < race.trafficCount; t++) {
                    TrafficCar *tc = &race.traffic[t];
                    if (!tc->active) continue;
                    render_draw_traffic(&race.stage, tc->pos.x, tc->pos.z, tc->color,
                        race.racers[0].pos.z, tc->speed);
                }
                for (int r = 1; r < MAX_RACERS; r++) {
                    render_draw_opponent(&race.stage, race.racers[r].pos.x,
                        race.racers[r].pos.z, race.racers[r].car.color, race.racers[0].pos.z);
                }
                render_draw_car(race.racers[0].pos.x / 1200.0f, race.racers[0].car.color, 0.0f);
                ui_draw_countdown(race.countdown);
                ui_draw_fps_counter();
                break;
            case RACING:
                render_draw_sky(&render);
                render_draw_background(&render, race.racers[0].pos.z, selectedStage);
                render_draw_clouds(&render, race.racers[0].pos.z);
                render_draw_road(&render, &race.stage, race.racers[0].pos.z);
                render_draw_scenery(&render, race.scenery, race.sceneryCount, &race.stage, race.racers[0].pos.z);
                for (int t = 0; t < race.trafficCount; t++) {
                    TrafficCar *tc = &race.traffic[t];
                    if (!tc->active) continue;
                    render_draw_traffic(&race.stage, tc->pos.x, tc->pos.z, tc->color,
                        race.racers[0].pos.z, tc->speed);
                }
                for (int r = 1; r < MAX_RACERS; r++) {
                    render_draw_opponent(&race.stage, race.racers[r].pos.x,
                        race.racers[r].pos.z, race.racers[r].car.color, race.racers[0].pos.z);
                }
                render_draw_car(race.racers[0].pos.x / 1200.0f, race.racers[0].car.color, race.racers[0].speed);
                ui_draw_hud(&race, input_get_active(&input));
                ui_draw_speed_lines(race.racers[0].speed);
                ui_draw_brake_effect(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN));
                ui_draw_fps_counter();
                if (race.collisionTimer > 0) {
                    unsigned char alpha = (unsigned char)(race.collisionTimer * 2.0f * 120);
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 255, 50, 50, alpha });
                }
                break;
            case PAUSED:
                render_draw_sky(&render);
                render_draw_background(&render, race.racers[0].pos.z, selectedStage);
                render_draw_clouds(&render, race.racers[0].pos.z);
                render_draw_road(&render, &race.stage, race.racers[0].pos.z);
                render_draw_scenery(&render, race.scenery, race.sceneryCount, &race.stage, race.racers[0].pos.z);
                for (int t = 0; t < race.trafficCount; t++) {
                    TrafficCar *tc = &race.traffic[t];
                    if (!tc->active) continue;
                    render_draw_traffic(&race.stage, tc->pos.x, tc->pos.z, tc->color,
                        race.racers[0].pos.z, tc->speed);
                }
                for (int r = 1; r < MAX_RACERS; r++) {
                    render_draw_opponent(&race.stage, race.racers[r].pos.x,
                        race.racers[r].pos.z, race.racers[r].car.color, race.racers[0].pos.z);
                }
                render_draw_car(race.racers[0].pos.x / 1200.0f, race.racers[0].car.color, race.racers[0].speed);
                if (race.collisionTimer > 0) {
                    unsigned char alpha = (unsigned char)(race.collisionTimer * 2.0f * 120);
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 255, 50, 50, alpha });
                }
                ui_draw_pause_menu(&menu, input_get_active(&input));
                break;
            case FINISH_SCREEN:
                ui_draw_finish_screen(&race, &menu, input_get_active(&input));
                break;
            case SETTINGS:
                ui_draw_settings(&menu, input_get_active(&input), &config);
                break;
            case HIGHSCORES:
                ui_draw_highscores(input_get_active(&input), &scores);
                break;
            case EXIT_REQUESTED:
                DrawText("Exit Game? Press ENTER to confirm", 300, 350, 24, YELLOW);
                break;
            default: break;
        }

        game_render(&game);
        EndDrawing();
    }

cleanup:
    config_save(&config, CONFIG_PATH);
    models_cleanup();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
