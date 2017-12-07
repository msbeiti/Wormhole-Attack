/**
 * @file scheduler.cc
 * @date 14.08.2012
 * @author Eugen Paul, Mohamad Sbeiti
 */

#include "Scheduler.h"

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

Scheduler::Scheduler() {
    init = false;
    // initialize all sockets
    sock = new SocketWH();
    if (sock->isAllSocketOK() == 0)
        return;

    // initialize CRC object to compute and check CRC
    crc32 = new CRC();

    init = true;
    memset(dataFromWlan, '+', SO_RECVBUF_SIZE);
    dataFromWlanLength = 0;
    memset(dataFromWlanOld, '-', SO_RECVBUF_SIZE);
    dataFromWlanLengthOld = SO_RECVBUF_SIZE;
    memset(dataFromWH, ':', SO_RECVBUF_SIZE);
    dataFromWHLength = 0;
}

Scheduler::~Scheduler() {
    if (sock)
        delete sock;
    if (crc32)
        delete crc32;
}

void Scheduler::start() {
    if (!init)
        return;

    int maxFD = 0;
    int numberOfRrequests = 0;
    fd_set rset;
    timeval waiting;

    // main scheduler loop
    while (true) {
        maxFD = 0;
        FD_ZERO(&rset);

        FD_SET(sock->getSocketToRead(), &rset);
        if (maxFD < sock->getSocketToRead()) {
            maxFD = sock->getSocketToRead();
        }

        FD_SET(sock->getSocketToWhNode(), &rset);
        if (maxFD < sock->getSocketToWhNode()) {
            maxFD = sock->getSocketToWhNode();
        }

        maxFD++;

        // wait for data/connect/timeouts...
        waiting.tv_sec = 20;
        waiting.tv_usec = 0;
        numberOfRrequests = select(maxFD, &rset, NULL, NULL, &waiting);

        if (numberOfRrequests <= 0) {
            // print ERROR
            continue;
        }

        // check socketToRead
        if (FD_ISSET(sock->getSocketToRead(), &rset)) {
            // read data from WLAN card
            dataFromWlanLength = readDataFromSocket(sock->getSocketToRead(), dataFromWlan);
            if (dataFromWlanLength > 1450) {
                printf("DROP DATA PACKET!\n");
            } else if (isOk(dataFromWlan, dataFromWlanLength)) {
                unsigned short radioHeaderLength;
                //read radio Header Length
                radioHeaderLength = dataFromWlan[2] | (dataFromWlan[3] << 8);
                if (true) {
//                if (memcmp(dataFromWlanOld + radioHeaderLength, dataFromWlan + radioHeaderLength, dataFromWlanLengthOld - radioHeaderLength)
//                        != 0) {
                    // Check the type of the received packet. Only selected types of packets are forwarded.
                    // 0x80 - beacon
                    if (dataFromWlan[radioHeaderLength] == 0x80) {
                        sendDataOverUDP(dataFromWlan, dataFromWlanLength - 4);
//                        memcpy(dataFromWlanOld, dataFromWlan, dataFromWlanLength);
//                        dataFromWlanLengthOld = dataFromWlanLength;
                    }
                    // 0xb0 - auth
                    if (dataFromWlan[radioHeaderLength] == 0xb0) {
                        // WLAN card adds the CRC to all authentication packets.
                        sendDataOverUDP(dataFromWlan, dataFromWlanLength - 4);
//                        memcpy(dataFromWlanOld, dataFromWlan, dataFromWlanLength);
//                        dataFromWlanLengthOld = dataFromWlanLength;
                    }
                    // 0x00, 0x10, 0x40, 0x50 - wpa reg
                    if (dataFromWlan[radioHeaderLength] == 0x00 || dataFromWlan[radioHeaderLength] == 0x10
                            || dataFromWlan[radioHeaderLength] == 0x40 || dataFromWlan[radioHeaderLength] == 0x50) {
                        // WLAN card adds the CRC to all WPA registration/authentication packets.
                        sendDataOverUDP(dataFromWlan, dataFromWlanLength - 4);
//                        memcpy(dataFromWlanOld, dataFromWlan, dataFromWlanLength);
//                        dataFromWlanLengthOld = dataFromWlanLength;
                    }
                    // 0x08, 0x88 - data, arp ...
                    if (dataFromWlan[radioHeaderLength] == 0x08 || dataFromWlan[radioHeaderLength] == 0x88) {
                        // WLAN card adds the CRC at Data packets if the packet is encrypted.
                        if ((dataFromWlan[radioHeaderLength + 1] & 0x40) == 0x40) {
                            sendDataOverUDP(dataFromWlan, dataFromWlanLength - 4);
                        } else {
                            sendDataOverUDP(dataFromWlan, dataFromWlanLength - 4);
                        }
//                        memcpy(dataFromWlanOld, dataFromWlan, dataFromWlanLength);
//                        dataFromWlanLengthOld = dataFromWlanLength;
                    }
                    // 0xd0 - mesh data
                    if (dataFromWlan[radioHeaderLength] == 0xd0) {
                        // WLAN card adds the CRC to all mesh packets.
                        sendDataOverUDP(dataFromWlan, dataFromWlanLength - 4);
//                        memcpy(dataFromWlanOld, dataFromWlan, dataFromWlanLength);
//                        dataFromWlanLengthOld = dataFromWlanLength;
                    }
//                    // 0x84, 0x94 - block ack
//                    if ((dataFromWlan[radioHeaderLength] == 0x84) || (dataFromWlan[radioHeaderLength] == 0x94)) {
//                        // WLAN card adds the CRC to all packets.
//                        sendDataOverUDP(dataFromWlan, dataFromWlanLength - 4);
////                        memcpy(dataFromWlanOld, dataFromWlan, dataFromWlanLength);
////                        dataFromWlanLengthOld = dataFromWlanLength;
//                    }
                } else {
                    printf("dub\n");
                }
            }
        }
        // check socketToWhNode
        if (FD_ISSET(sock->getSocketToWhNode(), &rset)) {
            // read data from WH node and sent it over WLAN card
            dataFromWHLength = readDataFromSocket(sock->getSocketToWhNode(), dataFromWH);
            if (dataFromWHLength > 0) {
                sendData(dataFromWH, dataFromWHLength, sock->getSocketToWrite());
//                std::cout << "Data read:\n";
//                for (int i = 0; i < data.len; i++) {
//                    std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) data.buf[i] << std::dec;
//                }
//                std::cout << "\n";
            }
        }
    }
}

