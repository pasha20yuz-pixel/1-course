#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- Управление памятью ---------- */
static int malloc_count = 0;
static int calloc_count = 0;
static int realloc_count = 0;
static int free_count = 0;

void* my_malloc(size_t size) {
    malloc_count++;
    return malloc(size);
}

void* my_calloc(size_t nmemb, size_t size) {
    calloc_count++;
    return calloc(nmemb, size);
}

void* my_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        malloc_count++;
    } else {
        realloc_count++;
    }
    return realloc(ptr, size);
}

void my_free(void *ptr) {
    if (ptr) {
        free_count++;
        free(ptr);
    }
}

void write_memstat(void) {
    FILE *f = fopen("memstat.txt", "w");
    if (f) {
        fprintf(f, "malloc:%d\n", malloc_count);
        fprintf(f, "calloc:%d\n", calloc_count);
        fprintf(f, "realloc:%d\n", realloc_count);
        fprintf(f, "free:%d\n", free_count);
        fclose(f);
    }
}

/* ---------- Типы данных ---------- */
typedef enum {
    WEATHER_FAIR,
    WEATHER_RAIN,
    WEATHER_CLOUDY,
    WEATHER_SNOW
} Weather;

typedef struct Record {
    int geo_id;
    char *geo_pos;
    char *mea_date;   // YYYY-MM-DD
    int level;
    char *sunrise;    // HH:MM:SS
    Weather weather;
    char *sundown;    // HH:MM:SS
    struct Record *next;
} Record;

static Record *db_head = NULL;
static Record *db_tail = NULL;

/* ---------- Вспомогательные функции ---------- */
typedef enum {
    TYPE_INT,
    TYPE_STRING,
    TYPE_DATE,
    TYPE_TIME,
    TYPE_ENUM
} FieldType;

static FieldType get_field_type(const char *field) {
    if (strcmp(field, "geo_id") == 0) return TYPE_INT;
    if (strcmp(field, "geo_pos") == 0) return TYPE_STRING;
    if (strcmp(field, "mea_date") == 0) return TYPE_DATE;
    if (strcmp(field, "level") == 0) return TYPE_INT;
    if (strcmp(field, "sunrise") == 0) return TYPE_TIME;
    if (strcmp(field, "weather") == 0) return TYPE_ENUM;
    if (strcmp(field, "sundown") == 0) return TYPE_TIME;
    return -1;
}

static void* get_field_ptr(Record *rec, const char *field) {
    if (strcmp(field, "geo_id") == 0) return &rec->geo_id;
    if (strcmp(field, "geo_pos") == 0) return &rec->geo_pos;
    if (strcmp(field, "mea_date") == 0) return &rec->mea_date;
    if (strcmp(field, "level") == 0) return &rec->level;
    if (strcmp(field, "sunrise") == 0) return &rec->sunrise;
    if (strcmp(field, "weather") == 0) return &rec->weather;
    if (strcmp(field, "sundown") == 0) return &rec->sundown;
    return NULL;
}

