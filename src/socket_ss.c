#include "socket_ss.h"

#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "bus.h"
#include "events.h"
#include "packet_event.h"
#include "packet_handlers.h"
#include "parse_options.h"


void read_packet_from_socket(bus_t* bus, options_t* opts, int raw_socket)
{
    u8_t buffer[65536];
    int datasize;

    datasize = recvfrom(raw_socket, buffer, sizeof(buffer), 0, NULL, NULL);

    struct packet_data data;
    data.chrs = malloc(datasize);
    memcpy(data.chrs, buffer, datasize);
    data.sz = datasize;

    bus_enqueue_packet_event(bus, new_packet_event(&data, ID_PACKET_RECIEVED));
}

int set_reuse_socket(options_t* opts, int fd)
{
    (void) opts;
    int sockopt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt)) {
        return 1;
    }

    return 0;
}

int bind_to_interface(options_t* opts, int fd)
{
    struct ifreq ifopts;

    memset(&ifopts, 0, sizeof(ifopts));
    strncpy(ifopts.ifr_name, opts->interface, sizeof(ifopts.ifr_name));

    return setsockopt(fd,
            SOL_SOCKET,
            SO_BINDTODEVICE,
            &ifopts,
            sizeof(ifopts));
}

int run_with_raw_socket(bus_t* bus, options_t* opts, int raw_socket)
{
    int ret;
    ret = set_reuse_socket(opts, raw_socket);

    if(ret) {
        perror("Unable to set reuse socket");
        return 1;
    }

    ret = bind_to_interface(opts, raw_socket);

    if(ret) {
        perror("Unable to bind socket to interface");
        return 1;
    }

    while(1) {
        read_packet_from_socket(bus, opts, raw_socket);
    }

    return 0;
}

int run_socket_ss(options_t* opts, bus_t* bus)
{
    int raw_socket = socket(PF_PACKET, SOCK_RAW, htons(0x0800));

    if(raw_socket < 0) {
        perror("Error creating raw socket");
    } else {
        run_with_raw_socket(bus, opts, raw_socket);
        close(raw_socket);
    }
}
