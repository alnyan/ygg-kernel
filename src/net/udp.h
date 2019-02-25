#pragma once
#include "inet.h"

struct udp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
};
