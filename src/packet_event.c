#include "packet_event.h"

void delete_packet_data(struct packet_data* data)
{
    free(data->chrs);
}

DEFINE_EVENT_TYPE(packet_event, struct packet_data, delete_packet_data);
