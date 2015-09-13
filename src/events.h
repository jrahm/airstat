#ifndef EVENTS_
#define EVENTS_

#include "bus.h"

#define RAISE_EVENT(type, event) \
    BUS_RAISE(type, global_bus, event)

#define BIND(type, handler, data, id) \
    BUS_BIND(type, global_bus, handler, data, id)

extern bus_t* global_bus;

enum {
      ID_PACKET_RECIEVED
    , ID_IP_HEADER_READ
};


#endif /* EVENTS_ */
