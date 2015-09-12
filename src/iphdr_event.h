#ifndef IPHDR_EVENT_
#define IPHDR_EVENT_

#include <linux/ip.h>

#include <stdlib.h>
#include "bus.h"
#include "types.h"

DECLARE_EVENT_TYPE(iphdr_event, struct iphdr);

#endif /* IPHDR_EVENT_ */
