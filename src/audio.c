#include "audio.h"
#include <string.h>
#include <math.h>

static const char *trackFiles[MAX_TRACKS] = {
    "assets/music/midnight_drive.ogg",
    "assets/music/ganymede.ogg"
};

void audio_init(AudioEngine *engine) {
    memset(engine, 0, sizeof(AudioEngine));
    engine->masterVolume = 0.7f;
    engine->playing = false;
    engine->music = (Music){ 0 };
    InitAudioDevice();
}

void audio_play(AudioEngine *engine, StageType stage) {
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
    if (!engine->playing) return;
    UpdateMusicStream(engine->music);
    if (!IsMusicStreamPlaying(engine->music)) {
        PlayMusicStream(engine->music);
    }
}

void audio_stop(AudioEngine *engine) {
    if (engine->playing) {
        StopMusicStream(engine->music);
        engine->playing = false;
    }
}

void audio_set_volume(AudioEngine *engine, float volume) {
    engine->masterVolume = fmaxf(0.0f, fminf(1.0f, volume));
    if (engine->playing) {
        SetMusicVolume(engine->music, engine->masterVolume);
    }
}
