#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include "common.h"

#define HIGHSCORE_PATH "racing_highscores.json"

typedef struct {
    char name[20];
    float time;
    int stage;
    int car;
    char date[11];
} HighScoreEntry;

typedef struct {
    HighScoreEntry entries[MAX_HIGHSCORES];
    int count;
} HighScoreTable;

void highscore_init(HighScoreTable *table);
bool highscore_load(HighScoreTable *table, const char *path);
bool highscore_save(const HighScoreTable *table, const char *path);
bool highscore_add(HighScoreTable *table, const HighScoreEntry *entry);
void highscore_sort(HighScoreTable *table);

#endif
