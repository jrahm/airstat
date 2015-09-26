#include "plugin.h"
#include <stdio.h>

AIRSTAT_PLUGIN(CONSUMER, "TestPlugin")

void print_address(airstat_packet_t* p)
{
    printf("ADDRESS: %p\n", p);
}

void print_length(airstat_packet_t* p)
{
    printf("Read packet of length: %lu\n", p->sz);
}

AIRSTAT_EXPORT_BEGIN()
AIRSTAT_EXPORT(print_address)
AIRSTAT_EXPORT(print_length)
AIRSTAT_EXPORT_END()
