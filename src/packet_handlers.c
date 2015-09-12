#include "packet_handlers.h"

#include "events.h"
#include "packet_event.h"
#include "types.h"

#include <stdio.h>

static void print_hex(const u8_t* chrs, size_t sz)
{
    size_t i;
    uint_t ch;
    printf("+=");
    for(i = 0; i < 16; ++ i) {
        printf("===");
    }
    printf("\n| ");
    for(i = 0; i < 16; ++ i) {
        printf("%02x ", i);
    }
    printf("\n+-");
    for(i = 0; i < 16; ++ i) {
        printf("---");
    }
    printf("\n| ");
    for(i = 0; i < sz; ++ i) {
        ch = chrs[i];
        printf("%02x ", ch);
        if(((i + 1) & 0xF) == 0) {
            printf("\n| ");
        }
    }
    printf("\n+=");
    for(i = 0; i < 16; ++ i) {
        printf("===");
    }
    printf("\n");
}

static void handle_packet_event(void* data, struct packet_event* evt)
{
    (void) data;
    printf("Packet of size %d bytes recieved\n", evt->data.sz);
    print_hex(evt->data.chrs, evt->data.sz);
}

int init_packet_handlers(bus_t* bus)
{
    bus_packet_event_bind(bus, handle_packet_event, NULL, ID_PACKET_RECIEVED);
    return 0;
}
