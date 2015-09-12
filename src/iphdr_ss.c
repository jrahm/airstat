#include "iphdr_event.h"
#include "events.h"

#include <stdio.h>

void handle_iphdr(void* data, struct iphdr_event* evt)
{
    (void)data;
    printf("Read IP header\n");
    printf("From address: %x\n", evt->data.saddr);
    printf("To address: %x\n", evt->data.daddr);
}

void init_iphdr_ss(bus_t* bus)
{
    bus_iphdr_event_bind(bus, handle_iphdr, NULL, ID_IP_HEADER_READ);
}
