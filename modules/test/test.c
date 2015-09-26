#include "plugin.h"
#include <stdio.h>

AIRSTAT_PLUGIN(CONSUMER, "TestPlugin")

void print_address(void* p)
{
    printf("ADDRESS: %p\n", p);
}

AIRSTAT_EXPORT_BEGIN()
AIRSTAT_EXPORT(print_address)
AIRSTAT_EXPORT_END()