static int compare_values(FieldType type, void *field_val, const char *op, const char *value_str) {
    if (type == TYPE_INT) {
        int field_int = *(int*)field_val;
        int val_int = atoi(value_str);
        if (strcmp(op, "=") == 0) return field_int == val_int;
        if (strcmp(op, "!=") == 0) return field_int != val_int;
        if (strcmp(op, ">") == 0) return field_int > val_int;
        if (strcmp(op, "<") == 0) return field_int < val_int;
        if (strcmp(op, ">=") == 0) return field_int >= val_int;
        if (strcmp(op, "<=") == 0) return field_int <= val_int;
    }
    else if (type == TYPE_STRING) {
        char *field_str = *(char**)field_val;
        if (!field_str) return 0;
        if (strcmp(op, "=") == 0) return strcmp(field_str, value_str) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(field_str, value_str) != 0;
        if (strcmp(op, ">") == 0) return strcmp(field_str, value_str) > 0;
        if (strcmp(op, "<") == 0) return strcmp(field_str, value_str) < 0;
        if (strcmp(op, ">=") == 0) return strcmp(field_str, value_str) >= 0;
        if (strcmp(op, "<=") == 0) return strcmp(field_str, value_str) <= 0;
    }
    else if (type == TYPE_DATE || type == TYPE_TIME) {
        char *field_str = *(char**)field_val;
        if (!field_str) return 0;
        if (strcmp(op, "=") == 0) return strcmp(field_str, value_str) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(field_str, value_str) != 0;
        if (strcmp(op, ">") == 0) return strcmp(field_str, value_str) > 0;
        if (strcmp(op, "<") == 0) return strcmp(field_str, value_str) < 0;
        if (strcmp(op, ">=") == 0) return strcmp(field_str, value_str) >= 0;
        if (strcmp(op, "<=") == 0) return strcmp(field_str, value_str) <= 0;
    }
    else if (type == TYPE_ENUM) {
        Weather field_enum = *(Weather*)field_val;
        Weather val_enum;
        if (strcmp(value_str, "fair") == 0) val_enum = WEATHER_FAIR;
        else if (strcmp(value_str, "rain") == 0) val_enum = WEATHER_RAIN;
        else if (strcmp(value_str, "cloudy") == 0) val_enum = WEATHER_CLOUDY;
        else if (strcmp(value_str, "snow") == 0) val_enum = WEATHER_SNOW;
        else return 0;
        if (strcmp(op, "=") == 0) return field_enum == val_enum;
        if (strcmp(op, "!=") == 0) return field_enum != val_enum;
    }
    return 0;
}

static char* convert_date_to_iso(const char *date_str) {
    int day, month, year;
    if (sscanf(date_str, "%d.%d.%d", &day, &month, &year) != 3)
        return NULL;
    char *iso = my_malloc(11);
    if (!iso) return NULL;
    sprintf(iso, "%04d-%02d-%02d", year, month, day);
    return iso;
}

static char* convert_date_to_display(const char *iso) {
    int year, month, day;
    if (sscanf(iso, "%d-%d-%d", &year, &month, &day) != 3)
        return NULL;
    char *disp = my_malloc(11);
    if (!disp) return NULL;
    sprintf(disp, "%02d.%02d.%04d", day, month, year);
    return disp;
}

static int validate_time(const char *time_str) {
    int h, m, s;
    if (sscanf(time_str, "%d:%d:%d", &h, &m, &s) != 3) return 0;
    if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) return 0;
    return 1;
}

static Weather string_to_weather(const char *str) {
    if (strcmp(str, "fair") == 0) return WEATHER_FAIR;
    if (strcmp(str, "rain") == 0) return WEATHER_RAIN;
    if (strcmp(str, "cloudy") == 0) return WEATHER_CLOUDY;
    if (strcmp(str, "snow") == 0) return WEATHER_SNOW;
    return -1;
}

static const char* weather_to_string(Weather w) {
    switch (w) {
        case WEATHER_FAIR: return "fair";
        case WEATHER_RAIN: return "rain";
        case WEATHER_CLOUDY: return "cloudy";
        case WEATHER_SNOW: return "snow";
        default: return "unknown";
    }
}

static int parse_conditions(char **tokens, int token_count, int start, char ***conds) {
    *conds = NULL;
    int cond_count = 0;
    if (start < token_count) {
        *conds = my_malloc((token_count - start) * sizeof(char*));
        if (!*conds) return 0;
        for (int i = start; i < token_count; i++) {
            (*conds)[cond_count++] = tokens[i];
        }
    }
    return cond_count;
}

/* ---------- Работа с БД ---------- */
static Record* create_record(void) {
    Record *rec = my_malloc(sizeof(Record));
    if (rec) {
        rec->geo_pos = NULL;
        rec->mea_date = NULL;
        rec->sunrise = NULL;
        rec->sundown = NULL;
        rec->next = NULL;
    }
    return rec;
}

static void free_record(Record *rec) {
    if (!rec) return;
    if (rec->geo_pos) my_free(rec->geo_pos);
    if (rec->mea_date) my_free(rec->mea_date);
    if (rec->sunrise) my_free(rec->sunrise);
    if (rec->sundown) my_free(rec->sundown);
    my_free(rec);
}

static void add_record(Record *rec) {
    if (!db_head) {
        db_head = rec;
        db_tail = rec;
    } else {
        db_tail->next = rec;
        db_tail = rec;
    }
    rec->next = NULL;
}

