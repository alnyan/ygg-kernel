#pragma once
#include "eth/eth.h"

#define INET_PROTO_UDP  0x11

#if defined(ARCH_X86)
#define inet_htons(x) ((((x) & 0xFF) << 8) | ((x >> 8) & 0xFF))
#define inet_htonl(x) ((((x) & 0xFF) << 24) | (((x >> 8) & 0xFF) << 16) | (((x >> 16) & 0xFF) << 8) | ((x >> 24) & 0xFF))
#else
#endif

typedef uint32_t inaddr_t;

struct in_hdr {
    uint8_t verlen;
    uint8_t tos;
    uint16_t length;
    uint16_t id;
    uint16_t flags;
    uint8_t ttl;
    uint8_t proto;
    uint16_t checksum;
    inaddr_t src_addr;
    inaddr_t dst_addr;
} __attribute__((packed));

void inet_ntoa(char *out, inaddr_t a);
uint32_t inet_aton(const char *in);
