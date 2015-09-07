#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "parse_options.h"

void print_hex(const char* chrs, size_t sz)
{
    size_t i;
    for(i = 0; i < sz; ++ i) {
        printf("%02x ", (int)chrs[i]);
        if(i & 0xF == 0) {
            printf("\n");
        }
    }

}

void read_packet_from_socket(int raw_socket)
{
    struct sockaddr saddr;
    socklen_t saddr_size;
    char buffer[65536];
    int datasize;

    datasize = recvfrom(raw_socket, buffer, sizeof(buffer), 0, &saddr, &saddr_size);
    print_hex(buffer, datasize);
}

int run_with_raw_socket(options_t* opts, int raw_socket)
{
    int ret;
    ret = setsockopt(raw_socket,
           SOL_SOCKET,
           SO_BINDTODEVICE,
           opts->interface,
           strlen(opts->interface));

    if(ret) {
        perror("Unable to bind to interface");
        exit(1);
    }

    while(1) {
        read_packet_from_socket(raw_socket);
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
        raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    
        if(raw_socket < 0) {
            perror("Error creating raw socket");
        } else {
            run_with_raw_socket(&opts, raw_socket);
            close(raw_socket);
        }
    }
}
