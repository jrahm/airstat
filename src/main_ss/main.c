#include "socket_ss/socket_ss.h"
#include "ether_ss/ether_ss.h"
#include "iphdr_ss/iphdr_ss.h"

#include "bus.h"
#include "events.h"
#include "chain_ss/chain.h"
#include "chain_ss/chain_ss.h"
#include "plugin_ss/plugin_ss.h"
#include "intmap.h"
#include "exported_structures.h"

#include <stdio.h>
#include <poll.h>

static string_map_t* plugin_chain_to_string_map(struct plugin* chain)
{
    string_map_t* ret = new_string_map();
    for(; chain != NULL; ++ chain) {
        ret->insert(ret, chain->name, chain);
    }
    return ret;
}

static int initialize_source_plugins(struct plugin* chain)
{
    int rc;
    int count = 0;
    while(chain->next_plugin) {
        if(chain->type == PLUGIN_TYPE_SOURCE) {
            rc = chain->source.init_routine(0, NULL, &chain->source.ctx);
            if(rc) {
                chain->source.is_initialized = 0;
                fprintf(stderr, "Failed to initialize plugin %s\n", chain->name);
            } else {
                chain->source.is_initialized = 1;
                ++ count;
            }
        }
    }

    return count;
}

static struct pollfd* setup_polls(struct plugin* chain, size_t count, size_t* rsize, intmap_t* fd_to_plugin)
{
    struct pollfd* pollfds = calloc(sizeof(struct pollfd), count);
    size_t i = 0;
    int fd;

    for(; chain && i < count; chain = chain->next_plugin) {
        if(chain->type == PLUGIN_TYPE_SOURCE &&
            chain->source.is_initialized) {

            fd = chain->source.fd_routine(chain->source.ctx);
            if(fd >= 0) {
                pollfds[i].events = POLLIN | POLLPRI;
                pollfds[i ++].fd = fd;
                intmap_insert(fd_to_plugin, fd, chain);
            } else {
                perror("Invalid file descriptor");
            }
        }
    }

    *rsize = i;
    return pollfds;
}

static int run_source_plugins(struct chain_ctx* ctx, struct plugin* chain, size_t count)
{
    size_t npolls;
    intmap_t* fd_to_plugin = new_intmap();
    struct pollfd* pollfds = setup_polls(chain, count, &npolls, fd_to_plugin);
    struct plugin* fired_plugin;
    int rc;
    size_t i;
    airstat_packet_t* packet;

    while(1) {
        rc = poll(pollfds, npolls, -1);
        if(rc > 0) {
            for(i = 0; i < npolls; ++ i) {
                if(pollfds[i].revents & POLLIN ||
                   pollfds[i].revents & POLLPRI) {
                   fired_plugin = intmap_get(fd_to_plugin, pollfds[i].fd);

                   packet = fired_plugin->source.read_packet_routine(
                        fired_plugin->source.ctx, pollfds[i].fd
                    );

                   chain_ctx_handle_incoming_packet(ctx, packet, fired_plugin->source.start_chain);
                }
            }
        } else {
            perror("Poll returned bad status");
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    struct chain_set* chains;
    struct chain_ctx* chain_context;
    struct plugin* plugin_chain;
    string_map_t* plugin_map;

    global_bus = new_bus();
    if(bus_start(NULL, global_bus)) {
        perror("Failed to start communication bus.");
        exit(1);
    }

    plugin_chain = load_plugins("plugins");
    if(!plugin_chain) {
        fprintf(stderr, "Failed to read plugins: %s\n", plugin_error);
    }
    print_plugin_chain(stdout, plugin_chain);
    plugin_map = plugin_chain_to_string_map(plugin_chain);

    struct chain_parse_ctx ctx;
    ctx.start_plugin_chain = plugin_chain;
    ctx.plugin_map_by_name = plugin_map;
    ctx.link_map = NULL;
    chains = parse_chains_from_file("tmp.conf", &ctx);
    if(!chains) {
        printf("Error parsing chains: %s\n", get_error());
        return 1;
    }

    chain_context = create_chain_ctx(1, chains);
    if(!chain_context) {
        fprintf(stderr, "Failed to create chain context\n");
        return 2;
    }

    int n_plugins;
    if((n_plugins = initialize_source_plugins(plugin_chain)) < 0) {
        fprintf(stderr, "Failed to initialize source plugins");
        return 1;
    }

    if(start_chain_ctx(chain_context)) {
        fprintf(stderr, "Failed to start chain context");
        return 3;
    }

    return run_source_plugins(chain_context, plugin_chain, n_plugins);
}
