#include "ether_chain.h"
#include <string.h>
#include <netinet/if_ether.h>

typedef void (*handler_t)(struct chain_raw_packet_data*);

static int packet_matches_pattern(struct chain_raw_packet_data* data,
                                  struct pattern* pat)
{
    /* NULL pattern matches everything */
    if(pat == NULL) return 1;

    struct ether_header* as_ether_header;
    as_ether_header = (struct ether_header*) data->bytes;

    if(!memcmp(as_ether_header->ether_dhost, pat->dest_mac_addr, 6) &&
        memcmp(as_ether_header->ether_shost, pat->src_mac_addr, 6)) {
        return 1;
    }

    return 0;
}

static void print_hex(const u8_t* chrs, size_t sz)
{
    unsigned int i;
    uint_t ch;
    printf("+=");
    for(i = 0; i < 16; ++ i) {
        printf("===");
    }
    printf("\n| ");
    for(i = 0; i < 16; ++ i) {
        printf("%02x ", i);
    }
    printf("\n+-");
    for(i = 0; i < 16; ++ i) {
        printf("---");
    }
    printf("\n| ");
    for(i = 0; i < sz; ++ i) {
        ch = chrs[i];
        printf("%02x ", ch);
        if(((i + 1) & 0xF) == 0) {
            printf("\n| ");
        }
    }
    printf("\n+=");
    for(i = 0; i < 16; ++ i) {
        printf("===");
    }
    printf("\n");
}

static handler_t rule_type_to_handler(int type)
{
    void (*ret)(struct chain_raw_packet_data*);
    switch (type) {
        case RULE_TYPE_RETURN:
            ret = ether_chain_handle_return;
            break;
        case RULE_TYPE_CONTINUE:
            ret = ether_chain_handle_continue;
            break;
        case RULE_TYPE_DROP:
            ret = ether_chain_handle_drop;
            break;
        case RULE_TYPE_CALL:
            ret = ether_chain_handle_call;
            break;
        case RULE_TYPE_GOTO:
            ret = ether_chain_handle_goto;
            break;
        case RULE_TYPE_LOG:
            ret = ether_chain_handle_log;
            break;
        default:
            fprintf(stderr, "No handler for type: %d **THIS IS A BUG**", type);
            ret = NULL;
    }
    return ret;
}

void ether_chain_handle_BEGIN(struct chain_raw_packet_data* data)
{
    struct chain_rule* current_rule;
    current_rule = data->current_chain_rule;

    data->next_handler = rule_type_to_handler(current_rule->m_type);
}

void ether_chain_handle_return(struct chain_raw_packet_data* data)
{
    fprintf(stderr, "return NOT implemented\n");
}

void ether_chain_handle_continue(struct chain_raw_packet_data* data)
{
    fprintf(stderr, "continue NOT implemented\n");
}

void ether_chain_handle_drop(struct chain_raw_packet_data* data)
{
    int match = packet_matches_pattern(data, data->current_chain_rule->m_pattern);
    if(match) {
        data->next_handler = NULL;
    } else {
        data->current_chain_rule = data->current_chain_rule->next;
        ether_chain_handle_BEGIN(data);
    }
}

void ether_chain_handle_call(struct chain_raw_packet_data* data)
{
    data->current_chain_rule = data->current_chain_rule->next;
    ether_chain_handle_BEGIN(data);

    fprintf(stderr, "call NOT implemented\n");
}

void ether_chain_handle_log(struct chain_raw_packet_data* data)
{
    int match = packet_matches_pattern(data, data->current_chain_rule->m_pattern);
    if(match) {
        print_hex(data->bytes, data->sz);
    }

    data->current_chain_rule = data->current_chain_rule->next;
    ether_chain_handle_BEGIN(data);
}

void ether_chain_handle_goto(struct chain_raw_packet_data* data)
{
    data->current_chain_rule = data->current_chain_rule->next;
    ether_chain_handle_BEGIN(data);

    fprintf(stderr, "goto NOT implemented\n");
}
