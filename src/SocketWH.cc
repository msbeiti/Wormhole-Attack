/**
 * @file Socket.cc
 * @date 21.08.2012
 * @author Eugen Paul, Mohamad Sbeiti
 */

#include "SocketWH.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "wh.h"

SocketWH::SocketWH() {
    socketToRead = 0;
    socketToWrite = 0;
    socketToWhNode = 0;
    isOk = 0;

    // open sockets
    if (open_socket_to_read() < 0 || open_socket_to_write() < 0 || open_socket_to_whNode() < 0)
        return;
    isOk = 1;
}

SocketWH::~SocketWH() {
    if (socketToRead)
        close(socketToRead);
    if (socketToWrite)
        close(socketToWrite);
    if (socketToWhNode)
        close(socketToWhNode);
}

int SocketWH::open_socket_to_whNode() {
    struct sockaddr_in wh_addr;
    int on = 1;
    int fd = -1;

    // Create an UDP socket
    if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("socket() failed\nError: (%d)%s", errno, strerror(errno));
        return -1;
    }

    // Set socket as a reuseaddr one
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\nError: (%d)%s\n", errno, strerror(errno));
        return -1;
    }

    // Bind to DYMO port number
    memset(&wh_addr, 0, sizeof(struct sockaddr_in));
    wh_addr.sin_family = AF_INET;
    wh_addr.sin_port = htons(WH_PORT);
    wh_addr.sin_addr.s_addr = htonl(INADDR_ANY );
    if (bind(fd, (struct sockaddr *) &wh_addr, sizeof(struct sockaddr)) < 0) {
        printf("bind() failed\nError: (%d)%s\n", errno, strerror(errno));
        return -1;
    }

    socketToWhNode = fd;
    return fd;
}

int SocketWH::set_monitor(char *iface, int fd) {
    struct iwreq wrq;

    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, iface, IFNAMSIZ);
    wrq.u.mode = IW_MODE_MONITOR;

    if (ioctl(fd, SIOCSIWMODE, &wrq) < 0) {
        perror("ioctl(SIOCSIWMODE) failed");
        return (1);
    }

    return (0);
}

int SocketWH::open_socket_to_read() {
    // Create a raw socket
    int on = 1;
    char ifname[16];
    int socketFD;

    if ((socketFD = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        printf("socket() failed on interface.\nError: (%d)%s", errno, strerror(errno));
        goto exit;
    }

    // Enable the datagram socket as a broadcast one
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed on interface\nError: (%d)%s\n", errno, strerror(errno));
        goto close_socket;
    }

    // Make the socket only process packets received from an interface
    strncpy(ifname, WLAN_DEVICE, sizeof(ifname));

    if (setsockopt(socketFD, SOL_SOCKET, SO_BINDTODEVICE, &ifname, sizeof(ifname)) < 0) {
        printf("setsockopt(BINDTODEVICE) failed for %s\nError: (%d)%s\n", ifname, errno, strerror(errno));
        goto close_socket;
    }

    socketToRead = socketFD;
    return socketFD;
    close_socket: close(socketFD);
    exit: return -1;
}

int SocketWH::open_socket_to_write() {
    int socketFD;
    struct sockaddr_ll sll;
    struct ifreq ifr;
    struct iwreq wrq;
    struct packet_mreq mr;

    if ((socketFD = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        printf("socket() failed on interface.\nError: (%d)%s", errno, strerror(errno));
        goto exit;
    }

    /* find the interface index */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, WLAN_DEVICE, sizeof(ifr.ifr_name) - 1);
    if (ioctl(socketFD, SIOCGIFINDEX, &ifr) < 0) {
        printf("Interface %s: \n", WLAN_DEVICE);
        perror("ioctl(SIOCGIFINDEX) failed");
        goto close_socket;
    }

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
//    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_protocol = htons(25);

    /* lookup the hardware type */
    if (ioctl(socketFD, SIOCGIFHWADDR, &ifr) < 0) {
        printf("Interface %s: \n", WLAN_DEVICE);
        perror("ioctl(SIOCGIFHWADDR) failed");
        goto close_socket;
    }

    /* lookup iw mode */
    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, WLAN_DEVICE, IFNAMSIZ);
    if (ioctl(socketFD, SIOCGIWMODE, &wrq) < 0) {
        /* most probably not supported (ie for rtap ipw interface) *
         * so just assume its correctly set...                     */
        wrq.u.mode = IW_MODE_MONITOR;
    }

    if ((ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211 && ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_PRISM
            && ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_FULL) || (wrq.u.mode != IW_MODE_MONITOR)) {
        char ifname[16] = WLAN_DEVICE;
        if (set_monitor(ifname, socketFD)) {
            ifr.ifr_flags &= ~(IFF_UP | IFF_BROADCAST | IFF_RUNNING);

            if (ioctl(socketFD, SIOCSIFFLAGS, &ifr) < 0) {
                perror("ioctl(SIOCSIFFLAGS) failed");
                goto close_socket;
            }

            if (set_monitor(ifname, socketFD)) {
                printf("Error setting monitor mode on %s\n", WLAN_DEVICE);
                goto close_socket;
            }
        }
    }

    /* Is interface st to up, broadcast & running ? */
    if ((ifr.ifr_flags | IFF_UP | IFF_BROADCAST | IFF_RUNNING) != ifr.ifr_flags) {
        /* Bring interface up*/
        ifr.ifr_flags |= IFF_UP | IFF_BROADCAST | IFF_RUNNING;

        if (ioctl(socketFD, SIOCSIFFLAGS, &ifr) < 0) {
            perror("ioctl(SIOCSIFFLAGS) failed");
            goto close_socket;
        }
    }

    /* bind the raw socket to the interface */
    if (bind(socketFD, (struct sockaddr *) &sll, sizeof(sll)) < 0) {
        printf("Interface %s: \n", WLAN_DEVICE);
        perror("bind(ETH_P_ALL) failed");
        goto close_socket;
    }

    /* lookup the hardware type */

    if (ioctl(socketFD, SIOCGIFHWADDR, &ifr) < 0) {
        printf("Interface %s: \n", WLAN_DEVICE);
        perror("ioctl(SIOCGIFHWADDR) failed");
        goto close_socket;
    }

    if (ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211 && ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_PRISM
            && ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_FULL) {
        if (ifr.ifr_hwaddr.sa_family == 1)
            fprintf(stderr, "\nARP linktype is set to 1 (Ethernet) ");
        else
            fprintf(stderr, "\nUnsupported hardware link type %4d ", ifr.ifr_hwaddr.sa_family);

        fprintf(stderr, "- expected ARPHRD_IEEE80211,\nARPHRD_IEEE80211_"
                "FULL or ARPHRD_IEEE80211_PRISM instead.  Make\n"
                "sure RFMON is enabled: run 'airmon-ng start %s"
                " <#>'\nSysfs injection support was not found "
                "either.\n\n", WLAN_DEVICE);
        goto close_socket;
    }

    /* enable promiscuous mode */
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = sll.sll_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    if (setsockopt(socketFD, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0) {
        perror("setsockopt(PACKET_MR_PROMISC) failed");
        goto close_socket;
    }

    socketToWrite = socketFD;
    return socketFD;
    close_socket: close(socketFD);
    exit: return -1;
}
