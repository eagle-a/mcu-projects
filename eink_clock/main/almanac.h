#ifndef ALMANAC_H
#define ALMANAC_H

#include <stdbool.h>

typedef struct {
    char date[32];
    char lunar[96];
    char weekday[32];
    char jieqi[48];
    char season[32];
    char chongsha[96];
    char constellation[32];
    char suitable[256];
    char avoid[256];
    bool is_valid;
} almanac_data_t;

bool almanac_fetch(almanac_data_t *out);
const char *almanac_last_error(void);

#endif // ALMANAC_H
