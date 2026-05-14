#ifndef UTILS_H
#define UTILS_H

#include "database.h"

typedef enum {
    TYPE_INT,
    TYPE_STRING,
    TYPE_DATE,
    TYPE_TIME,
    TYPE_ENUM
} FieldType;

FieldType get_field_type(const char *field);
void* get_field_ptr(Record *rec, const char *field);
int compare_values(FieldType type, void *field_val, const char *op, const char *value_str);
char* convert_date_to_iso(const char *date_str);
char* convert_date_to_display(const char *iso);
int validate_time(const char *time_str);
Weather string_to_weather(const char *str);
const char* weather_to_string(Weather w);
int parse_conditions(char **tokens, int token_count, int start, char ***conds);

#endif