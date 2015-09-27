#ifndef INCLUDE_INTMAP_
#define INCLUDE_INTMAP_

#include "types.h"

struct intmap_0;
struct intmap {
    struct intmap_0* internal;

    void* (*get)(struct intmap*, u32_t key);
    void* (*insert)(struct intmap*, u32_t key, void* val);
};
typedef struct intmap intmap_t;

intmap_t* new_intmap();
void* intmap_get(intmap_t* map, u32_t key);
void* intmap_insert(intmap_t* map, u32_t key, void* val);
void* intmap_remove(intmap_t* map, u32_t key);
void  intmap_free(intmap_t* map, void(*)(void*));

#endif /* INCLUDE_INTMAP_ */
