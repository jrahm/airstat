#ifndef PACKET_HANDLERS_
#define PACKET_HANDLERS_

#include "types.h"
#include <netinet/ether.h>


struct ether_pattern {
    bool address_enabled;
    char address[6]; /* The ethernet address */
};

struct ether_rule {
    struct ether_pattern m_pattern; 
    struct ether_rule* m_next;

    enum {
        ETHER_RULE_GOTO,
        ETHER_RULE_
    } m_type;
};

struct ether_chain {

};

int init_packet_handlers();

#endif /* PACKET_HANDLERS_ */
