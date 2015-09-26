#include <stdlib.h>
#include "chain_ss.h"

#include <events.h>
#include <ether_ss/ether_event.h>
#include <chain_ss/ether_chain.h>

void packet_delete(struct chain_raw_packet_data* data)
{
    free(data->packet_data.bytes);
    free(data);
}

void *chain_thread__main__(void *ctx_)
{
    struct chain_ctx *ctx = (struct chain_ctx*)ctx_;
    struct chain_raw_packet_data* packet_ptr = NULL;
    int rc;
    void(*handler)(struct chain_raw_packet_data*);

    while(true) {
        rc = blocking_queue_take(ctx->m_packet_queue_, (void**)&packet_ptr, 1000000);
        if(rc == ETIMEDOUT) {
            continue;
        }

        if(rc) {
            fprintf(stderr, "Problem with blocking queue. %d\n", rc);
        } else {
            if(!packet_ptr) {
                fprintf(stderr, "Thread exiting\n");
            } else {
                handler = packet_ptr->next_handler;
                packet_ptr->next_handler = NULL;
                handler(packet_ptr);
                if(packet_ptr->next_handler) {
                    blocking_queue_add(ctx->m_packet_queue_, packet_ptr);
                } else {
                    packet_delete(packet_ptr);
                }
            }
        }
    }

}

struct chain_ctx* create_chain_ctx(size_t nworkers, struct chain_set* chainset)
{
    struct chain_ctx* ret = NULL;
    if(!chainset)
        return NULL;

    ret = calloc(sizeof(struct chain_ctx), 1);

    ret->m_nworkers_ = nworkers;
    ret->m_workers_ = NULL;
    ret->m_packet_queue_ = new_blocking_queue();
    ret->m_chain_set_ = *chainset;

    return ret;
/* error:
    if(ret) {
        free(ret->m_packet_queue_);
    }
    free(ret);
    return NULL; */
}

void on_packet_event__(void* ctx_, struct packet_event* evt)
{
    struct chain_ctx *ctx = (struct chain_ctx*) ctx_;

    struct chain_raw_packet_data *chain_packet;
    chain_packet = calloc(sizeof(struct chain_raw_packet_data), 1);
    chain_packet->packet_data.bytes = evt->data.chrs;
    chain_packet->packet_data.sz = evt->data.sz;

    chain_packet->current_chain_rule = ctx->m_chain_set_.ether_chain_head;
    chain_packet->next_handler = to_handler(ether_chain_handle_BEGIN);

    blocking_queue_add(ctx->m_packet_queue_, chain_packet);
}

int start_chain_ctx(struct chain_ctx *ctx)
{
    ctx->m_workers_ = calloc(sizeof(pthread_t), ctx->m_nworkers_);
    size_t i;

    for(i = 0; i < ctx->m_nworkers_; ++ i) {
        pthread_create(&ctx->m_workers_[i],
            NULL, chain_thread__main__, ctx);
    }

    BIND(packet_event, on_packet_event__, ctx, ID_PACKET_RECIEVED);

    return 0;
}
