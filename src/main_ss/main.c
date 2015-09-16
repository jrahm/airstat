#include "socket_ss/socket_ss.h"
#include "ether_ss/ether_ss.h"
#include "iphdr_ss/iphdr_ss.h"

#include "bus.h"
#include "events.h"
#include "chain_ss/chain.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    int raw_socket, ret;
    options_t opts;

    global_bus = new_bus();
    if(bus_start(NULL, global_bus)) {
        perror("Failed to start communication bus.");
        exit(1);
    }

    struct chain_set* chains;
    chains = parse_chains_from_file("tmp.conf");
    if(!chains) {
        printf("Error parsing chains: %s\n", get_error());
    } else {
        printf("Ether: ");
        print_chain(chains->ether_chain_head);
        printf("\nIp:    ");
        print_chain(chains->ip_chain_head);
        printf("\nTcp:   ");
        print_chain(chains->tcp_chain_head);
        printf("\nUdp:   ");
        print_chain(chains->udp_chain_head);
        printf("\n");

        free_chain(chains->ether_chain_head);
        free_chain(chains->ip_chain_head);
        free_chain(chains->tcp_chain_head);
        free_chain(chains->udp_chain_head);
        free(chains);
    }

    init_packet_handlers();
    init_iphdr_ss();

    ret = parse_options(&opts, argc, argv);
    if(ret) {
        printf("Error parsing command line options: %s\n", get_options_error());
        return ret;
    } else {
        return run_socket_ss(&opts);
    }
}
