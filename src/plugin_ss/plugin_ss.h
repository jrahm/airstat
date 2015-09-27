#ifndef SRC_PLUGIN_SS_PLUGIN_SS_
#define SRC_PLUGIN_SS_PLUGIN_SS_

#include <stdlib.h>
#include <stdio.h>

enum plugin_type {
    PLUGIN_TYPE_SINK
};

struct sink_routine {
    char* name;
    void* routine;
};

struct plugin {
    enum plugin_type type;
    char* name;

    union {
        struct {
            /* not owned by this structure! */
            size_t n_routines;
            struct sink_routine* routines;
        } sink;
    };

    struct plugin* next_plugin;
};

extern char plugin_error[128];
struct plugin* load_plugins(const char* directory);
void free_plugin_chain(struct plugin* plugin);
void print_plugin_chain(FILE* out, struct plugin* pl);

#endif /* SRC_PLUGIN_SS_PLUGIN_SS_ */
