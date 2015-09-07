#ifndef SRC_PARSE_OPTIONS_
#define SRC_PARSE_OPTIONS_

typedef struct {
    char interface[1024];
} options_t;

int parse_options(options_t* opts, int argc, char** argv);
const char* get_options_error();

#endif /* SRC_PARSE_OPTIONS_ */