static void delete_record(Record *prev, Record *curr) {
    if (prev)
        prev->next = curr->next;
    else
        db_head = curr->next;
    if (db_tail == curr)
        db_tail = prev;
    free_record(curr);
}

static int record_matches_conditions(Record *rec, char **conds, int cond_count) {
    for (int i = 0; i < cond_count; i++) {
        char *cond = conds[i];
        printf("DEBUG: checking condition [%s]\n", cond);
        fflush(stdout);
        char *op = NULL;
        if ((op = strstr(cond, ">=")) != NULL) {
            printf("DEBUG: found op >= at pos %ld\n", op - cond);
        } else if ((op = strstr(cond, "<=")) != NULL) {
            printf("DEBUG: found op <=\n");
        } else if ((op = strstr(cond, "!=")) != NULL) {
            printf("DEBUG: found op !=\n");
        } else if ((op = strstr(cond, "=")) != NULL) {
            printf("DEBUG: found op =\n");
        } else if ((op = strstr(cond, ">")) != NULL) {
            printf("DEBUG: found op >\n");
        } else if ((op = strstr(cond, "<")) != NULL) {
            printf("DEBUG: found op <\n");
        } else {
            printf("DEBUG: no operator found\n");
            return 0;
        }

        char field_name[64];
        char value[256];
        int field_len = op - cond;
        if (field_len >= sizeof(field_name)) return 0;
        strncpy(field_name, cond, field_len);
        field_name[field_len] = '\0';
        strcpy(value, op + strlen(op));
        printf("DEBUG: field=[%s], value=[%s]\n", field_name, value);
        fflush(stdout);

        FieldType ft = get_field_type(field_name);
        if (ft == -1) {
            printf("DEBUG: unknown field\n");
            return 0;
        }
        void *ptr = get_field_ptr(rec, field_name);
        if (!ptr) {
            printf("DEBUG: null pointer\n");
            return 0;
        }

        int cmp = compare_values(ft, ptr, op, value);
        printf("DEBUG: compare result = %d\n", cmp);
        fflush(stdout);
        if (!cmp) return 0;
    }
    return 1;
}

static void update_record(Record *rec, char **upd_fields, char **upd_values, int upd_count) {
    for (int i = 0; i < upd_count; i++) {
        const char *field = upd_fields[i];
        const char *value = upd_values[i];
        if (strcmp(field, "geo_id") == 0) {
            rec->geo_id = atoi(value);
        } else if (strcmp(field, "geo_pos") == 0) {
            if (rec->geo_pos) my_free(rec->geo_pos);
            rec->geo_pos = my_malloc(strlen(value) + 1);
            if (rec->geo_pos) strcpy(rec->geo_pos, value);
        } else if (strcmp(field, "mea_date") == 0) {
            char *iso = convert_date_to_iso(value);
            if (rec->mea_date) my_free(rec->mea_date);
            rec->mea_date = iso;
        } else if (strcmp(field, "level") == 0) {
            rec->level = atoi(value);
        } else if (strcmp(field, "sunrise") == 0) {
            if (rec->sunrise) my_free(rec->sunrise);
            rec->sunrise = my_malloc(strlen(value) + 1);
            if (rec->sunrise) strcpy(rec->sunrise, value);
        } else if (strcmp(field, "weather") == 0) {
            rec->weather = string_to_weather(value);
        } else if (strcmp(field, "sundown") == 0) {
            if (rec->sundown) my_free(rec->sundown);
            rec->sundown = my_malloc(strlen(value) + 1);
            if (rec->sundown) strcpy(rec->sundown, value);
        }
    }
}

static Record* find_records(char **conds, int cond_count, int *found_count) {
    Record *result_list = NULL;
    Record *result_tail = NULL;
    int found = 0;
    Record *cur = db_head;
    while (cur) {
        printf("DEBUG: checking record geo_id=%d level=%d\n", cur->geo_id, cur->level);
        fflush(stdout);
        if (record_matches_conditions(cur, conds, cond_count)) {
            printf("DEBUG: record matches\n");
            Record *node = my_malloc(sizeof(Record));
            if (!node) break;
            *node = *cur;
            node->next = NULL;
            if (!result_list) result_list = node;
            else result_tail->next = node;
            result_tail = node;
            found++;
        } else {
            printf("DEBUG: record does NOT match\n");
        }
        cur = cur->next;
    }
    if (found_count) *found_count = found;
    return result_list;
}

