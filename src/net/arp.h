#pragma once
#include <stdint.h>
#include "dev/net.h"

#define ARP_WHO_HAS     1
#define ARP_RESPONSE    2

struct arp_msg {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];
    uint32_t spa;
    uint8_t tha[6];
    uint32_t tpa;
} __attribute__((packed));

void arp_dump_packet(struct arp_msg *msg);
int arp_handle_packet(netdev_t *dev, struct arp_msg *msg);
void arp_request_in_hwaddr(netdev_t *dev, uint32_t inaddr, uint32_t src);
