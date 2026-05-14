#include "utils.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

FieldType get_field_type(const char *field) {
    if (strcmp(field, "geo_id") == 0) return TYPE_INT;
    if (strcmp(field, "geo_pos") == 0) return TYPE_STRING;
    if (strcmp(field, "mea_date") == 0) return TYPE_DATE;
    if (strcmp(field, "level") == 0) return TYPE_INT;
    if (strcmp(field, "sunrise") == 0) return TYPE_TIME;
    if (strcmp(field, "weather") == 0) return TYPE_ENUM;
    if (strcmp(field, "sundown") == 0) return TYPE_TIME;
    return -1;
}

void* get_field_ptr(Record *rec, const char *field) {
    if (strcmp(field, "geo_id") == 0) return &rec->geo_id;
    if (strcmp(field, "geo_pos") == 0) return &rec->geo_pos;
    if (strcmp(field, "mea_date") == 0) return &rec->mea_date;
    if (strcmp(field, "level") == 0) return &rec->level;
    if (strcmp(field, "sunrise") == 0) return &rec->sunrise;
    if (strcmp(field, "weather") == 0) return &rec->weather;
    if (strcmp(field, "sundown") == 0) return &rec->sundown;
    return NULL;
}

int compare_values(FieldType type, void *field_val, const char *op, const char *value_str) {
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
        Weather val_enum = string_to_weather(value_str);
        if (val_enum == -1) return 0;
        if (strcmp(op, "=") == 0) return field_enum == val_enum;
        if (strcmp(op, "!=") == 0) return field_enum != val_enum;
    }
    return 0;
}

char* convert_date_to_iso(const char *date_str) {
    int day, month, year;
    if (sscanf(date_str, "%d.%d.%d", &day, &month, &year) != 3)
        return NULL;
    char *iso = my_malloc(11);
    if (!iso) return NULL;
    sprintf(iso, "%04d-%02d-%02d", year, month, day);
    return iso;
}

char* convert_date_to_display(const char *iso) {
    int year, month, day;
    if (sscanf(iso, "%d-%d-%d", &year, &month, &day) != 3)
        return NULL;
    char *disp = my_malloc(11);
    if (!disp) return NULL;
    sprintf(disp, "%02d.%02d.%04d", day, month, year);
    return disp;
}

int validate_time(const char *time_str) {
    int h, m, s;
    if (sscanf(time_str, "%d:%d:%d", &h, &m, &s) != 3) return 0;
    if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) return 0;
    return 1;
}

Weather string_to_weather(const char *str) {
    if (strcmp(str, "fair") == 0) return WEATHER_FAIR;
    if (strcmp(str, "rain") == 0) return WEATHER_RAIN;
    if (strcmp(str, "cloudy") == 0) return WEATHER_CLOUDY;
    if (strcmp(str, "snow") == 0) return WEATHER_SNOW;
    return -1;
}

const char* weather_to_string(Weather w) {
    switch (w) {
        case WEATHER_FAIR: return "fair";
        case WEATHER_RAIN: return "rain";
        case WEATHER_CLOUDY: return "cloudy";
        case WEATHER_SNOW: return "snow";
        default: return "unknown";
    }
}

int parse_conditions(char **tokens, int token_count, int start, char ***conds) {
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