#ifndef BUS_H_
#define BUS_H_

/*
 * Author: jrahm
 * created: 2015/09/11
 * bus.h: A communication bus between different modules
 *        of the same solution.
 */

#include <stdio.h>
#include <pthread.h>

struct BUS__;
typedef struct BUS__ bus_t;

typedef unsigned int event_id_t;

struct BUS__EVENT__ {
    const char* evt_type;
    event_id_t evt_id;
    void (*delete_internal)(void*);
};

#define DECLARE_EVENT_TYPE(evt_name, evt_internal, internal_destructor) \
    struct evt_name { \
        struct BUS__EVENT__ super; \
        evt_internal data; \
    }; \
    struct evt_name* new_##evt_name(const evt_internal* in, event_id_t id); \
    void bus_enqueue_##evt_name(bus_t*b, struct evt_name* evt); \
    void bus_##evt_name##_bind(bus_t* bus, \
        void(*handler)(void* extrnl_data, struct evt_name* evt), \
        void* extrnl_data, event_id_t evt_id);

#define DEFINE_EVENT_TYPE(EVT_NAME, EVT_INTERNAL, INTERNAL_DESTRUCTOR) \
    struct EVT_NAME* new_##EVT_NAME(const EVT_INTERNAL* internal, event_id_t id) { \
        struct EVT_NAME* ret = malloc(sizeof(struct EVT_NAME)); \
        memset(ret, 0, sizeof(struct EVT_NAME)); \
        ret->super.evt_type = #EVT_NAME; \
        ret->super.evt_id = id; \
        ret->super.delete_internal = (void(*)(void*))INTERNAL_DESTRUCTOR; \
        ret->data = *internal; \
        return ret; \
    } \
    void bus_enqueue_##EVT_NAME(bus_t* bus, struct EVT_NAME* evt) { \
        bus__enqueue_event__(bus, (struct BUS__EVENT__*) evt); \
    } \
    void bus_##EVT_NAME##_bind(bus_t* bus,\
        void(*hdlr)(void* extrnl_data, struct EVT_NAME* evt), void* extrl,\
        event_id_t evt_id) { \
        bus__bind_event__(bus, #EVT_NAME, hdlr, extrl, evt_id); \
    }

bus_t* new_bus();
int bus_start(pthread_t* out, bus_t* bus);

void bus__bind_event__(bus_t* bus, const char* event_type, void* hdlr, void* external, event_id_t evt);
void bus__enqueue_event__(bus_t* bus, struct BUS__EVENT__* evt);

#endif /* BUS_ */
