#include "parse_options.h"

#include <string.h>

char option_error[1024];

const char* get_options_error()
{
    return option_error;
}

int parse_options(options_t* opts, int argc, char** argv)
{
    (void) argc; (void) argv;
    if(argc < 2) {
        strncpy(option_error, "Need to specify interface to listen on", 1024);
        return 1;
    }

    strncpy(opts->interface, argv[1], sizeof(opts->interface));

    return 0;
}
