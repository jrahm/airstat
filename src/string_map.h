#ifndef SRC_STRING_MAP_
#define SRC_STRING_MAP_


/*
 * Author: jrahm
 * created: 2015/09/15
 * string_map.h: <description>
 */

struct string_map_;
struct string_map {
    struct string_map_* map;

    void* (*insert)(struct string_map* ths, const char* key, void* value);
    void* (*remove)(struct string_map* map, const char* key);
    int   (*has_key)(struct string_map* map, const char* key);
    void* (*get)(struct string_map* map, const char* key);
    void  (*free)(struct string_map* map, void(*)(void*));
};

typedef struct string_map string_map_t;

struct string_map* new_string_map();
void* string_map_insert(struct string_map* map, const char* key, void* value);
void* string_map_remove(struct string_map* map, const char* key);
int   string_map_has_key(struct string_map* map, const char* key);
void* string_map_get(struct string_map* map, const char* key);
void string_map_free(struct string_map* map, void(*)(void*));

#endif /* SRC_STRING_MAP_ */
