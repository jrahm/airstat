#include <string_map.h>
#include <tree/tree.h>

#include <string.h>
#include <stdlib.h>

struct string_map_node {
    RB_ENTRY(string_map_node) entry;

    char* key;
    void* value;
};

static int string_map_node_cmp(struct string_map_node* n1,
                               struct string_map_node* n2)
{
    return strcmp(n1->key, n2->key);
}


RB_HEAD(string_map, string_map_node);
RB_GENERATE(string_map, string_map_node, entry, string_map_node_cmp);

static struct string_map_node* string_map_find_node(struct string_map* map, const char* key)
{
    struct string_map_node fnode;
    fnode.key = strdup(key);

    struct string_map_node* node = RB_FIND(string_map, map, &fnode);
    free(fnode.key);

    return node;
}

struct string_map* new_string_map()
{
    struct string_map* init = malloc(sizeof(struct string_map));
    struct string_map tmp = RB_INITIALIZER(&tmp);
    *init = tmp;
    return init;
}

void* string_map_insert(struct string_map* map, const char* key, void* value)
{
    struct string_map_node* node;
    void* ret;

    node = string_map_find_node(map, key);

    if(node) {
        ret = node->value;
        node->value = value;
    } else {
        ret = NULL;
        node = malloc(sizeof(struct string_map_node));
        node->key = strdup(key);
        node->value = value;
        RB_INSERT(string_map, map, node);
    }

    return ret;
}

void* string_map_remove(struct string_map* map, const char* key)
{
    struct string_map_node* node;
    void* ret = NULL;

    node = string_map_find_node(map, key);

    if(node) {
        ret = node->value;
        free(node->key);
        RB_REMOVE(string_map, map, node);
    }

    return ret;
}

int string_map_has_key(struct string_map* map, const char* key)
{
    return string_map_find_node(map, key) != NULL;
}

void* string_map_get(struct string_map* map, const char* key)
{
    struct string_map_node* node;
    node = string_map_find_node(map, key);
    if(node) {
        return node->value;
    } else {
        return NULL;
    }
}

void string_map_free(struct string_map* map, void(*callback)(void*))
{
	/* Free tree. */
    struct string_map_node* np;
    struct string_map_node* op;

	for (np = RB_MIN(string_map, map); np != NULL; np = op) {
		op = RB_NEXT(string_map, map, np);
		RB_REMOVE(string_map, map, np);
        callback(np->value);
        free(np->key);
		free(np);
	}

    free(map);
}

