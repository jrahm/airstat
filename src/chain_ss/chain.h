#ifndef SRC_CHAIN_SS_CHAIN_
#define SRC_CHAIN_SS_CHAIN_

#include <stdio.h>

struct pattern {
    int m_type;
};

struct hdr {
    int m_type;
    void(*matches)(struct pattern* pattern);
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

    int m_type;

    union {
        const char* call_fn_name;
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

struct chain_set* parse_chains_from_file(FILE* fd);
void free_chain(struct chain_rule* chain);

#endif /* SRC_CHAIN_SS_CHAIN_ */
