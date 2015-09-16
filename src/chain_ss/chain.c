#include "chain.h"

#include <string_map.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char error[1024];

char* next_token(FILE* fd, size_t* len, int (*get_class)(char))
{
    int ch;

    char* retbuf;
    size_t nalloc = 128;
    size_t size = 0;
    int class;

    ch = fgetc(fd);
    if(ch == -1) return NULL;

    class = get_class(ch);
    size = 1;
    retbuf = malloc(nalloc);
    retbuf[0] = ch;

    while((ch = fgetc(fd)) != -1 && class == get_class(ch)) {
        if(size == nalloc) {
            nalloc *= 2;
            retbuf = realloc(retbuf, nalloc * sizeof(char));
        }
        retbuf[size ++] = (char)ch;
    }

    if(ch != -1) {
        ungetc(ch, fd);
    }

    if(size == nalloc) {
        nalloc ++;
        retbuf = realloc(retbuf, nalloc * sizeof(char));
    }
    retbuf[size] = 0;

    if(len) {
        *len = size;
    }

    return retbuf;
}

char* next_token_skip_space(FILE* fd, size_t* len, int(*get_class)(char))
{
    char* token;
    token = next_token(fd, len, get_class);
    if(token && isspace(token[0])) {
        free(token);
        return next_token(fd, len, get_class);
    } else {
        return token;
    }
}

static int default_class(char ch)
{
    if(isspace(ch)) {
        return 0;
    } else if(ch == ';') {
        return 3;
    } else if(ch == '(') {
        return 4;
    } else if(ch == ')') {
        return 5;
    } else if(isalnum(ch) || ch == '_') {
        return 1;
    } else {
        return 2;
    }
}

static int not_paren(char ch)
{
    if(ch == ')') return 1;
    else return 3;
}

static const char* next_word(const char* in, char* out, size_t len)
{
    size_t i;
    -- len;
    while(isspace(*in)) ++ in;
    for(i = 0; i < len && !isspace(*in) && *in ; ++ in, ++ i) {
        out[i] = *in;
    }
    out[i] = 0;
    return in;
}

static int parse_mac_addr(u8_t* mac, const char* value)
{
    size_t i = 0;
    size_t j = 0;

    if(strlen(value) != 17) return 1;

    mac[i] = (value[j++] - 0x30) << 4;
    mac[i++] += value[j++] - 0x30;
    if(value[j++] != ':') return 1;

    mac[i] = (value[j++] - 0x30) << 4;
    mac[i++] += value[j++] - 0x30;
    if(value[j++] != ':') return 1;

    mac[i] = (value[j++] - 0x30) << 4;
    mac[i++] += value[j++] - 0x30;
    if(value[j++] != ':') return 1;

    mac[i] = (value[j++] - 0x30) << 4;
    mac[i++] += value[j++] - 0x30;
    if(value[j++] != ':') return 1;

    mac[i] = (value[j++] - 0x30) << 4;
    mac[i++] += value[j++] - 0x30;
    if(value[j++] != ':') return 1;

    mac[i] = (value[j++] - 0x30) << 4;
    mac[i++] += value[j++] - 0x30;

    return 0;
}

static void add_key_to_pattern(struct pattern* pat, const char* key, const char* val)
{
    if(!strcmp(key, "src_mac")) {
        if(parse_mac_addr(pat->src_mac_addr, val)) {
            fprintf(stderr, "Failed to parse mac address '%s'", val);
        } else {
            pat->features |= HAS_SRC_MAC_ADDR;
        }
    } else if(!strcmp(key, "dest_mac")) {
        if(parse_mac_addr(pat->dest_mac_addr, val)) {
            fprintf(stderr, "Failed to parse mac address '%s'", val);
        } else {
            pat->features |= HAS_DEST_MAC_ADDR;
        }
    } else {
        fprintf(stderr, "WARN: unimplemented key: %s\n", key);
    }
}

struct pattern* compile_pattern(const char* pat)
{
    printf("Compile: %s\n", pat);
    char word[1024];
    char* ptr;
    struct pattern* ret = calloc(sizeof(struct pattern), 1);
    pat = next_word(pat, word, sizeof(word));

    while(word[0]) {
        ptr = strchr(word, '=');
        if(ptr) {
            *ptr = 0;
            ptr ++;
            add_key_to_pattern(ret, word, ptr);
        } else {
            fprintf(stderr, "handles (%s) not yet implemented\n", word);
            /* it is a handle now */;
        }
        pat = next_word(pat, word, sizeof(word));
    }
    
