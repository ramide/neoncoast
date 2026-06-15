#include "audio.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

static float wave_generate(WaveType type, float phase) {
    switch (type) {
        case WAVE_SINE: return sinf(phase * 2.0f * PI);
        case WAVE_SAWTOOTH: return 2.0f * (phase - floorf(phase + 0.5f));
        case WAVE_SQUARE: return (sinf(phase * 2.0f * PI) >= 0) ? 1.0f : -1.0f;
        case WAVE_NOISE: return ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        default: return 0.0f;
    }
}

void audio_init(AudioEngine *engine) {
    memset(engine, 0, sizeof(AudioEngine));
    engine->masterVolume = 0.7f;
    engine->voiceCount = 0;
    engine->playing = false;

    InitAudioDevice();
    engine->stream = LoadAudioStream(AUDIO_SAMPLE_RATE, 16, 1);
}

void audio_play(AudioEngine *engine, StageType stage) {
    engine->playing = true;
    engine->currentBeat = 0;
    engine->beatTime = 0;

    switch (stage) {
        case STAGE_COASTAL: engine->tempo = 95; break;
        case STAGE_TOKYO: engine->tempo = 110; break;
        case STAGE_INDIA: engine->tempo = 100; break;
        case STAGE_MEDITERRANEAN: engine->tempo = 90; break;
        case STAGE_NYC: engine->tempo = 105; break;
        case STAGE_HAWAII: engine->tempo = 95; break;
        case STAGE_SAHARA: engine->tempo = 90; break;
        case STAGE_GUATEMALA: engine->tempo = 100; break;
        default: engine->tempo = 95; break;
    }

    engine->voiceCount = 4;
    engine->voices[0] = (Voice){ WAVE_SAWTOOTH, 55.0f, 0.3f, 0 };
    engine->voices[1] = (Voice){ WAVE_SQUARE, 220.0f, 0.15f, 0 };
    engine->voices[2] = (Voice){ WAVE_SINE, 440.0f, 0.2f, 0 };
    engine->voices[3] = (Voice){ WAVE_NOISE, 0, 0.1f, 0 };

    PlayAudioStream(engine->stream);
}

static void generate_samples(AudioEngine *engine, int frameCount) {
    for (int i = 0; i < frameCount; i++) {
        float sample = 0.0f;
        for (int v = 0; v < engine->voiceCount; v++) {
            sample += wave_generate(engine->voices[v].type, engine->voices[v].phase)
                      * engine->voices[v].amplitude;
        }
        sample *= engine->masterVolume * 0.3f;

        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        engine->sampleBuffer[i] = (short)(sample * 32767.0f);

        for (int v = 0; v < engine->voiceCount; v++) {
            if (engine->voices[v].frequency > 0) {
                engine->voices[v].phase += engine->voices[v].frequency / AUDIO_SAMPLE_RATE;
                if (engine->voices[v].phase >= 1.0f) engine->voices[v].phase -= 1.0f;
            }
        }
    }
}

void audio_update(AudioEngine *engine, float dt) {
    if (!engine->playing) return;

    engine->beatTime += dt;
    float beatDuration = 60.0f / engine->tempo;

    if (engine->beatTime >= beatDuration) {
        engine->beatTime -= beatDuration;
        engine->currentBeat = (engine->currentBeat + 1) % 16;

        float bassNotes[] = { 55.0f, 55.0f, 65.41f, 73.42f };
        int chordIndex = (engine->currentBeat / 4) % 4;
        engine->voices[0].frequency = bassNotes[chordIndex];
        engine->voices[1].frequency = bassNotes[chordIndex] * 4.0f;
        engine->voices[2].frequency = bassNotes[chordIndex] * 8.0f;

        if (engine->currentBeat % 4 == 0) {
            engine->voices[3].amplitude = 0.3f;
        } else {
            engine->voices[3].amplitude = 0.05f;
        }
    }

    int frames = (int)(AUDIO_SAMPLE_RATE * dt);
    if (frames <= 0) frames = 1;
    if (frames > AUDIO_BUFFER_FRAMES) frames = AUDIO_BUFFER_FRAMES;

    generate_samples(engine, frames);
    UpdateAudioStream(engine->stream, engine->sampleBuffer, frames);
}

void audio_stop(AudioEngine *engine) {
    engine->playing = false;
    StopAudioStream(engine->stream);
}

void audio_set_volume(AudioEngine *engine, float volume) {
    engine->masterVolume = fmaxf(0.0f, fminf(1.0f, volume));
}

void audio_sfx_countdown(AudioEngine *engine, int count) {
    (void)engine; (void)count;
}

void audio_sfx_finish(AudioEngine *engine) {
    (void)engine;
}

void audio_sfx_collision(AudioEngine *engine) {
    (void)engine;
}
