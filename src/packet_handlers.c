#include "packet_handlers.h"

#include "events.h"
#include "packet_event.h"
#include "iphdr_event.h"
#include "types.h"

#include <stdio.h>
#include <netinet/ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>

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

static void handle_packet_event(void* bus_, struct packet_event* evt)
{
    bus_t* bus = bus_;
    printf("Packet of size %d bytes recieved\n", evt->data.sz);
    print_hex(evt->data.chrs, evt->data.sz);

    struct ether_header* eth_header = (struct ether_header*)evt->data.chrs;
    struct iphdr* ip_header = (struct iphdr*) (evt->data.chrs + sizeof(struct ether_header));

    BUS_RAISE(iphdr_event, bus, new_iphdr_event(ip_header, ID_PACKET_RECIEVED));
}

int init_packet_handlers(bus_t* bus)
{
    BUS_BIND(packet_event, bus, handle_packet_event, bus, ID_PACKET_RECIEVED);
    return 0;
}
