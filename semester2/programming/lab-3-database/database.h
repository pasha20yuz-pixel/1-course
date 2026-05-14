#ifndef DATABASE_H
#define DATABASE_H

typedef enum {
    WEATHER_FAIR,
    WEATHER_RAIN,
    WEATHER_CLOUDY,
    WEATHER_SNOW
} Weather;

typedef struct Record {
    int geo_id;
    char *geo_pos;
    char *mea_date;
    int level;
    char *sunrise;
    Weather weather;
    char *sundown;
    struct Record *next;
} Record;

extern Record *db_head;
extern Record *db_tail;

Record* create_record(void);
void free_record(Record *rec);
void add_record(Record *rec);
void delete_record(Record *prev, Record *curr);
int record_matches_conditions(Record *rec, char **conds, int cond_count);
void update_record(Record *rec, char **upd_fields, char **upd_values, int upd_count);
Record* find_records(char **conds, int cond_count, int *found_count);
int uniq_by_fields(char **fields, int field_count);
void free_database(void);

#endif