bool Scheduler::isOk(u_int8_t* buf, int length) {
    if (length <= 0) {
        return false;
    }
    if (buf[0] == 0xFF) {
        return false;
    }
    if (crc32->checkCRC(buf, length) == false) {
        return false;
    }
    unsigned short radioHeaderLength;
    //read radio Header Length
    radioHeaderLength = buf[2] | (buf[3] << 8);

    u_int8_t mac1[] = MAC_ADDRESS_SOURCE;
    u_int8_t mac2[] = MAC_ADDRESS_DEST;
    u_int8_t macBC[] = MAC_ADDRESS_BC;
    if (radioHeaderLength + 4 + 6 + 6 > length) {
        return false;
    }
    // Check source MAC address
    if ((memcmp(mac1, buf + radioHeaderLength + 4 + 6, 6) != 0)) {
//        std::cout << "Data read:\n";
//        for (int i = 0; i < length; i++) {
//            std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) buf[i] << std::dec;
//        }
//        std::cout << "\n";
//        printf("err mac: ");
//        for (int i = 0; i < 6; i++) {
//            std::cout << std::hex << std::setw(2) << std::setfill('0')
//                    << (unsigned short) (unsigned char) buf[i + radioHeaderLength + 4 + 6] << std::dec;
//        }
//        std::cout << "\n";
        return false;
    }
//    printf("ok mac1: ");
//    for (int i = 0; i < 6; i++) {
//        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) buf[i + radioHeaderLength + 4 + 6]
//                << std::dec;
//    }
//    std::cout << "\n";

    // Check destination MAC address
    if ((memcmp(macBC, buf + radioHeaderLength + 4, 6) != 0) && (memcmp(mac2, buf + radioHeaderLength + 4, 6) != 0)) {
//    if ((buf[radioHeaderLength + 4] != 0xff && buf[radioHeaderLength + 5] != 0xff && buf[radioHeaderLength + 6] != 0xff
//            && buf[radioHeaderLength + 7] != 0xff) && (memcmp(mac2, buf + radioHeaderLength + 4, 6) != 0)) {
//        printf("err mac2: ");
//        for (int i = 0; i < 6; i++) {
//            std::cout << std::hex << std::setw(2) << std::setfill('0')
//                    << (unsigned short) (unsigned char) buf[i + radioHeaderLength + 4 + 6] << std::dec;
//        }
//        std::cout << "\n";
        return false;
    }

//    std::cout << "Data read OK:\n";
//    for (int i = 0; i < length; i++) {
//        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) buf[i] << std::dec;
//    }
//    std::cout << "\n";

    // Check strength of the Signal
    if (radioHeaderLength >= 18) {
        char SSI_Signal;
        SSI_Signal = buf[radioHeaderLength - 4];
        //    printf("0x%02d\n",SSI_Signal);
        if (SSI_Signal < -80) {
            return false;
        }
    }
    return true;
}

void Scheduler::sendWlanAck(u_int8_t *mac) {
    const char *src = "00001a002f480000a6f4017c000000001002a809a000bb000000d4000000442a60f4ca20" "984f229b";
    u_int8_t buffer[40];

    u_int8_t *dst = buffer;
    u_int8_t *end = buffer + sizeof(buffer);
    unsigned int u;

    while (dst < end && sscanf(src, "%2x", &u) == 1) {
        *dst++ = u;
        src += 2;
    }
    unsigned short radioHeaderLength;
    //read radio Header Length
    radioHeaderLength = buffer[2] | (buffer[3] << 8);

    memcpy(buffer + radioHeaderLength + 4, mac, 6);

    u_int32_t c = crc32->chksum_crc32(buffer, 34);

    memcpy(buffer + radioHeaderLength + 4 + 6, &c, 4);

    sendData(buffer, sizeof(buffer), sock->getSocketToWrite());

    std::cout << "send wlan ack:\n";
    for (int i = 0; i < 40; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) buffer[i] << std::dec;
    }
    std::cout << "\n";
}

