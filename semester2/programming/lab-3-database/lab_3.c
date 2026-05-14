#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- Управление памятью ---------- */
static int malloc_count = 0;
static int calloc_count = 0;
static int realloc_count = 0;
static int free_count = 0;

void* my_malloc(size_t size) { malloc_count++; return malloc(size); }
void* my_calloc(size_t n, size_t s) { calloc_count++; return calloc(n, s); }
void* my_realloc(void *p, size_t s) {
    if (p == NULL) malloc_count++;
    else realloc_count++;
    return realloc(p, s);
}
void my_free(void *p) { if (p) { free_count++; free(p); } }
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
typedef enum { WEATHER_FAIR, WEATHER_RAIN, WEATHER_CLOUDY, WEATHER_SNOW } Weather;
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

static Record *db_head = NULL, *db_tail = NULL;

/* ---------- Вспомогательные функции ---------- */
typedef enum { TYPE_INT, TYPE_STRING, TYPE_DATE, TYPE_TIME, TYPE_ENUM } FieldType;

static FieldType get_field_type(const char *f) {
    if (strcmp(f, "geo_id") == 0) return TYPE_INT;
    if (strcmp(f, "geo_pos") == 0) return TYPE_STRING;
    if (strcmp(f, "mea_date") == 0) return TYPE_DATE;
    if (strcmp(f, "level") == 0) return TYPE_INT;
    if (strcmp(f, "sunrise") == 0) return TYPE_TIME;
    if (strcmp(f, "weather") == 0) return TYPE_ENUM;
    if (strcmp(f, "sundown") == 0) return TYPE_TIME;
    return -1;
}
static void* get_field_ptr(Record *r, const char *f) {
    if (strcmp(f, "geo_id") == 0) return &r->geo_id;
    if (strcmp(f, "geo_pos") == 0) return &r->geo_pos;
    if (strcmp(f, "mea_date") == 0) return &r->mea_date;
    if (strcmp(f, "level") == 0) return &r->level;
    if (strcmp(f, "sunrise") == 0) return &r->sunrise;
    if (strcmp(f, "weather") == 0) return &r->weather;
    if (strcmp(f, "sundown") == 0) return &r->sundown;
    return NULL;
}
static int compare_values(FieldType t, void *val, const char *op, const char *vstr) {
    if (t == TYPE_INT) {
        int a = *(int*)val, b = atoi(vstr);
        if (strcmp(op, "=") == 0) return a == b;
        if (strcmp(op, "!=") == 0) return a != b;
        if (strcmp(op, ">") == 0) return a > b;
        if (strcmp(op, "<") == 0) return a < b;
        if (strcmp(op, ">=") == 0) return a >= b;
        if (strcmp(op, "<=") == 0) return a <= b;
    } else if (t == TYPE_STRING || t == TYPE_DATE || t == TYPE_TIME) {
        char *s = *(char**)val;
        if (!s) return 0;
        if (strcmp(op, "=") == 0) return strcmp(s, vstr) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(s, vstr) != 0;
        if (strcmp(op, ">") == 0) return strcmp(s, vstr) > 0;
        if (strcmp(op, "<") == 0) return strcmp(s, vstr) < 0;
        if (strcmp(op, ">=") == 0) return strcmp(s, vstr) >= 0;
        if (strcmp(op, "<=") == 0) return strcmp(s, vstr) <= 0;
    } else if (t == TYPE_ENUM) {
        Weather a = *(Weather*)val, b;
        if (strcmp(vstr, "fair") == 0) b = WEATHER_FAIR;
        else if (strcmp(vstr, "rain") == 0) b = WEATHER_RAIN;
        else if (strcmp(vstr, "cloudy") == 0) b = WEATHER_CLOUDY;
        else if (strcmp(vstr, "snow") == 0) b = WEATHER_SNOW;
        else return 0;
        if (strcmp(op, "=") == 0) return a == b;
        if (strcmp(op, "!=") == 0) return a != b;
    }
    return 0;
}
static char* date_to_iso(const char *d) {
    int day, mon, year;
    if (sscanf(d, "%d.%d.%d", &day, &mon, &year) != 3) return NULL;
    char *iso = my_malloc(11);
    if (iso) sprintf(iso, "%04d-%02d-%02d", year, mon, day);
    return iso;
}
static char* date_to_display(const char *iso) {
    int y, m, d;
    if (sscanf(iso, "%d-%d-%d", &y, &m, &d) != 3) return NULL;
    char *disp = my_malloc(11);
    if (disp) sprintf(disp, "%02d.%02d.%04d", d, m, y);
    return disp;
}
static int valid_time(const char *t) {
    int h, m, s;
    if (sscanf(t, "%d:%d:%d", &h, &m, &s) != 3) return 0;
    return (h>=0 && h<=23 && m>=0 && m<=59 && s>=0 && s<=59);
}
static Weather str_to_weather(const char *s) {
    if (strcmp(s, "fair") == 0) return WEATHER_FAIR;
    if (strcmp(s, "rain") == 0) return WEATHER_RAIN;
    if (strcmp(s, "cloudy") == 0) return WEATHER_CLOUDY;
    if (strcmp(s, "snow") == 0) return WEATHER_SNOW;
    return -1;
}
static const char* weather_to_str(Weather w) {
    switch(w) {
        case WEATHER_FAIR: return "fair";
        case WEATHER_RAIN: return "rain";
        case WEATHER_CLOUDY: return "cloudy";
        case WEATHER_SNOW: return "snow";
        default: return "unknown";
    }
}
static int parse_conditions(char **toks, int n, int start, char ***conds) {
    *conds = NULL;
    int cnt = 0;
    if (start < n) {
        *conds = my_malloc((n - start) * sizeof(char*));
        if (!*conds) return 0;
        for (int i = start; i < n; i++) (*conds)[cnt++] = toks[i];
    }
    return cnt;
}

