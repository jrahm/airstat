#include "intmap.h"

#include <stdlib.h>

struct intmap_3 {
    void* arr[256];
};

struct intmap_2 {
    struct intmap_3* arr[256]; 
};

struct intmap_1 {
    struct intmap_2* arr[256]; 
};

struct intmap_0 {
    struct intmap_1* arr[256]; 
};

void* intmap_0_get(struct intmap_0* ths, u32_t key)
{
    struct intmap_1* l1;
    struct intmap_2* l2;
    struct intmap_3* l3;

    if ((l1 = ths->arr[key & 0xFF]) == NULL) {
        return NULL;
    }

    key = key >> 8;
    if ((l2 = l1->arr[key & 0xFF]) == NULL) {
        return NULL;
    }

    key = key >> 8;
    if ((l3 = l2->arr[key & 0xFF]) == NULL) {
        return NULL;
    }

    key = key >> 8;
    return l3->arr[key & 0xFF];
}

void* intmap_0_insert(struct intmap_0* ths, u32_t key, void* val)
{
    struct intmap_1* l1;
    struct intmap_2* l2;
    struct intmap_3* l3;

    if ((l1 = ths->arr[key & 0xFF]) == NULL) {
        l1 = ths->arr[key & 0xFF] = calloc(sizeof(struct intmap_1), 1);
    }

    key = key >> 8;
    if ((l2 = l1->arr[key & 0xFF]) == NULL) {
        l2 = l1->arr[key & 0xFF] = calloc(sizeof(struct intmap_2), 1);
    }

    key = key >> 8;
    if ((l3 = l2->arr[key & 0xFF]) == NULL) {
        l3 = l2->arr[key & 0xFF] = calloc(sizeof(struct intmap_3), 1);
    }

    key = key >> 8;
    void* ret = l3->arr[key & 0xFF];
    l3->arr[key & 0xFF] = val;
    return ret;
}

void* intmap_0_remove(struct intmap_0* ths, u32_t key)
{
    return intmap_0_insert(ths, key, NULL);
}

void intmap_free(intmap_t* map, void(*cb)(void*)) {
    size_t i, j, k, l;

    struct intmap_1* l1;
    struct intmap_2* l2;
    struct intmap_3* l3;

    for(i = 0; i < 256; ++ i) {
        if((l1 = map->internal->arr[i]) != NULL) {
            for(j = 0; j < 256; ++ j) {
                if((l2 = l1->arr[i]) != NULL) {
                    for(k = 0; k < 256; ++ k) {
                        if((l3 = l2->arr[i]) != NULL) {
                            for(l = 0; l < 256; ++ l) {
                                if(l) cb(l2->arr[l]);
                            }
                            free(l3);
                        }
                    }
                    free(l2);
                }
            }
            free(l1);
        }
    }

    free(map->internal);
    free(map);
}

intmap_t* new_intmap()
{
    intmap_t* ret = calloc(sizeof(intmap_t), 1);
    ret->internal = calloc(sizeof(struct intmap_0), 1);

    ret->get = intmap_get;
    ret->insert = intmap_insert;
    return ret;
}
void* intmap_get(intmap_t* map, u32_t key)
{
    return intmap_0_get(map->internal, key);
}

void* intmap_insert(intmap_t* map, u32_t key, void* val)
{
    return intmap_0_insert(map->internal, key, val);
}

void* intmap_remove(intmap_t* map, u32_t key)
{
    return intmap_0_remove(map->internal, key);
}