void Scheduler::sendCTS(u_int8_t *mac) {
    const char *src = "00001a002f4800004983f1e30100000010028f09a000c3000000" "c4000000000b6b0201e4" "f044d1b4";
    u_int8_t buffer[40];

    u_int8_t *dst = buffer;
    u_int8_t *end = buffer + sizeof(buffer);
    unsigned int u;

    while (dst < end && sscanf(src, "%2x", &u) == 1) {
        *dst++ = u;
        src += 2;
    }

    unsigned short radioHeaderLength;
    //read radio Header Length
    radioHeaderLength = buffer[2] | (buffer[3] << 8);

    memcpy(buffer + radioHeaderLength + 4, mac, 6);

    u_int32_t c = crc32->chksum_crc32(buffer, 34);

    memcpy(buffer + radioHeaderLength + 4 + 6, &c, 4);

    sendData(buffer, sizeof(buffer), sock->getSocketToWrite());

    std::cout << "send CTS:\n";
    for (int i = 0; i < 40; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) buffer[i] << std::dec;
    }
    std::cout << "\n";
}

int Scheduler::readDataFromSocket(int fd, u_int8_t* data) {
    int dataLength = 0;
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // Receive message
    if ((dataLength = recvfrom(fd, data, SO_RECVBUF_SIZE, 0, (struct sockaddr *) &sender_addr, &addr_len)) < 0) {
        return 0;
    }
    return dataLength;
}

bool Scheduler::sendDataHEXArray(int fd) {
    const char *src =
            "00001a002f4800007f9a1b410200000010026c09a000ab00000080400000ffffffffffff0014a81487f20014a81487f2b03192c19c016000000064002104000849544d432d56504e010882848b0c129618240301010504000100000706444520010d170b0502004e8d5b2a010032043048606c851e02008f000f00ff0359006972662d333035000000000000000000020000259606004096001100dd06004096010104dd050040960305dd050040960b09dd050040961400dd180050f2020101800003a4000027a4000042435e0062322f00"
                    "2f71547a";
    u_int8_t buffer[214];

    u_int8_t *dst = buffer;
    u_int8_t *end = buffer + sizeof(buffer);
    unsigned int u;

    while (dst < end && sscanf(src, "%2x", &u) == 1) {
        *dst++ = u;
        src += 2;
    }

    std::cout << "Send HEX-Array:\n";
    for (int i = 0; i < 40; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) buffer[i] << std::dec;
    }
    std::cout << "\n";

    return sendData(buffer, sizeof(buffer), fd);
}

bool Scheduler::sendData(u_int8_t* data, int length, int fd) {
    char frameBuffer[1600];
    int bufLength;
    unsigned short radioHeaderLength;

    //read radio Header Length
    radioHeaderLength = data[2] | (data[3] << 8);
    if (radioHeaderLength > (length - 4)) {
        return false;
    }

    //read frameBuffer without CRC
    memcpy(frameBuffer, data + radioHeaderLength, length - radioHeaderLength - 4);

//    std::cout << "Want to send:\n";
//    for (int i = 0; i < length; i++) {
//        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) data[i] << std::dec;
//    }
//    std::cout << "\n";

    for (int j = 0; j < 1; j++) {
        bufLength = write(fd, data, length);
        if (bufLength <= 0) {
            printf("write Error :(\nError: (%d)%s\n", errno, strerror(errno));
            goto close_socket;
        }
    }

    return true;
    close_socket: return false;
}

void Scheduler::sendDataOverUDP(u_int8_t *s, int length) {

    std::cout << "Data send over WH:\n";
    for (int i = 0; i < length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned short) (unsigned char) s[i] << std::dec;
    }
    std::cout << "\n";

    struct sockaddr_in dest_sockaddr;
//    u_int8_t ttl;

    dest_sockaddr.sin_family = AF_INET;
    inet_aton(WH_ADDRESS, &dest_sockaddr.sin_addr);
    dest_sockaddr.sin_port = htons(WH_PORT);

//    // Set TTL
//    ttl = 64;
//    if (setsockopt(socketToWhNode, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
//        printf("setsockopt(IP_TTL) failed: %d - %s\n", errno, strerror(errno));
//        exit(1);
//    }
    // Send
    if (sendto(sock->getSocketToWhNode(), s, length, 0, (struct sockaddr *) &dest_sockaddr, sizeof(dest_sockaddr)) < 0) {
        printf("failed send %d - %s\n", errno, strerror(errno));
        return;
    }
}
