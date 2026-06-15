#include "audio.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

static const char *trackFiles[MAX_TRACKS] = {
    "assets/music/midnight_drive.ogg",
    "assets/music/ganymede.ogg"
};

static Wave wave_generate_sine(float freq, float duration, float volume) {
    int sampleRate = 44100;
    int samples = (int)(sampleRate * duration);
    unsigned char *data = (unsigned char *)malloc(samples * 2);
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float val = sinf(t * 2.0f * (float)PI * freq);
        short s = (short)(val * 32767.0f * volume);
        data[i * 2] = (unsigned char)(s & 0xFF);
        data[i * 2 + 1] = (unsigned char)((s >> 8) & 0xFF);
    }
    Wave wave = { 0 };
    wave.data = data;
    wave.frameCount = (unsigned int)samples;
    wave.sampleRate = (unsigned int)sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    return wave;
}

static Wave wave_generate_noise(float duration, float volume) {
    int sampleRate = 44100;
    int samples = (int)(sampleRate * duration);
    unsigned char *data = (unsigned char *)malloc(samples * 2);
    for (int i = 0; i < samples; i++) {
        float val = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float env = 1.0f - (float)i / samples;
        short s = (short)(val * 32767.0f * volume * env);
        data[i * 2] = (unsigned char)(s & 0xFF);
        data[i * 2 + 1] = (unsigned char)((s >> 8) & 0xFF);
    }
    Wave wave = { 0 };
    wave.data = data;
    wave.frameCount = (unsigned int)samples;
    wave.sampleRate = (unsigned int)sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    return wave;
}

void audio_init(AudioEngine *engine) {
    memset(engine, 0, sizeof(AudioEngine));
    engine->masterVolume = 0.7f;
    engine->playing = false;
    engine->sfxLoaded = false;
    InitAudioDevice();

    Wave countWave = wave_generate_sine(600.0f, 0.15f, 0.5f);
    Wave goWave = wave_generate_sine(880.0f, 0.3f, 0.6f);
    Wave crashWave = wave_generate_noise(0.2f, 0.7f);
    Wave finishWave = wave_generate_sine(440.0f, 0.5f, 0.5f);

    engine->sndCountdown = LoadSoundFromWave(countWave);
    free(countWave.data);
    engine->sndGo = LoadSoundFromWave(goWave);
    free(goWave.data);
    engine->sndCollision = LoadSoundFromWave(crashWave);
    free(crashWave.data);
    engine->sndFinish = LoadSoundFromWave(finishWave);
    free(finishWave.data);
    engine->sfxLoaded = true;
}

void audio_play(AudioEngine *engine, StageType stage) {
    if (!engine->sfxLoaded) return;
    if (engine->playing) {
        StopMusicStream(engine->music);
        UnloadMusicStream(engine->music);
    }
    engine->currentTrack = (int)stage % MAX_TRACKS;
    engine->music = LoadMusicStream(trackFiles[engine->currentTrack]);
    SetMusicVolume(engine->music, engine->masterVolume);
    PlayMusicStream(engine->music);
    engine->playing = true;
}

void audio_update(AudioEngine *engine, float dt) {
    (void)dt;
    if (!engine->playing || !engine->sfxLoaded) return;
    UpdateMusicStream(engine->music);
    if (!IsMusicStreamPlaying(engine->music)) {
        PlayMusicStream(engine->music);
    }
}

void audio_stop(AudioEngine *engine) {
    if (engine->playing && engine->sfxLoaded) {
        StopMusicStream(engine->music);
        engine->playing = false;
    }
}

void audio_set_volume(AudioEngine *engine, float volume) {
    engine->masterVolume = fmaxf(0.0f, fminf(1.0f, volume));
    if (engine->playing && engine->sfxLoaded) {
        SetMusicVolume(engine->music, engine->masterVolume);
    }
}

void audio_sfx_countdown(AudioEngine *engine, int tick) {
    if (!engine->sfxLoaded) return;
    if (tick <= 0) {
        PlaySound(engine->sndGo);
    } else {
        PlaySound(engine->sndCountdown);
    }
}

void audio_sfx_collision(AudioEngine *engine) {
    if (!engine->sfxLoaded) return;
    PlaySound(engine->sndCollision);
}

void audio_sfx_finish(AudioEngine *engine) {
    if (!engine->sfxLoaded) return;
    PlaySound(engine->sndFinish);
}
