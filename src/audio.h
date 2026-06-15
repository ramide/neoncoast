#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "road.h"

#define MAX_TRACKS 2

typedef struct {
    Music music;
    float masterVolume;
    bool playing;
    int currentTrack;
    Sound sndCountdown;
    Sound sndGo;
    Sound sndCollision;
    Sound sndFinish;
    bool sfxLoaded;
} AudioEngine;

void audio_init(AudioEngine *engine);
void audio_update(AudioEngine *engine, float dt);
void audio_play(AudioEngine *engine, StageType stage);
void audio_stop(AudioEngine *engine);
void audio_set_volume(AudioEngine *engine, float volume);
void audio_sfx_countdown(AudioEngine *engine, int tick);
void audio_sfx_collision(AudioEngine *engine);
void audio_sfx_finish(AudioEngine *engine);

#endif