    return ret;
}

struct chain_rule* read_one_chain_rule(char* token, FILE* fd, string_map_t* strmap)
{
    struct chain_rule* ret = calloc(sizeof(struct chain_rule), 1);
    struct pattern* pat = NULL;
    char* goto_name;

    if(token[0] == '(') {
        free(token);
        token = next_token_skip_space(fd, NULL, not_paren);
        pat = compile_pattern(token);
        free(token);
        token = next_token_skip_space(fd, NULL, not_paren);
        if(strcmp(token, ")") != 0) {
            sprintf(error, "Syntax error: expected ')' near %s\n", token);
            free(token);
            goto error;
        }
        free(token);
        token = next_token_skip_space(fd, NULL, default_class);
    }

    ret->m_pattern = pat;
    ret->call_fn_name = NULL;

    if(strcmp(token, "return") == 0) {
        ret->m_type = RULE_TYPE_RETURN;
    } else if(strcmp(token, "continue") == 0) {
        ret->m_type = RULE_TYPE_CONTINUE;
    } else if(strcmp(token, "log") == 0) {
        ret->m_type = RULE_TYPE_LOG;
    } else if(strcmp(token, "drop") == 0) {
        ret->m_type = RULE_TYPE_DROP;
    } else if(strcmp(token, "call") == 0) {
        ret->m_type = RULE_TYPE_CALL;
        ret->call_fn_name = next_token_skip_space(fd, NULL, default_class);
        if(!ret->call_fn_name || !isalnum(ret->call_fn_name[0])) {
            sprintf(error, "Syntax error: missing identifier after `call'\n");
            free(ret->call_fn_name);
            goto error;
        }
    } else if(strcmp(token, "goto") == 0) {
        ret->m_type = RULE_TYPE_GOTO;
        goto_name = next_token_skip_space(fd, NULL, default_class);
        if(!goto_name || !isalnum(goto_name[0])) {
            sprintf(error, "Syntax error: missing identifier after `goto'\n");
            free(goto_name);
            goto error;
        }

        ret->goto_chain = string_map_get(strmap, goto_name);
        free(goto_name);
        if(!ret->goto_chain) {
            sprintf(error, "Error: %s: no such chain\n", goto_name);
            goto error;
        }
        ret->goto_chain->m_ref ++;
    } else {
        sprintf(error, "Unknown command %s\n", token);
        free(token);
        goto error;
    }

    free(token);
    token = next_token_skip_space(fd, NULL, default_class);
    if(!token || token[0] != ';') {
        sprintf(error, "Expected ';', got %s\n", token);
        free(token);
        goto error;
    }

    free(token);

    return ret;

error:
    free(ret);
    return NULL;
}

struct chain_rule* read_chain(FILE* fd, string_map_t* chain_map)
{
    /* read a single chain from the file
     * given above. This already assumes
     * that the word `chain` and the name
     * have already been consumed */

    char* token;
    size_t len;
    int rc = 0;
    struct chain_rule super_head;
    super_head.next = NULL;
    struct chain_rule* cursor = &super_head;
    struct chain_rule* next;
    struct chain_rule* tmp;

    token = next_token_skip_space(fd, &len, default_class);

    if(strcmp(token, "{")) {
        free(token);
        snprintf(error, sizeof(error), "Syntax error near %s\n", token);
        goto error;
    }
    free(token);

    while(1) {
        token = next_token_skip_space(fd, &len, default_class);
        if(!token) {
            sprintf(error, "Premature EOF\n");
            goto error;
        }
        if(token[0] == '}') {
            free(token);
            break;
        } else if(strcmp(token, "chain") == 0) {
            printf("Inner chain\n");
            free(token);
            token = next_token_skip_space(fd, &len, default_class);

            if(token == NULL) {
                sprintf(error, "Unexpected EOF while parsing chain\n");
                goto error;
            } else {
                tmp = read_chain(fd, chain_map);
                if(tmp) {
                    string_map_insert(chain_map, token, tmp);
                    free(token);
                } else {
                    free(token);
                    goto error;
                }
            }
        } else {
            tmp = read_one_chain_rule(token, fd, chain_map);
            if(tmp) {
                cursor->next = tmp;
                cursor = cursor->next;
            } else {
                goto error;
            }
        }
    }

