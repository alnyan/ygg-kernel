#include "arp.h"
#include "eth/eth.h"
#include "inet.h"
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/assert.h"

static void arp_reply(netdev_t *dev, struct arp_msg *ask) {
    char buf[sizeof(struct eth_hdr) + sizeof(struct arp_msg)];

    struct eth_hdr *eth = (struct eth_hdr *) buf;
    memcpy(eth->src_mac, dev->mac, 6);
    memcpy(eth->dst_mac, ask->sha, 6);
    eth->type = inet_htons(ETH_TYPE_ARP);
    struct arp_msg *arp = (struct arp_msg *) &eth[1];
    arp->htype = ask->htype;
    arp->ptype = ask->ptype;
    arp->hlen = ask->hlen;
    arp->plen = ask->plen;
    arp->oper = inet_htons(ARP_RESPONSE);
    memcpy(arp->sha, dev->mac, 6);
    arp->spa = ask->tpa;
    memcpy(arp->tha, ask->sha, 6);
    arp->tpa = ask->spa;

    assert(net_send_from(dev, buf, sizeof(struct eth_hdr) + sizeof(struct arp_msg)) == 0);
}

int arp_handle_packet(netdev_t *dev, struct arp_msg *msg) {
    // For debugging purpose, respond with "me" to "who-has" with any addr
    switch (inet_htons(msg->oper)) {
    case ARP_WHO_HAS:
        // TODO: check if dev has the requested address bound
        arp_reply(dev, msg);
        break;
    case ARP_RESPONSE:
        // TODO: DEV_ARP_WAITING
        if (dev->flags & (1 << 30)) {
            // Try to lookup the route to resolve
            net_route_resolve(dev, msg->spa, msg->sha);
        }
        // Otherwise, we didn't ask anyone yet
        break;
    }

    return -1;
}

void arp_request_in_hwaddr(netdev_t *dev, uint32_t inaddr) {
    assert(dev);

    kinfo("req hwaddr of %08x dev %s\n", inaddr, dev->name);

    char buf[sizeof(struct eth_hdr) + sizeof(struct arp_msg)];
    struct eth_hdr *eth = (struct eth_hdr *) buf;
    memset(eth->dst_mac, 0xFF, 6);
    memcpy(eth->src_mac, dev->mac, 6);
    eth->type = inet_htons(ETH_TYPE_ARP);
    struct arp_msg *arp = (struct arp_msg *) &eth[1];
    arp->htype = inet_htons(0x01);
    arp->ptype = inet_htons(ETH_TYPE_INET);
    arp->hlen = 6;
    arp->plen = 4;
    arp->oper = inet_htons(ARP_WHO_HAS);
    memcpy(arp->sha, dev->mac, 6);

    // TODO: support multiple inaddrs per dev
    uint32_t spa = net_inaddr_from(dev);
    arp->spa = spa;

    memset(arp->tha, 0, 6);
    arp->tpa = inaddr;

    debug_dump(DEBUG_INFO, buf, sizeof(struct eth_hdr) + sizeof(struct arp_msg));

    // Mark device as "awaiting arp response"
    dev->flags |= (1 << 30);

    assert(net_send_from(dev, buf, sizeof(struct eth_hdr) + sizeof(struct arp_msg)) == 0);
}

void arp_dump_packet(struct arp_msg *msg) {
    char spa[32];
    char tpa[32];
    memset(spa, 0, 32);
    memset(tpa, 0, 32);

    switch (inet_htons(msg->ptype)) {
    case ETH_TYPE_INET:
        inet_ntoa(spa, msg->spa);
        inet_ntoa(tpa, msg->tpa);
        break;
    }

    kdebug("<arp "
           "htype=%u "
           "ptype=%04x "
           "hlen=%u "
           "plen=%u "
           "oper=%04x "
           "sha=" MAC_FMT " "
           "spa=%s "
           "tha=" MAC_FMT " "
           "tpa=%s>\n",
           inet_htons(msg->htype),
           inet_htons(msg->ptype),
           msg->hlen,
           msg->plen,
           inet_htons(msg->oper),
           MAC_BYTES(msg->sha),
           spa,
           MAC_BYTES(msg->tha),
           tpa);
}
