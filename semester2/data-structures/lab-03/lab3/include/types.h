#ifndef _TYPES_H_
#define _TYPES_H_

#include <time.h>

#define MAX_NAME 256
#define MAX_NAMES 16
#define MAX_CONDS 16
#define MAX_FV 16

typedef enum
{
    cmd,
    file
} input_type;

typedef enum
{
    CMD_INSERT,
    CMD_SELECT,
    CMD_DELETE,
    CMD_UPDATE,
    CMD_UNIQ,
    CMD_UNKNOWN
} command;

typedef enum
{
    fair,
    rain,
    cloudy,
    snow
} weather_type;

typedef struct
{
    int geo_id;
    char geo_pos[MAX_NAME];
    time_t mea_date;   // date only (time set to 00:00:00)
    int level;
    time_t sunrise;    // time only (date set to 1970-01-01)
    weather_type weather;
    time_t sundown;    // time only
} weather_station;

typedef struct node
{
    weather_station data;
    struct node* next;
} node;

typedef struct
{
    node* head;
    size_t size;
} list;

typedef struct 
{
    char f[64]; 
    char v[256]; 
} fv_pair;

typedef enum 
{ 
    OP_LT, 
    OP_LE, 
    OP_GT, 
    OP_GE, 
    OP_EQ, 
    OP_NE, 
    OP_IN 
} op_t;

typedef struct 
{ 
    char f[64];
    op_t op; 
    char v[256]; 
} cond;

void free_list(list *db);

#endif