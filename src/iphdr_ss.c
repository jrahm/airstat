#include "iphdr_event.h"
#include "events.h"

#include <stdio.h>
#include <arpa/inet.h>

char* sprint_ip_addr(char* buf, size_t sz, unsigned int addr)
{
    addr = ntohl(addr);
    snprintf(buf, sz, "%d.%d.%d.%d",(addr >> 24) & 0xFF,
                                    (addr >> 16) & 0xFF,
                                    (addr >> 8)  & 0xFF,
                                    (addr >> 0)  & 0xFF);
    return buf;
}

void handle_iphdr(void* data, struct iphdr_event* evt)
{
    (void)data;
    char buf[128];
    printf("Read IP header\n");
    printf("From address: %s\n", sprint_ip_addr(buf, 128, evt->data.saddr));
    printf("To address: %s\n", sprint_ip_addr(buf, 128, evt->data.daddr));
}

void init_iphdr_ss(bus_t* bus)
{
    BUS_BIND(iphdr_event, bus, handle_iphdr, NULL, ID_PACKET_RECIEVED);
}
