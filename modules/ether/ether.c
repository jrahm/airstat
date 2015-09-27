#include "plugin.h"
#include <stdio.h>
#include <memory.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/if_ether.h>

#define ETHER_MAGIC_NUMBER 0x1
AIRSTAT_SOURCE_PLUGIN("ether", ETHER_MAGIC_NUMBER)

#define HAS_SRC_MAC 0x1
#define HAS_DEST_MAC 0x2

struct ether_pattern {
    pattern_t super;

    int flags;

    u8_t src_mac[6];
    u8_t dest_mac[6];
};

const char* get_airstat_initial_chain() { return "ether"; }

int airstat_plugin_initialize(int argc, char** argv, void** out)
{
    *out = NULL;
}

int get_airstat_fd(void* ctx)
{
    int sock = socket(PF_PACKET, SOCK_RAW, htons(0x0800));
    int rc = 0;
    int sockopt = 1;

    if(sock < 0) {
        perror("Error creating raw socket.");
        return sock;
    }

    if((rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt))))
        goto error;

    return sock;

error:
    close(sock);
    return -rc;
}

airstat_packet_t* get_airstat_packet(void* ctx, int fd)
{
    u8_t buffer[65536];
    int datasize;

    datasize = recvfrom(fd, buffer, sizeof(buffer), 0, NULL, NULL);

    airstat_packet_t* packet = calloc(sizeof(airstat_packet_t), 1);
    packet->bytes = malloc(datasize);
    memcpy(packet->bytes, buffer, datasize);
    packet->sz = datasize;
    packet->packet_type = ETHER_MAGIC_NUMBER;

    return packet;
}

static int parse_mac_addr(u8_t* mac, const char* value);
static int ether_pattern_match(airstat_packet_t* packet, pattern_t* pattern_)
{
    if(pattern_->type != ETHER_MAGIC_NUMBER) {
        return 0;
    }

    if(packet->packet_type != ETHER_MAGIC_NUMBER) {
        return 0;
    }

    struct ether_pattern* pattern = (struct ether_pattern*) pattern;
    struct ether_header* as_ether_header;
    as_ether_header = (struct ether_header*) packet->bytes;

    int ret = 1;
    int flags = pattern->flags;

    if(flags & HAS_SRC_MAC)
        ret &= !memcmp(as_ether_header->ether_shost, pattern->src_mac, 6);

    if(flags & HAS_DEST_MAC)
        ret &= !memcmp(as_ether_header->ether_dhost, pattern->dest_mac, 6);

    return ret;

}

pattern_t* compile_airstat_pattern(struct string_map* map)
{
    struct ether_pattern* ret = calloc(sizeof(struct ether_pattern), 1);
    ret->super.type = ETHER_MAGIC_NUMBER;
    ret->super.pattern_matches = ether_pattern_match;

    if(map->has_key(map, "src_mac")) {
        if(parse_mac_addr(ret->src_mac, map->get(map, "src_mac")))
            goto error;
    }
    if(map->has_key(map, "dest_mac")) {
        if(parse_mac_addr(ret->dest_mac, map->get(map, "dest_mac")))
            goto error;
    }

    return (pattern_t*) ret;

error:
    free(ret);
    return NULL;
}

static u8_t from_hex(char ch)
{
    if(ch < 0x40 && ch > 0x30) return ch - 0x30;
    if(ch >= 0x41 && ch <= 0x46) return (ch - 0x41) + 10;
    if(ch >= 0x61 && ch <= 0x66) return (ch - 0x61) + 10;
    return 255;
}

static int parse_mac_addr(u8_t* mac, const char* value)
{
    size_t i = 0;
    size_t j = 0;

    if(strlen(value) != 17) return 1;

    mac[i] = from_hex(value[j++]) << 4;
    mac[i++] += from_hex(value[j++]);
    if(value[j++] != ':') return 1;

    mac[i] = from_hex(value[j++]) << 4;
    mac[i++] += from_hex(value[j++]);
    if(value[j++] != ':') return 1;

    mac[i] = from_hex(value[j++]) << 4;
    mac[i++] += from_hex(value[j++]);
    if(value[j++] != ':') return 1;

    mac[i] = from_hex(value[j++]) << 4;
    mac[i++] += from_hex(value[j++]);
    if(value[j++] != ':') return 1;

    mac[i] = from_hex(value[j++]) << 4;
    mac[i++] += from_hex(value[j++]);
    if(value[j++] != ':') return 1;

    mac[i] = from_hex(value[j++]) << 4;
    mac[i++] += from_hex(value[j++]);

    return 0;
}
