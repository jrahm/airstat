#ifndef SRC_CHAIN_SS_CHAIN_
#define SRC_CHAIN_SS_CHAIN_

#include <stdio.h>

#include "exported_structures.h"

#define HAS_DEST_MAC_ADDR 0x1
#define HAS_SRC_MAC_ADDR 0x2
#define HAS_SRC_IP4_ADDR 0x4
#define HAS_DEST_IP4_ADDR 0x8
#define HAS_SRC_IP_PORT 0x10
#define HAS_DEST_IP_PORT 0x20

struct pattern_handler {
    struct pattern_handler* next;
    char* name;
};

struct pattern {
    uint64_t features;

    u8_t dest_mac_addr[6];
    u8_t src_mac_addr[6];

    uint32_t src_ip4_addr;
    uint32_t dest_ip4_addr;

    uint16_t src_ip_port;
    uint16_t dest_ip_port;

    struct pattern_handler* handlers;
};

struct hdr {
    int m_type;
    void(*matches)(void* self, struct pattern* pattern);
};

enum chain_rule_type {
     RULE_TYPE_RETURN
   , RULE_TYPE_CONTINUE
   , RULE_TYPE_DROP
   , RULE_TYPE_CALL
   , RULE_TYPE_GOTO
   , RULE_TYPE_LOG
};

struct chain_rule {
    pattern_t* m_pattern;
    struct chain_rule* next;

    int m_ref; /* reference count */
    int m_type;

    char* call_or_goto_name;
    union {
        void(*call_fn)(void*);
        struct chain_rule* goto_chain;
    };
};

struct chain_parse_ctx {
    /* the context used while parsing chains */
    struct plugin*     start_plugin_chain;
    struct string_map* plugin_map_by_name;
    struct string_map* link_map;
};

/*
 * Set of chains for the packets to filter through
 */
struct chain_set {
    struct string_map* chains;
};

const char* get_error();

struct plugin;
struct chain_set* parse_chains_from_file(const char* filename, struct chain_parse_ctx*);

void free_chain(struct chain_rule* chain);
void print_chain(struct chain_rule* chain);

#endif /* SRC_CHAIN_SS_CHAIN_ */
