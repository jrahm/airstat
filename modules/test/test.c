#include "plugin.h"
#include <stdio.h>

AIRSTAT_PLUGIN(SINK, "TestPlugin")

size_t count = 0;
size_t next = 0;

void print_address(airstat_packet_t* p)
{
    printf("ADDRESS: %p\n", p);
}

void print_length(airstat_packet_t* p)
{
    printf("Read packet of length: %lu\n", p->sz);
}

void count_packet(airstat_packet_t* p)
{
    count += p->sz;
    if(count > next) {
        printf("Recieved %lu bytes\n", count);
        next += (1024 * 1024);
    }
}

AIRSTAT_EXPORT_BEGIN()
AIRSTAT_EXPORT(print_address)
AIRSTAT_EXPORT(print_length)
AIRSTAT_EXPORT(count_packet)
AIRSTAT_EXPORT_END()
