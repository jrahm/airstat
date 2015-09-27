#ifndef INCLUDE_EXPORTED_STRUCTURES_
#define INCLUDE_EXPORTED_STRUCTURES_

#include <stdlib.h>

typedef struct airstat_packet {
    int packet_continue;
    int packet_type;

    size_t sz;
    unsigned char* bytes;

    char rest[128]; /* extra bytes for extending the structure */
} airstat_packet_t;

#endif /* INCLUDE_EXPORTED_STRUCTURES_ */
