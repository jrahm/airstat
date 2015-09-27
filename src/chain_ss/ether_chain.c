#include "ether_chain.h"
#include <string.h>
#include <netinet/if_ether.h>

typedef void (*handler_t)(struct chain_raw_packet_data*);

static void print_hex(const u8_t* chrs, size_t sz);
static int packet_matches_pattern(struct chain_raw_packet_data* data,
                                  pattern_t* pat)
{
    /* NULL pattern matches everything */
    if(pat == NULL) return 1;
    return pat->pattern_matches(&data->packet_data, pat);
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
            ret = chain_handle_return;
            break;
        case RULE_TYPE_CONTINUE:
            ret = chain_handle_continue;
            break;
        case RULE_TYPE_DROP:
            ret = chain_handle_drop;
            break;
        case RULE_TYPE_CALL:
            ret = chain_handle_call;
            break;
        case RULE_TYPE_GOTO:
            ret = chain_handle_goto;
            break;
        case RULE_TYPE_LOG:
            ret = chain_handle_log;
            break;
        default:
            fprintf(stderr, "No handler for type: %d **THIS IS A BUG**", type);
            ret = NULL;
    }
    return ret;
}

void chain_handle_BEGIN(struct chain_raw_packet_data* data)
{
    struct chain_rule* current_rule;
    current_rule = data->current_chain_rule;

    if(current_rule) {
        data->next_handler = rule_type_to_handler(current_rule->m_type);
    } else {
        /* default behavior of continue */
        data->next_handler = chain_handle_continue;
    }
}

void chain_handle_return(struct chain_raw_packet_data* data)
{
    fprintf(stderr, "return NOT implemented\n");
}

void chain_handle_continue(struct chain_raw_packet_data* data)
{
    u32_t new_magic;
    struct plugin* new_plugin;

    if(data->handling_plugin &&
        data->handling_plugin->type == PLUGIN_TYPE_SOURCE) {
        new_magic = data->handling_plugin->source.continue_packet(&data->packet_data);
        if(new_magic == 0) {
            fprintf(stderr, "Packet flagged to not continue\n");
            return;
        } else {
            new_plugin = intmap_get(data->issuer->m_magic_to_plugin_, new_magic);
            if(!new_plugin || new_plugin->type != PLUGIN_TYPE_SOURCE) {
                fprintf(stderr, "No plugin for continuing magic number %u\n", new_magic);
                return ;
            } else {
                data->packet_data.packet_type = new_magic;
                data->current_chain_rule = new_plugin->source.start_chain;
                chain_handle_BEGIN(data);
            }
        }
    }

    fprintf(stderr, "No handling plugin for packet_data?\n");
}

void chain_handle_drop(struct chain_raw_packet_data* data)
{
    struct chain_rule *rule = data->current_chain_rule;
    pattern_t* pat = rule->m_pattern;
    int match = packet_matches_pattern(data, pat);

    if(match) {
        data->next_handler = NULL;
    } else {
        data->current_chain_rule = data->current_chain_rule->next;
        chain_handle_BEGIN(data);
    }
}

void chain_handle_call(struct chain_raw_packet_data* data)
{
    if(data->current_chain_rule &&
        data->current_chain_rule->call_fn) {
        data->current_chain_rule->call_fn(&data->packet_data);
    }
    data->current_chain_rule = data->current_chain_rule->next;
    chain_handle_BEGIN(data);
}

void chain_handle_log(struct chain_raw_packet_data* data)
{
    struct chain_rule *rule = data->current_chain_rule;
    pattern_t *pat = rule->m_pattern;
    int match = packet_matches_pattern(data, pat);

    if(match) {
        print_hex(data->packet_data.bytes, data->packet_data.sz);
    }

    data->current_chain_rule = data->current_chain_rule->next;
    chain_handle_BEGIN(data);
}

void chain_handle_goto(struct chain_raw_packet_data* data)
{
    struct chain_rule *rule = data->current_chain_rule;
    pattern_t *pat = rule->m_pattern;
    int match = packet_matches_pattern(data, pat);

    if(match) {
        data->current_chain_rule =
            data->current_chain_rule->goto_chain;
    } else {
        data->current_chain_rule =
            data->current_chain_rule->next;
    }

    chain_handle_BEGIN(data);
}