static int uniq_by_fields(char **fields, int field_count) {
    int deleted = 0;
    Record *cur = db_head;
    Record *prev = NULL;
    while (cur) {
        Record *next = cur->next;
        int duplicate_found = 0;
        Record *later = cur->next;
        while (later) {
            int equal = 1;
            for (int i = 0; i < field_count; i++) {
                const char *f = fields[i];
                FieldType ft = get_field_type(f);
                void *ptr_a = get_field_ptr(cur, f);
                void *ptr_b = get_field_ptr(later, f);
                if (!ptr_a || !ptr_b) { equal = 0; break; }
                if (ft == TYPE_INT) {
                    if (*(int*)ptr_a != *(int*)ptr_b) { equal = 0; break; }
                } else if (ft == TYPE_STRING || ft == TYPE_DATE || ft == TYPE_TIME) {
                    char *sa = *(char**)ptr_a;
                    char *sb = *(char**)ptr_b;
                    if ((sa == NULL && sb != NULL) || (sa != NULL && sb == NULL)) { equal = 0; break; }
                    if (sa && sb && strcmp(sa, sb) != 0) { equal = 0; break; }
                } else if (ft == TYPE_ENUM) {
                    if (*(Weather*)ptr_a != *(Weather*)ptr_b) { equal = 0; break; }
                }
            }
            if (equal) {
                duplicate_found = 1;
                break;
            }
            later = later->next;
        }
        if (duplicate_found) {
            delete_record(prev, cur);
            deleted++;
            cur = next;
            continue;
        }
        prev = cur;
        cur = next;
    }
    return deleted;
}

static void free_database(void) {
    Record *cur = db_head;
    while (cur) {
        Record *next = cur->next;
        free_record(cur);
        cur = next;
    }
    db_head = NULL;
    db_tail = NULL;
}

/* ---------- Парсеры команд ---------- */
static void print_incorrect(FILE *out, const char *cmd_line) {
    char first20[21];
    strncpy(first20, cmd_line, 20);
    first20[20] = '\0';
    fprintf(out, "incorrect %s\n", first20);
    fprintf(stdout, "incorrect %s\n", first20);
}

static void parse_insert(FILE *out, const char *line) {
    const char *args = line + 6; // пропускаем "insert"
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
        printf("DEBUG: insert successful (geo_id=%d, level=%d)\n", rec->geo_id, rec->level);
        fflush(stdout);
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
    printf("DEBUG: select: cond_count=%d\n", cond_count);
    for (int i = 0; i < cond_count; i++) {
        printf("DEBUG: cond[%d]=[%s]\n", i, conds[i]);
    }
    fflush(stdout);

    int found = 0;
    Record *result = find_records(conds, cond_count, &found);

    fprintf(out, "select: %d\n", found);
    fprintf(stdout, "select: %d\n", found);

    Record *cur = result;
    while (cur) {
        for (int i = 0; i < field_count; i++) {
            const char *f = fields[i];
            if (i > 0) {
                fprintf(out, " ");
                fprintf(stdout, " ");
            }
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
        cur = cur->next;
    }

    cur = result;
    while (cur) {
        Record *tmp = cur;
        cur = cur->next;
        my_free(tmp);
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

static void process_command(FILE *out, char *line) {
    while (*line == ' ') line++;
    if (*line == '\0') return;

    // Копируем строку для разбора токенов, чтобы не повредить оригинал
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

/* ---------- main ---------- */
int main() {
    FILE *out = fopen("output.txt", "w");
    if (!out) {
        fprintf(stderr, "Cannot open output.txt\n");
        return 1;
    }

    FILE *in = fopen("input.txt", "r");
    if (in) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), in)) {
            size_t len = strlen(buffer);
            while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
                buffer[--len] = '\0';
            process_command(out, buffer);
        }
        fclose(in);
    }

    printf("Enter commands (Ctrl+Z to finish):\n");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
            buffer[--len] = '\0';
        process_command(out, buffer);
    }

    fclose(out);
    free_database();
    write_memstat();
    return 0;
}