/* ---------- Операции с БД ---------- */
static Record* create_record(void) {
    Record *r = my_malloc(sizeof(Record));
    if (r) { r->geo_pos = r->mea_date = r->sunrise = r->sundown = NULL; r->next = NULL; }
    return r;
}
static void free_record(Record *r) {
    if (!r) return;
    if (r->geo_pos) my_free(r->geo_pos);
    if (r->mea_date) my_free(r->mea_date);
    if (r->sunrise) my_free(r->sunrise);
    if (r->sundown) my_free(r->sundown);
    my_free(r);
}
static void add_record(Record *r) {
    if (!db_head) db_head = db_tail = r;
    else { db_tail->next = r; db_tail = r; }
    r->next = NULL;
}
static void delete_record(Record *prev, Record *curr) {
    if (prev) prev->next = curr->next;
    else db_head = curr->next;
    if (db_tail == curr) db_tail = prev;
    free_record(curr);
}
static int matches(Record *r, char **conds, int cnt) {
    for (int i = 0; i < cnt; i++) {
        char *cond = conds[i];
        char *op = NULL;
        if ((op = strstr(cond, ">=")) != NULL) {}
        else if ((op = strstr(cond, "<=")) != NULL) {}
        else if ((op = strstr(cond, "!=")) != NULL) {}
        else if ((op = strstr(cond, "=")) != NULL) {}
        else if ((op = strstr(cond, ">")) != NULL) {}
        else if ((op = strstr(cond, "<")) != NULL) {}
        else return 0;
        char field[64], value[256];
        int flen = op - cond;
        if (flen >= 64) return 0;
        strncpy(field, cond, flen);
        field[flen] = '\0';
        const char *vstart = op + strlen(op);
        while (*vstart == ' ') vstart++;
        strcpy(value, vstart);
        FieldType ft = get_field_type(field);
        if (ft == -1) return 0;
        void *ptr = get_field_ptr(r, field);
        if (!ptr) return 0;
        if (!compare_values(ft, ptr, op, value)) return 0;
    }
    return 1;
}
static void update_record(Record *r, char **fields, char **vals, int cnt) {
    for (int i = 0; i < cnt; i++) {
        const char *f = fields[i], *v = vals[i];
        if (strcmp(f, "geo_id") == 0) r->geo_id = atoi(v);
        else if (strcmp(f, "geo_pos") == 0) {
            if (r->geo_pos) my_free(r->geo_pos);
            r->geo_pos = my_malloc(strlen(v)+1);
            if (r->geo_pos) strcpy(r->geo_pos, v);
        } else if (strcmp(f, "mea_date") == 0) {
            char *iso = date_to_iso(v);
            if (r->mea_date) my_free(r->mea_date);
            r->mea_date = iso;
        } else if (strcmp(f, "level") == 0) r->level = atoi(v);
        else if (strcmp(f, "sunrise") == 0) {
            if (r->sunrise) my_free(r->sunrise);
            r->sunrise = my_malloc(strlen(v)+1);
            if (r->sunrise) strcpy(r->sunrise, v);
        } else if (strcmp(f, "weather") == 0) r->weather = str_to_weather(v);
        else if (strcmp(f, "sundown") == 0) {
            if (r->sundown) my_free(r->sundown);
            r->sundown = my_malloc(strlen(v)+1);
            if (r->sundown) strcpy(r->sundown, v);
        }
    }
}
static int uniq_by_fields(char **fields, int fcnt) {
    int deleted = 0;
    Record *cur = db_head, *prev = NULL;
    while (cur) {
        Record *next = cur->next;
        int dup = 0;
        Record *later = cur->next;
        while (later) {
            int eq = 1;
            for (int i = 0; i < fcnt; i++) {
                const char *f = fields[i];
                FieldType ft = get_field_type(f);
                void *a = get_field_ptr(cur, f);
                void *b = get_field_ptr(later, f);
                if (!a || !b) { eq = 0; break; }
                if (ft == TYPE_INT) {
                    if (*(int*)a != *(int*)b) { eq = 0; break; }
                } else if (ft == TYPE_STRING || ft == TYPE_DATE || ft == TYPE_TIME) {
                    char *sa = *(char**)a, *sb = *(char**)b;
                    if ((sa == NULL && sb != NULL) || (sa != NULL && sb == NULL)) { eq = 0; break; }
                    if (sa && sb && strcmp(sa, sb) != 0) { eq = 0; break; }
                } else if (ft == TYPE_ENUM) {
                    if (*(Weather*)a != *(Weather*)b) { eq = 0; break; }
                }
            }
            if (eq) { dup = 1; break; }
            later = later->next;
        }
        if (dup) {
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
static void free_db(void) {
    Record *cur = db_head;
    while (cur) { Record *next = cur->next; free_record(cur); cur = next; }
    db_head = db_tail = NULL;
}

/* ---------- Парсеры команд ---------- */
static void print_incorrect(FILE *out, const char *line) {
    char buf[21];
    strncpy(buf, line, 20);
    buf[20] = '\0';
    fprintf(out, "incorrect %s\n", buf);
    fprintf(stdout, "incorrect %s\n", buf);
}

static void parse_insert(FILE *out, const char *line) {
    const char *args = line + 6;
    while (*args == ' ') args++;

    Record *rec = create_record();
    if (!rec) return;

    int set[7] = {0};
    char *work = my_malloc(strlen(args)+1);
    if (!work) { free_record(rec); return; }
    strcpy(work, args);
    char *tok = strtok(work, ",");
    int err = 0;
    while (tok && !err) {
        char *eq = strchr(tok, '=');
        if (!eq) { err = 1; break; }
        *eq = '\0';
        char *field = tok, *val = eq+1;
        int idx = -1;
        if (strcmp(field, "geo_id") == 0) idx = 0;
        else if (strcmp(field, "geo_pos") == 0) idx = 1;
        else if (strcmp(field, "mea_date") == 0) idx = 2;
        else if (strcmp(field, "level") == 0) idx = 3;
        else if (strcmp(field, "sunrise") == 0) idx = 4;
        else if (strcmp(field, "weather") == 0) idx = 5;
        else if (strcmp(field, "sundown") == 0) idx = 6;
        else { err = 1; break; }
        if (set[idx]) { err = 1; break; }
        set[idx] = 1;
        if (idx == 0) {
            char *end; long v = strtol(val, &end, 10);
            if (*end) err = 1; else rec->geo_id = (int)v;
        } else if (idx == 1) {
            rec->geo_pos = my_malloc(strlen(val)+1);
            if (rec->geo_pos) strcpy(rec->geo_pos, val); else err = 1;
        } else if (idx == 2) {
            char *iso = date_to_iso(val);
            if (!iso) err = 1; else rec->mea_date = iso;
        } else if (idx == 3) {
            char *end; long v = strtol(val, &end, 10);
            if (*end) err = 1; else rec->level = (int)v;
        } else if (idx == 4) {
            if (!valid_time(val)) err = 1;
            else { rec->sunrise = my_malloc(strlen(val)+1); if (rec->sunrise) strcpy(rec->sunrise, val); else err = 1; }
        } else if (idx == 5) {
            Weather w = str_to_weather(val);
            if (w == -1) err = 1; else rec->weather = w;
        } else if (idx == 6) {
            if (!valid_time(val)) err = 1;
            else { rec->sundown = my_malloc(strlen(val)+1); if (rec->sundown) strcpy(rec->sundown, val); else err = 1; }
        }
        tok = strtok(NULL, ",");
    }
    int all = 1;
    for (int i=0; i<7; i++) if (!set[i]) all = 0;
    if (err || !all) {
        if (rec->geo_pos) my_free(rec->geo_pos);
        if (rec->mea_date) my_free(rec->mea_date);
        if (rec->sunrise) my_free(rec->sunrise);
        if (rec->sundown) my_free(rec->sundown);
        my_free(rec);
        print_incorrect(out, line);
    } else add_record(rec);
    my_free(work);
}

static void parse_select(FILE *out, char **tokens, int n) {
    if (n < 2) { print_incorrect(out, "select"); return; }
    char *fields_str = tokens[1];
    char *fields[20];
    int fcnt = 0;
    char *copy = my_malloc(strlen(fields_str)+1);
    if (!copy) return;
    strcpy(copy, fields_str);
    char *tok = strtok(copy, ",");
    while (tok && fcnt < 20) { fields[fcnt++] = tok; tok = strtok(NULL, ","); }
    my_free(copy);

    char **conds = NULL;
    int condcnt = parse_conditions(tokens, n, 2, &conds);

    // Первый проход: подсчёт
    int found = 0;
    Record *cur = db_head;
    while (cur) {
        if (matches(cur, conds, condcnt)) found++;
        cur = cur->next;
    }
    fprintf(out, "select: %d\n", found);
    fprintf(stdout, "select: %d\n", found);

    // Второй проход: вывод
    cur = db_head;
    while (cur) {
        if (matches(cur, conds, condcnt)) {
            for (int i = 0; i < fcnt; i++) {
                if (i > 0) { fprintf(out, " "); fprintf(stdout, " "); }
                const char *f = fields[i];
                if (strcmp(f, "geo_id") == 0) {
                    fprintf(out, "geo_id=%d", cur->geo_id);
                    fprintf(stdout, "geo_id=%d", cur->geo_id);
                } else if (strcmp(f, "geo_pos") == 0) {
                    fprintf(out, "geo_pos=%s", cur->geo_pos ? cur->geo_pos : "");
                    fprintf(stdout, "geo_pos=%s", cur->geo_pos ? cur->geo_pos : "");
                } else if (strcmp(f, "mea_date") == 0) {
                    char *d = date_to_display(cur->mea_date);
                    fprintf(out, "mea_date=%s", d ? d : "");
                    fprintf(stdout, "mea_date=%s", d ? d : "");
                    if (d) my_free(d);
                } else if (strcmp(f, "level") == 0) {
                    fprintf(out, "level=%d", cur->level);
                    fprintf(stdout, "level=%d", cur->level);
                } else if (strcmp(f, "sunrise") == 0) {
                    fprintf(out, "sunrise=%s", cur->sunrise ? cur->sunrise : "");
                    fprintf(stdout, "sunrise=%s", cur->sunrise ? cur->sunrise : "");
                } else if (strcmp(f, "weather") == 0) {
                    fprintf(out, "weather=%s", weather_to_str(cur->weather));
                    fprintf(stdout, "weather=%s", weather_to_str(cur->weather));
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

static void parse_delete(FILE *out, char **tokens, int n) {
    if (n < 2) { print_incorrect(out, "delete"); return; }
    char **conds = NULL;
    int condcnt = parse_conditions(tokens, n, 1, &conds);
    int del = 0;
    Record *prev = NULL, *cur = db_head;
    while (cur) {
        Record *next = cur->next;
        if (matches(cur, conds, condcnt)) {
            delete_record(prev, cur);
            del++;
            cur = next;
            continue;
        }
        prev = cur;
        cur = next;
    }
    fprintf(out, "delete: %d\n", del);
    fprintf(stdout, "delete: %d\n", del);
    if (conds) my_free(conds);
}

static void parse_update(FILE *out, char **tokens, int n) {
    if (n < 2) { print_incorrect(out, "update"); return; }
    char *upd_str = tokens[1];
    char *upd_fields[20], *upd_values[20];
    int upd_cnt = 0;
    char *copy = my_malloc(strlen(upd_str)+1);
    if (!copy) return;
    strcpy(copy, upd_str);
    char *tok = strtok(copy, ",");
    int err = 0;
    while (tok && upd_cnt < 20 && !err) {
        char *eq = strchr(tok, '=');
        if (!eq) { err = 1; break; }
        *eq = '\0';
        upd_fields[upd_cnt] = tok;
        upd_values[upd_cnt] = eq+1;
        if (get_field_type(upd_fields[upd_cnt]) == -1) err = 1;
        upd_cnt++;
        tok = strtok(NULL, ",");
    }
    if (err) { my_free(copy); print_incorrect(out, "update"); return; }
    char **conds = NULL;
    int condcnt = parse_conditions(tokens, n, 2, &conds);
    int updated = 0;
    Record *cur = db_head;
    while (cur) {
        if (matches(cur, conds, condcnt)) {
            update_record(cur, upd_fields, upd_values, upd_cnt);
            updated++;
        }
        cur = cur->next;
    }
    fprintf(out, "update: %d\n", updated);
    fprintf(stdout, "update: %d\n", updated);
    my_free(copy);
    if (conds) my_free(conds);
}

static void parse_uniq(FILE *out, char **tokens, int n) {
    if (n != 2) { print_incorrect(out, "uniq"); return; }
    char *fields_str = tokens[1];
    char *fields[20];
    int fcnt = 0;
    char *copy = my_malloc(strlen(fields_str)+1);
    if (!copy) return;
    strcpy(copy, fields_str);
    char *tok = strtok(copy, ",");
    while (tok && fcnt < 20) {
        if (get_field_type(tok) == -1) { my_free(copy); print_incorrect(out, "uniq"); return; }
        fields[fcnt++] = tok;
        tok = strtok(NULL, ",");
    }
    my_free(copy);
    int del = uniq_by_fields(fields, fcnt);
    fprintf(out, "uniq: %d\n", del);
    fprintf(stdout, "uniq: %d\n", del);
}

static void process_command(FILE *out, char *line) {
    while (*line == ' ') line++;
    if (*line == '\0') return;
    char line_copy[1024];
    strcpy(line_copy, line);
    char *tokens[100];
    int n = 0;
    char *tok = strtok(line_copy, " ");
    while (tok && n < 100) { tokens[n++] = tok; tok = strtok(NULL, " "); }
    if (n == 0) return;
    const char *cmd = tokens[0];
    if (strcmp(cmd, "insert") == 0) parse_insert(out, line);
    else if (strcmp(cmd, "select") == 0) parse_select(out, tokens, n);
    else if (strcmp(cmd, "delete") == 0) parse_delete(out, tokens, n);
    else if (strcmp(cmd, "update") == 0) parse_update(out, tokens, n);
    else if (strcmp(cmd, "uniq") == 0) parse_uniq(out, tokens, n);
    else print_incorrect(out, line);
}

/* ---------- main ---------- */
int main() {
    FILE *out = fopen("output.txt", "w");
    if (!out) { fprintf(stderr, "Cannot open output.txt\n"); return 1; }
    FILE *in = fopen("input.txt", "r");
    if (in) {
        char buf[1024];
        while (fgets(buf, sizeof(buf), in)) {
            size_t len = strlen(buf);
            while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
            process_command(out, buf);
        }
        fclose(in);
    }
    printf("Enter commands (Ctrl+Z to finish):\n");
    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
        process_command(out, buf);
    }
    fclose(out);
    free_db();
    write_memstat();
    return 0;
}