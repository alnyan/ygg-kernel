#include "arp.h"
#include "eth/eth.h"
#include "inet.h"
#include "sys/debug.h"
#include "sys/mem.h"

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
