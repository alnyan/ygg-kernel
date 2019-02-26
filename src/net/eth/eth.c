#include "eth.h"
#include "sys/debug.h"
#include "net/inet.h"
#include "net/arp.h"

void eth_dump_packet(const void *buf, size_t size) {
    // Dump eth header
    struct eth_hdr *eth = (struct eth_hdr *) buf;
    const char *type = "unk";

    switch (inet_htons(eth->type)) {
    case ETH_TYPE_INET:
        type = "inet";
        break;
    case ETH_TYPE_ARP:
        type = "arp";
        break;
    }

    kdebug("<eth dst=" MAC_FMT " src=" MAC_FMT " type=%s\n",
        MAC_BYTES(eth->dst_mac),
        MAC_BYTES(eth->src_mac),
        type);

    switch (inet_htons(eth->type)) {
    case ETH_TYPE_ARP:
        arp_dump_packet((struct arp_msg *) (&eth[1]));
        break;
    }

    kdebug(">\n");
}
