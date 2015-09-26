#include "ether_ss.h"
#include "ether_event.h"

#include "events.h"
#include "iphdr_ss/iphdr_event.h"
#include "types.h"

#include <stdio.h>
#include <netinet/ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>

// static void print_hex(const u8_t* chrs, size_t sz)
// {
//     uint_t i;
//     uint_t ch;
//     printf("+=");
//     for(i = 0; i < 16; ++ i) {
//         printf("===");
//     }
//     printf("\n| ");
//     for(i = 0; i < 16; ++ i) {
//         printf("%02x ", i);
//     }
//     printf("\n+-");
//     for(i = 0; i < 16; ++ i) {
//         printf("---");
//     }
//     printf("\n| ");
//     for(i = 0; i < sz; ++ i) {
//         ch = chrs[i];
//         printf("%02x ", ch);
//         if(((i + 1) & 0xF) == 0) {
//             printf("\n| ");
//         }
//     }
//     printf("\n+=");
//     for(i = 0; i < 16; ++ i) {
//         printf("===");
//     }
//     printf("\n");
// }

static void handle_packet_event(void* data, struct packet_event* evt)
{
    (void) data;
    (void) evt;
    // printf("Packet of size %lu bytes recieved\n", evt->data.sz);
    // print_hex(evt->data.chrs, evt->data.sz);

    // struct iphdr* ip_header = (struct iphdr*) (evt->data.chrs + sizeof(struct ether_header));

    // RAISE_EVENT(iphdr_event, new_iphdr_event(ip_header, ID_PACKET_RECIEVED));
}

int init_packet_handlers()
{
    BIND(packet_event, handle_packet_event, NULL, ID_PACKET_RECIEVED);
    return 0;
}
