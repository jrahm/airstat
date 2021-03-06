#include "plugin_ss.h"

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

char plugin_error[128];

static void free_one_plugin(struct plugin* plug)
{
    free(plug->name);
}

void free_plugin_chain(struct plugin* plug)
{
    if(!plug) return;
    free_one_plugin(plug);
    free_plugin_chain(plug->next_plugin);
    free(plug);
}

static struct plugin* new_plugin()
{
    return calloc(sizeof(struct plugin), 1);
}

static int get_plugin_type(void* so_handle, char* out_type, size_t n)
{
    const char* (*get_type)();
    const char* error;
    get_type = dlsym(so_handle, "get_airstat_plugin_type");

    if((error = dlerror()) != NULL) {
        snprintf(plugin_error, sizeof(plugin_error), "%s", error);
        return 1;
    }

    printf("well this is a pain in the ass!! = %s\n", get_type());
    snprintf(out_type, n, "%s", get_type());
    return 0;
}

static int make_consumer_plugin(void* handle, struct plugin** out)
{
    struct consumer_routine*(*get_routines)(size_t*);

    const char* (*get_name)();
    get_name = dlsym(handle, "get_airstat_plugin_name");
    const char* error;
    if((error = dlerror()) != NULL) {
        snprintf(plugin_error, sizeof(plugin_error), "%s", error);
        return 1;
    }

    get_routines = dlsym(handle, "get_airstat_exported_routines");
    if((error = dlerror()) != NULL) {
        snprintf(plugin_error, sizeof(plugin_error), "%s", error);
        return 1;
    }

    *out = new_plugin();
    (*out)->type = PLUGIN_TYPE_CONSUMER;
    (*out)->name = strdup(get_name());
    (*out)->consumer.routines = get_routines(&(*out)->consumer.n_routines);

    return 0;
}

static int read_plugin(const char* filepath, struct plugin** out)
{
    void* handle;
    char type[128];
    int rc = 0;
    type[127] = 0;

    handle = dlopen(filepath, RTLD_NOW);
    if(!handle) {
        snprintf(plugin_error, 128, "Failed to open .so: %s", dlerror());
        rc = 1;
        goto error;
    }

    rc = get_plugin_type(handle, type, 127);
    if(rc) goto error;

    if(!strcmp(type, "CONSUMER")) {
        rc = make_consumer_plugin(handle, out);
        if(rc) goto error;
    } else {
        /* other plugin types not implemented */
        snprintf(plugin_error, sizeof(plugin_error), "Unknown plugin type: %s", type);
        rc = 2;
        goto error;
    }

    return rc;
error:
    if(handle)
        dlclose(handle);
    return rc;

}

static int file_loadable(const char* file)
{
    struct stat statf;
    stat(file, &statf);
    return S_ISREG(statf.st_mode) ||
           S_ISLNK(statf.st_mode);
}

struct plugin* load_plugins(const char* dir)
{
    struct dirent* ent;
    struct plugin* start = NULL;
    struct plugin* cursor = NULL;
    struct plugin* tmp;

    int rc;
    char fpath[1024];

    DIR* dirp;

    if(!(dirp = opendir(dir))) {
        snprintf(plugin_error, sizeof(plugin_error), "%s", strerror(errno));
        return NULL;
    }

    strcpy(plugin_error, "No plugins in directory");
    while((ent = readdir(dirp))) {
        snprintf(fpath, sizeof(fpath), "%s/%s", dir, ent->d_name);

        if(file_loadable(fpath)) {
            rc = read_plugin(fpath, &tmp);
            if(rc) {
                free_plugin_chain(start);
                start = NULL;
                break;
            }
    
            if(cursor == NULL) {
                cursor = start = tmp;
            } else {
                cursor->next_plugin = tmp;
                cursor = cursor->next_plugin;
            }
        }
    }

    closedir(dirp);

    return start;
}

static void print_one_plugin(FILE* out, struct plugin* pl)
{
    size_t i;
    fprintf(out, "Plugin %s loaded type %d\n", pl->name, pl->type);
    if(pl->type == PLUGIN_TYPE_CONSUMER) {
        for(i = 0; i < pl->consumer.n_routines; ++ i) {
            fprintf(out, "    Consumer %s %p\n", pl->consumer.routines[i].name
                                               , pl->consumer.routines[i].routine);
        }
    }
}

void print_plugin_chain(FILE* out, struct plugin* pl)
{
    if(!pl) return;
    print_one_plugin(out, pl);
    print_plugin_chain(out, pl->next_plugin);
}
