#include "bus.h"

#include <pthread.h>
#include "tree/tree.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "blocking_queue.h"

struct bus_event_tree;

/* Each event type has its own tree. This
 * is the definition of a node that maps
 * a string to one of these trees. */
struct bus_event_type_node {
    RB_ENTRY(bus_event_type_node) entry;

    const char* key;
    struct bus_event_tree* val;
};

/* compare two nodes. */
static int bus_event_type_node_cmp(struct bus_event_type_node* v1,
                                   struct bus_event_type_node* v2)
{
    return strcmp(v1->key, v2->key);
}

RB_HEAD(bus_event_type_tree, bus_event_type_node);
RB_GENERATE(bus_event_type_tree, bus_event_type_node, entry, bus_event_type_node_cmp);

struct handler {
    void(*hdlr)(void*, void*);
    void* data;
};

struct bus_event_node {
    RB_ENTRY(bus_event_node) entry;  

    event_id_t key;

    /* Normal vector type */
    struct handler* m_listeners;
    size_t n_listeners;
    size_t n_alloc;
};

static int bus_event_node_cmp(struct bus_event_node* v1,
                              struct bus_event_node* v2)
{
    return v1->key - v2->key;
}

RB_HEAD(bus_event_tree, bus_event_node);
RB_GENERATE(bus_event_tree, bus_event_node, entry, bus_event_node_cmp);

struct BUS__ {
    struct bus_event_type_tree* m_event_tree;
    blocking_queue_t* m_event_queue;
};

static struct bus_event_type_node* bus_event_type_tree_index(struct bus_event_type_tree* tr,
                                                             const char* key)
{
    struct bus_event_type_node tmp;
    struct bus_event_type_node* ret;

    tmp.key = key;
    ret = RB_FIND(bus_event_type_tree, tr, &tmp);
    if(ret) {
        return ret;
    } else {
        ret = calloc(sizeof(struct bus_event_type_node), 1);
        ret->key = key;
        ret->val = NULL;
        RB_INSERT(bus_event_type_tree, tr, ret);
        return ret;
    }
}

static struct bus_event_tree* make_new_event_tree()
{
    struct bus_event_tree tmp = RB_INITIALIZER(&tmp);
    struct bus_event_tree* ret = calloc(sizeof(struct bus_event_tree), 1);
    *ret = tmp;
    return ret;
}

static void bus_event_node_add_handler(struct bus_event_node* node,
                                       struct handler* hdlr)
{
    if(!node->m_listeners) {
        node->m_listeners = calloc(sizeof(struct handler), 10);
        node->n_alloc = 10;
        node->n_listeners = 0;
    }

    if(node->n_alloc == node->n_listeners) {
        node->n_alloc *= 2;
        node->m_listeners = realloc(node->m_listeners, sizeof(struct handler) * node->n_alloc);
    }

    node->m_listeners[node->n_listeners++] = *hdlr;
}

static void bus_event_tree_insert_handler(struct bus_event_tree* tr,
                                          struct handler* hdlr,
                                          event_id_t evt)
{
    struct bus_event_node tmp;
    struct bus_event_node* node;
    tmp.key = evt;

    node = RB_FIND(bus_event_tree, tr, &tmp);

    if(!node) {
        node = calloc(sizeof(struct bus_event_node), 1);
        node->key = evt;
        RB_INSERT(bus_event_tree, tr, node);
    }
    
    bus_event_node_add_handler(node, hdlr);
}

static void bus_insert_event_handler(bus_t* bus,
                                     const char* event_type, 
                                     struct handler* hdlr,
                                     event_id_t evt)
{
    struct bus_event_type_node* node =
        bus_event_type_tree_index(bus->m_event_tree, event_type);

    if(!node->val) {
        node->val = make_new_event_tree();
    }

    bus_event_tree_insert_handler(node->val, hdlr, evt);
}


void bus__bind_event__(bus_t* bus,
                       const char* event_type,
                       void* hdlr,
                       void* external,
                       event_id_t evt)
{
    struct handler hdlr_str;
    hdlr_str.hdlr = hdlr;
    hdlr_str.data = external;

    bus_insert_event_handler(bus, event_type, &hdlr_str, evt);
}

static void bus_event_node_fire_event(struct bus_event_node* node,
                                      struct BUS__EVENT__* evt)
{
    size_t i;
    struct handler* hdlr;

    for(i = 0; i < node->n_listeners; ++ i) {
        hdlr = &node->m_listeners[i];
        hdlr->hdlr(hdlr->data, evt);
    }
}

static void bus_event_delete(struct BUS__EVENT__* evt)
{
    if(evt->delete_internal) {
        evt->delete_internal(evt + 1);
    }
}

static void bus_handle_event(bus_t* bus, struct BUS__EVENT__* evt)
{
    if(!evt) return;

    struct bus_event_type_node* node =
        bus_event_type_tree_index(bus->m_event_tree, evt->evt_type);

    struct bus_event_tree* tr = node->val;
    if(tr) {
        struct bus_event_node node;
        node.key = evt->evt_id;
        struct bus_event_node* rnode = RB_FIND(bus_event_tree, tr, &node);
        if(rnode) {
            bus_event_node_fire_event(rnode, evt);
        }
    }
}

void* bus_pthread_main(void* bus_)
{
    bus_t* bus = bus_;
    struct BUS__EVENT__* evt;
    int rc;
    while(1) {
        rc = blocking_queue_take(bus->m_event_queue, (void**)(&evt), 10000);
        if(rc == BQ_OK) {
            bus_handle_event(bus, evt);
            bus_event_delete(evt);
        }
    }
    pthread_exit(NULL);
}

int bus_start(pthread_t* out, bus_t* bus)
{
    pthread_t ret;
    int rc;
    rc = pthread_create(&ret, NULL, bus_pthread_main, bus);
    if(out) *out = ret;
    return rc;
}

void bus__enqueue_event__(bus_t* bus, struct BUS__EVENT__* evt)
{
    blocking_queue_add(bus->m_event_queue, evt);
}


bus_t* new_bus()
{
    bus_t* ret = malloc(sizeof(bus_t));   
    ret->m_event_tree = calloc(sizeof(struct bus_event_type_tree), 1);
    struct bus_event_type_tree tmp = RB_INITIALIZER(&tmp);
    *ret->m_event_tree = tmp;

    ret->m_event_queue = new_blocking_queue();

    return ret;
}
