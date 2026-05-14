#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"

command read_console(const char *line)
{
    char keyword[32];
    sscanf(line, "%31s", keyword);

    if (strcmp(keyword, "insert") == 0) return CMD_INSERT;
    if (strcmp(keyword, "select") == 0) return CMD_SELECT;
    if (strcmp(keyword, "delete") == 0) return CMD_DELETE;
    if (strcmp(keyword, "update") == 0) return CMD_UPDATE;
    if (strcmp(keyword, "uniq")   == 0) return CMD_UNIQ;
    return CMD_UNKNOWN;
}   