    return super_head.next;
error:
    free_chain(super_head.next);
    return NULL;
}

struct string_map* read_chains(FILE* fd)
{
    char* chain = NULL;
    char* name = NULL;
    string_map_t* ret = new_string_map();
    struct chain_rule* chainr;

    while(1) {
        chain = next_token_skip_space(fd, NULL, default_class);
        if(!chain) {
            break;
        }

        if(strcmp(chain, "chain") != 0) {
            sprintf(error, "Expected 'chain', got %s\n", chain);
            goto error;
        }

        name = next_token_skip_space(fd, NULL, default_class);
        if(!name || !isalnum(name[0])) {
            sprintf(error, "Expected identifier after 'chain', got %s", name);
            goto error;
        }
        
        chainr = read_chain(fd, ret);
        if(!chainr) goto error;

        string_map_insert(ret, name, chainr);

        free(chain);
        free(name);
        name = chain = NULL;
    }

    return ret;
error:
    free(chain);
    free(name);
    free(ret);
    return NULL;
}

void try_delete_chain(struct chain_rule* cr)
{
    if(cr->m_ref == 0) free_chain(cr);
}

void test_incref(struct chain_rule* rule)
{
    if(rule) {
        rule->m_ref ++;
    }
}

struct chain_set* parse_chains_from_file(const char* filename)
{
    FILE* fd;
    fd = fopen(filename, "r");
    if(!fd) {
        sprintf(error, "Unable to open file: %s", filename);
        return NULL;
    }
    string_map_t* map = read_chains(fd);
    if(!map) {
        return NULL;
    }

    struct chain_set* ret = malloc(sizeof(struct chain_set));

    ret->ether_chain_head = string_map_get(map, "ether");
    ret->ip_chain_head = string_map_get(map, "ip");
    ret->tcp_chain_head = string_map_get(map, "tcp");
    ret->udp_chain_head = string_map_get(map, "udp");

    test_incref(ret->ether_chain_head);
    test_incref(ret->ip_chain_head);
    test_incref(ret->tcp_chain_head);
    test_incref(ret->udp_chain_head);

    string_map_free(map, try_delete_chain);
    return ret;
}

const char* get_error()
{
    return error;
}

void free_chain(struct chain_rule* chain)
{
    struct chain_rule* cur = chain;
    struct chain_rule* next;

    while(cur != NULL) {
        next = cur->next;
        if(cur->m_type == RULE_TYPE_GOTO) {
            cur->goto_chain->m_ref --;
            try_delete_chain(cur->goto_chain);
        } else if(cur->m_type == RULE_TYPE_CALL) {
            free(cur->call_fn_name);
        }
        free(cur);
        cur = next;
    }

    return;
}

void print_chain(struct chain_rule* chain)
{
    if(chain == NULL) {
        printf("(null)");
    } else {
        print_pattern(chain->m_pattern);
        switch(chain->m_type) {
            case(RULE_TYPE_LOG):
                printf(" log -> ");
                break;
            case(RULE_TYPE_GOTO):
                printf(" goto (");
                print_chain(chain->goto_chain);
                printf(") -> ");
                break;
            case(RULE_TYPE_RETURN):
                printf(" return -> ");
                break;
            case(RULE_TYPE_CONTINUE):
                printf(" continue -> ");
                break;
            case(RULE_TYPE_CALL):
                printf(" call %s -> ", chain->call_fn_name);
                break;
            case(RULE_TYPE_DROP):
                printf(" drop -> ");
                break;
        }
        print_chain(chain->next);
    }
}

void print_pattern(struct pattern* pat)
{
    if(!pat) return;
    printf("(");
    if(pat->features & HAS_SRC_MAC_ADDR) {
        printf("src_mac=%02x:%02x:%02x:%02x:%02x:%02x ",
            pat->src_mac_addr[0],
            pat->src_mac_addr[1],
            pat->src_mac_addr[2],
            pat->src_mac_addr[3],
            pat->src_mac_addr[4],
            pat->src_mac_addr[5]
        );
    }
    if(pat->features & HAS_DEST_MAC_ADDR) {
        printf("dest_mac=%02x:%02x:%02x:%02x:%02x:%02x ",
            pat->src_mac_addr[0],
            pat->src_mac_addr[1],
            pat->src_mac_addr[2],
            pat->src_mac_addr[3],
            pat->src_mac_addr[4],
            pat->src_mac_addr[5]
        );
    }
    printf(")");
}
