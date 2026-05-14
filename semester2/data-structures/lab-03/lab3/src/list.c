#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "../include/types.h"
#include "../include/memstat.h"
#include "../include/cmd.h"

static void out(FILE *f, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt); vfprintf(f,      fmt, ap); va_end(ap);
    va_start(ap, fmt); vfprintf(stdout, fmt, ap); va_end(ap);
}

static const char *weather_str(weather_type w)
{
    switch (w)
    {
        case fair:   return "fair";
        case rain:   return "rain";
        case cloudy: return "cloudy";
        case snow:   return "snow";
        default:     return "unknown";
    }
}

static int str_to_weather(const char *s, weather_type *out_w)
{
    if (!strcmp(s,"fair"))   { *out_w = fair;   return 1; }
    if (!strcmp(s,"rain"))   { *out_w = rain;   return 1; }
    if (!strcmp(s,"cloudy")) { *out_w = cloudy; return 1; }
    if (!strcmp(s,"snow"))   { *out_w = snow;   return 1; }
    return 0;
}

// Парсинг даты в формате dd.mm.yyyy (время устанавливается в 00:00:00)
static int parse_date(const char *s, time_t *t)
{
    struct tm tm = {0};
    int d, mo, y;
    if (sscanf(s, "%d.%d.%d", &d, &mo, &y) != 3) return 0;
    tm.tm_mday = d;
    tm.tm_mon = mo - 1;
    tm.tm_year = y - 1900;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    *t = mktime(&tm);
    return (*t != (time_t)-1);
}

// Парсинг времени в формате hh:mm (дата фиксированная 1970-01-01)
static int parse_time_of_day(const char *s, time_t *t)
{
    struct tm tm = {0};
    int h, m;
    if (sscanf(s, "%d:%d", &h, &m) != 2) return 0;
    tm.tm_year = 70;   // 1970
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_hour = h;
    tm.tm_min = m;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    *t = mktime(&tm);
    return (*t != (time_t)-1);
}

// Форматирование даты (без времени)
static void fmt_date(time_t t, char *buf, size_t sz)
{
    struct tm *tm = localtime(&t);
    strftime(buf, sz, "%d.%m.%Y", tm);
}

// Форматирование времени (без даты)
static void fmt_time_of_day(time_t t, char *buf, size_t sz)
{
    struct tm *tm = localtime(&t);
    strftime(buf, sz, "%H:%M", tm);
}

static const char *FIELDS[] =
{
    "geo_id","geo_pos","mea_date","level","sunrise","weather","sundown", NULL
};

static int valid_field(const char *f)
{
    for (int i = 0; FIELDS[i]; i++)
        if (!strcmp(f, FIELDS[i])) return 1;
    return 0;
}

static void get_field(const weather_station *s, const char *f, char *buf, size_t sz)
{
    if      (!strcmp(f,"geo_id"))   snprintf(buf, sz, "%d",   s->geo_id);
    else if (!strcmp(f,"geo_pos"))  snprintf(buf, sz, "%s",   s->geo_pos);
    else if (!strcmp(f,"mea_date")) fmt_date(s->mea_date, buf, sz);
    else if (!strcmp(f,"level"))    snprintf(buf, sz, "%d",   s->level);
    else if (!strcmp(f,"sunrise"))  fmt_time_of_day(s->sunrise, buf, sz);
    else if (!strcmp(f,"weather"))  snprintf(buf, sz, "%s",   weather_str(s->weather));
    else if (!strcmp(f,"sundown"))  fmt_time_of_day(s->sundown, buf, sz);
    else                            snprintf(buf, sz, "?");
}

static int set_field(weather_station *s, const char *f, const char *v)
{
    if (!strcmp(f,"geo_id")) {
        char *e; long n = strtol(v, &e, 10);
        if (*e) return 0;
        s->geo_id = (int)n;
    } else if (!strcmp(f,"geo_pos")) {
        strncpy(s->geo_pos, v, MAX_NAME-1); s->geo_pos[MAX_NAME-1] = '\0';
    } else if (!strcmp(f,"mea_date")) {
        if (!parse_date(v, &s->mea_date)) return 0;
    } else if (!strcmp(f,"level")) {
        char *e; long n = strtol(v, &e, 10);
        if (*e) return 0;
        s->level = (int)n;
    } else if (!strcmp(f,"sunrise")) {
        if (!parse_time_of_day(v, &s->sunrise)) return 0;
    } else if (!strcmp(f,"weather")) {
        if (!str_to_weather(v, &s->weather)) return 0;
    } else if (!strcmp(f,"sundown")) {
        if (!parse_time_of_day(v, &s->sundown)) return 0;
    } else {
        return 0;
    }
    return 1;
}

