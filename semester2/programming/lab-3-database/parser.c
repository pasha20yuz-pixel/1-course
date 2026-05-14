#include "parser.h"
#include "database.h"
#include "memory.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static void print_incorrect(FILE *out, const char *cmd_line) {
    char first20[21];
    strncpy(first20, cmd_line, 20);
    first20[20] = '\0';
    fprintf(out, "incorrect %s\n", first20);
    fprintf(stdout, "incorrect %s\n", first20);
}

static void parse_insert(FILE *out, const char *line) {
    const char *args = line + 6;
    while (*args == ' ') args++;

    Record *rec = create_record();
    if (!rec) return;

    int fields_set[7] = {0};
    char *work = my_malloc(strlen(args) + 1);
    if (!work) {
        free_record(rec);
        return;
    }
    strcpy(work, args);

    char *token = strtok(work, ",");
    int error = 0;
    while (token && !error) {
        char *eq = strchr(token, '=');
        if (!eq) {
            error = 1;
            break;
        }
        *eq = '\0';
        char *field = token;
        char *value = eq + 1;

        int idx = -1;
        if (strcmp(field, "geo_id") == 0) idx = 0;
        else if (strcmp(field, "geo_pos") == 0) idx = 1;
        else if (strcmp(field, "mea_date") == 0) idx = 2;
        else if (strcmp(field, "level") == 0) idx = 3;
        else if (strcmp(field, "sunrise") == 0) idx = 4;
        else if (strcmp(field, "weather") == 0) idx = 5;
        else if (strcmp(field, "sundown") == 0) idx = 6;
        else {
            error = 1;
            break;
        }
        if (fields_set[idx]) {
            error = 1;
            break;
        }
        fields_set[idx] = 1;

        if (idx == 0) {
            char *end;
            long val = strtol(value, &end, 10);
            if (*end != '\0') error = 1;
            else rec->geo_id = (int)val;
        } else if (idx == 1) {
            rec->geo_pos = my_malloc(strlen(value) + 1);
            if (rec->geo_pos) strcpy(rec->geo_pos, value);
            else error = 1;
        } else if (idx == 2) {
            char *iso = convert_date_to_iso(value);
            if (!iso) error = 1;
            else rec->mea_date = iso;
        } else if (idx == 3) {
            char *end;
            long val = strtol(value, &end, 10);
            if (*end != '\0') error = 1;
            else rec->level = (int)val;
        } else if (idx == 4) {
            if (!validate_time(value)) error = 1;
            else {
                rec->sunrise = my_malloc(strlen(value) + 1);
                if (rec->sunrise) strcpy(rec->sunrise, value);
                else error = 1;
            }
        } else if (idx == 5) {
            Weather w = string_to_weather(value);
            if (w == -1) error = 1;
            else rec->weather = w;
        } else if (idx == 6) {
            if (!validate_time(value)) error = 1;
            else {
                rec->sundown = my_malloc(strlen(value) + 1);
                if (rec->sundown) strcpy(rec->sundown, value);
                else error = 1;
            }
        }
        token = strtok(NULL, ",");
    }

    int all_set = 1;
    for (int i = 0; i < 7; i++) if (!fields_set[i]) all_set = 0;
    if (error || !all_set) {
        if (rec->geo_pos) my_free(rec->geo_pos);
        if (rec->mea_date) my_free(rec->mea_date);
        if (rec->sunrise) my_free(rec->sunrise);
        if (rec->sundown) my_free(rec->sundown);
        my_free(rec);
        print_incorrect(out, line);
    } else {
        add_record(rec);
    }
    my_free(work);
}

static void parse_select(FILE *out, char **tokens, int token_count) {
    if (token_count < 2) {
        print_incorrect(out, "select");
        return;
    }
    char *fields_str = tokens[1];
    char *fields[20];
    int field_count = 0;
    char *copy = my_malloc(strlen(fields_str) + 1);
    if (!copy) return;
    strcpy(copy, fields_str);
    char *token = strtok(copy, ",");
    while (token && field_count < 20) {
        fields[field_count++] = token;
        token = strtok(NULL, ",");
    }
    my_free(copy);

    char **conds = NULL;
    int cond_count = parse_conditions(tokens, token_count, 2, &conds);

    // Первый проход: подсчёт
    int found = 0;
    Record *cur = db_head;
    while (cur) {
        if (record_matches_conditions(cur, conds, cond_count)) found++;
        cur = cur->next;
    }
    fprintf(out, "select: %d\n", found);
    fprintf(stdout, "select: %d\n", found);

    // Второй проход: вывод записей
    cur = db_head;
    while (cur) {
        if (record_matches_conditions(cur, conds, cond_count)) {
            for (int i = 0; i < field_count; i++) {
                if (i > 0) {
                    fprintf(out, " ");
                    fprintf(stdout, " ");
                }
                const char *f = fields[i];
                if (strcmp(f, "geo_id") == 0) {
                    fprintf(out, "geo_id=%d", cur->geo_id);
                    fprintf(stdout, "geo_id=%d", cur->geo_id);
                } else if (strcmp(f, "geo_pos") == 0) {
                    fprintf(out, "geo_pos=%s", cur->geo_pos ? cur->geo_pos : "");
                    fprintf(stdout, "geo_pos=%s", cur->geo_pos ? cur->geo_pos : "");
                } else if (strcmp(f, "mea_date") == 0) {
                    char *disp = convert_date_to_display(cur->mea_date);
                    fprintf(out, "mea_date=%s", disp ? disp : "");
                    fprintf(stdout, "mea_date=%s", disp ? disp : "");
                    if (disp) my_free(disp);
                } else if (strcmp(f, "level") == 0) {
                    fprintf(out, "level=%d", cur->level);
                    fprintf(stdout, "level=%d", cur->level);
                } else if (strcmp(f, "sunrise") == 0) {
                    fprintf(out, "sunrise=%s", cur->sunrise ? cur->sunrise : "");
                    fprintf(stdout, "sunrise=%s", cur->sunrise ? cur->sunrise : "");
                } else if (strcmp(f, "weather") == 0) {
                    fprintf(out, "weather=%s", weather_to_string(cur->weather));
                    fprintf(stdout, "weather=%s", weather_to_string(cur->weather));
                } else if (strcmp(f, "sundown") == 0) {
                    fprintf(out, "sundown=%s", cur->sundown ? cur->sundown : "");
                    fprintf(stdout, "sundown=%s", cur->sundown ? cur->sundown : "");
                }
            }
            fprintf(out, "\n");
            fprintf(stdout, "\n");
        }
        cur = cur->next;
    }

    if (conds) my_free(conds);
}

