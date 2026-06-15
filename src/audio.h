#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "road.h"

#define MAX_VOICES 8
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BUFFER_FRAMES 1024

typedef enum {
    WAVE_SINE,
    WAVE_SAWTOOTH,
    WAVE_SQUARE,
    WAVE_NOISE
} WaveType;

typedef struct {
    WaveType type;
    float frequency;
    float amplitude;
    float phase;
} Voice;

typedef struct {
    Voice voices[MAX_VOICES];
    int voiceCount;
    float tempo;
    float beatTime;
    int currentBeat;
    bool playing;
    float masterVolume;
    AudioStream stream;
    short sampleBuffer[AUDIO_BUFFER_FRAMES];
} AudioEngine;

void audio_init(AudioEngine *engine);
void audio_update(AudioEngine *engine, float dt);
void audio_play(AudioEngine *engine, StageType stage);
void audio_stop(AudioEngine *engine);
void audio_set_volume(AudioEngine *engine, float volume);

void audio_sfx_countdown(AudioEngine *engine, int count);
void audio_sfx_finish(AudioEngine *engine);
void audio_sfx_collision(AudioEngine *engine);

#endif
