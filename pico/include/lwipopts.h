#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// KISForth lwIP configuration for Raspberry Pi Pico W
// This file configures the lightweight IP stack for WiFi networking

// Platform configuration
#define NO_SYS                          1
#define LWIP_SOCKET                     0

// Memory configuration - optimized for Pico W's limited RAM
#define MEM_LIBC_MALLOC                 1
#define MEMP_NUM_TCP_PCB                4
#define MEMP_NUM_TCP_PCB_LISTEN         2
#define MEMP_NUM_TCP_SEG                8
#define MEMP_NUM_REASSDATA              1
#define MEMP_NUM_FRAG_PBUF              4
#define MEMP_NUM_ARP_QUEUE              2
#define MEMP_NUM_IGMP_GROUP             2
#define MEMP_NUM_SYS_TIMEOUT            8
#define MEMP_NUM_NETBUF                 0
#define MEMP_NUM_NETCONN                0
#define MEMP_NUM_TCPIP_MSG_API          0
#define MEMP_NUM_TCPIP_MSG_INPKT        0

#define TCP_MSS                         1460
#define TCP_WND                         (2 * TCP_MSS)
#define TCP_SND_BUF                     (2 * TCP_MSS)

// Buffer configuration
#define PBUF_POOL_SIZE                  8
#define PBUF_POOL_BUFSIZE               592

// ARP configuration
#define LWIP_ARP                        1
#define ARP_QUEUEING                    0

// IP configuration
#define IP_FORWARD                      0
#define IP_REASSEMBLY                   1
#define IP_FRAG                         1
#define IP_OPTIONS_ALLOWED              1

// ICMP configuration
#define LWIP_ICMP                       1

// DHCP configuration
#define LWIP_DHCP                       1

// UDP configuration
#define LWIP_UDP                        1

// TCP configuration
#define LWIP_TCP                        1
#define LWIP_LISTEN_BACKLOG             1

// Statistics (disable for production to save memory)
#define LWIP_STATS                      0

// Debug configuration (disable for production)
#define LWIP_DEBUG                      0

// Platform includes
#include <stdlib.h>

#endif /* _LWIPOPTS_H */