#include "highscore.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void highscore_init(HighScoreTable *table) {
    table->count = 0;
    memset(table->entries, 0, sizeof(table->entries));
}

static int compare_scores(const void *a, const void *b) {
    const HighScoreEntry *ea = (const HighScoreEntry *)a;
    const HighScoreEntry *eb = (const HighScoreEntry *)b;
    if (ea->time < eb->time) return -1;
    if (ea->time > eb->time) return 1;
    return 0;
}

void highscore_sort(HighScoreTable *table) {
    qsort(table->entries, table->count, sizeof(HighScoreEntry), compare_scores);
    if (table->count > MAX_HIGHSCORES) {
        table->count = MAX_HIGHSCORES;
    }
}

bool highscore_add(HighScoreTable *table, const HighScoreEntry *entry) {
    if (table->count >= MAX_HIGHSCORES) {
        if (entry->time >= table->entries[MAX_HIGHSCORES - 1].time) {
            return false;
        }
        table->count = MAX_HIGHSCORES - 1;
    }
    table->entries[table->count] = *entry;
    table->count++;
    highscore_sort(table);
    return true;
}

bool highscore_load(HighScoreTable *table, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;

    char buf[4096];
    size_t len = fread(buf, 1, sizeof(buf) - 1, f);
    buf[len] = '\0';
    fclose(f);

    highscore_init(table);

    char *ptr = buf;
    while ((ptr = strstr(ptr, "\"name\"")) != NULL && table->count < MAX_HIGHSCORES) {
        HighScoreEntry *e = &table->entries[table->count];
        sscanf(ptr, "\"name\" : \"%19[^\"]\"", e->name);
        ptr = strstr(ptr, "\"time\"");
        if (!ptr) break;
        sscanf(ptr, "\"time\" : %f", &e->time);
        ptr = strstr(ptr, "\"stage\"");
        if (!ptr) break;
        sscanf(ptr, "\"stage\" : %d", &e->stage);
        ptr = strstr(ptr, "\"car\"");
        if (!ptr) break;
        sscanf(ptr, "\"car\" : %d", &e->car);
        ptr = strstr(ptr, "\"date\"");
        if (!ptr) break;
        sscanf(ptr, "\"date\" : \"%10[^\"]\"", e->date);
        table->count++;
        ptr++;
    }

    highscore_sort(table);
    return true;
}

bool highscore_save(const HighScoreTable *table, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "[\n");
    for (int i = 0; i < table->count; i++) {
        const HighScoreEntry *e = &table->entries[i];
        fprintf(f, "  { \"name\" : \"%s\", \"time\" : %.2f, \"stage\" : %d, \"car\" : %d, \"date\" : \"%s\" }%s\n",
                e->name, e->time, e->stage, e->car, e->date,
                (i < table->count - 1) ? "," : "");
    }
    fprintf(f, "]\n");

    fclose(f);
    return true;
}