static int parse_fv(const char *src, fv_pair *pairs)
{
    char buf[4096];
    strncpy(buf, src, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
    int n = 0;
    char *tok = strtok(buf, ",");
    while (tok && n < MAX_FV)
    {
        char *eq = strchr(tok, '=');
        if (!eq) return -1;
        *eq = '\0';
        strncpy(pairs[n].f, tok,   sizeof(pairs[n].f)-1);
        strncpy(pairs[n].v, eq+1,  sizeof(pairs[n].v)-1);
        n++;
        tok = strtok(NULL, ",");
    }
    return n;
}

static int parse_names(const char *src, char names[][64], int max)
{
    char buf[1024];
    strncpy(buf, src, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
    int n = 0;
    char *tok = strtok(buf, ",");
    while (tok && n < max)
    {
        strncpy(names[n], tok, 63); names[n][63] = '\0';
        n++;
        tok = strtok(NULL, ",");
    }
    return n;
}

static void strip_quotes(char *s)
{
    size_t len = strlen(s);
    if (len >= 2 && s[0] == '\'' && s[len-1] == '\'')
    {
        memmove(s, s+1, len-2);
        s[len-2] = '\0';
    }
}

/* Парсит одно условие, возвращает 1/0 */
static int parse_cond(const char *src, cond *c)
{
    /* /in/ */
    const char *sl = strchr(src, '/');
    if (sl) {
        size_t fl = sl - src;
        strncpy(c->f, src, fl < 63 ? fl : 63); c->f[fl < 63 ? fl : 63] = '\0';
        const char *sl2 = strchr(sl+1, '/');
        if (!sl2) return 0;
        char op[32]; size_t ol = sl2 - sl - 1;
        strncpy(op, sl+1, ol < 31 ? ol : 31); op[ol < 31 ? ol : 31] = '\0';
        if (!strcmp(op,"in")) c->op = OP_IN; else return 0;
        strncpy(c->v, sl2+1, 255); c->v[255] = '\0';
        return 1;
    }
    /* двухсимвольные операторы первыми, чтобы не срабатывал '<' раньше '<=' */
    static const struct { const char *str; op_t op; } OPS[] = {
        {"<=",OP_LE},{">=",OP_GE},{"!=",OP_NE},{"<",OP_LT},{">",OP_GT},{"=",OP_EQ}
    };
    for (int i = 0; i < 6; i++) {
        const char *pos = strstr(src, OPS[i].str);
        if (!pos) continue;
        size_t fl = pos - src;
        strncpy(c->f, src, fl < 63 ? fl : 63); c->f[fl < 63 ? fl : 63] = '\0';
        c->op = OPS[i].op;
        strncpy(c->v, pos + strlen(OPS[i].str), 255); c->v[255] = '\0';
        strip_quotes(c->v);
        return 1;
    }
    return 0;
}

/* Разбивает оставшуюся строку на условия через пробел */
static int parse_conds(char *rest, cond *conds) {
    int n = 0;
    char *tok = strtok(rest, " \t");
    while (tok && n < MAX_CONDS) {
        if (!parse_cond(tok, &conds[n])) return -1;
        n++;
        tok = strtok(NULL, " \t");
    }
    return n;
}

/* Вычисление одного условия */
static int eval_cond(const weather_station *s, const cond *c) {
    char fval[256];
    get_field(s, c->f, fval, sizeof(fval));

    if (c->op == OP_IN) {
        /* ['val1','val2',...] */
        char list[256];
        strncpy(list, c->v, 255);
        char *p = list;
        if (*p == '[') p++;
        size_t len = strlen(p);
        if (len && p[len-1] == ']') p[len-1] = '\0';
        char *tok = strtok(p, ",");
        while (tok) {
            char v[256]; strncpy(v, tok, 255); strip_quotes(v);
            if (!strcmp(fval, v)) return 1;
            tok = strtok(NULL, ",");
        }
        return 0;
    }

    /* сравнение: числовое для geo_id/level, время для mea_date/sunrise/sundown, иначе строка */
    int cmp;
    if (!strcmp(c->f,"geo_id") || !strcmp(c->f,"level")) {
        long a = atol(fval), b = atol(c->v);
        cmp = (a > b) - (a < b);
    } else if (!strcmp(c->f,"mea_date") || !strcmp(c->f,"sunrise") || !strcmp(c->f,"sundown")) {
        time_t ta, tb;
        // Для mea_date нужно парсить как дату, для времени как время
        if (!strcmp(c->f,"mea_date")) {
            if (!parse_date(fval, &ta) || !parse_date(c->v, &tb)) return 0;
        } else {
            if (!parse_time_of_day(fval, &ta) || !parse_time_of_day(c->v, &tb)) return 0;
        }
        cmp = (ta > tb) - (ta < tb);
    } else if (!strcmp(c->f,"weather")) {
        cmp = strcmp(fval, c->v);
    } else {
        cmp = strcmp(fval, c->v);
    }

    switch (c->op) {
        case OP_LT: return cmp <  0;
        case OP_LE: return cmp <= 0;
        case OP_GT: return cmp >  0;
        case OP_GE: return cmp >= 0;
        case OP_EQ: return cmp == 0;
        case OP_NE: return cmp != 0;
        default:    return 0;
    }
}

static int eval_all(const weather_station *s, cond *conds, int n) {
    for (int i = 0; i < n; i++)
        if (!eval_cond(s, &conds[i])) return 0;
    return 1;
}

static node *new_node(const weather_station *s) {
    node *n = MALLOC(sizeof(node));
    if (!n) return NULL;
    n->data = *s; n->next = NULL;
    return n;
}

static void list_append(list *db, const weather_station *s) {
    node *n = new_node(s); if (!n) return;
    if (!db->head) { db->head = n; }
    else {
        node *cur = db->head;
        while (cur->next) cur = cur->next;
        cur->next = n;
    }
    db->size++;
}

static void print_record(FILE *f, const weather_station *s,
                          char names[][64], int nnames) {
    for (int i = 0; i < nnames; i++) {
        char v[256]; get_field(s, names[i], v, sizeof(v));
        out(f, "%s%s=%s", i ? " " : "", names[i], v);
    }
    out(f, "\n");
}

/* ── команды ──────────────────────────────────────────────────────── */

static void cmd_insert(const char *args, list *db, FILE *f) {
    fv_pair pairs[MAX_FV];
    int n = parse_fv(args, pairs);
    if (n < 0) { out(f, "incorrect %.20s\n", args); return; }

    /* все 7 полей, без дублей */
    int found[7] = {0};
    for (int i = 0; i < n; i++) {
        int idx = -1;
        for (int j = 0; FIELDS[j]; j++)
            if (!strcmp(pairs[i].f, FIELDS[j])) { idx = j; break; }
        if (idx < 0 || !valid_field(pairs[i].f)) {
            out(f, "incorrect %.20s\n", args); return;
        }
        if (found[idx]) { out(f, "incorrect %.20s\n", args); return; }
        found[idx] = 1;
    }
    for (int j = 0; FIELDS[j]; j++)
        if (!found[j]) { out(f, "incorrect %.20s\n", args); return; }

    weather_station s; memset(&s, 0, sizeof(s));
    for (int i = 0; i < n; i++)
        if (!set_field(&s, pairs[i].f, pairs[i].v)) {
            out(f, "incorrect %.20s\n", args); return;
        }

    list_append(db, &s);
    out(f, "insert: ok\n");
}

static void cmd_select(const char *args, list *db, FILE *f) {
    char buf[4096];
    strncpy(buf, args, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';

    /* первый токен — имена полей */
    char *sp = buf; while (*sp == ' ') sp++;
    char *fe = sp;  while (*fe && *fe != ' ' && *fe != '\t') fe++;
    char names_str[512]; size_t fl = fe - sp;
    strncpy(names_str, sp, fl < 511 ? fl : 511);
    names_str[fl < 511 ? fl : 511] = '\0';

    char names[MAX_NAMES][64]; int nn = parse_names(names_str, names, MAX_NAMES);
    for (int i = 0; i < nn; i++)
        if (!valid_field(names[i])) { out(f, "incorrect %.20s\n", args); return; }

    cond conds[MAX_CONDS]; int nc = 0;
    if (*fe) {
        char rest[4096]; strncpy(rest, fe+1, sizeof(rest)-1);
        nc = parse_conds(rest, conds);
        if (nc < 0) { out(f, "incorrect %.20s\n", args); return; }
    }

    int cnt = 0;
    for (node *cur = db->head; cur; cur = cur->next)
        if (eval_all(&cur->data, conds, nc)) cnt++;

    out(f, "select: %d\n", cnt);
    for (node *cur = db->head; cur; cur = cur->next)
        if (eval_all(&cur->data, conds, nc))
            print_record(f, &cur->data, names, nn);
}

static void cmd_delete(const char *args, list *db, FILE *f) {
    cond conds[MAX_CONDS]; int nc = 0;
    if (args && *args) {
        char buf[4096]; strncpy(buf, args, sizeof(buf)-1);
        nc = parse_conds(buf, conds);
        if (nc < 0) { out(f, "incorrect %.20s\n", args); return; }
    }

    int del = 0;
    node **pp = &db->head;
    while (*pp) {
        node *cur = *pp;
        if (eval_all(&cur->data, conds, nc)) {
            *pp = cur->next; FREE(cur); db->size--; del++;
        } else { pp = &cur->next; }
    }
    out(f, "delete: %d\n", del);
}

static void cmd_update(const char *args, list *db, FILE *f) {
    char buf[4096];
    strncpy(buf, args, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';

    char *sp = buf; while (*sp == ' ') sp++;
    char *fe = sp;  while (*fe && *fe != ' ' && *fe != '\t') fe++;

    char fv_str[2048]; size_t fl = fe - sp;
    strncpy(fv_str, sp, fl < 2047 ? fl : 2047);
    fv_str[fl < 2047 ? fl : 2047] = '\0';

    fv_pair pairs[MAX_FV]; int n = parse_fv(fv_str, pairs);
    if (n < 0) { out(f, "incorrect %.20s\n", args); return; }
    for (int i = 0; i < n; i++)
        if (!valid_field(pairs[i].f)) { out(f, "incorrect %.20s\n", args); return; }

    cond conds[MAX_CONDS]; int nc = 0;
    if (*fe) {
        char rest[4096]; strncpy(rest, fe+1, sizeof(rest)-1);
        nc = parse_conds(rest, conds);
        if (nc < 0) { out(f, "incorrect %.20s\n", args); return; }
    }

    int upd = 0;
    for (node *cur = db->head; cur; cur = cur->next) {
        if (eval_all(&cur->data, conds, nc)) {
            for (int i = 0; i < n; i++)
                set_field(&cur->data, pairs[i].f, pairs[i].v);
            upd++;
        }
    }
    out(f, "update: %d\n", upd);
}

static void cmd_uniq(const char *args, list *db, FILE *f) {
    char buf[1024]; strncpy(buf, args, sizeof(buf)-1);
    char names[MAX_NAMES][64]; int nn = parse_names(buf, names, MAX_NAMES);
    for (int i = 0; i < nn; i++)
        if (!valid_field(names[i])) { out(f, "incorrect %.20s\n", args); return; }

    int del = 0;
    node **pp = &db->head;
    while (*pp) {
        node *cur = *pp;
        /* ищем дубликат позже в списке */
        int dup = 0;
        for (node *later = cur->next; later && !dup; later = later->next) {
            int match = 1;
            for (int i = 0; i < nn && match; i++) {
                char v1[256], v2[256];
                get_field(&cur->data,   names[i], v1, sizeof(v1));
                get_field(&later->data, names[i], v2, sizeof(v2));
                if (strcmp(v1, v2)) match = 0;
            }
            if (match) dup = 1;
        }
        if (dup) { *pp = cur->next; FREE(cur); db->size--; del++; }
        else       pp = &cur->next;
    }
    out(f, "uniq: %d\n", del);
}

void execute_command(char *line, list *db, FILE *f) {
    char kw[32];
    if (sscanf(line, "%31s", kw) != 1) return;

    const char *args = line + strlen(kw);
    while (*args == ' ' || *args == '\t') args++;

    if      (!strcmp(kw,"insert")) cmd_insert(args, db, f);
    else if (!strcmp(kw,"select")) cmd_select(args, db, f);
    else if (!strcmp(kw,"delete")) cmd_delete(args, db, f);
    else if (!strcmp(kw,"update")) cmd_update(args, db, f);
    else if (!strcmp(kw,"uniq"))   cmd_uniq(args, db, f);
    else out(f, "incorrect %.20s\n", line);
}

void free_list(list *db) {
    node *cur = db->head;
    while (cur) {
        node *next = cur->next;
        FREE(cur);
        cur = next;
    }
    db->head = NULL;
    db->size = 0;
}