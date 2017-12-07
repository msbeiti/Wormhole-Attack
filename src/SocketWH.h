/**
 * @file SocketWH.h
 * @date 21.08.2012
 * @author Eugen Paul, Mohamad Sbeiti
 */

#ifndef SOCKETWH_H_
#define SOCKETWH_H_

#define ARPHRD_IEEE80211        801
#define ARPHRD_IEEE80211_PRISM  802
#define ARPHRD_IEEE80211_FULL   803

class SocketWH {
private:
    int socketToRead;
    int socketToWrite;
    int socketToWhNode;
    int isOk;

    int open_socket_to_whNode();
    int set_monitor(char *iface, int fd);
    int open_socket_to_read();
    int open_socket_to_write();

public:
    SocketWH();
    virtual ~SocketWH();

    int getSocketToRead(){
        return socketToRead;
    }
    int getSocketToWrite(){
        return socketToWrite;
    }
    int getSocketToWhNode(){
        return socketToWhNode;
    }
    int isAllSocketOK(){
        return isOk;
    }
};

#endif /* SOCKETWH_H_ */
