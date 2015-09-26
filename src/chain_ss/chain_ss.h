#ifndef SRC_CHAIN_SS_CHAIN_SS_
#define SRC_CHAIN_SS_CHAIN_SS_

#include "types.h"
#include "blocking_queue.h"

#include <pthread.h>
#include "chain.h"

#define to_handler(a) \
    ((void(*)(struct chain_raw_packet_data*))a)

struct chain_ctx {
    /* blocking queue of type chain_raw_packet_data */
    blocking_queue_t *m_packet_queue_;
    size_t            m_nworkers_;
    pthread_t        *m_workers_;

    struct chain_set  m_chain_set_;
};

struct chain_raw_packet_data {
    void (*next_handler)(struct chain_raw_packet_data* data);
    struct chain_rule* current_chain_rule;
    struct chain_ctx* issuer;
    bool packet_continue;

    size_t sz;
    u8_t* bytes;
};


struct chain_ctx *create_chain_ctx(size_t nworkers, struct chain_set* chainset);
int start_chain_ctx(struct chain_ctx *ctx);

#endif /* SRC_CHAIN_SS_CHAIN_SS_ */
