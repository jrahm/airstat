#ifndef SRC_PLUGIN_SS_PLUGIN_SS_
#define SRC_PLUGIN_SS_PLUGIN_SS_

#include <stdlib.h>
#include <stdio.h>

#include "chain_ss/chain.h"
#include "exported_structures.h"

struct string_map;

enum plugin_type {
      PLUGIN_TYPE_SINK
    , PLUGIN_TYPE_SOURCE
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

        struct {
            /* Each source has to have a type associated with it */
            u32_t magic;

            /* Initializes the plugin. It initializes the context
             * in ctx_out and returns an error code if the routine fails. */
            int (*init_routine)(int argc, char** argv, void** ctx_out);

            /* Routine to construct the file descriptor to listen on.
             * The main program will the use poll to extract the packet
             * from the routine */
            int   (*fd_routine)(void*);

            /* Once the file descriptor fires, we then call this
             * routine on the correct plugin to create the packet */
            airstat_packet_t* (*read_packet_routine)(void*, int);

            /* This is used in the chain compilation phase, it constructs
             * a pattern using the string map provided */
            pattern_t* (*compile_pattern)(void*, struct string_map* st);

            /* The name of the default chain to use */
            char* init_chain;

            /* Not provided by the plugin, rather it is 'linked' by
             * the name */
            struct chain_rule* start_chain;
            
            void* ctx;
            int is_initialized;
        } source;
    };

    struct plugin* next_plugin;
};

extern char plugin_error[128];
struct plugin* load_plugins(const char* directory);
void free_plugin_chain(struct plugin* plugin);
void free_one_plugin(struct plugin* plugin);
void print_plugin_chain(FILE* out, const struct plugin* pl);

#endif /* SRC_PLUGIN_SS_PLUGIN_SS_ */
