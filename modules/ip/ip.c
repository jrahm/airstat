#include "plugin.h"
#include <stdio.h>
#include <memory.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/if_ether.h>

#define IP_MAGIC_NUMBER 0x2
AIRSTAT_SOURCE_PLUGIN("ip", IP_MAGIC_NUMBER)

#define HAS_SRC_IP  0x1
#define HAS_DEST_IP 0x2

struct ip_pattern {
    pattern_t super;
};

const char* get_airstat_initial_chain() { return "ip"; }

int airstat_plugin_initialize(int argc, char** argv, void** out)
{ 
    *out = NULL;
    return 0;
}

int get_airstat_fd(void* ctx)
{ return ENOIMPL; }

airstat_packet_t* get_airstat_packet(void* ctx, int fd)
{ return NULL; }

u32_t airstat_continue_packet(airstat_packet_t* packet)
{ return 0; }

pattern_t* compile_airstat_pattern(void* ctx, struct string_map* map)
{
    struct ip_pattern* ret = calloc(sizeof(struct ip_pattern), 1);
    return (pattern_t*) ret;
}
