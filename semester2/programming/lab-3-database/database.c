#include "database.h"
#include "memory.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

Record *db_head = NULL;
Record *db_tail = NULL;

Record* create_record(void) {
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

void free_record(Record *rec) {
    if (!rec) return;
    if (rec->geo_pos) my_free(rec->geo_pos);
    if (rec->mea_date) my_free(rec->mea_date);
    if (rec->sunrise) my_free(rec->sunrise);
    if (rec->sundown) my_free(rec->sundown);
    my_free(rec);
}

void add_record(Record *rec) {
    if (!db_head) {
        db_head = rec;
        db_tail = rec;
    } else {
        db_tail->next = rec;
        db_tail = rec;
    }
    rec->next = NULL;
}

void delete_record(Record *prev, Record *curr) {
    if (prev)
        prev->next = curr->next;
    else
        db_head = curr->next;
    if (db_tail == curr)
        db_tail = prev;
    free_record(curr);
}

int record_matches_conditions(Record *rec, char **conds, int cond_count) {
    for (int i = 0; i < cond_count; i++) {
        char *cond = conds[i];
        char *op = NULL;
        if ((op = strstr(cond, ">=")) != NULL) {}
        else if ((op = strstr(cond, "<=")) != NULL) {}
        else if ((op = strstr(cond, "!=")) != NULL) {}
        else if ((op = strstr(cond, "=")) != NULL) {}
        else if ((op = strstr(cond, ">")) != NULL) {}
        else if ((op = strstr(cond, "<")) != NULL) {}
        else return 0;

        char field_name[64];
        char value[256];
        int field_len = op - cond;
        if (field_len >= sizeof(field_name)) return 0;
        strncpy(field_name, cond, field_len);
        field_name[field_len] = '\0';
        // пропускаем пробелы после оператора
        const char *vstart = op + strlen(op);
        while (*vstart == ' ') vstart++;
        strcpy(value, vstart);

        FieldType ft = get_field_type(field_name);
        if (ft == -1) return 0;
        void *ptr = get_field_ptr(rec, field_name);
        if (!ptr) return 0;

        if (!compare_values(ft, ptr, op, value))
            return 0;
    }
    return 1;
}

void update_record(Record *rec, char **upd_fields, char **upd_values, int upd_count) {
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

Record* find_records(char **conds, int cond_count, int *found_count) {
    Record *result_list = NULL;
    Record *result_tail = NULL;
    int found = 0;
    Record *cur = db_head;
    while (cur) {
        if (record_matches_conditions(cur, conds, cond_count)) {
            Record *node = my_malloc(sizeof(Record));
            if (!node) break;
            *node = *cur; // копируем указатели 
            node->next = NULL;
            if (!result_list) result_list = node;
            else result_tail->next = node;
            result_tail = node;
            found++;
        }
        cur = cur->next;
    }
    if (found_count) *found_count = found;
    return result_list;
}

int uniq_by_fields(char **fields, int field_count) {
    int deleted = 0;
    Record *cur = db_head;
    Record *prev = NULL;
    while (cur) {
        Record *next = cur->next;
        int duplicate_found = 0;
        Record *later = cur->next;
        while (later) {
            // сравниваем по полям
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

void free_database(void) {
    Record *cur = db_head;
    while (cur) {
        Record *next = cur->next;
        free_record(cur);
        cur = next;
    }
    db_head = NULL;
    db_tail = NULL;
}