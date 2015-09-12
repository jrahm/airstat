#include "socket_ss.h"
#include "packet_handlers.h"
#include "bus.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    int raw_socket, ret;
    options_t opts;

    bus_t* com_bus = new_bus();
    if(bus_start(NULL, com_bus)) {
        perror("Failed to start communication bus.");
        exit(1);
    }

    if(init_packet_handlers(com_bus)) {
        fprintf(stderr, "Failed to initialize listeners\n");
        exit(2);
    }

    ret = parse_options(&opts, argc, argv);
    if(ret) {
        printf("Error parsing command line options: %s\n", get_options_error());
        return ret;
    } else {
        return run_socket_ss(&opts, com_bus);
    }
}
