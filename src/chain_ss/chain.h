#ifndef SRC_CHAIN_SS_CHAIN_
#define SRC_CHAIN_SS_CHAIN_

#include <stdio.h>

#include "types.h"

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

void print_pattern(struct pattern* pat) ;

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
    struct pattern* m_pattern;
    struct chain_rule* next;

    int m_ref; /* reference count */
    int m_type;

    union {
        char* call_fn_name;
        struct chain_rule* goto_chain;
    };
};

/*
 * Set of chains for the packets to filter through
 */
struct chain_set {
    struct chain_rule* ether_chain_head;
    struct chain_rule* ip_chain_head;
    struct chain_rule* tcp_chain_head;
    struct chain_rule* udp_chain_head;
};

const char* get_error();
struct chain_set* parse_chains_from_file(const char* filename);
void free_chain(struct chain_rule* chain);
void print_chain(struct chain_rule* chain);

#endif /* SRC_CHAIN_SS_CHAIN_ */
