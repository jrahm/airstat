#ifndef SRC_PLUGIN_
#define SRC_PLUGIN_

#include "exported_structures.h"
#define ENOIMPL -0xff

#define AIRSTAT_SOURCE_PLUGIN(name, magic) \
    const char* get_airstat_plugin_type() { return "SOURCE"; } \
    const char* get_airstat_plugin_name() { return name; } \
    u32_t get_airstat_plugin_magic() { return magic; } \


#define AIRSTAT_SINK_PLUGIN(name) \
    const char* get_airstat_plugin_type() { return "SINK"; } \
    const char* get_airstat_plugin_name() { return name; }

#define AIRSTAT_EXPORT_BEGIN() \
    struct airstat_export_plugin_routine_ { \
        const char* name; \
        void* fn; \
    }; \
    struct airstat_export_plugin_routine_ exported_routines__[] = {

#define AIRSTAT_EXPORT(fn) \
    {#fn, fn},

#define AIRSTAT_EXPORT_END() \
    }; \
    struct airstat_export_plugin_routine_* get_airstat_exported_routines(size_t* n) \
    { \
        *n = sizeof(exported_routines__) / sizeof(struct airstat_export_plugin_routine_); \
        return exported_routines__; \
    }

#endif /* SRC_PLUGIN_ */
