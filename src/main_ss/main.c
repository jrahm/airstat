#include "socket_ss/socket_ss.h"
#include "ether_ss/ether_ss.h"
#include "iphdr_ss/iphdr_ss.h"

#include "bus.h"
#include "events.h"
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