static void parse_delete(FILE *out, char **tokens, int token_count) {
    if (token_count < 2) {
        print_incorrect(out, "delete");
        return;
    }
    char **conds = NULL;
    int cond_count = parse_conditions(tokens, token_count, 1, &conds);
    int deleted = 0;
    Record *prev = NULL;
    Record *cur = db_head;
    while (cur) {
        Record *next = cur->next;
        if (record_matches_conditions(cur, conds, cond_count)) {
            delete_record(prev, cur);
            deleted++;
            cur = next;
            continue;
        }
        prev = cur;
        cur = next;
    }
    fprintf(out, "delete: %d\n", deleted);
    fprintf(stdout, "delete: %d\n", deleted);
    if (conds) my_free(conds);
}

static void parse_update(FILE *out, char **tokens, int token_count) {
    if (token_count < 2) {
        print_incorrect(out, "update");
        return;
    }
    char *upd_str = tokens[1];
    char *upd_fields[20];
    char *upd_values[20];
    int upd_count = 0;
    char *copy = my_malloc(strlen(upd_str) + 1);
    if (!copy) return;
    strcpy(copy, upd_str);
    char *token = strtok(copy, ",");
    int error = 0;
    while (token && upd_count < 20 && !error) {
        char *eq = strchr(token, '=');
        if (!eq) {
            error = 1;
            break;
        }
        *eq = '\0';
        upd_fields[upd_count] = token;
        upd_values[upd_count] = eq + 1;
        if (get_field_type(upd_fields[upd_count]) == -1) error = 1;
        upd_count++;
        token = strtok(NULL, ",");
    }
    if (error) {
        my_free(copy);
        print_incorrect(out, "update");
        return;
    }
    char **conds = NULL;
    int cond_count = parse_conditions(tokens, token_count, 2, &conds);
    int updated = 0;
    Record *cur = db_head;
    while (cur) {
        if (record_matches_conditions(cur, conds, cond_count)) {
            update_record(cur, upd_fields, upd_values, upd_count);
            updated++;
        }
        cur = cur->next;
    }
    fprintf(out, "update: %d\n", updated);
    fprintf(stdout, "update: %d\n", updated);
    my_free(copy);
    if (conds) my_free(conds);
}

static void parse_uniq(FILE *out, char **tokens, int token_count) {
    if (token_count != 2) {
        print_incorrect(out, "uniq");
        return;
    }
    char *fields_str = tokens[1];
    char *fields[20];
    int field_count = 0;
    char *copy = my_malloc(strlen(fields_str) + 1);
    if (!copy) return;
    strcpy(copy, fields_str);
    char *token = strtok(copy, ",");
    while (token && field_count < 20) {
        if (get_field_type(token) == -1) {
            my_free(copy);
            print_incorrect(out, "uniq");
            return;
        }
        fields[field_count++] = token;
        token = strtok(NULL, ",");
    }
    my_free(copy);
    int deleted = uniq_by_fields(fields, field_count);
    fprintf(out, "uniq: %d\n", deleted);
    fprintf(stdout, "uniq: %d\n", deleted);
}

void process_command(FILE *out, char *line) {
    while (*line == ' ') line++;
    if (*line == '\0') return;

    // Копируем строку для strtok, чтобы не повредить оригинал
    char line_copy[1024];
    strcpy(line_copy, line);
    char *tokens[100];
    int token_count = 0;
    char *token = strtok(line_copy, " ");
    while (token && token_count < 100) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }
    if (token_count == 0) return;

    const char *cmd = tokens[0];
    if (strcmp(cmd, "insert") == 0) {
        parse_insert(out, line);
    } else if (strcmp(cmd, "select") == 0) {
        parse_select(out, tokens, token_count);
    } else if (strcmp(cmd, "delete") == 0) {
        parse_delete(out, tokens, token_count);
    } else if (strcmp(cmd, "update") == 0) {
        parse_update(out, tokens, token_count);
    } else if (strcmp(cmd, "uniq") == 0) {
        parse_uniq(out, tokens, token_count);
    } else {
        print_incorrect(out, line);
    }
}