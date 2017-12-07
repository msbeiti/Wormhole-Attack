/**
 * @file CRC.cc
 * @date 14.08.2012
 * @author Eugen Paul, Mohamad Sbeiti
 */

#include "CRC.h"

#include <string.h>
#include <stdio.h>

CRC::CRC() {
    chksum_crc32gentab();
}

CRC::~CRC() {

}

u_int32_t CRC::chksum_crc32(unsigned char *block, unsigned int length) {
    register unsigned long crc;
    unsigned long i;

    crc = 0xFFFFFFFF;
    for (i = 0; i < length; i++) {
        crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_tab[(crc ^ *block++) & 0xFF];
    }
    return (crc ^ 0xFFFFFFFF);
}

void CRC::chksum_crc32gentab() {
    unsigned long crc, poly;
    int i, j;

    poly = 0xEDB88320L;
    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ poly;
            } else {
                crc >>= 1;
            }
        }
        crc_tab[i] = crc;
    }
}

bool CRC::checkCRC(u_int8_t* data, int length) {
    char frameBuffer[1600];
    unsigned short radioHeaderLength;

    u_int32_t crc;
    u_int32_t crcRadio;

    //read radio Header Length
    radioHeaderLength = data[2] | (data[3] << 8);
    if (radioHeaderLength > (length - 4)) {
        return false;
    }
    //read radio CRC
    crcRadio = ((data[length - 1] << 24) & 0xFF000000) | ((data[length - 2] << 16) & 0x00FF0000) | ((data[length - 3] << 8) & 0x0000FF00)
            | ((data[length - 4]) & 0x000000FF);

    //read frameBuffer without CRC
    memcpy(frameBuffer, data + radioHeaderLength, length - radioHeaderLength - 4);

    crc = chksum_crc32((unsigned char*) frameBuffer, length - radioHeaderLength - 4);
	//printf("CRC32 computed: %08x\n", crc);
	//printf("CRC32 received: %08x\n", crcRadio);

    if (crc != crcRadio) {
        return false;
    }
    return true;
}
