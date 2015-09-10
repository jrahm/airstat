#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>

#include <net/if.h>

#include "parse_options.h"

typedef unsigned int uint_t;
typedef unsigned char u8_t;

void print_hex(const u8_t* chrs, size_t sz)
{
    size_t i;
    uint_t ch;
    for(i = 0; i < sz; ++ i) {
        ch = chrs[i];
        printf("%02x ", ch);
        if(i & 0xF == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void read_packet_from_socket(options_t* opts, int raw_socket)
{
    printf("Reading packet from interface %s\n", opts->interface);
    u8_t buffer[65536];
    int datasize;

    datasize = recvfrom(raw_socket, buffer, sizeof(buffer), 0, NULL, NULL);
    printf("Recieved %d bytes from fd %d.\n", datasize, raw_socket);
    print_hex(buffer, datasize);
}

int set_promiscuous_mode(options_t* opts, int fd)
{
    /*
     * Put a socket into promiscuous mode. If this function
     * fails, it will fail with the return code and the errno
     * will be set properly.
     */

    int ret;
    struct ifreq ifopts;
    strncpy(ifopts.ifr_name, opts->interface, sizeof(ifopts.ifr_name));

    ret = ioctl(fd, SIOCSIFFLAGS, &ifopts);
    if(ret) return ret;
    
    ifopts.ifr_flags |= IFF_PROMISC;

    ret = ioctl(fd, SIOCSIFFLAGS, &ifopts);
    if(ret) return ret;
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
    return setsockopt(fd,
            SOL_SOCKET,
            SO_BINDTODEVICE,
            opts->interface,
            strlen(opts->interface));
}

int run_with_raw_socket(options_t* opts, int raw_socket)
{
    int ret;

    // ret = set_promiscuous_mode(opts, raw_socket);

    // if(ret) {
    //     perror("Unable to set promiscuous mode");
    //     return 1;
    // }

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
        read_packet_from_socket(opts, raw_socket);
    }

    return 0;
}

int main(int argc, char** argv)
{
    int raw_socket, ret;
    options_t opts;

    ret = parse_options(&opts, argc, argv);
    if(ret) {
        printf("Error parsing command line options: %s\n", get_options_error());
        return ret;
    } else {
        raw_socket = socket(PF_PACKET, SOCK_RAW, htons(0x0800));
    
        if(raw_socket < 0) {
            perror("Error creating raw socket");
        } else {
            run_with_raw_socket(&opts, raw_socket);
            close(raw_socket);
        }
    }
}
