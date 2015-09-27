#include <string_map.h>
#include <tree/tree.h>

#include <string.h>
#include <stdlib.h>

struct string_map__node {
    RB_ENTRY(string_map__node) entry;

    char* key;
    void* value;
};

static int string_map_node_cmp(struct string_map__node* n1,
                               struct string_map__node* n2)
{
    return strcmp(n1->key, n2->key);
}


RB_HEAD(string_map_, string_map__node);
RB_GENERATE(string_map_, string_map__node, entry, string_map_node_cmp);

static struct string_map__node* string_map_find_node(struct string_map_* map, const char* key)
{
    struct string_map__node fnode;
    fnode.key = strdup(key);

    struct string_map__node* node = RB_FIND(string_map_, map, &fnode);
    free(fnode.key);

    return node;
}

struct string_map_* new_string_map_()
{
    struct string_map_* init = malloc(sizeof(struct string_map_));
    struct string_map_ tmp = RB_INITIALIZER(&tmp);
    *init = tmp;
    return init;
}

void* string_map_insert_(struct string_map_* map, const char* key, void* value)
{
    struct string_map__node* node;
    void* ret;

    node = string_map_find_node(map, key);

    if(node) {
        ret = node->value;
        node->value = value;
    } else {
        ret = NULL;
        node = malloc(sizeof(struct string_map__node));
        node->key = strdup(key);
        node->value = value;
        RB_INSERT(string_map_, map, node);
    }

    return ret;
}

void* string_map_remove_(struct string_map_* map, const char* key)
{
    struct string_map__node* node;
    void* ret = NULL;

    node = string_map_find_node(map, key);

    if(node) {
        ret = node->value;
        free(node->key);
        RB_REMOVE(string_map_, map, node);
    }

    return ret;
}

int string_map_has_key_(struct string_map_* map, const char* key)
{
    return string_map_find_node(map, key) != NULL;
}

void* string_map_get_(struct string_map_* map, const char* key)
{
    struct string_map__node* node;
    node = string_map_find_node(map, key);
    if(node) {
        return node->value;
    } else {
        return NULL;
    }
}

void string_map_free_(struct string_map_* map, void(*callback)(void*))
{
	/* Free tree. */
    struct string_map__node* np;
    struct string_map__node* op;

	for (np = RB_MIN(string_map_, map); np != NULL; np = op) {
		op = RB_NEXT(string_map_, map, np);
		RB_REMOVE(string_map_, map, np);
        if(callback)
            callback(np->value);
        free(np->key);
		free(np);
	}

    free(map);
}


struct string_map* new_string_map() 
{
    struct string_map* ret = calloc(sizeof(struct string_map), 1);
    ret->map = new_string_map_();
    ret->insert = string_map_insert;
    ret->remove = string_map_remove;
    ret->has_key = string_map_has_key;
    ret->get = string_map_get;
    ret->free = string_map_free;
    return ret;
}

void* string_map_insert(struct string_map* map, const char* key, void* value)
{
    return string_map_insert_(map->map, key, value);
}

void* string_map_remove(struct string_map* map, const char* key)
{
    return string_map_remove_(map->map, key);
}

int   string_map_has_key(struct string_map* map, const char* key)
{
    return string_map_has_key_(map->map, key);
}

void* string_map_get(struct string_map* map, const char* key)
{
    return string_map_get_(map->map, key);
}

void string_map_free(struct string_map* map, void(*cb)(void*))
{
    string_map_free_(map->map, cb);
    free(map);
}
