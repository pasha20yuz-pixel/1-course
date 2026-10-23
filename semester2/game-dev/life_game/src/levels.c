#include "levels.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Level* load_level(int level_id) {
    char filename[64];
    snprintf(filename, sizeof(filename), "levels/level%d.txt", level_id);
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    /* Пропуск UTF-8 BOM, если он есть */
    int c1 = fgetc(f);
    if (c1 == 0xEF) {
        int c2 = fgetc(f);
        int c3 = fgetc(f);
        if (c2 != 0xBB || c3 != 0xBF) rewind(f);
    } else if (c1 != EOF) {
        ungetc(c1, f);
    }

    Level *l = (Level*)malloc(sizeof(Level));
    if (!l) {
        fclose(f);
        return NULL;
    }

    if (fscanf(f, "%d %d", &l->width, &l->height) != 2 || l->width < 2 || l->height < 2) {
        free(l);
        fclose(f);
        return NULL;
    }
    if (fscanf(f, "%d %d", &l->max_start_cells, &l->target_generations) != 2) {
        free(l);
        fclose(f);
        return NULL;
    }

    l->barriers = create_barriers(l->width, l->height);
    if (!l->barriers) {
        free(l);
        fclose(f);
        return NULL;
    }

    char line[256];
    for (int y = 0; y < l->height; y++) {
        if (fscanf(f, "%255s", line) != 1) {
            free_level(l);
            fclose(f);
            return NULL;
        }
        for (int x = 0; x < l->width - 1 && line[x] != '\0'; x++) {
            if (line[x] == '1') set_h_barrier(l->barriers, y, x, true);
        }
    }

    for (int y = 0; y < l->height - 1; y++) {
        if (fscanf(f, "%255s", line) != 1) {
            free_level(l);
            fclose(f);
            return NULL;
        }
        for (int x = 0; x < l->width && line[x] != '\0'; x++) {
            if (line[x] == '1') set_v_barrier(l->barriers, y, x, true);
        }
    }

    int ch;
    while ((ch = fgetc(f)) != EOF && (ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t')) {}
    if (ch != EOF) {
        ungetc(ch, f);
        if (!fgets(l->description, sizeof(l->description), f)) {
            snprintf(l->description, sizeof(l->description), "Level %d", level_id);
        }
    } else {
        snprintf(l->description, sizeof(l->description), "Level %d", level_id);
    }

    size_t len = strlen(l->description);
    while (len > 0 && (l->description[len - 1] == '\n' || l->description[len - 1] == '\r')) {
        l->description[--len] = '\0';
    }

    l->extra.int_param = 0;
    l->extra.float_param = 0.0f;
    l->extra.string_param[0] = '\0';

    l->id = level_id;
    fclose(f);
    return l;
}

void free_level(Level *l) {
    if (l) {
        if (l->barriers) free_barriers(l->barriers);
        free(l);
    }
}

int get_total_levels() {
    return 10;
}