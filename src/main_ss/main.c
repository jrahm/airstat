#include "socket_ss/socket_ss.h"
#include "ether_ss/ether_ss.h"
#include "iphdr_ss/iphdr_ss.h"

#include "bus.h"
#include "events.h"
#include "chain_ss/chain.h"
#include "chain_ss/chain_ss.h"
#include "plugin_ss/plugin_ss.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    int ret;
    options_t opts;
    struct chain_set* chains;
    struct chain_ctx* chain_context;
    struct plugin* plugin_chain;

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

    chains = parse_chains_from_file("tmp.conf", plugin_chain);
    if(!chains) {
        printf("Error parsing chains: %s\n", get_error());
        return 1;
    }

    printf("Ether: ");
    print_chain(chains->ether_chain_head);
    printf("\nIp:    ");
    print_chain(chains->ip_chain_head);
    printf("\nTcp:   ");
    print_chain(chains->tcp_chain_head);
    printf("\nUdp:   ");
    print_chain(chains->udp_chain_head);
    printf("\n");

    chain_context = create_chain_ctx(1, chains);
    if(!chain_context) {
        fprintf(stderr, "Failed to create chain context\n");
        return 2;
    }

    if(start_chain_ctx(chain_context)) {
        fprintf(stderr, "Failed to start chain context");
        return 3;
    }

    ret = parse_options(&opts, argc, argv);
    if(ret) {
        printf("Error parsing command line options: %s\n", get_options_error());
        return ret;
    } else {
        return run_socket_ss(&opts);
    }
}
