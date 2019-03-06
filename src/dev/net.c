#include "net.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include "net/eth/eth.h"
#include "net/arp.h"
#include "net/inet.h"
#include "sys/atoi.h"
#include "sys/string.h"

#define NET_SAME_SUBNET(mask, a, b)     (((a) & net_inaddr_mask(mask)) == ((b) & net_inaddr_mask(mask)))

uint32_t net_inaddr_mask(int8_t n) {
    switch (n) {
    case 0:
        return 0;
    case 8:
        return 0xFF;
    case 16:
        return 0xFFFF;
    case 24:
        return 0xFFFFFF;
    case 32:
        return 0xFFFFFFFF;
    default:
        panic("Unsupported netmask\n");
    }
}

struct net_inaddr_spec {
    uint32_t flags;
    uint32_t addr;
};

struct net_inaddr_route_spec {
    uint32_t flags;
    uint32_t addr;
    netdev_t *dev;
    uint32_t dst;
    uint8_t dst_hwaddr[6];
};

struct net_priv {
    uint32_t flags;
    struct net_inaddr_spec inaddrs[4];
};

static int s_eth_last = 0;
static int s_dev_last = 0;

// TODO: a better way?
static netdev_t *s_devices[8];
static net_priv_t s_dev_privs[8];
static struct net_inaddr_route_spec s_routes[32];

static netdev_t *net_find_iface(const char *name) {
    for (int i = 0; i < sizeof(s_devices) / sizeof(s_devices[0]); ++i) {
        if (!s_devices[i]) {
            break;
        }
        if (!strcmp(name, s_devices[i]->name)) {
            return s_devices[i];
        }
    }
    return NULL;
}

// Allocate an address structure from pool
static struct net_priv *net_priv_alloc(void) {
    for (int i = 0; i < sizeof(s_dev_privs) / sizeof(s_dev_privs[0]); ++i) {
        if (s_dev_privs[i].flags & (1 << 31)) {
            continue;
        }

        s_dev_privs[i].flags |= (1 << 31);
        return &s_dev_privs[i];
    }

    return NULL;
}

static void net_priv_reset(net_priv_t *p) {
    memset(p->inaddrs, 0, 4 * sizeof(struct net_inaddr_spec));
}

void net_dump_ifaces(void) {
    kinfo("Network:\n");
    kinfo("Interfaces:\n");
    for (int i = 0; i < sizeof(s_devices) / sizeof(s_devices[0]); ++i) {
        if (!s_devices[i]) {
            break;
        }

        assert(s_devices[i]->priv);

        kinfo("%d: %s: hw " MAC_FMT "\n", i, s_devices[i]->name, MAC_BYTES(s_devices[i]->mac));

        for (int j = 0; j < 4; ++j) {
            if (s_devices[i]->priv->inaddrs[j].flags) {
                char inaddr_text[24];
                inet_ntoa(inaddr_text, s_devices[i]->priv->inaddrs[j].addr);
                uint32_t flags = s_devices[i]->priv->inaddrs[j].flags;

                kinfo("  inet %s/%u\n", inaddr_text, flags & 0xFF);
            }
        }
    }
    kinfo("Routes:\n");
    for (int i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); ++i) {
        if (!s_routes[i].flags) {
            break;
        }

        char inaddr_text[24];
        char inaddr_via_text[24];
        uint32_t flags = s_routes[i].flags;
        netdev_t *dev;
        assert(dev = s_routes[i].dev);

        inet_ntoa(inaddr_text, s_routes[i].addr);
        inet_ntoa(inaddr_via_text, s_routes[i].dst);

        kinfo("%d: %s/%u via %s dev %s\n", i, inaddr_text, flags & 0xFF, inaddr_via_text, dev->name);
    }
}

void net_init(void) {
    memset(s_routes, 0, sizeof(s_routes));
}

void net_post_config(void) {
    // TODO: make sure we don't perform this twice on the same iface/gateway
    for (int i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); ++i) {
        if (!s_routes[i].flags) {
            break;
        }

        assert(s_routes[i].dev);

        netdev_t *dev = s_routes[i].dev;
        assert(dev->priv);

        for (int j = 0; j < 4; ++j) {
            if (!dev->priv->inaddrs[j].flags) {
                break;
            }

            // Check if subnets match
            if (!NET_SAME_SUBNET(dev->priv->inaddrs[j].flags & 0xFF, dev->priv->inaddrs[j].addr, s_routes[i].dst)) {
                continue;
            }

            // Request hwaddr via arp
            arp_request_in_hwaddr(s_routes[i].dev, s_routes[i].dst, dev->priv->inaddrs[j].addr);
            break;
        }
    }
}

void net_register(netdev_t *dev) {
    // We don't support devices other than ethernet yet
    assert(dev->flags & NETDEV_FLG_ETH);

    memcpy(dev->name, "eth", 3);
    dev->name[3] = s_eth_last++ + '0'; // Lol
    dev->name[4] = 0;

    assert(dev->priv = net_priv_alloc());
    net_priv_reset(dev->priv);

    s_devices[s_dev_last++] = dev;

    kinfo("Registered network device %s\n", dev->name);
}

int net_send_from(netdev_t *dev, const void *buf, size_t len) {
    assert(dev && dev->tx);
    return dev->tx(dev, buf, len, 0);
}

