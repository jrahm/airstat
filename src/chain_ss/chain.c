#include "chain.h"

#include "string_map.h"
#include <stdlib.h>
#include <ctype.h>

char error[1024];

char* next_token(FILE* fd, size_t* len, int (*get_class)(char))
{
    int ch;

    char* retbuf = malloc(128);
    size_t nalloc = 128;
    size_t size = 0;
    int class;

    ch = fgetc(fd);
    if(ch == -1) return NULL;

    class = get_class(ch);
    size = 1;
    retbuf[0] = ch;

    while((ch = fgetc(fd)) != -1 && class == get_class(ch)) {
        if(size == nalloc) {
            nalloc *= 2;
            retbuf = realloc(retbuf, nalloc * sizeof(char));
        }
        retbuf[size ++] = (char)ch;
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
    } else if(isalnum(ch)) {
        return 1;
    } else {
        return 2;
    }
}

static int not_paren(char ch)
{
    if(ch == ')') return 1;
    else if(isspace(ch)) return 2;
    else return 3;
}

struct pattern* compile_pattern(const char* pat)
{
    (void) pat;
    return NULL;
}

struct chain_rule* read_one_chain_rule(char* token, FILE* fd)
{
    struct chain_rule* ret = calloc(sizeof(struct chain_rule), 1);
    struct pattern* pat = NULL;

    if(token[0] == '(') {
        token = next_token_skip_space(fd, NULL, not_paren);
        pat = compile_pattern(token);
        free(token);
        token = next_token_skip_space(fd, NULL, not_paren);
        free(token);
    }

    if(strcmp(token, "return") == 0) {
        
    }
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
        snprintf(error, sizeof(error), "Syntax error near %s\n", token);
        goto error;
    }

    while(1) {
        token = next_token_skip_space(fd, &len, default_class);
        if(token && token[0] == ';') {
            free(token);
        } else if(strcmp(token, "chain") == 0) {
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
            tmp = read_one_chain_rule(token, fd);
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
}

struct chain_set* parse_chains_from_file(FILE* fd)
{
}
