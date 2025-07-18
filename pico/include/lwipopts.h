#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Minimal lwIP configuration for Pico W
// Based on pico-examples/pico_w configurations

// Platform specific locking
#define NO_SYS                          1
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0

// Memory options
#define MEM_LIBC_MALLOC                 1
#define MEMP_NUM_TCP_PCB                8
#define MEMP_NUM_TCP_PCB_LISTEN         8
#define MEMP_NUM_UDP_PCB                4
#define MEMP_NUM_SYS_TIMEOUT            (LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2*LWIP_DHCP) + LWIP_AUTOIP + LWIP_IGMP + LWIP_DNS + (PPP_SUPPORT*6*MEMP_NUM_PPP_PCB) + (LWIP_IPV6 ? (1 + LWIP_IPV6_REASS + LWIP_IPV6_MLD) : 0))

// ARP options
#define LWIP_ARP                        1
#define ARP_TABLE_SIZE                  10
#define ARP_QUEUEING                    0

// IP options
#define LWIP_IPV4                       1
#define LWIP_IPV6                       0
#define IP_REASSEMBLY                   0
#define IP_FRAG                         0

// ICMP options
#define LWIP_ICMP                       1

// DHCP options
#define LWIP_DHCP                       1
#define DHCP_DOES_ARP_CHECK             0

// UDP options
#define LWIP_UDP                        1
#define LWIP_UDPLITE                    0

// TCP options
#define LWIP_TCP                        1
#define TCP_TTL                         255
#define TCP_WND                         (4 * TCP_MSS)
#define TCP_MAXRTX                      12
#define TCP_SYNMAXRTX                   6

// Pbuf options
#define PBUF_LINK_HLEN                  14
#define PBUF_POOL_BUFSIZE               592

// Network interface options
#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        1

// LWIP debug options (disable for minimal size)
#define LWIP_DEBUG                      0

// Disable stats for minimal footprint
#define LWIP_STATS                      0

// Threading options
#define TCPIP_THREAD_STACKSIZE          0

// Sequential layer options
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0

// Don't need these for basic WiFi
#define LWIP_DNS                        1
#define DNS_TABLE_SIZE                  4
#define DNS_MAX_NAME_LENGTH             256

#endif /* _LWIPOPTS_H */