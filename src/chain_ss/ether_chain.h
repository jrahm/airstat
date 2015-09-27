#ifndef SRC_CHAIN_SS_ETHER_CHAIN_
#define SRC_CHAIN_SS_ETHER_CHAIN_

#include "chain_ss.h"

void chain_handle_BEGIN   (struct chain_raw_packet_data* data);
void chain_handle_return  (struct chain_raw_packet_data* data);
void chain_handle_continue(struct chain_raw_packet_data* data);
void chain_handle_drop    (struct chain_raw_packet_data* data);
void chain_handle_call    (struct chain_raw_packet_data* data);
void chain_handle_log     (struct chain_raw_packet_data* data);
void chain_handle_goto    (struct chain_raw_packet_data* data);

#endif /* SRC_CHAIN_SS_ETHER_CHAIN_ */
