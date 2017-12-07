/**
 * @file Scheduler.h
 * @date 14.08.2012
 * @author Eugen Paul, Mohamad Sbeiti
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "CRC.h"
#include "wh.h"
#include "SocketWH.h"

class Scheduler {
private:
    SocketWH *sock;
    CRC *crc32;
    bool init;
    u_int8_t dataFromWlan[SO_RECVBUF_SIZE];
    int dataFromWlanLength;
    u_int8_t dataFromWlanOld[SO_RECVBUF_SIZE];
    int dataFromWlanLengthOld;
    u_int8_t dataFromWH[SO_RECVBUF_SIZE];
    int dataFromWHLength;

public:
    Scheduler();
    virtual ~Scheduler();
    void start();

private:
    int set_monitor(char *iface, int fd);
    int readDataFromSocket(int fd, u_int8_t* data);
    bool sendData(u_int8_t* data, int length, int fd);
    bool sendDataHEXArray(int fd);
    void sendDataOverUDP(u_int8_t *s, int length);

    void sendWlanAck(u_int8_t *mac);
    void sendCTS(u_int8_t *mac);

    bool isOk(u_int8_t* buf, int length);
};

#endif /* SCHEDULER_H_ */
