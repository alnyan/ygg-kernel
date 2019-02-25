#pragma once
#include <stddef.h>
#include <stdint.h>

#define MAC_FMT             "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_BYTES(x)        x[0], x[1], x[2], x[3], x[4], x[5]

#define ETH_TYPE_INET   0x0800
#define ETH_TYPE_ARP    0x0806
#define ETH_TYPE_RARP   0x8035
#define ETH_TYPE_INET6  0x86DD

struct eth_hdr {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
} __attribute__((packed));

void eth_dump_packet(const void *buf, size_t size);
int eth_handle_packet(const void *buf, size_t size);
