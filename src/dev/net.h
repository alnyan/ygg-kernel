#pragma once
#include <stddef.h>
#include <stdint.h>

#define NETDEV_FLG_ETH      (1 << 0)

typedef struct netdev netdev_t;
typedef struct net_priv net_priv_t;

typedef int (*netdev_tx_func)(netdev_t *, const void *, size_t, uint32_t);

struct netdev {
    char name[12];
    uint32_t flags;
    uint8_t mac[6];
    // Contains things like addresses etc.
    net_priv_t *priv;

    netdev_tx_func tx;
};

void net_dump_ifaces(void);
void net_init(void);

void net_register(netdev_t *dev);
int net_load_config(const char *path);
void net_inaddr_add(netdev_t *dev, uint32_t addr, uint32_t flags);
void net_route_add(uint32_t addr, uint32_t via, netdev_t *dev, uint32_t flags);

int net_send_packet(const char *iface, const void *buf, size_t len);
int net_send_from(netdev_t *dev, const void *buf, size_t len);
void net_handle_packet(netdev_t *src, const void *buf, size_t len);