void net_handle_packet(netdev_t *dev, const void *buf, size_t len) {
    struct eth_hdr *eth = (struct eth_hdr *) buf;

    kdebug("Incoming packet from %s\n", dev->name);
    eth_dump_packet(buf, len);

    // Route the packet to appropriate subsystem
    switch (inet_htons(eth->type)) {
    case ETH_TYPE_ARP:
        arp_handle_packet(dev, (struct arp_msg *) &eth[1]);
        break;
    default:
        kdebug("Unhandled packet\n");
        break;
    }
}

////

uint32_t net_inaddr_from(netdev_t *dev) {
    assert(dev);
    assert(dev->priv);
    assert(dev->priv->inaddrs[0].flags);
    // Just return the first address
    return dev->priv->inaddrs[0].addr;
}

////

void net_inaddr_add(netdev_t *dev, uint32_t addr, uint32_t flags) {
    assert(dev);
    assert(dev->priv);

    for (int i = 0; i < 4; ++i) {
        if (!dev->priv->inaddrs[i].addr && !dev->priv->inaddrs[i].flags) {
            dev->priv->inaddrs[i].addr = addr;
            dev->priv->inaddrs[i].flags = flags | (1 << 31);

            return;
        }
    }

    // No free address slots
    panic("Failed to add network interface address\n");
}

void net_route_add(uint32_t addr, uint32_t via, netdev_t *dev, uint32_t flags) {
    assert(dev);
    assert((flags & 0x7) == 0);

    for (int i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); ++i) {
        if (s_routes[i].flags) {
            continue;
        }

        s_routes[i].flags = flags | (1 << 31);
        s_routes[i].addr = addr;
        s_routes[i].dev = dev;
        s_routes[i].dst = via;

        return;
    }

    panic("Failed to add route\n");
}

void net_route_resolve(netdev_t *dev, uint32_t inaddr, const uint8_t *hwaddr) {
    assert(dev);
    assert(hwaddr);

    for (int i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); ++i) {
        if (!s_routes[i].flags) {
            break;
        }

        if (s_routes[i].dev == dev && s_routes[i].dst == inaddr) {
            memcpy(s_routes[i].dst_hwaddr, hwaddr, 6);

            char inaddr_via_text[24];
            inet_ntoa(inaddr_via_text, inaddr);
            kinfo("%s: resolved route via %s as hwaddr " MAC_FMT "\n", dev->name, inaddr_via_text, MAC_BYTES(hwaddr));
            return;
        }
    }

    panic("Failed to add hwaddr to resolved route dev %s via %08x\n", dev->name, inaddr);
}

int net_load_config(const char *path) {
    return -1;
//     // TODO: move this to userspace
//     /*
//      * The config file format is something like:
//      *
//      * [eth0]
//      * inet=192.168.1.2/24
//      * route 192.168.1.0/24 192.168.1.1
//      */
//
//     vfs_file_t *f = vfs_open(path, VFS_FLG_RD);
//     if (!f) {
//         return -1;
//     }
//
//     char linebuf[128];
//     char buf0[64];
//     const char *e, *p, *n;
//     netdev_t *curr_iface = NULL;
//     uint32_t inaddr = 0;
//     uint8_t mask = 0;
//     uint32_t inaddr1 = 0;
//
//     while (vfs_gets(f, linebuf, sizeof(linebuf)) > 0) {
//         kdebug("LINE \"%s\"\n", linebuf);
//         // Interface spec line
//         if (linebuf[0] == '[') {
//             assert((e = strchr(linebuf, ']')));
//             strncpy(buf0, linebuf + 1, e - linebuf - 1);
//             buf0[e - linebuf - 1] = 0;
//
//             curr_iface = net_find_iface(buf0);
//         } else if (!strncmp(linebuf, "inet=", 5)) {
//             if (curr_iface) {
//                 e = linebuf + 5;
//                 p = strchr(e, '/');
//
//                 // Parse mask
//                 if (p) {
//                     mask = atoi(p + 1);
//                 }
//                 // Parse inaddr
//                 if (p) {
//                     strncpy(buf0, e, p - e);
//                     buf0[p - e] = 0;
//                 } else {
//                     strcpy(buf0, e);
//                 }
//
//                 assert(inaddr = inet_aton(buf0));
//
//                 net_inaddr_add(curr_iface, inaddr, mask);
//             }
//         } else if (!strncmp(linebuf, "route ", 6)) {
//             if (curr_iface) {
//                 e = linebuf + 6;
//                 p = strchr(e, '/');
//                 n = strchr(e, ' ');
//
//                 assert(n);
//
//                 // Parse route real destination address
//                 if (p) {
//                     mask = atoi(p + 1);
//                     strncpy(buf0, e, p - e);
//                     buf0[p - e] = 0;
//                 } else {
//                     strncpy(buf0, e, n - e);
//                     buf0[n - e] = 0;
//                 }
//
//                 assert(inaddr = inet_aton(buf0));
//                 e = n + 1;
//
//                 // Parse via-destination address
//                 assert(inaddr1 = inet_aton(e));
//
//                 net_route_add(inaddr, inaddr1, curr_iface, mask);
//             }
//         }
//     }
//
//     vfs_close(f);
//
//     return 0;
}
