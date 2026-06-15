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
} AudioEngine;

void audio_init(AudioEngine *engine);
void audio_update(AudioEngine *engine, float dt);
void audio_play(AudioEngine *engine, StageType stage);
void audio_stop(AudioEngine *engine);
void audio_set_volume(AudioEngine *engine, float volume);

#endif
