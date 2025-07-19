#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// KISForth lwIP Configuration
// Minimal configuration for WiFi networking on Pico W

// Platform specific locking
#define NO_SYS                     1
#define LWIP_SOCKET                0
#define LWIP_NETCONN               0

// Memory options - conservative for 32KB target
#define MEM_LIBC_MALLOC            0
#define MEMP_MEM_MALLOC            0
#define MEM_ALIGNMENT              4
#define MEM_SIZE                   4000
#define MEMP_NUM_PBUF              8
#define MEMP_NUM_UDP_PCB           4
#define MEMP_NUM_TCP_PCB           4
#define MEMP_NUM_TCP_PCB_LISTEN    2
#define MEMP_NUM_TCP_SEG           8
#define MEMP_NUM_SYS_TIMEOUT       8

// Protocol options - enable what KISForth needs
#define LWIP_ARP                   1
#define LWIP_ETHERNET              1
#define LWIP_ICMP                  1
#define LWIP_RAW                   0
#define LWIP_DHCP                  1
#define LWIP_AUTOIP                0
#define LWIP_SNMP                  0
#define LWIP_IGMP                  0
#define LWIP_DNS                   1

// TCP options
#define LWIP_TCP                   1
#define TCP_TTL                    255
#define TCP_WND                    2048
#define TCP_MAXRTX                 12
#define TCP_SYNMAXRTX              6
#define TCP_QUEUE_OOSEQ            0
#define TCP_MSS                    536
#define TCP_SND_BUF                2048
#define TCP_SND_QUEUELEN           8
#define LWIP_TCP_KEEPALIVE         1

// UDP options
#define LWIP_UDP                   1
#define UDP_TTL                    255

// Statistics and debugging - minimal for KISForth
#define LWIP_STATS                 0
#define LWIP_PROVIDE_ERRNO         1

// Checksum options - let hardware handle where possible
#define CHECKSUM_GEN_IP            0
#define CHECKSUM_GEN_UDP           0
#define CHECKSUM_GEN_TCP           0
#define CHECKSUM_CHECK_IP          0
#define CHECKSUM_CHECK_UDP         0
#define CHECKSUM_CHECK_TCP         0
#define CHECKSUM_CHECK_ICMP        0

// Debug options - disable for production
#define LWIP_DEBUG                 0

// Thread options
#define TCPIP_THREAD_STACKSIZE     1024
#define DEFAULT_UDP_RECVMBOX_SIZE  6
#define DEFAULT_TCP_RECVMBOX_SIZE  6
#define DEFAULT_ACCEPTMBOX_SIZE    6
#define DEFAULT_THREAD_STACKSIZE   1024

// Sequential layer options
#define LWIP_NETIF_HOSTNAME        1

#endif /* _LWIPOPTS_H */