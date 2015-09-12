#ifndef PACKET_EVENT_
#define PACKET_EVENT_

#include <stdlib.h>

#include "bus.h"
#include "types.h"

struct packet_data {
    u8_t* chrs;
    size_t sz;
};

DECLARE_EVENT_TYPE(packet_event, struct packet_data);

#endif /* PACKET_EVENT_ */
