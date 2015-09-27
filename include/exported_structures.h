#ifndef INCLUDE_EXPORTED_STRUCTURES_
#define INCLUDE_EXPORTED_STRUCTURES_

#include <stdlib.h>
#include "types.h"
#include "string_map.h"
#include <stdio.h>


typedef struct packet_data {
    u8_t* chrs;
    size_t sz;
} packet_data_t;

typedef struct airstat_packet {
    int packet_continue;
    int packet_type;

    size_t sz;
    u8_t* bytes;

    char rest[128]; /* extra bytes for extending the structure */
} airstat_packet_t;

typedef struct airstat_pattern {
    int type;
    int (*pattern_matches)(airstat_packet_t*, struct airstat_pattern* ths);
    void (*print)(struct airstat_pattern* ths, FILE* out);

    /* rest to be defined */
} pattern_t ;

#endif /* INCLUDE_EXPORTED_STRUCTURES_ */
