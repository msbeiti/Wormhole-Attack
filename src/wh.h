/**
 * @file wh.h
 * @date 08.08.2012
 * @author Eugen Paul, Mohamad Sbeiti
 */

#ifndef WH_H_
#define WH_H_

#include <stdlib.h>

#define WLAN_DEVICE "wlan0"
#define SO_RECVBUF_SIZE (10 * 1024)
#define WH_PORT 6666

#define MAC_ADDRESS_BC { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }

//#define WH_NODE_1
#ifdef WH_NODE_1
#define MAC_ADDRESS_SOURCE { 0x00, 0x0b, 0x6b, 0x02, 0x01, 0xa4 }
#define MAC_ADDRESS_DEST { 0x00, 0x0b, 0x6b, 0x02, 0x01, 0xe4 }
#else
// Start of the tunnel Neighbour of wormhole node WH_NODE_1
#define MAC_ADDRESS_SOURCE { 0x00, 0x0b, 0x6b, 0x02, 0x01, 0xa4 }
// End of the tunnel Neighbour of wormhole node counterpart WH_NODE_2
#define MAC_ADDRESS_DEST { 0x00, 0x0b, 0x6b, 0x02, 0x02, 0x86 }
#endif

#define WH_ADDRESS "172.17.1.78"

typedef struct {
    u_int8_t *buf;
    int32_t len;
} lv_block;

#endif /* WH_H_ */
